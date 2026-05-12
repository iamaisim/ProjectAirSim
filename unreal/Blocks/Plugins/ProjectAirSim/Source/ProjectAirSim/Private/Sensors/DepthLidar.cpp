// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// Depth-camera-based lidar implementation.

#include "DepthLidar.h"

#include <cmath>
#include <limits>
#include <vector>

#include "Components/LineBatchComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "ProjectAirSim.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealCameraRenderRequest.h"
#include "UnrealLogger.h"
#include "UnrealTransforms.h"

// Overall flow of this DepthLidar sensor:
// 1. `Initialize` copies the simulation lidar configuration and prepares the
//    capture component together with the floating-point render target.
// 2. On each `TickComponent`, the sensor computes how much simulation time has
//    advanced and calls `Simulate` to sweep the pending horizontal angle.
// 3. `Simulate` splits the tick sweep into small horizontal slices because a
//    single perspective camera cannot cover very wide angles without distorting
//    the depth reconstruction.
// 4. Each slice reorients the `SceneCaptureComponent2D`, renders a planar depth
//    image, and reads it back from the GPU to reconstruct rays for each
//    horizontal sample and each vertical lidar channel.
// 5. Valid hits are appended to the current sweep buffers; no-return samples are
//    appended only when the configuration requests them.
// 6. When new data exists, it is copied into publication buffers so the same
//    `TickComponent` can publish a `LidarMessage` aligned with the current pose.
// 7. When a full 360-degree revolution completes, the sweep accumulators are
//    cleared and the next sweep starts from an empty state.

namespace projectairsim = microsoft::projectairsim;

namespace {

constexpr float kMaxCaptureHorizontalFovDeg = 120.0f;
constexpr uint32 kMinCaptureWidth = 32;
constexpr uint32 kMaxCaptureWidth = 2048;
constexpr uint32 kMaxCaptureHeight = 2048;

float NormalizeAngleDeg(float AngleDeg) {
  return std::fmod(360.0f + std::fmod(AngleDeg, 360.0f), 360.0f);
}

}  // namespace

UDepthLidar::UDepthLidar(const FObjectInitializer& ObjectInitializer)
    : UUnrealSensor(ObjectInitializer), DepthPlanarMaterialStatic(nullptr) {
  bAutoActivate = true;

  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;

  static ConstructorHelpers::FObjectFinder<UMaterial> DepthPlanarMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "DepthPlannerMaterial.DepthPlannerMaterial'"));
  if (DepthPlanarMat.Succeeded()) {
    DepthPlanarMaterialStatic = DepthPlanarMat.Object;
  }
}

void UDepthLidar::Initialize(const projectairsim::Lidar& SimLidar) {
  Lidar = SimLidar;
  // This keeps the Unreal component and the simulation object on the same base
  // configuration before any readings start being published.
  SetupLidarFromSettings(SimLidar.GetLidarSettings());

  UnrealLogger::Log(
      projectairsim::LogLevel::kWarning,
      TEXT("[DepthLidar] Lidar '%S': initialized report_point_cloud=%d channels=%d range=%f rotation_hz=%f"),
      Lidar.GetId().c_str(), Settings.report_point_cloud ? 1 : 0,
      Settings.number_of_channels, Settings.range,
      Settings.horizontal_rotation_frequency);

  LastSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  RegisterComponent();
}

