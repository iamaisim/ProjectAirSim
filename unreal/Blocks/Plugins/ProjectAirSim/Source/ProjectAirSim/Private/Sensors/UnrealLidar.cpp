// Copyright (C) Microsoft Corporation.  All rights reserved.
// Unreal Lidar Implementation

#include "UnrealLidar.h"

#include <cmath>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "ProjectAirSim.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "UnrealLogger.h"

namespace projectairsim = microsoft::projectairsim;

static constexpr double M_2PI = 2.0 * M_PI;

UUnrealLidar::UUnrealLidar(const FObjectInitializer& ObjectInitializer)
    : UUnrealSensor(ObjectInitializer), PScanPattern(nullptr) {
  bAutoActivate = true;

  // Tick in PostUpdateWork to update at the same time as UnrealCamera, instead
  // of waiting for the PrePhysics tick on the next loop
  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UUnrealLidar::Initialize(const projectairsim::Lidar& SimLidar) {
  Lidar = SimLidar;
  OwnerActor = GetOwner();

  SetupLidarFromSettings(SimLidar.GetLidarSettings());
  LastSimTime = projectairsim::SimClock::Get()->NowSimNanos();

  RegisterComponent();
}

void UUnrealLidar::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  const TimeNano CurSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  const TimeSec SimTimeDeltaSec =
      projectairsim::SimClock::Get()->NanosToSec(CurSimTime - LastSimTime);
  const TimeNano SimTimeDeltaLastReport = CurSimTime - LastSimTimeReport;

  // Accumulate new sensor returns
  Simulate(SimTimeDeltaSec);

  // Move sensor data to pending report buffers
  if (!Settings.report_no_return_points) {
    // Filter through the arrays and remove all no-return points
    for (int32 i = 0; i < ReturnCloud.size(); i++) {
      if (ReturnCloud[i]) {
        if (Settings.report_point_cloud) {
          PointCloudPending.push_back(PointCloud[i * 3]);
          PointCloudPending.push_back(PointCloud[i * 3 + 1]);
          PointCloudPending.push_back(PointCloud[i * 3 + 2]);
        }
        if (Settings.report_azimuth_elevation_range) {
          AzimuthElevationRangeCloudPending.push_back(
              AzimuthElevationRangeCloud[i * 3]);
          AzimuthElevationRangeCloudPending.push_back(
              AzimuthElevationRangeCloud[i * 3 + 1]);
          AzimuthElevationRangeCloudPending.push_back(
              AzimuthElevationRangeCloud[i * 3 + 2]);
        }
        SegmentationCloudPending.push_back(SegmentationCloud[i]);
        IntensityCloudPending.push_back(IntensityCloud[i]);
        LaserIndexCloudPending.push_back(LaserIndexCloud[i]);
      }
    }
  } else {
    if (PointCloudPending.empty())
      PointCloudPending = std::move(PointCloud);
    else {
      PointCloudPending.reserve(PointCloudPending.size() + PointCloud.size());
      PointCloudPending.insert(PointCloudPending.end(),
                               std::make_move_iterator(PointCloud.begin()),
                               std::make_move_iterator(PointCloud.end()));
    }

    if (AzimuthElevationRangeCloudPending.empty())
      AzimuthElevationRangeCloudPending = std::move(AzimuthElevationRangeCloud);
    else {
      AzimuthElevationRangeCloudPending.reserve(
          AzimuthElevationRangeCloudPending.size() +
          AzimuthElevationRangeCloud.size());
      AzimuthElevationRangeCloudPending.insert(
          AzimuthElevationRangeCloudPending.end(),
          std::make_move_iterator(AzimuthElevationRangeCloud.begin()),
          std::make_move_iterator(AzimuthElevationRangeCloud.end()));
    }

    if (SegmentationCloudPending.empty())
      SegmentationCloudPending = std::move(SegmentationCloud);
    else {
      SegmentationCloudPending.reserve(SegmentationCloudPending.size() +
                                       SegmentationCloud.size());
      SegmentationCloudPending.insert(
          SegmentationCloudPending.end(),
          std::make_move_iterator(SegmentationCloud.begin()),
          std::make_move_iterator(SegmentationCloud.end()));
    }

    if (IntensityCloudPending.empty())
      IntensityCloudPending = std::move(IntensityCloud);
    else {
      IntensityCloudPending.reserve(IntensityCloudPending.size() +
                                    IntensityCloud.size());
      IntensityCloudPending.insert(
          IntensityCloudPending.end(),
          std::make_move_iterator(IntensityCloud.begin()),
          std::make_move_iterator(IntensityCloud.end()));
    }

    if (LaserIndexCloudPending.empty())
      LaserIndexCloudPending = std::move(LaserIndexCloud);
    else {
      LaserIndexCloudPending.reserve(LaserIndexCloudPending.size() +
                                     LaserIndexCloud.size());
      LaserIndexCloudPending.insert(
          LaserIndexCloudPending.end(),
          std::make_move_iterator(IntensityCloud.begin()),
          std::make_move_iterator(IntensityCloud.end()));
    }
  }
  AzimuthElevationRangeCloud.clear();
  SegmentationCloud.clear();
  IntensityCloud.clear();
  LaserIndexCloud.clear();
  PointCloud.clear();

  // Generate report message according to reporting frequency setting
  if ((LastSimTimeReport == 0) ||
      (SimTimeDeltaLastReport >= SimTimeDeltaReportTarget)) {
    const auto LidarTransformStamped = UnrealTransform::GetPoseNed(this);
    projectairsim::Pose LidarPose(LidarTransformStamped.translation_,
                                  LidarTransformStamped.rotation_);

    projectairsim::LidarMessage LidarMsg(
        CurSimTime, PointCloudPending, AzimuthElevationRangeCloudPending,
        SegmentationCloudPending, IntensityCloudPending, LaserIndexCloudPending,
        LidarPose);
    Lidar.PublishLidarMsg(LidarMsg);

    // Clear reported sensor returns
    PointCloudPending.clear();
    AzimuthElevationRangeCloudPending.clear();
    SegmentationCloudPending.clear();
    IntensityCloudPending.clear();
    LaserIndexCloudPending.clear();

    // Save report time, adjusting for the difference between actual and target
    // interval to achieve the target reporting interval on average
    {
      auto DSimTime = (SimTimeDeltaLastReport - SimTimeDeltaReportTarget);

      if (DSimTime > 0) {
        if (DSimTime > (2 * SimTimeDeltaReportTarget))
          DSimTime =
              2 * SimTimeDeltaReportTarget;  // Limit the time adjustment so it
                                             // doesn't grow out of control

        LastSimTimeReport = CurSimTime - DSimTime;
      }
    }
  }

  LastSimTime = CurSimTime;
}

