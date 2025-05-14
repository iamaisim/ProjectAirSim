// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// Unreal Lidar Implementation

#include "GPULidar.h"

#include "ProjectAirSim.h"
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
#include "SceneView.h"
#include "Serialization/BufferArchive.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealCameraRenderRequest.h"
#include "UnrealLogger.h"

namespace projectairsim = microsoft::projectairsim;

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

void ExecuteOnRenderThread1(const TFunctionRef<void()>& Function) {
  check(IsInGameThread());

  ENQUEUE_RENDER_COMMAND(ExecuteOnRenderThread1)
  ([Function](FRHICommandListImmediate& /*RHICmdList*/) { Function(); });
  FlushRenderingCommands();
}

void UGPULidar::TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  const TimeNano CurSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  const TimeSec SimTimeDeltaSec =
      projectairsim::SimClock::Get()->NanosToSec(CurSimTime - LastSimTime);

  Simulate(SimTimeDeltaSec);

  const auto LidarTransformStamped = UnrealTransform::GetPoseNed(this);
  projectairsim::Pose LidarPose(LidarTransformStamped.translation_,
                              LidarTransformStamped.rotation_);

  projectairsim::LidarMessage LidarMsg(CurSimTime, PointCloud, AzimuthElevationRangeCloud, SegmentationCloud,
                                     IntensityCloud, LaserIndexCloud, LidarPose);
  Lidar.PublishLidarMsg(LidarMsg);
  LastSimTime = CurSimTime;
}

void UGPULidar::SetupLidarFromSettings(
    const projectairsim::LidarSettings& LidarSettings) {
  Settings = projectairsim::LidarSettings(LidarSettings);

  InitializePose();
  SetUpCams();
}

FMatrix GetProjectionMat(USceneCaptureComponent2D* CaptureComp,
                         float aspectRatio) {
  FMinimalViewInfo ViewInfo;
  ViewInfo.Location = CaptureComp->GetComponentTransform().GetLocation();
  ViewInfo.Rotation =
      CaptureComp->GetComponentTransform().GetRotation().Rotator();
  ViewInfo.FOV = CaptureComp->FOVAngle;
  ViewInfo.ProjectionMode = CaptureComp->ProjectionType;
  ViewInfo.AspectRatio = aspectRatio;
  ViewInfo.OrthoNearClipPlane = GNearClippingPlane;  // need for prespective?
  ViewInfo.OrthoFarClipPlane = 10000;                // TODO
  ViewInfo.bConstrainAspectRatio = true;

  if (CaptureComp->bUseCustomProjectionMatrix == true) {
    return CaptureComp->CustomProjectionMatrix;
  } else {
    return ViewInfo.CalculateProjectionMatrix();
  }
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
  OutSceneCaptureComp->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->LineBatcher));
  OutSceneCaptureComp->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->PersistentLineBatcher));
  OutSceneCaptureComp->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->ForegroundLineBatcher));

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

  NumCams = 1;  // TODO: add support for larger FOVs with more cams.
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

    // Projection matrix for all cams should be same
    FMatrix proj = GetProjectionMat(SceneCapture, aspectRatio);
    ProjectionMat = proj;

    if (i == 0) {
      FIntRect ScreenRect(0, 0, CamFrustrumWidth, CamFrustrumHeight);
      FSceneViewProjectionData ProjectionData;
      ProjectionData.ViewOrigin =
          SceneCapture->GetComponentTransform().GetLocation();
      // Apply rotation matrix of camera's pose plus the rotation matrix for
      // converting Unreal's world axes to the camera's view axes (z forward,
      // etc).
      ProjectionData.ViewRotationMatrix =
          FInverseRotationMatrix(
              SceneCapture->GetComponentTransform().GetRotation().Rotator()) *
          FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0),
                  FPlane(0, 0, 0, 1));
      ProjectionData.ProjectionMatrix = ProjectionMat;
      ProjectionData.SetConstrainedViewRectangle(ScreenRect);
      Cam1ProjData = ProjectionData;
    }

    if (i == 0) {
      cam1loc = SceneCapture->GetComponentTransform().GetLocation();
      cam1Rot = SceneCapture->GetComponentTransform().GetRotation();
    }

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

void UGPULidar::Simulate(const float SimTimeDeltaSec) {
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
    return;
  }

  bool useCPUVersion = false;

  const float AngleDistanceOfTickDeg =
      Settings.horizontal_rotation_frequency * 360.0f * SimTimeDeltaSec;
  // Create parameters & pass data
  FLidarPointCloudCSParameters PointCloudParams;
  PointCloudParams.ProjectionMat = ProjectionMat;
  PointCloudParams.ViewProjectionMatInv =
      FMatrix44f(Cam1ProjData.ComputeViewProjectionMatrix().Inverse());
  PointCloudParams.HorizontalResolution = HorizontalResolution;
  PointCloudParams.LaserNums = Settings.number_of_channels;
  PointCloudParams.LaserRange =
      projectairsim::TransformUtils::ToCentimeters(Settings.range);
  PointCloudParams.HorizontalFOV = AngleDistanceOfTickDeg;
  PointCloudParams.CurrentHorizontalAngleDeg = CurrentHorizontalAngleDeg;
  PointCloudParams.VerticalFOV = Settings.vertical_fov_upper_deg *
                                 2.f;  // assuming symmetrical < 30 for now
  PointCloudParams.CamFrustrumHeight = CamFrustrumHeight;
  PointCloudParams.CamFrustrumWidth = CamFrustrumWidth;
  PointCloudParams.DepthTexture1 =
      DepthSceneCaptures[0]
          ->TextureTarget->GameThread_GetRenderTargetResource()
          ->GetRenderTargetTexture();  // TODO: add rest for 360 fov

  PointCloudParams.RotationMatCam1 = FMatrix44f(CamRotationMats[0]);
  PointCloudParams.RotationMatCam2 = FMatrix44f(CamRotationMats[1]);
  PointCloudParams.RotationMatCam3 = FMatrix44f(CamRotationMats[2]);
  PointCloudParams.RotationMatCam4 = FMatrix44f(CamRotationMats[3]);

  auto RenderTargetResource =
      DepthSceneCaptures[0]
          ->TextureTarget->GameThread_GetRenderTargetResource();

  IntensityExtension->UpdateParameters(PointCloudParams);

  auto world = this->GetWorld();

  static int startIndex = 0;
  static int numPointsToDraw =
      HorizontalResolution *
      Settings.number_of_channels;  // TODO: extend for more cams

  auto pointCloudData = IntensityExtension->LidarPointCloudData;
  auto cameraTransform = DepthSceneCaptures[0]->GetComponentTransform();

  if (pointCloudData.size()) {
    for (int i = startIndex; i < pointCloudData.size(); i++) {
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

  CurrentHorizontalAngleDeg =
      std::fmod(CurrentHorizontalAngleDeg + AngleDistanceOfTickDeg, 360.0f);
}

void UGPULidar::BeginFrameCallback() { bool f = false; }

void UGPULidar::EndFrameCallback() { bool f = false; }

void UGPULidar::BeginFrameCallbackRT() { bool f = false; }

void UGPULidar::EndFrameCallbackRT() {}