void UDepthLidar::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  const TimeNano CurSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  const TimeSec SimTimeDeltaSec =
      projectairsim::SimClock::Get()->NanosToSec(CurSimTime - LastSimTime);

  // First update the internal buffers with returns corresponding to the
  // simulation time that elapsed since the previous tick.
  Simulate(SimTimeDeltaSec);

  const auto LidarTransformStamped = UnrealTransform::GetPoseNed(this);
  projectairsim::Pose LidarPose(LidarTransformStamped.translation_,
                                LidarTransformStamped.rotation_);

  if (bHasPendingLidarMsg) {
    // Publication happens outside `Simulate` so the sweep can be associated with
    // the most recent sensor pose in the same Unreal frame.
    UnrealLogger::Log(
      projectairsim::LogLevel::kWarning,
      TEXT("[DepthLidar] Lidar '%S': publishing sweep point_count=%d intensity_count=%d laser_index_count=%d"),
      Lidar.GetId().c_str(), PointCloud.size() / 3, IntensityCloud.size(),
      LaserIndexCloud.size());

    projectairsim::LidarMessage LidarMsg(
        CurSimTime, PointCloud, AzimuthElevationRangeCloud, SegmentationCloud,
        IntensityCloud, LaserIndexCloud, LidarPose);
    Lidar.PublishLidarMsg(LidarMsg);

    PointCloud.clear();
    AzimuthElevationRangeCloud.clear();
    SegmentationCloud.clear();
    IntensityCloud.clear();
    LaserIndexCloud.clear();
    bHasPendingLidarMsg = false;
  }

  LastSimTime = CurSimTime;
}

void UDepthLidar::SetupLidarFromSettings(
    const projectairsim::LidarSettings& LidarSettings) {
  Settings = projectairsim::LidarSettings(LidarSettings);
  const float HorizontalFovSpanDeg =
      FMath::Abs(LidarSettings.horizontal_fov_end_deg -
                 LidarSettings.horizontal_fov_start_deg);
  // Normalize the configured angles once so every later azimuth comparison works
  // even when the input spans across the 0/360-degree wrap point.
  bUseFullHorizontalFov = HorizontalFovSpanDeg >= 359.9f;
  Settings.horizontal_fov_start_deg =
      NormalizeAngleDeg(Settings.horizontal_fov_start_deg);
  Settings.horizontal_fov_end_deg =
      NormalizeAngleDeg(Settings.horizontal_fov_end_deg);

  ChannelVerticalAnglesDeg.clear();
  ChannelVerticalAnglesDeg.reserve(Settings.number_of_channels);
  if (Settings.number_of_channels <= 1) {
    // With a single channel, use the center of the vertical FOV to represent a
    // single beam aligned with the configured opening.
    ChannelVerticalAnglesDeg.push_back(
        0.5f * (Settings.vertical_fov_upper_deg + Settings.vertical_fov_lower_deg));
  } else {
    // With multiple channels, linearly interpolate between the upper and lower
    // limits to obtain the elevation angle for each logical laser.
    for (int Channel = 0; Channel < Settings.number_of_channels; ++Channel) {
      const float T = static_cast<float>(Channel) /
                      static_cast<float>(Settings.number_of_channels - 1);
      ChannelVerticalAnglesDeg.push_back(FMath::Lerp(
          Settings.vertical_fov_upper_deg, Settings.vertical_fov_lower_deg, T));
    }
  }

  const float PointsPerRotation =
      static_cast<float>(Settings.points_per_second) /
      FMath::Max(Settings.horizontal_rotation_frequency, 0.01f) /
      FMath::Max(Settings.number_of_channels, 1);
  // The render target width approximates the horizontal sample count for one full
  // revolution; height starts at one row per laser channel and can grow later if
  // a slice needs extra vertical resolution to preserve aspect ratio.
  CurrentCaptureWidth = FMath::Clamp(FMath::RoundToInt(PointsPerRotation),
                                     static_cast<int32>(kMinCaptureWidth),
                                     static_cast<int32>(kMaxCaptureWidth));
  CurrentCaptureHeight = FMath::Max<uint32>(Settings.number_of_channels, 1u);

  InitializePose();
  SetupDepthCapture();
}

void UDepthLidar::BeginPlay() { Super::BeginPlay(); }

void UDepthLidar::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
  Lidar.EndUpdate();
}

bool UDepthLidar::IsAngleInRange(float AngleDeg, float StartAngleDeg,
                                 float EndAngleDeg) {
  if (StartAngleDeg <= EndAngleDeg) {
    return AngleDeg >= StartAngleDeg && AngleDeg <= EndAngleDeg;
  }

  return AngleDeg >= StartAngleDeg || AngleDeg <= EndAngleDeg;
}