void UUnrealLidar::SetupLidarFromSettings(
    const projectairsim::LidarSettings& LidarSettings) {
  Settings = LidarSettings;

  switch (Settings.lidar_kind) {
    default:
      UnrealLogger::Log(
          projectairsim::LogLevel::kFatal,
          TEXT("[UnrealLidar] Lidar '%S': SetupLidarFromSettings(): Unhandled "
               "lidar_kind %d"),
          Lidar.GetId().c_str(), Settings.lidar_kind);
      PScanPattern = std::make_unique<CylindricalScanPattern>();
      break;

    case projectairsim::LidarKind::kGenericCylindrical:
      PScanPattern = std::make_unique<CylindricalScanPattern>();
      break;

    case projectairsim::LidarKind::kGenericRosette:
      PScanPattern = std::make_unique<RosetteScanPattern>();
      break;
  }

  InitializePose(Settings.origin);

  // normalize
  Settings.horizontal_fov_start_deg =
      std::fmod(360.0f + Settings.horizontal_fov_start_deg, 360.0f);
  Settings.horizontal_fov_end_deg =
      std::fmod(360.0f + Settings.horizontal_fov_end_deg, 360.0f);

  // Calculate reporting interval from frequency
  SimTimeDeltaReportTarget = (Settings.report_frequency < 0.01f)
                                 ? 0
                                 : projectairsim::SimClock::Get()->SecToNanos(
                                       1.0f / Settings.report_frequency);

  // Save scan axis orientation
  {
    auto quaternion = Settings.scan_orientation.normalized();

    bScanOrientationIsDefault = (quaternion.isApprox(
        microsoft::projectairsim::Quaternion::Identity(), 0.0001));
    if (!bScanOrientationIsDefault)
      ScanOrientationRot = UnrealHelpers::ToFRotator(quaternion);
  }

  PScanPattern->Setup(Settings);
}

