// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// Unreal Lidar Implementation

#include "GPULidar.h"

#include "ProjectAirSim.h"
#include "UnrealCompatibility.h"
#include "Components/LineBatchComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "LidarPointCloudCS.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Serialization/BufferArchive.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealCameraRenderRequest.h"
#include "UnrealLogger.h"
#include "UnrealTransforms.h"

#include <cmath>

namespace projectairsim = microsoft::projectairsim;

namespace {

float NormalizeGPULidarAngleDeg(float AngleDeg) {
  return std::fmod(360.0f + std::fmod(AngleDeg, 360.0f), 360.0f);
}

float GetHorizontalFovSpanDeg(float StartAngleDeg, float EndAngleDeg) {
  const float SpanDeg = NormalizeGPULidarAngleDeg(EndAngleDeg - StartAngleDeg);
  return FMath::IsNearlyZero(SpanDeg) ? 360.0f : SpanDeg;
}

}  // namespace

UGPULidar::UGPULidar(const FObjectInitializer& ObjectInitializer)
    : UUnrealSensor(ObjectInitializer), IntensityExtension(nullptr) {
  bAutoActivate = true;

  // Tick in PostUpdateWork to update at the same time as UnrealCamera, instead
  // of waiting for the PrePhysics tick on the next loop
  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;

  static ConstructorHelpers::FObjectFinder<UMaterial> LidarIntensityMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "LidarIntensityMaterial.LidarIntensityMaterial'"));

  LidarIntensityMaterialStatic = LidarIntensityMat.Object;
}

void UGPULidar::Initialize(const projectairsim::Lidar& SimLidar) {
  Lidar = SimLidar;
  SetupLidarFromSettings(SimLidar.GetLidarSettings());

  RegisterComponent();

  FCoreDelegates::OnBeginFrame.AddUObject(this, &UGPULidar::BeginFrameCallback);
  FCoreDelegates::OnEndFrame.AddUObject(this, &UGPULidar::EndFrameCallback);

  FCoreDelegates::OnBeginFrameRT.AddUObject(this,
                                            &UGPULidar::BeginFrameCallbackRT);
  FCoreDelegates::OnEndFrameRT.AddUObject(this, &UGPULidar::EndFrameCallbackRT);
}

void UGPULidar::TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  const TimeNano CurSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  const TimeSec SimTimeDeltaSec =
      projectairsim::SimClock::Get()->NanosToSec(CurSimTime - LastSimTime);

  const bool bHasNewLidarData = Simulate(SimTimeDeltaSec, CurSimTime);

  if (bHasNewLidarData) {
    projectairsim::LidarMessage LidarMsg(
        PointCloudTime, PointCloud, AzimuthElevationRangeCloud,
        SegmentationCloud, IntensityCloud, LaserIndexCloud, PointCloudPose);
    Lidar.PublishLidarMsg(LidarMsg);
  }

  LastSimTime = CurSimTime;
}

void UGPULidar::SetupLidarFromSettings(
    const projectairsim::LidarSettings& LidarSettings) {
  Settings = projectairsim::LidarSettings(LidarSettings);
  Settings.horizontal_fov_start_deg =
      NormalizeGPULidarAngleDeg(Settings.horizontal_fov_start_deg);
  Settings.horizontal_fov_end_deg =
      NormalizeGPULidarAngleDeg(Settings.horizontal_fov_end_deg);

  InitializePose();
  SetUpCams();
}