void UDepthLidar::InitializePose() {
  SetRelativeTransform(UnrealTransform::FromGlobalNed(Settings.origin));

  projectairsim::Transform InitializedPose = UnrealTransform::GetPoseNed(this);
  projectairsim::Vector3 InitializedRPY =
      projectairsim::TransformUtils::ToDegrees(
          projectairsim::TransformUtils::ToRPY(InitializedPose.rotation_));
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[DepthLidar] Lidar '%S': InitializePose(). RelativeLocation "
           "(%f,%f,%f) RelativeRotationRPY (%f,%f,%f)"),
      Lidar.GetId().c_str(), InitializedPose.translation_.x(),
      InitializedPose.translation_.y(), InitializedPose.translation_.z(),
      InitializedRPY.x(), InitializedRPY.y(), InitializedRPY.z());
}

void UDepthLidar::SetupDepthCapture() {
  if (!DepthCaptureComponent) {
    DepthCaptureComponent =
        NewObject<USceneCaptureComponent2D>(this, TEXT("DepthLidarCapture"));
    DepthCaptureComponent->SetupAttachment(this);
    // The custom material writes planar depth into a floating-point render target,
    // which we later reinterpret as lidar ranges.
    DepthCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    DepthCaptureComponent->bAutoActivate = true;
    DepthCaptureComponent->bCaptureEveryFrame = false;
    DepthCaptureComponent->bCaptureOnMovement = false;
    DepthCaptureComponent->bAlwaysPersistRenderingState = true;
    if (DepthPlanarMaterialStatic) {
      DepthCaptureComponent->AddOrUpdateBlendable(DepthPlanarMaterialStatic, 1.0f);
    }

    if (Settings.disable_self_hits && GetOwner()) {
      DepthCaptureComponent->HideActorComponents(GetOwner());
    }

    if (UnrealWorld) {
#if UE_IS_5_2
      // UE < 5.6: individual LineBatcher pointers on UWorld
      DepthCaptureComponent->HideComponent(
          Cast<UPrimitiveComponent>(UnrealWorld->LineBatcher));
      DepthCaptureComponent->HideComponent(
          Cast<UPrimitiveComponent>(UnrealWorld->PersistentLineBatcher));
      DepthCaptureComponent->HideComponent(
          Cast<UPrimitiveComponent>(UnrealWorld->ForegroundLineBatcher));
#elif UE_IS_5_7
      // UE 5.6+: LineBatcher/PersistentLineBatcher/ForegroundLineBatcher were
      // replaced by GetLineBatcher(ELineBatcherType)
      DepthCaptureComponent->HideComponent(
          UnrealWorld->GetLineBatcher(UWorld::ELineBatcherType::World));
      DepthCaptureComponent->HideComponent(
          UnrealWorld->GetLineBatcher(UWorld::ELineBatcherType::WorldPersistent));
      DepthCaptureComponent->HideComponent(
          UnrealWorld->GetLineBatcher(UWorld::ELineBatcherType::Foreground));
#endif
    }

    DepthCaptureComponent->RegisterComponent();
  }

  if (!DepthRenderTarget) {
    DepthRenderTarget =
        NewObject<UTextureRenderTarget2D>(this, TEXT("DepthLidarRenderTarget"));
  }

  DepthRenderTarget->InitCustomFormat(
      CurrentCaptureWidth, CurrentCaptureHeight, EPixelFormat::PF_FloatRGBA,
      true);
  DepthCaptureComponent->TextureTarget = DepthRenderTarget;
}