void UUnrealLidar::BeginPlay() { Super::BeginPlay(); }

void UUnrealLidar::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
  Lidar.EndUpdate();
}

void UUnrealLidar::InitializePose(const projectairsim::Transform& PoseNed) {
  SetRelativeTransform(UnrealTransform::FromGlobalNed(PoseNed));

  // Check that the initial pose was set correctly
  projectairsim::Transform InitializedPose = UnrealTransform::GetPoseNed(this);
  projectairsim::Vector3 InitializedRPY =
      projectairsim::TransformUtils::ToDegrees(
          projectairsim::TransformUtils::ToRPY(InitializedPose.rotation_));
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[UnrealLidar] Lidar '%S': InitializePose(). "
           "RelativeLocation (%f,%f,%f) RelativeRotationRPY (%f,%f,%f)"),
      Lidar.GetId().c_str(), InitializedPose.translation_.x(),
      InitializedPose.translation_.y(), InitializedPose.translation_.z(),
      InitializedRPY.x(), InitializedRPY.y(), InitializedRPY.z());
}

// Simulate shooting a laser via Unreal ray-tracing.
FHitResult UUnrealLidar::ShootSingleLaser(const LaserDirection& LaserDir,
                                          const FVector& LidarBodyLoc,
                                          const FRotator& LidarBodyRot) {
  const FRotator LaserRotInSensorFrame(LaserDir.VerticalAngleDeg,   /*pitch*/
                                       LaserDir.HorizontalAngleDeg, /*yaw*/
                                       0 /*roll*/);

  const FRotator LaserRotInWorldFrame =
      UKismetMathLibrary::ComposeRotators(LaserRotInSensorFrame, LidarBodyRot);

  const FVector RayDirectionVector =
      UKismetMathLibrary::GetForwardVector(LaserRotInWorldFrame);

  const FVector LaserLoc =
      LidarBodyLoc + UKismetMathLibrary::GetUpVector(LaserRotInWorldFrame) *
                         LaserDir.OffsetDistanceVertical;

  // Calculate "EndTrace": point corresponding to end of ray trace.
  const FVector EndTrace =
      LaserLoc + (projectairsim::TransformUtils::ToCentimeters(Settings.range) *
                  RayDirectionVector);

  // Shoot ray via LineTraceSingleByChannel, result is saved in HitInfo
  FCollisionQueryParams TraceParams;
  if (Settings.disable_self_hits) {
    TraceParams.AddIgnoredActor(OwnerActor);  // don't hit yourself
  }
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

  FHitResult HitInfo(ForceInit);

  UnrealWorld->LineTraceSingleByChannel(
      HitInfo, LaserLoc, EndTrace, ECC_Visibility, TraceParams,
      FCollisionResponseParams::DefaultResponseParam);

  return HitInfo;
}

// Material-independent calculation of intensity based on
// atmospheric attenuation (distance) and surface impact normal.
float CalculateIntensity(const FHitResult& HitInfo) {
  auto AttenuationCoeff = 0.0009f;  // 0.00004f;

  auto LaserDirection = HitInfo.TraceStart - HitInfo.ImpactPoint;
  LaserDirection.Normalize();
  auto AngleSimilarity =
      FVector::DotProduct(LaserDirection, HitInfo.ImpactNormal);
  AngleSimilarity = AngleSimilarity < 0 ? 0.0 : AngleSimilarity;

  // Distance values are in cm.
  // TODO: the following are arbitrary and configured to look
  // good on a livox Graz scene.
  auto DistOffset = 5200;  // anything closer will be interpreted as if it was
                           // MinDistance away.
  auto MinDistance = 40.0;
  auto Distance = (HitInfo.Distance < (DistOffset + MinDistance))
                      ? MinDistance
                      : (HitInfo.Distance - DistOffset);

  auto OriginalIntensity = 1.0;
  auto intensity = (0.4 * AngleSimilarity) +
                   (OriginalIntensity * exp(-AttenuationCoeff * Distance));

  return intensity;
}