void UGPULidar::SetupSceneCapture(
    UCameraComponent* CameraComponent, float HorizontalAngle, float Width,
    float Height, USceneCaptureComponent2D* OutSceneCaptureComp) {
  OutSceneCaptureComp->SetupAttachment(CameraComponent);
  OutSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  OutSceneCaptureComp->bAutoActivate = true;
  OutSceneCaptureComp->bCaptureEveryFrame = false;
  OutSceneCaptureComp->bCaptureOnMovement = false;
  OutSceneCaptureComp->FOVAngle = HorizontalAngle;
  OutSceneCaptureComp->ProjectionType = ECameraProjectionMode::Perspective;

  if (Settings.disable_self_hits) {
    OutSceneCaptureComp->HideActorComponents(GetOwner());  // don't hit yourself
  }

  // Hide debug points in case "draw-debug-points" is set to true
  #if UE_IS_5_7 
    OutSceneCaptureComp->HideComponent(Cast<UPrimitiveComponent>(
        UnrealWorld->GetLineBatcher(UWorld::ELineBatcherType::World)));
    OutSceneCaptureComp->HideComponent(Cast<UPrimitiveComponent>(
        UnrealWorld->GetLineBatcher(UWorld::ELineBatcherType::WorldPersistent)));
    OutSceneCaptureComp->HideComponent(Cast<UPrimitiveComponent>(
        UnrealWorld->GetLineBatcher(UWorld::ELineBatcherType::Foreground)));
  #elif UE_IS_5_2
  OutSceneCaptureComp->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->LineBatcher));
  OutSceneCaptureComp->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->PersistentLineBatcher));
  OutSceneCaptureComp->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->ForegroundLineBatcher));
  #endif

  OutSceneCaptureComp->RegisterComponent();
  auto RenderTarget = NewObject<UTextureRenderTarget2D>();
  RenderTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_FloatRGBA,
                                 true);
  OutSceneCaptureComp->TextureTarget = RenderTarget;
}

void UGPULidar::SetUpCams() {
  CameraComponents.clear();
  DepthSceneCaptures.clear();

  int NumCams = 4;
  int WidthTotal = 1024;
  int WidthEachCam = WidthTotal / NumCams;

  double VerticleAngle = 30;
  double HorizontalAngle = 360.0 / (double)NumCams;  // 90 degree for 4

  // some ideas from here
  // https://b3d.interplanety.org/en/vertical-and-horizontal-camera-fov-angles/

  VerticleAngle = 2 * std::atan(std::tan(projectairsim::TransformUtils::ToRadians(
                                    VerticleAngle / 2.0f)) /
                                std::cos(projectairsim::TransformUtils::ToRadians(
                                    HorizontalAngle / 2.0f)));

  VerticleAngle = projectairsim::TransformUtils::ToDegrees(VerticleAngle);

  auto aspectRatio =
      std::tan(projectairsim::TransformUtils::ToRadians(HorizontalAngle / 2.0)) /
      std::tan(projectairsim::TransformUtils::ToRadians(VerticleAngle / 2.0));

  int HeightEachCam = (int)WidthEachCam / aspectRatio;

  CamFrustrumHeight = HeightEachCam;
  CamFrustrumWidth = WidthEachCam;

  // Keep all cameras enabled so the GPU path can distribute the sweep across
  // four 90-degree captures instead of collapsing back to a single forward view.
  for (int i = 0; i < NumCams; i++) {
    std::string dcamstr = "DepthCam_" + std::to_string(i);
    std::string capturestr = "SceneCapture_" + std::to_string(i);
    auto SceneCapture =
        NewObject<USceneCaptureComponent2D>(this, capturestr.c_str());
    auto CameraComponent = NewObject<UCameraComponent>(this, dcamstr.c_str());
    CameraComponent->AttachToComponent(
        this, FAttachmentTransformRules::KeepRelativeTransform);
    SetupSceneCapture(CameraComponent, HorizontalAngle, WidthEachCam,
                      HeightEachCam, SceneCapture);
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;

    auto quat = FQuat::MakeFromEuler(FVector(0, 0, i * 360.0f / 4.f));
    CamRotationMats.push_back(FRotationMatrix::Make(quat));
    CameraComponent->AddLocalRotation(quat);

    CameraComponent->ProjectionMode = ECameraProjectionMode::Perspective;
    CameraComponent->FieldOfView = HorizontalAngle;
    CameraComponent->AspectRatio = aspectRatio;

    // Render intensity to target texture
    std::string IntensityCaptureStr = "IntensityCapture_" + std::to_string(i);
    auto IntensityCaptureComponent =
        NewObject<USceneCaptureComponent2D>(this, IntensityCaptureStr.c_str());
    SetupSceneCapture(CameraComponent, HorizontalAngle, WidthEachCam,
                      HeightEachCam, IntensityCaptureComponent);

    IntensityExtension =
        FSceneViewExtensions::NewExtension<FLidarIntensitySceneViewExtension>(
            IntensityCaptureComponent->TextureTarget);
    IntensityCaptureComponent->SceneViewExtensions.Add(IntensityExtension);

    // Add to stack of captures
    DepthSceneCaptures.push_back(SceneCapture);
    CameraComponents.push_back(CameraComponent);
    IntensityCaptureComponents.push_back(IntensityCaptureComponent);
  }
}