void UDepthLidar::UpdateCaptureConfiguration(float HorizontalFovDeg,
                                             float YawDeg,
                                             int32 SliceSamples) {
  if (!DepthCaptureComponent) {
    return;
  }

  const float VerticalFovDeg =
      FMath::Max(Settings.vertical_fov_upper_deg - Settings.vertical_fov_lower_deg,
                 1.0f);
  const float HalfHorizontalRad = projectairsim::TransformUtils::ToRadians(
      0.5f * FMath::Max(HorizontalFovDeg, 1.0f));
  const float HalfVerticalRad = projectairsim::TransformUtils::ToRadians(
      0.5f * VerticalFovDeg);

  const uint32 RequestedWidth = FMath::Clamp(
      SliceSamples, static_cast<int32>(kMinCaptureWidth),
      static_cast<int32>(kMaxCaptureWidth));
  // Keeping the aspect ratio aligned with the angular window prevents the
  // pixel-to-ray conversion from drifting away from the actual perspective
  // camera view volume used by the capture.
  const float DesiredAspectRatio =
      FMath::Tan(HalfHorizontalRad) / FMath::Max(FMath::Tan(HalfVerticalRad),
                                                 KINDA_SMALL_NUMBER);
  const uint32 RequestedHeight = FMath::Clamp(
      FMath::RoundToInt(static_cast<float>(RequestedWidth) /
                        FMath::Max(DesiredAspectRatio, KINDA_SMALL_NUMBER)),
      FMath::Max(Settings.number_of_channels, 1),
      static_cast<int32>(kMaxCaptureHeight));

  if (CurrentCaptureWidth != RequestedWidth ||
      CurrentCaptureHeight != RequestedHeight) {
    CurrentCaptureWidth = RequestedWidth;
    CurrentCaptureHeight = RequestedHeight;
    DepthRenderTarget->InitCustomFormat(CurrentCaptureWidth, CurrentCaptureHeight,
                                        EPixelFormat::PF_FloatRGBA, true);
  }

  DepthCaptureComponent->ProjectionType = ECameraProjectionMode::Perspective;
  DepthCaptureComponent->FOVAngle = FMath::Clamp(HorizontalFovDeg, 1.0f, 170.0f);
  DepthCaptureComponent->SetRelativeRotation(FRotator(0.0f, YawDeg, 0.0f));

  const float AspectRatio = static_cast<float>(CurrentCaptureWidth) /
                            static_cast<float>(FMath::Max(CurrentCaptureHeight, 1u));
  const float VerticalForProjection = projectairsim::TransformUtils::ToDegrees(
      2.0f * std::atan(std::tan(HalfHorizontalRad) / AspectRatio));
  DepthCaptureComponent->FOVAngle = DepthCaptureComponent->FOVAngle;
}

void UDepthLidar::ReadDepthPixels(TArray<FFloat16Color>& OutDepthPixels) const {
  OutDepthPixels.Reset();

  if (!DepthCaptureComponent || !DepthCaptureComponent->TextureTarget) {
    return;
  }

  FTextureRenderTargetResource* RenderTargetResource =
      DepthCaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();
  if (!RenderTargetResource) {
    return;
  }

  UnrealCameraRenderRequest::RenderResult RenderResult;
  FTextureRHIRef TextureRef = RenderTargetResource->GetRenderTargetTexture();
  // Scene capture rendering completes on the render thread, so the readback is queued
  // there and then synchronized before the CPU consumes the pixels.
  ENQUEUE_RENDER_COMMAND(ReadDepthLidarPixels)(
      [TextureRef, &RenderResult](FRHICommandListImmediate& RHICmdList) {
        if (!TextureRef.IsValid()) {
          return;
        }

        FRHITexture* Texture2D = TextureRef->GetTexture2D();
        if (!Texture2D) {
          return;
        }

        UnrealCameraRenderRequest::ReadPixels(Texture2D, true, &RenderResult);
      });
  FlushRenderingCommands();

  OutDepthPixels = MoveTemp(RenderResult.UnrealImageFloat);
}