void UUnrealLidar::Simulate(const float SimTimeDeltaSec) {
  TArray<LaserDirection> LasersToShoot;

  PointCloud.clear();
  SegmentationCloud.clear();
  IntensityCloud.clear();
  LaserIndexCloud.clear();
  ReturnCloud.clear();

  // 1. Decide lasers to shoot for this tick

  PScanPattern->BeginScan(SimTimeDeltaSec);
  if (!PScanPattern->GetLasersToShoot(&LasersToShoot)) return;

  // 2. Shoot the lasers

  const FVector LidarBodyLoc = GetComponentLocation();
  const FRotator LidarBodyRot = GetComponentRotation();
  const FRotator LidarScanRot = bScanOrientationIsDefault
                                    ? LidarBodyRot
                                    : UKismetMathLibrary::ComposeRotators(
                                          ScanOrientationRot, LidarBodyRot);

  const float NoReturnX = Settings.no_return_point_value.x();
  const float NoReturnY = Settings.no_return_point_value.y();
  const float NoReturnZ = Settings.no_return_point_value.z();
  const bool bReportNoReturnPoints = Settings.report_no_return_points;
  const bool bReportPointCloud = Settings.report_point_cloud;
  const bool bReportAzimuthElevationRange =
      Settings.report_azimuth_elevation_range;

  FCriticalSection Mutex;
  TArray<FHitResult> GroundTruthHits;

  if (bReportPointCloud) {
    for (int32 i = 0; i < LasersToShoot.Num(); i++) {
      PointCloud.push_back(NoReturnX);
      PointCloud.push_back(NoReturnY);
      PointCloud.push_back(NoReturnZ);
    }
  }
  if (bReportAzimuthElevationRange) {
    AzimuthElevationRangeCloud.insert(AzimuthElevationRangeCloud.begin(),
                                      LasersToShoot.Num() * 3, 0);
  }
  SegmentationCloud.insert(SegmentationCloud.begin(), LasersToShoot.Num(), 0);
  IntensityCloud.insert(IntensityCloud.begin(), LasersToShoot.Num(), 0);
  LaserIndexCloud.insert(LaserIndexCloud.begin(), LasersToShoot.Num(), 0);
  ReturnCloud.insert(ReturnCloud.begin(), LasersToShoot.Num(), false);

  ParallelFor(LasersToShoot.Num(), [&](int32 Idx) {
    const FHitResult HitInfo =
        ShootSingleLaser(LasersToShoot[Idx], LidarBodyLoc, LidarScanRot);

    if (HitInfo.bBlockingHit == false) {
      // No laser return
      if (bReportNoReturnPoints) {
        // Add a special point value indicating no laser return
        Mutex.Lock();

        if (bReportPointCloud) {
          PointCloud[Idx * 3] = NoReturnX;
          PointCloud[Idx * 3 + 1] = NoReturnY;
          PointCloud[Idx * 3 + 2] = NoReturnZ;
        }

        if (bReportAzimuthElevationRange) {
          double Azimuth = LasersToShoot[Idx].HorizontalAngleDeg;
          double Elevation = LasersToShoot[Idx].VerticalAngleDeg;
          AzimuthElevationRangeCloud[Idx * 3] = Azimuth;
          AzimuthElevationRangeCloud[Idx * 3 + 1] = Elevation;
          AzimuthElevationRangeCloud[Idx * 3 + 2] = -1;
        }

        SegmentationCloud[Idx] = NoSegmentationID;
        IntensityCloud[Idx] = 0.0f;
        LaserIndexCloud[Idx] = LasersToShoot[Idx].Channel;
        ReturnCloud[Idx] = false;

        Mutex.Unlock();
      }
    } else {
      // Get CustomDepthStencilValue directly from the hit component
      const UPrimitiveComponent* HitComp = HitInfo.GetComponent();
      const int SegmentationID =
          HitComp ? HitComp->CustomDepthStencilValue : NoSegmentationID;

      // Calculate relative location in world frame from sensor to hit point
      FVector Point = HitInfo.ImpactPoint - LidarBodyLoc;
      // Transform to lidar frame by doing inverse rotation of lidar's
      // rotation from world frame
      Point = LidarBodyRot.UnrotateVector(Point);

      const projectairsim::Vector3 PointNed =
          UnrealTransform::UnrealToNedLinear(Point);

      Mutex.Lock();

      if (bReportPointCloud) {
        PointCloud[Idx * 3] = PointNed.x();
        PointCloud[Idx * 3 + 1] = PointNed.y();
        PointCloud[Idx * 3 + 2] = PointNed.z();
      }

      if (bReportAzimuthElevationRange) {
        double AzimuthRadians = projectairsim::TransformUtils::ToRadians(
            LasersToShoot[Idx].HorizontalAngleDeg);
        double ElevationRadians = projectairsim::TransformUtils::ToRadians(
            LasersToShoot[Idx].VerticalAngleDeg);
        double Range = Point.Size();

        AzimuthElevationRangeCloud[Idx * 3] = AzimuthRadians;
        AzimuthElevationRangeCloud[Idx * 3 + 1] = ElevationRadians;
        AzimuthElevationRangeCloud[Idx * 3 + 2] = Range;
      }

      SegmentationCloud[Idx] = SegmentationID;
      IntensityCloud[Idx] = CalculateIntensity(HitInfo);
      LaserIndexCloud[Idx] = LasersToShoot[Idx].Channel;
      ReturnCloud[Idx] = true;

      if (Settings.draw_debug_points) {
        GroundTruthHits.Add(HitInfo);
      }

      Mutex.Unlock();
    }
  });

  // 3. Optional - draw debug hit points on Unreal scene

  if (Settings.draw_debug_points && UnrealWorld) {
    for (const auto& HitInfo : GroundTruthHits) {
      DrawDebugPoint(UnrealWorld, HitInfo.ImpactPoint,
                     10,                   // size
                     FColor(255, 0, 255),  // RGB
                     false,                // persistent (never goes away)
                     0.1                   // time that point persists on object
      );
    }
  }

  // 4. Do end of scan
  PScanPattern->EndScan();
}