void UGPULidar::BeginPlay() { Super::BeginPlay(); }

void UGPULidar::BeginDestroy() { Super::BeginDestroy(); }

void UGPULidar::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
  Lidar.EndUpdate();
}

void UGPULidar::InitializePose() {
  SetRelativeTransform(UnrealTransform::FromGlobalNed(Settings.origin));

  // Check that the initial pose was set correctly
  projectairsim::Transform InitializedPose = UnrealTransform::GetPoseNed(this);
  projectairsim::Vector3 InitializedRPY = projectairsim::TransformUtils::ToDegrees(
      projectairsim::TransformUtils::ToRPY(InitializedPose.rotation_));
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[UnrealLidar] Lidar '%S': InitializePose(). "
           "RelativeLocation (%f,%f,%f) RelativeRotationRPY (%f,%f,%f)"),
      Lidar.GetId().c_str(), InitializedPose.translation_.x(),
      InitializedPose.translation_.y(), InitializedPose.translation_.z(),
      InitializedRPY.x(), InitializedRPY.y(), InitializedRPY.z());
}

bool UGPULidar::Simulate(const float SimTimeDeltaSec,
                         const TimeNano CurSimTime) {
  PointCloud.clear();
  AzimuthElevationRangeCloud.clear();
  SegmentationCloud.clear();
  IntensityCloud.clear();
  LaserIndexCloud.clear();

  if (resetCams) {
    SetUpCams();
    resetCams = false;
  }

  for (int CaptureIdx = 0; CaptureIdx < DepthSceneCaptures.size();
       ++CaptureIdx) {
    // TODO: we shouldn't need two capture components since the full scene
    // capture should already have depth information.
    DepthSceneCaptures[CaptureIdx]->CaptureScene();
    IntensityCaptureComponents[CaptureIdx]->CaptureScene();
  }

  HorizontalResolution =
      FMath::RoundHalfFromZero(Settings.points_per_second * SimTimeDeltaSec /
                               static_cast<float>(Settings.number_of_channels));

  if (HorizontalResolution <= 0) {
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("[UnrealLidar] No points requested this frame, "
                           "try increasing the number of points per second."));
    return false;
  }

  const float HorizontalFOVDeg =
      GetHorizontalFovSpanDeg(Settings.horizontal_fov_start_deg,
                              Settings.horizontal_fov_end_deg);
  // Create parameters & pass data
  FLidarPointCloudCSParameters PointCloudParams;
  PointCloudParams.HorizontalResolution = HorizontalResolution;
  PointCloudParams.LaserNums = Settings.number_of_channels;
  PointCloudParams.LaserRange =
      projectairsim::TransformUtils::ToCentimeters(Settings.range);
  PointCloudParams.HorizontalFOV = HorizontalFOVDeg;
  // For this test path, do not rotate a simulated sweep over time. Sample the
  // configured horizontal FOV from its configured start angle every frame.
  PointCloudParams.HorizontalFOVStartDeg = Settings.horizontal_fov_start_deg;
  PointCloudParams.HorizontalFOVEndDeg = Settings.horizontal_fov_end_deg;
  PointCloudParams.CurrentHorizontalAngleDeg =
      Settings.horizontal_fov_start_deg;
  const auto LidarTransformStamped = UnrealTransform::GetPoseNed(this);
  PointCloudParams.CaptureTime = CurSimTime;
  PointCloudParams.LidarPose =
      projectairsim::Pose(LidarTransformStamped.translation_,
                          LidarTransformStamped.rotation_);
  PointCloudParams.VerticalFOVUpperDeg = Settings.vertical_fov_upper_deg;
  PointCloudParams.VerticalFOVLowerDeg = Settings.vertical_fov_lower_deg;
  PointCloudParams.NumCams = DepthSceneCaptures.size();
  PointCloudParams.CameraHorizontalFOVDeg =
      360.0f / static_cast<float>(PointCloudParams.NumCams);
  PointCloudParams.CamFrustrumHeight = CamFrustrumHeight;
  PointCloudParams.CamFrustrumWidth = CamFrustrumWidth;
  // Bind one depth texture per quadrant so the compute shader can project each
  // point into the camera that actually covers that azimuth.
  PointCloudParams.DepthTexture1 =
      DepthSceneCaptures[0]
          ->TextureTarget->GameThread_GetRenderTargetResource()
          ->GetRenderTargetTexture();
  PointCloudParams.DepthTexture2 =
      DepthSceneCaptures[1]
          ->TextureTarget->GameThread_GetRenderTargetResource()
          ->GetRenderTargetTexture();
  PointCloudParams.DepthTexture3 =
      DepthSceneCaptures[2]
          ->TextureTarget->GameThread_GetRenderTargetResource()
          ->GetRenderTargetTexture();
  PointCloudParams.DepthTexture4 =
      DepthSceneCaptures[3]
          ->TextureTarget->GameThread_GetRenderTargetResource()
          ->GetRenderTargetTexture();

  PointCloudParams.RotationMatCam1 = FMatrix44f(CamRotationMats[0]);
  PointCloudParams.RotationMatCam2 = FMatrix44f(CamRotationMats[1]);
  PointCloudParams.RotationMatCam3 = FMatrix44f(CamRotationMats[2]);
  PointCloudParams.RotationMatCam4 = FMatrix44f(CamRotationMats[3]);

  IntensityExtension->UpdateParameters(PointCloudParams);

  auto world = this->GetWorld();

  if (!IntensityExtension->bHasUnreadLidarPointCloudData) {
    return false;
  }

  auto pointCloudData = IntensityExtension->LidarPointCloudData;
  PointCloudTime = IntensityExtension->LidarPointCloudTime;
  PointCloudPose = IntensityExtension->LidarPointCloudPose;
  IntensityExtension->bHasUnreadLidarPointCloudData = false;
  auto cameraTransform = DepthSceneCaptures[0]->GetComponentTransform();

  if (pointCloudData.size()) {
    for (int i = 0; i < pointCloudData.size(); i++) {
      auto point = FVector(pointCloudData[i].X, pointCloudData[i].Y,
                           pointCloudData[i].Z);

      if (point == FVector(-1.f, -1.f, -1.f)) {
        continue;
      }

      const projectairsim::Vector3 PointNed =
          UnrealTransform::UnrealToNedLinear(point);
      PointCloud.emplace_back(PointNed.x());
      PointCloud.emplace_back(PointNed.y());
      PointCloud.emplace_back(PointNed.z());
      AzimuthElevationRangeCloud.emplace_back(0.0); //TODO: support azelrange format
      AzimuthElevationRangeCloud.emplace_back(0.0);
      AzimuthElevationRangeCloud.emplace_back(0.0);
      SegmentationCloud.emplace_back(-1);  // TODO: find segmentation id
      IntensityCloud.emplace_back(pointCloudData[i].W);
      LaserIndexCloud.emplace_back(-1); // TODO: find laser index

      if (Settings.draw_debug_points) {
        DrawDebugPoint(world, cameraTransform.TransformPosition(point),
                       10,                   // size
                       FColor(255, 0, 255),  // RGB
                       false,                // persistent (never goes away)
                       0.1                   // time point persists on object
        );
      }
    }
  }

  return true;
}

void UGPULidar::BeginFrameCallback() { bool f = false; }

void UGPULidar::EndFrameCallback() { bool f = false; }

void UGPULidar::BeginFrameCallbackRT() { bool f = false; }

void UGPULidar::EndFrameCallbackRT() {}