void UDepthLidar::LogDepthPixels(const TArray<FFloat16Color>& DepthPixels,
                                 float SliceAngleDeg) const {
  if (DepthPixels.Num() == 0) {
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("[DepthLidar] Lidar '%S': empty depth buffer."),
                      Lidar.GetId().c_str());
    return;
  }

  float MinR = std::numeric_limits<float>::max();
  float MaxR = 0.0f;
  float MinG = std::numeric_limits<float>::max();
  float MaxG = 0.0f;
  float MinB = std::numeric_limits<float>::max();
  float MaxB = 0.0f;
  float MinA = std::numeric_limits<float>::max();
  float MaxA = 0.0f;
  int32 FiniteR = 0;

  for (const FFloat16Color& DepthPixel : DepthPixels) {
    const float R = DepthPixel.R.GetFloat();
    const float G = DepthPixel.G.GetFloat();
    const float B = DepthPixel.B.GetFloat();
    const float A = DepthPixel.A.GetFloat();

    if (FMath::IsFinite(R)) {
      MinR = FMath::Min(MinR, R);
      MaxR = FMath::Max(MaxR, R);
      ++FiniteR;
    }
    if (FMath::IsFinite(G)) {
      MinG = FMath::Min(MinG, G);
      MaxG = FMath::Max(MaxG, G);
    }
    if (FMath::IsFinite(B)) {
      MinB = FMath::Min(MinB, B);
      MaxB = FMath::Max(MaxB, B);
    }
    if (FMath::IsFinite(A)) {
      MinA = FMath::Min(MinA, A);
      MaxA = FMath::Max(MaxA, A);
    }
  }

  const int32 CenterIdx = DepthPixels.Num() / 2;
  const FFloat16Color& CenterPixel = DepthPixels[CenterIdx];
}

void UDepthLidar::AppendReturnPoint(const FVector& PointSensorFrame,
                                    float AzimuthDeg, float ElevationDeg,
                                    int LaserIndex) {
  // The depth capture reconstructs points in Unreal sensor space; publishing happens in
  // NED coordinates to match the rest of the simulation APIs.
  const projectairsim::Vector3 PointNed =
      UnrealTransform::UnrealToNedLinear(PointSensorFrame);

  if (Settings.report_point_cloud) {
    SweepPointCloud.emplace_back(PointNed.x());
    SweepPointCloud.emplace_back(PointNed.y());
    SweepPointCloud.emplace_back(PointNed.z());
  }

  if (Settings.report_azimuth_elevation_range) {
    SweepAzimuthElevationRangeCloud.emplace_back(
        projectairsim::TransformUtils::ToRadians(AzimuthDeg));
    SweepAzimuthElevationRangeCloud.emplace_back(
        projectairsim::TransformUtils::ToRadians(ElevationDeg));
    SweepAzimuthElevationRangeCloud.emplace_back(PointSensorFrame.Size());
  }

  SweepSegmentationCloud.emplace_back(-1);
  SweepIntensityCloud.emplace_back(1.0f);
  SweepLaserIndexCloud.emplace_back(LaserIndex);
}

void UDepthLidar::AppendNoReturnPoint(float AzimuthDeg, float ElevationDeg,
                                      int LaserIndex) {
  if (!Settings.report_no_return_points) {
    return;
  }

  if (Settings.report_point_cloud) {
    SweepPointCloud.emplace_back(Settings.no_return_point_value.x());
    SweepPointCloud.emplace_back(Settings.no_return_point_value.y());
    SweepPointCloud.emplace_back(Settings.no_return_point_value.z());
  }

  if (Settings.report_azimuth_elevation_range) {
    SweepAzimuthElevationRangeCloud.emplace_back(
        projectairsim::TransformUtils::ToRadians(AzimuthDeg));
    SweepAzimuthElevationRangeCloud.emplace_back(
        projectairsim::TransformUtils::ToRadians(ElevationDeg));
    SweepAzimuthElevationRangeCloud.emplace_back(-1.0f);
  }

  SweepSegmentationCloud.emplace_back(-1);
  SweepIntensityCloud.emplace_back(0.0f);
  SweepLaserIndexCloud.emplace_back(LaserIndex);
}