// Assumes that angles are in 0-360 deg range
bool UUnrealLidar::IsAngleInRange(float Angle, float StartAngle,
                                  float EndAngle) {
  if (StartAngle < EndAngle) {
    return (StartAngle <= Angle && Angle <= EndAngle);
  } else
    return (StartAngle <= Angle || Angle <= EndAngle);
}

void UUnrealLidar::ScanPatternBase::Setup(
    const microsoft::projectairsim::LidarSettings& LidarSettings) {
  auto quaternion = LidarSettings.scan_orientation.normalized();

  LidarSettings_ = LidarSettings;
}

void UUnrealLidar::CylindricalScanPattern::EndScan(void) {
  ScanPatternBase::EndScan();

  // Update sensor's current horizontal angle to the end of this tick's sweep
  CurrentHorizontalAngleDeg_ =
      std::fmod(CurrentHorizontalAngleDeg_ + AngleDistanceOfTickDeg_, 360.0f);
}

bool UUnrealLidar::CylindricalScanPattern::GetLasersToShoot(
    TArray<LaserDirection>* prg_laser_direction_ret) {
  const uint32 num_channel = LidarSettings_.number_of_channels;
  const uint32 num_points_per_laser =
      FMath::RoundHalfFromZero(LidarSettings_.points_per_second * DSecSimTime_ /
                               static_cast<float>(num_channel));

  if (num_points_per_laser <= 0) {
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("[UnrealLidar] No points requested this frame, "
                           "try increasing the number of points per second."));
    return (false);
  }

  check(num_channel == LaserAnglesDeg_.size());

  AngleDistanceOfTickDeg_ =
      LidarSettings_.horizontal_rotation_frequency * 360.0f * DSecSimTime_;

  const float AngleDistanceOfLaserMeasureDeg =
      AngleDistanceOfTickDeg_ / num_points_per_laser;

  for (uint32 PointIdxSingleLaser = 0;
       PointIdxSingleLaser < num_points_per_laser; ++PointIdxSingleLaser) {
    for (uint32 ChannelIdx = 0; ChannelIdx < num_channel; ++ChannelIdx) {
      const float HorizontalAngleDeg =
          std::fmod(CurrentHorizontalAngleDeg_ +
                        AngleDistanceOfLaserMeasureDeg * PointIdxSingleLaser,
                    360.0f);

      // Check if the laser is outside the requested horizontal fov
      if (IsAngleInRange(HorizontalAngleDeg,
                         LidarSettings_.horizontal_fov_start_deg,
                         LidarSettings_.horizontal_fov_end_deg) == false) {
        continue;
      }

      LaserDirection Beam;
      Beam.VerticalAngleDeg = LaserAnglesDeg_[ChannelIdx];
      Beam.HorizontalAngleDeg = HorizontalAngleDeg;
      Beam.Channel = ChannelIdx;
      prg_laser_direction_ret->Add(Beam);
    }
  }

  return (true);
}