void UDepthLidar::Simulate(float SimTimeDeltaSec) {
  // Skip simulation if the capture camera is not ready or no time has elapsed.
  if (!DepthCaptureComponent || SimTimeDeltaSec <= 0.0f) {
    return;
  }

  // Remember the previous sizes so this tick can detect whether it produced new
  // data and only then prepare a publication at the end of the frame.
  const size_t InitialSweepPointCount = SweepPointCloud.size();
  const size_t InitialSweepAerCount = SweepAzimuthElevationRangeCloud.size();
  const size_t InitialSweepSegmentationCount = SweepSegmentationCloud.size();
  const size_t InitialSweepIntensityCount = SweepIntensityCloud.size();
  const size_t InitialSweepLaserIndexCount = SweepLaserIndexCloud.size();

  // Compute the number of horizontal ray samples expected from the elapsed time.
  // Dividing by the channel count gives the per-row sample budget for this tick.
  const uint32 HorizontalResolution = FMath::RoundHalfFromZero(
      Settings.points_per_second * SimTimeDeltaSec /
      static_cast<float>(FMath::Max(Settings.number_of_channels, 1)));
  // If no full sample can be produced this tick (very short delta), skip early.
  if (HorizontalResolution == 0) {
    return;
  }

  // Compute the total azimuth angle swept during this tick based on the
  // configured rotation frequency and the elapsed simulation time.
  const float AngleDistanceOfTickDeg =
      Settings.horizontal_rotation_frequency * 360.0f * SimTimeDeltaSec;
  const TimeNano CurSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  // Rate-limit depth debug logging to at most once per simulated second to
  // avoid flooding the log with per-tick captures.
  const bool bLogDepthThisTick =
      (LastDepthDebugLogSimTime == 0) ||
      (CurSimTime - LastDepthDebugLogSimTime >=
       projectairsim::SimClock::Get()->SecToNanos(1.0f));
  // A single perspective capture does not handle very wide sweeps well, so the
  // tick is split into slices with a bounded maximum horizontal FOV.
  const int32 CaptureSlices =
      FMath::Max(1, FMath::CeilToInt(AngleDistanceOfTickDeg / kMaxCaptureHorizontalFovDeg));
  // Angle covered by each individual slice.
  const float SliceAngleDeg = AngleDistanceOfTickDeg / CaptureSlices;
  const float MaxRangeM = Settings.range;
  const int32 NumLasers = FMath::Max(Settings.number_of_channels, 1);

  int32 SampleStart = 0;
  TArray<FFloat16Color> DepthPixels;
  // Iterate over each horizontal slice to keep individual capture FOVs narrow
  // enough to minimize perspective distortion across the sampled angle range.
  for (int32 SliceIdx = 0; SliceIdx < CaptureSlices; ++SliceIdx) {
    // Distribute the total horizontal resolution evenly across slices.
    const int32 SampleEnd =
        FMath::RoundToInt((static_cast<float>(SliceIdx + 1) / CaptureSlices) *
                          HorizontalResolution);
    const int32 SliceSamples = SampleEnd - SampleStart;
    // Skip degenerate slices that have no horizontal samples to process.
    if (SliceSamples <= 0) {
      SampleStart = SampleEnd;
      continue;
    }

    // The start angle of this slice in world azimuth; the center is used to
    // point the capture camera so equal margins cover both sides of the slice.
    const float SliceStartAngleDeg =
        CurrentHorizontalAngleDeg + (SliceIdx * SliceAngleDeg);
    const float SliceCenterAngleDeg = SliceStartAngleDeg + (0.5f * SliceAngleDeg);

    // Each slice reuses the same capture camera, changing only its yaw and the
    // resolution needed to cover the current subset of horizontal rays.
    UpdateCaptureConfiguration(SliceAngleDeg, SliceCenterAngleDeg, SliceSamples);
    DepthCaptureComponent->CaptureScene();
    ReadDepthPixels(DepthPixels);
    if (bLogDepthThisTick && SliceIdx == 0) {
      LogDepthPixels(DepthPixels, SliceAngleDeg);
      LastDepthDebugLogSimTime = CurSimTime;
    }
    // Verify the readback produced the expected number of pixels before
    // attempting to index into the buffer below.
    if (DepthPixels.Num() !=
        static_cast<int32>(CurrentCaptureWidth * CurrentCaptureHeight)) {
      SampleStart = SampleEnd;
      continue;
    }

    // Pre-compute half-FOV values in radians for the perspective projection
    // formulas used during per-sample ray reconstruction.
    const float VerticalFovDeg =
        FMath::Max(Settings.vertical_fov_upper_deg - Settings.vertical_fov_lower_deg,
                   1.0f);
    const float HalfHorizontalFovRad = projectairsim::TransformUtils::ToRadians(
        0.5f * SliceAngleDeg);
    const float HalfVerticalFovRad = projectairsim::TransformUtils::ToRadians(
        0.5f * VerticalFovDeg);

    // Process each horizontal sample within this slice.
    for (int32 SampleIndex = 0; SampleIndex < SliceSamples; ++SampleIndex) {
      // Normalized position [0, 1] of this sample within the slice.
      const float HorizontalAlpha =
          (static_cast<float>(SampleIndex) + 0.5f) /
          static_cast<float>(SliceSamples);
      // Reconstruct the local yaw inside the actual camera view volume rather
      // than assuming the angular spacing is linear in pixel space.
      const float HorizontalProjection = (2.0f * HorizontalAlpha) - 1.0f;
      const float LocalYawRad = FMath::Atan(
          HorizontalProjection * FMath::Tan(HalfHorizontalFovRad));
      const float LocalYawDeg =
          projectairsim::TransformUtils::ToDegrees(LocalYawRad);
      const float AbsoluteYawDeg = NormalizeAngleDeg(
          SliceCenterAngleDeg + LocalYawDeg);

      // Skip this ray if it falls outside the configured horizontal FOV window.
      if (!bUseFullHorizontalFov &&
          !IsAngleInRange(AbsoluteYawDeg, Settings.horizontal_fov_start_deg,
                          Settings.horizontal_fov_end_deg)) {
        continue;
      }

      // Map the normalized horizontal position to the nearest pixel column in
      // the render target buffer for this slice.
      const int32 PixelX = FMath::Clamp(
          FMath::RoundToInt(HorizontalAlpha * static_cast<float>(CurrentCaptureWidth) - 0.5f),
          0, static_cast<int32>(CurrentCaptureWidth) - 1);

      // For each vertical lidar channel, sample the corresponding depth pixel
      // and reconstruct the 3-D hit point if the depth is within valid range.
      for (int32 LaserIndex = 0; LaserIndex < NumLasers; ++LaserIndex) {
        const float ElevationDeg = ChannelVerticalAnglesDeg[LaserIndex];
        const float ElevationRad =
            projectairsim::TransformUtils::ToRadians(ElevationDeg);
        // Each vertical channel maps onto the row whose elevation best matches
        // the logical laser angle within the same perspective capture.
        const float VerticalProjection =
            FMath::Tan(ElevationRad) /
            FMath::Max(FMath::Tan(HalfVerticalFovRad), KINDA_SMALL_NUMBER);
        const float VerticalAlpha = 0.5f * (1.0f - VerticalProjection);
        const int32 PixelY = FMath::Clamp(
            FMath::RoundToInt(
                VerticalAlpha * static_cast<float>(CurrentCaptureHeight) - 0.5f),
            0, static_cast<int32>(CurrentCaptureHeight) - 1);

        const FFloat16Color& DepthPixel =
            DepthPixels[(PixelY * CurrentCaptureWidth) + PixelX];
        // Read the planar depth value encoded in the red channel by the custom material.
        const float DepthM = DepthPixel.R.GetFloat();
        // Reject pixels with non-finite depth, near-zero values, or values
        // beyond the configured sensor range.
        const bool bValidDepth = FMath::IsFinite(DepthM) && DepthM > 0.01f &&
                                 DepthM <= MaxRangeM;
        if (!bValidDepth) {
          AppendNoReturnPoint(AbsoluteYawDeg, ElevationDeg, LaserIndex);
          continue;
        }

        // Convert planar depth in meters to centimeters for Unreal's coordinate system.
        const float DepthCm =
            projectairsim::TransformUtils::ToCentimeters(DepthM);
        // Reconstruct the 3-D position of the hit in the local capture camera
        // frame using the perspective projection formula:
        //   X = depth (forward axis)
        //   Y = depth * tan(yaw)    (right)
        //   Z = depth * tan(pitch)  (up)
        const FVector PointCaptureFrame(
            DepthCm,
            DepthCm * FMath::Tan(LocalYawRad),
            DepthCm * FMath::Tan(ElevationRad));
        // The point is first reconstructed in the slice capture frame, then
        // rotated back into the full sensor frame before being appended.
        const FVector PointSensorFrame =
            FRotator(0.0f, SliceCenterAngleDeg, 0.0f).RotateVector(PointCaptureFrame);
        AppendReturnPoint(PointSensorFrame, AbsoluteYawDeg, ElevationDeg,
                          LaserIndex);

        if (Settings.draw_debug_points && UnrealWorld) {
          DrawDebugPoint(UnrealWorld,
                         GetComponentTransform().TransformPosition(PointSensorFrame),
                         10, FColor(255, 0, 255), false, 0.1f);
        }
      }
    }

    SampleStart = SampleEnd;
  }

  // Advance the accumulated angle tracker by the amount swept this tick.
  AccumulatedSweepAngleDeg += AngleDistanceOfTickDeg;
  // Detect whether any new samples were added to the sweep buffers this tick.
  const bool bSweepBuffersChanged =
      SweepPointCloud.size() != InitialSweepPointCount ||
      SweepAzimuthElevationRangeCloud.size() != InitialSweepAerCount ||
      SweepSegmentationCloud.size() != InitialSweepSegmentationCount ||
      SweepIntensityCloud.size() != InitialSweepIntensityCount ||
      SweepLaserIndexCloud.size() != InitialSweepLaserIndexCount;
  if (bSweepBuffersChanged) {
    // Take a snapshot of the accumulated sweep for `TickComponent` to publish
    // without exposing partially mutated buffers while accumulation continues.
    PointCloud = SweepPointCloud;
    AzimuthElevationRangeCloud = SweepAzimuthElevationRangeCloud;
    SegmentationCloud = SweepSegmentationCloud;
    IntensityCloud = SweepIntensityCloud;
    LaserIndexCloud = SweepLaserIndexCloud;
    bHasPendingLidarMsg = true;
  }

  if (AccumulatedSweepAngleDeg >= 360.0f) {
    // Once 360 degrees are completed, reset the sweep accumulators while keeping
    // any angular overshoot so fast ticks do not lose progress.
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[DepthLidar] Lidar '%S': completed sweep accumulated_angle=%f point_count=%d intensity_count=%d laser_index_count=%d"),
        Lidar.GetId().c_str(), AccumulatedSweepAngleDeg, PointCloud.size() / 3,
        IntensityCloud.size(), LaserIndexCloud.size());
    SweepPointCloud.clear();
    SweepAzimuthElevationRangeCloud.clear();
    SweepSegmentationCloud.clear();
    SweepIntensityCloud.clear();
    SweepLaserIndexCloud.clear();
    AccumulatedSweepAngleDeg =
        FMath::Fmod(AccumulatedSweepAngleDeg, 360.0f);
  }

  CurrentHorizontalAngleDeg = NormalizeAngleDeg(
      CurrentHorizontalAngleDeg + AngleDistanceOfTickDeg);
}