void UUnrealLidar::CylindricalScanPattern::Setup(
    const projectairsim::LidarSettings& LidarSettings) {
  ScanPatternBase::Setup(LidarSettings);

  const auto NumberOfLasers = LidarSettings.number_of_channels;

  // calculate verticle angle distance between each laser
  float DeltaAngleDeg = 0;
  if (NumberOfLasers > 1)
    DeltaAngleDeg = (LidarSettings.vertical_fov_upper_deg -
                     (LidarSettings.vertical_fov_lower_deg)) /
                    static_cast<float>(NumberOfLasers - 1);

  // store vertical angles for each laser
  LaserAnglesDeg_.clear();
  for (int i = 0; i < NumberOfLasers; ++i) {
    const float VerticalAngleDeg = LidarSettings.vertical_fov_upper_deg -
                                   static_cast<float>(i) * DeltaAngleDeg;
    LaserAnglesDeg_.emplace_back(VerticalAngleDeg);
  }
}

UUnrealLidar::RosetteScanPattern::RosetteScanPattern(void)
    : ScanPatternBase() {}

void UUnrealLidar::RosetteScanPattern::EndScan(void) {
  ScanPatternBase::EndScan();

  // Update angles past where we just scanned
  degree_major_ += ddegree_scan_major_;
  while (degree_major_ >= 360.0f) degree_major_ -= 360.0f;

  degree_minor_ += ddegree_scan_minor_;
  while (degree_minor_ >= 360.0f) degree_minor_ -= 360.0f;
}

bool UUnrealLidar::RosetteScanPattern::GetLasersToShoot(
    TArray<LaserDirection>* prg_laser_direction_ret) {
  const int num_channels = LidarSettings_.number_of_channels;
  const float vertical_offset_base =
      -distance_between_lasers_ * num_channels / 2.0 +
      distance_between_lasers_ / 2.0;
  const int num_points_per_laser = FMath::RoundHalfFromZero(
      LidarSettings_.points_per_second * DSecSimTime_ / num_channels);

  if (num_points_per_laser <= 0) {
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("[UnrealLidar] No points requested this frame, "
                           "try increasing the number of points per second."));
    return (false);
  }

  // Calculate and save the change in major and minor angles based on elapsed
  // simulation time
  ddegree_scan_major_ = degree_per_sec_major_ * DSecSimTime_;
  ddegree_scan_minor_ = degree_per_sec_minor_ * DSecSimTime_;

  // Generate the laser beam directions to sample
  {
    const float ddegree_per_point_major =
        ddegree_scan_major_ / num_points_per_laser;
    const float ddegree_per_point_minor =
        ddegree_scan_minor_ / num_points_per_laser;

    for (int ichannel = 0; ichannel < num_channels; ++ichannel) {
      for (int ipoint = 0; ipoint < num_points_per_laser; ++ipoint) {
        float degrees_horizontal;
        float degrees_vertical;
        double radians_major = projectairsim::TransformUtils::ToRadians(
            degree_major_ + ipoint * ddegree_per_point_major);
        double radians_minor = projectairsim::TransformUtils::ToRadians(
            degree_minor_ + ipoint * ddegree_per_point_minor);
        float pos_horizontal =
            0.5 * cos(radians_major) + 0.5 * cos(radians_minor);
        float pos_vertical =
            0.5 * sin(radians_major) -
            0.5 * sin(radians_minor);  // Note negated minor circle sin() so
                                       // minor circle rotates in opposite
                                       // direction of major circle

        // Convert from Cartesian coordinates to polar
        {
          double radians = fmod(atan2(pos_vertical, pos_horizontal), M_2PI);
          double radians_base = fmod(radians_major, M_2PI);
          double dradians = radians - radians_base;
          float radius = sqrt(pos_horizontal * pos_horizontal +
                              pos_vertical * pos_vertical);

          // Make sure the angular deviation from major circle angle is less
          // than half a circle
          if (dradians > M_PI) {
            dradians -= M_2PI;
            radians -= M_2PI;
          } else if (dradians <= -M_PI) {
            dradians += M_2PI;
            radians += M_2PI;
          }

          radians = radians_base + dradians * scaling_radial_;
          degrees_horizontal =
              projectairsim::TransformUtils::ToDegrees(radians);
          if (degrees_horizontal < 0.0)
            degrees_horizontal += 360.0f;
          else if (degrees_horizontal >= 360.0f)
            degrees_horizontal -= 360.0f;

          degrees_vertical =
              radius * degree_vertical_range_ + degree_vertical_min_;
        }

        // Add beam entry only if inside the horizontal FOV
        if (IsAngleInRange(degrees_horizontal,
                           LidarSettings_.horizontal_fov_start_deg,
                           LidarSettings_.horizontal_fov_end_deg)) {
          LaserDirection laser_direction;

          // Calculate the appropriate angles between lasers based on the
          // current position in the scan pattern
          float progress_angle = (M_PI * ipoint) / num_points_per_laser;
          float pitch_inc = (LidarSettings_.angle_between_lasers_pitch_max -
                             LidarSettings_.angle_between_lasers_pitch_min) *
                                cos(progress_angle) +
                            LidarSettings_.angle_between_lasers_pitch_min;
          float yaw_inc = (LidarSettings_.angle_between_lasers_yaw_max -
                           LidarSettings_.angle_between_lasers_yaw_min) *
                              sin(progress_angle) +
                          LidarSettings_.angle_between_lasers_yaw_min;

          // Use the angles between lasers to calculate the angle of the
          // specific laser indicated by ichannel
          float pitch_adjustment =
              -pitch_inc * num_channels / 2.0 + pitch_inc * (ichannel + 0.5);
          float yaw_adjustment =
              -yaw_inc * num_channels / 2.0 + yaw_inc * (ichannel + 0.5);

          laser_direction.VerticalAngleDeg =
              degrees_vertical + pitch_adjustment;
          laser_direction.HorizontalAngleDeg =
              degrees_horizontal + yaw_adjustment;
          laser_direction.Channel = ichannel;
          laser_direction.OffsetDistanceVertical =
              ichannel * distance_between_lasers_ + vertical_offset_base;
          prg_laser_direction_ret->Add(laser_direction);
        }
      }
    }
  }

  return (true);
}

void UUnrealLidar::RosetteScanPattern::Setup(
    const microsoft::projectairsim::LidarSettings& LidarSettings) {
  ScanPatternBase::Setup(LidarSettings);

  degree_vertical_min_ = LidarSettings.vertical_fov_lower_deg;
  degree_vertical_range_ =
      LidarSettings.vertical_fov_upper_deg - degree_vertical_min_;
  degree_per_sec_major_ = LidarSettings.horizontal_rotation_frequency * 360.0f;
  degree_per_sec_minor_ = LidarSettings.vertical_rotation_frequency * 360.0f;
  scaling_radial_ = LidarSettings.radial_scaling;
  distance_between_lasers_ = LidarSettings.distance_between_lasers;
}
