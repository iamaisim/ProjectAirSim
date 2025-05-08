// Copyright (C) Microsoft Corporation.  All rights reserved.
// Unreal Camera Implementation

#include "UnrealCamera.h"

#include <string>
#include <utility>
#include <vector>

#include "Components/LineBatchComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "ImagePackingAsyncTask.h"
#include "ImageUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProjectAirSim.h"
#include "RenderingThread.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealCameraRenderRequest.h"
#include "UnrealHelpers.h"
#include "UnrealLogger.h"
#include "UnrealScene.h"
#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/image_message.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace projectairsim = microsoft::projectairsim;

UUnrealCamera::UUnrealCamera(const FObjectInitializer& ObjectInitializer)
    : UUnrealSensor(ObjectInitializer) {
  bAutoActivate = true;

  // Tick in PostUpdateWork to update after PostPhysics tick where the camera
  // positions are updated for this loop, instead of waiting for the PrePhysics
  // tick on the next loop
  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;

  LoadCameraMaterials();
}

void UUnrealCamera::Initialize(const projectairsim::Camera& InCamera,
                               AUnrealScene* InUnrealScene) {
  SimCamera = InCamera;
  UnrealScene = InUnrealScene;

  CreateComponents();

  SetupCameraFromSettings(InCamera.GetCameraSettings());

  RegisterComponent();
}

void UUnrealCamera::LoadCameraMaterials() {
  static ConstructorHelpers::FObjectFinder<UMaterial> NoiseMat(TEXT(
      "Material'/ProjectAirSim/Sensors/CameraSensorNoise.CameraSensorNoise'"));
  if (NoiseMat.Succeeded()) {
    NoiseMaterialStatic = NoiseMat.Object;
    // NoiseMaterialStatic->BlendableLocation =
    // EBlendableLocation::BL_BeforeTonemapping;
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[UnrealCamera] Cannot create noise material"));
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> DepthPlanarMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "DepthPlannerMaterial.DepthPlannerMaterial'"));
  if (DepthPlanarMat.Succeeded()) {
    DepthPlanarMaterialStatic = DepthPlanarMat.Object;
    // DepthPlannerMaterialStatic->BlendableLocation =
    // EBlendableLocation::BL_BeforeTonemapping;
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealCamera] Cannot create depth planner material"));
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> DepthPerspectiveMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "DepthPerspectiveMaterial.DepthPerspectiveMaterial'"));
  if (DepthPerspectiveMat.Succeeded()) {
    DepthPerspectiveMaterialStatic = DepthPerspectiveMat.Object;
    // DepthPerspectiveMaterialStatic->BlendableLocation =
    // EBlendableLocation::BL_BeforeTonemapping;
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealCamera] Cannot create depth perspective material"));
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> SegmentationMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "SegmentationMaterial.SegmentationMaterial'"));
  if (SegmentationMat.Succeeded()) {
    SegmentationMaterialStatic = SegmentationMat.Object;
    // SegmentationMaterialStatic->BlendableLocation =
    // EBlendableLocation::BL_BeforeTonemapping;
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealCamera] Cannot create segmentation material"));
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> DepthVisMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "DepthVisMaterial.DepthVisMaterial'"));
  if (DepthVisMat.Succeeded()) {
    DepthVisMaterialStatic = DepthVisMat.Object;
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[UnrealCamera] Cannot create depth vis material"));
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> DisparityNormalizedMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "DisparityMaterial.DisparityMaterial'"));
  if (DisparityNormalizedMat.Succeeded()) {
    DisparityNormalizedMaterialStatic = DisparityNormalizedMat.Object;
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealCamera] Cannot create disparity normalized material"));
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> SurfaceNormalsMat(
      TEXT("Material'/ProjectAirSim/Sensors/"
           "NormalsMaterial.NormalsMaterial'"));
  if (SurfaceNormalsMat.Succeeded()) {
    SurfaceNormalsMaterialStatic = SurfaceNormalsMat.Object;
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealCamera] Cannot create surface normals material"));
  }
}

void HideDebugDrawComponent(USceneCaptureComponent2D* CaptureComponent,
                            UWorld* UnrealWorld) {
  CaptureComponent->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->LineBatcher));
  CaptureComponent->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->PersistentLineBatcher));
  CaptureComponent->HideComponent(
      Cast<UPrimitiveComponent>(UnrealWorld->ForegroundLineBatcher));
}

void UUnrealCamera::CreateComponents() {
  Captures.Init(nullptr, ImageTypeCount());
  RenderTargets.Init(nullptr, ImageTypeCount());

  // create camera component
  CameraComponent =
      NewObject<UCineCameraComponent>(this, TEXT("SimCameraComponent"));
  CameraComponent->SetupAttachment(this);
  CameraComponent->bAutoActivate = false;
  CameraComponent->RegisterComponent();

  // Note: Need to set bAlwaysPersistRenderingState to true to enable
  // post-processing blendable materials when doing manual scene captures with
  // bCaptureEveryFrame set to false. (See
  // UComposureLensBloomPass::BloomToRenderTarget() for reference.)

  USceneCaptureComponent2D* SceneCaptureComponent =
      NewObject<USceneCaptureComponent2D>(this, TEXT("SceneCaptureComponent"));
  SceneCaptureComponent->SetupAttachment(this);
  SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  SceneCaptureComponent->bAutoActivate = false;
  SceneCaptureComponent->bCaptureEveryFrame = false;
  SceneCaptureComponent->bCaptureOnMovement = false;
  SceneCaptureComponent->bAlwaysPersistRenderingState = true;
  SceneCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kScene)] = SceneCaptureComponent;

  USceneCaptureComponent2D* DepthPlanarCaptureComponent =
      NewObject<USceneCaptureComponent2D>(this,
                                          TEXT("DepthPlanarCaptureComponent"));
  DepthPlanarCaptureComponent->SetupAttachment(this);
  DepthPlanarCaptureComponent->CaptureSource =
      ESceneCaptureSource::SCS_FinalColorLDR;
  DepthPlanarCaptureComponent->bAutoActivate = false;
  DepthPlanarCaptureComponent->bCaptureEveryFrame = false;
  DepthPlanarCaptureComponent->bCaptureOnMovement = false;
  DepthPlanarCaptureComponent->bAlwaysPersistRenderingState = true;
  DepthPlanarCaptureComponent->AddOrUpdateBlendable(DepthPlanarMaterialStatic,
                                                    1.0);
  DepthPlanarCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kDepthPlanar)] = DepthPlanarCaptureComponent;

  USceneCaptureComponent2D* DepthPerspectiveCaptureComponent =
      NewObject<USceneCaptureComponent2D>(
          this, TEXT("DepthPerspectiveCaptureComponent"));
  DepthPerspectiveCaptureComponent->SetupAttachment(this);
  DepthPerspectiveCaptureComponent->CaptureSource =
      ESceneCaptureSource::SCS_FinalColorLDR;
  DepthPerspectiveCaptureComponent->bAutoActivate = false;
  DepthPerspectiveCaptureComponent->bCaptureEveryFrame = false;
  DepthPerspectiveCaptureComponent->bCaptureOnMovement = false;
  DepthPerspectiveCaptureComponent->bAlwaysPersistRenderingState = true;
  DepthPerspectiveCaptureComponent->AddOrUpdateBlendable(
      DepthPerspectiveMaterialStatic, 1.0);
  DepthPerspectiveCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kDepthPerspective)] =
      DepthPerspectiveCaptureComponent;

  USceneCaptureComponent2D* SegmentationCaptureComponent =
      NewObject<USceneCaptureComponent2D>(this,
                                          TEXT("SegmentationCaptureComponent"));
  SegmentationCaptureComponent->SetupAttachment(this);
  SegmentationCaptureComponent->CaptureSource =
      ESceneCaptureSource::SCS_FinalColorLDR;
  SegmentationCaptureComponent->bAutoActivate = false;
  SegmentationCaptureComponent->bCaptureEveryFrame = false;
  SegmentationCaptureComponent->bCaptureOnMovement = false;
  SegmentationCaptureComponent->bAlwaysPersistRenderingState = true;
  SegmentationCaptureComponent->AddOrUpdateBlendable(SegmentationMaterialStatic,
                                                     1.0);
  SegmentationCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kSegmentation)] = SegmentationCaptureComponent;

  USceneCaptureComponent2D* DepthVisCaptureComponent =
      NewObject<USceneCaptureComponent2D>(this,
                                          TEXT("DepthVisCaptureComponent"));
  DepthVisCaptureComponent->SetupAttachment(this);
  DepthVisCaptureComponent->CaptureSource =
      ESceneCaptureSource::SCS_FinalColorLDR;
  DepthVisCaptureComponent->bAutoActivate = false;
  DepthVisCaptureComponent->bCaptureEveryFrame = false;
  DepthVisCaptureComponent->bCaptureOnMovement = false;
  DepthVisCaptureComponent->bAlwaysPersistRenderingState = true;
  DepthVisCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kDepthVis)] = DepthVisCaptureComponent;

  USceneCaptureComponent2D* DisparityNormalizedCaptureComponent =
      NewObject<USceneCaptureComponent2D>(
          this, TEXT("DisparityNormalizedCaptureComponent"));
  DisparityNormalizedCaptureComponent->SetupAttachment(this);
  DisparityNormalizedCaptureComponent->CaptureSource =
      ESceneCaptureSource::SCS_FinalColorLDR;
  DisparityNormalizedCaptureComponent->bAutoActivate = false;
  DisparityNormalizedCaptureComponent->bCaptureEveryFrame = false;
  DisparityNormalizedCaptureComponent->bCaptureOnMovement = false;
  DisparityNormalizedCaptureComponent->bAlwaysPersistRenderingState = true;
  DisparityNormalizedCaptureComponent->AddOrUpdateBlendable(
      DisparityNormalizedMaterialStatic, 1.0);
  DisparityNormalizedCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kDisparityNormalized)] =
      DisparityNormalizedCaptureComponent;

  USceneCaptureComponent2D* SurfaceNormalsCaptureComponent =
      NewObject<USceneCaptureComponent2D>(
          this, TEXT("SurfaceNormalsCaptureComponent"));
  SurfaceNormalsCaptureComponent->SetupAttachment(this);
  SurfaceNormalsCaptureComponent->CaptureSource =
      ESceneCaptureSource::SCS_FinalColorLDR;
  SurfaceNormalsCaptureComponent->bAutoActivate = false;
  SurfaceNormalsCaptureComponent->bCaptureEveryFrame = false;
  SurfaceNormalsCaptureComponent->bCaptureOnMovement = false;
  SurfaceNormalsCaptureComponent->bAlwaysPersistRenderingState = true;
  SurfaceNormalsCaptureComponent->AddOrUpdateBlendable(
      SurfaceNormalsMaterialStatic, 1.0);
  SurfaceNormalsCaptureComponent->RegisterComponent();
  Captures[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kSurfaceNormals)] =
      SurfaceNormalsCaptureComponent;
}

int UUnrealCamera::ImageTypeCount() {
  return projectairsim::MathUtils::ToNumeric(projectairsim::ImageType::kCount);
}

void UUnrealCamera::SetupCameraFromSettings(
    const projectairsim::CameraSettings& CameraSetting) {
  SetRelativePoseFromNed(CameraSetting.origin_setting);

  NoiseMaterials.AddZeroed(ImageTypeCount());
  // by default all image types are disabled
  CameraTypeEnabled.assign(ImageTypeCount(), false);
  FrustumData.reserve(ImageTypeCount());

  StreamingCaptures.Empty();
  StreamingCaptureActiveIdx = 0;

  for (int ImageType = 0; ImageType < ImageTypeCount(); ++ImageType) {
    auto& Capture = Captures[ImageType];
    RenderTargets[ImageType] = NewObject<UTextureRenderTarget2D>();
    const auto& CaptureSetting = CameraSetting.capture_settings.at(ImageType);
    const auto& NoiseSetting = CameraSetting.noise_settings.at(ImageType);

    if (CaptureSetting.streaming_enabled) {
      StreamingCaptures.Add(Capture);
    }

    UpdateCaptureComponentSetting(Capture, RenderTargets[ImageType],
                                  CaptureSetting, UnrealWorld);

    SetNoiseMaterial(ImageType, Capture, Capture->PostProcessSettings,
                     NoiseSetting);

    if (ImageType == projectairsim::MathUtils::ToNumeric(
                         projectairsim::ImageType::kDepthVis)) {
      SetDepthVisMaterial(Capture, Capture->PostProcessSettings,
                          CaptureSetting);
    }

    Capture->TextureTarget = RenderTargets[ImageType];
  }
  UpdateCameraSettings();
}

void UUnrealCamera::SetRelativePoseFromNed(
    const projectairsim::Transform& PoseNed) {
  // Apply camera origin offset from the parent (either the parent link or
  // the spring arm attached at the parent link if gimbal rotation
  // stabilization is configured)
  FTransform DesiredPose = UnrealTransform::FromGlobalNed(PoseNed);
  this->SetRelativeTransform(DesiredPose);
}

void UUnrealCamera::UpdateCameraSettings() {
  auto CameraSettings = SimCamera.GetCameraSettings();
  CameraComponent->CurrentAperture = CameraSettings.aperture;
  // TODO: in the future we can update filmback and other lens settings here too
  CopyCameraSettingsToAllSceneCaptures(CameraSettings);
  CalculateProjectionMatrix();

  // Compute vertices for each clipping plane centered at the origin.
  // This makes rotating it and subsequently translating it easier.
  ComputeFrustumVertices();

  // change to only update captures
}

void UUnrealCamera::CopyCameraSettingsToSceneCapture(
    UCameraComponent* SourceCamComponent,
    USceneCaptureComponent2D* TargetCaptureComponent,
    const projectairsim::CaptureSettings& CaptureSetting) {
  if (SourceCamComponent && TargetCaptureComponent) {
    TargetCaptureComponent->FOVAngle = CaptureSetting.fov_degrees;

    FMinimalViewInfo ViewInfo;
    SourceCamComponent->GetCameraView(/*DeltaTime =*/0.0f, ViewInfo);

    const FPostProcessSettings& SourcePPSettings = ViewInfo.PostProcessSettings;
    FPostProcessSettings& TargetPPSettings =
        TargetCaptureComponent->PostProcessSettings;

    FWeightedBlendables TargetWeightedBlendables =
        TargetPPSettings.WeightedBlendables;

    // Copy all of the post processing settings
    TargetPPSettings = SourcePPSettings;

    UpdateCameraPostProcessingSetting(TargetPPSettings, CaptureSetting);

    //// But restore the original blendables
    TargetPPSettings.WeightedBlendables = TargetWeightedBlendables;
  }
}

void UUnrealCamera::CopyCameraSettingsToAllSceneCaptures(
    const projectairsim::CameraSettings& CameraSetting) {
  int image_count = static_cast<int>(
      projectairsim::MathUtils::ToNumeric(projectairsim::ImageType::kCount));
  for (int image_type = 0; image_type < image_count; ++image_type) {
    CopyCameraSettingsToSceneCapture(
        CameraComponent, Captures[image_type],
        CameraSetting.capture_settings.at(image_type));
  }
}

void UUnrealCamera::UpdateCaptureComponentSetting(
    USceneCaptureComponent2D* Capture, UTextureRenderTarget2D* RenderTarget,
    const projectairsim::CaptureSettings& CaptureSettings,
    UWorld* UnrealWorld) {
  if (!CaptureSettings.show_debug_plots)
    HideDebugDrawComponent(Capture, UnrealWorld);
  if (CaptureSettings.pixels_as_float ||
      CaptureSettings.image_type ==
          projectairsim::MathUtils::ToNumeric(
              projectairsim::ImageType::kDepthPerspective) ||
      CaptureSettings.image_type ==
          projectairsim::MathUtils::ToNumeric(
              projectairsim::ImageType::kDepthPlanar)) {
    // TODO Depth camera is set to use `PF_FloatRGBA` to have smooth depth
    // gradients when converted back to uint8 depth values in meters (R, G, and
    // B pixel values are each set to the same depth value to display as
    // grayscale image), but this conversion is very slow. In the future, depth
    // camera could be rendered as 16UC1 data using the B and G uint8 bytes of a
    // `PF_B8G8R8A8` render target to save the distance in centimeters, and then
    // the client side would unpack and convert these bytes into a float depth
    // in meters to avoid the conversion from a PF_FloatRGBA render target.
    RenderTarget->InitCustomFormat(CaptureSettings.width,
                                   CaptureSettings.height,
                                   EPixelFormat::PF_FloatRGBA, true);
  } else {
    RenderTarget->InitCustomFormat(CaptureSettings.width,
                                   CaptureSettings.height,
                                   EPixelFormat::PF_B8G8R8A8, true);
  }

  if (!std::isnan(CaptureSettings.target_gamma))
    RenderTarget->TargetGamma = CaptureSettings.target_gamma;

  Capture->ProjectionType =
      static_cast<ECameraProjectionMode::Type>(CaptureSettings.projection_mode);

  if (!std::isnan(CaptureSettings.fov_degrees))
    Capture->FOVAngle = CaptureSettings.fov_degrees;
  // TODO: projection mode
  // if (Capture->ProjectionType == ECameraProjectionMode::Orthographic &&
  // !std::isnan(CaptureSettings.ortho_Width))
  //    Capture->OrthoWidth =
  //    ned_transform.fromNed(CaptureSettings.ortho_Width);

  UpdateCameraPostProcessingSetting(Capture->PostProcessSettings,
                                    CaptureSettings);
}

void UUnrealCamera::UpdateCameraPostProcessingSetting(
    FPostProcessSettings& PostProcessSettings,
    const projectairsim::CaptureSettings& CaptureSettings) {
  if (CaptureSettings.auto_exposure_method >= 0) {
    PostProcessSettings.bOverride_AutoExposureMethod = true;
    PostProcessSettings.AutoExposureMethod =
        projectairsim::MathUtils::ToEnum<EAutoExposureMethod>(
            CaptureSettings.auto_exposure_method);
  }

  if (!std::isnan(CaptureSettings.auto_exposure_speed)) {
    PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
    PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
    PostProcessSettings.AutoExposureSpeedDown =
        PostProcessSettings.AutoExposureSpeedUp =
            CaptureSettings.auto_exposure_speed;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_bias)) {
    PostProcessSettings.bOverride_AutoExposureBias = true;
    PostProcessSettings.AutoExposureBias = CaptureSettings.auto_exposure_bias;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_max_brightness)) {
    PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    PostProcessSettings.AutoExposureMaxBrightness =
        CaptureSettings.auto_exposure_max_brightness;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_min_brightness)) {
    PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    PostProcessSettings.AutoExposureMinBrightness =
        CaptureSettings.auto_exposure_min_brightness;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_low_percent)) {
    PostProcessSettings.AutoExposureLowPercent =
        CaptureSettings.auto_exposure_low_percent;
    PostProcessSettings.bOverride_AutoExposureLowPercent = true;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_high_percent)) {
    PostProcessSettings.AutoExposureHighPercent =
        CaptureSettings.auto_exposure_high_percent;
    PostProcessSettings.bOverride_AutoExposureHighPercent = true;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_histogram_log_min)) {
    PostProcessSettings.bOverride_HistogramLogMin = true;
    PostProcessSettings.HistogramLogMin =
        CaptureSettings.auto_exposure_histogram_log_min;
  }

  if (!std::isnan(CaptureSettings.auto_exposure_histogram_log_max)) {
    PostProcessSettings.bOverride_HistogramLogMax = true;
    PostProcessSettings.HistogramLogMax =
        CaptureSettings.auto_exposure_histogram_log_max;
  }

  if (!std::isnan(CaptureSettings.motion_blur_amount)) {
    PostProcessSettings.bOverride_MotionBlurAmount = true;
    PostProcessSettings.MotionBlurAmount = CaptureSettings.motion_blur_amount;
  }

  if (!std::isnan(CaptureSettings.chromatic_aberration_intensity)) {
    PostProcessSettings.bOverride_SceneFringeIntensity = true;
    PostProcessSettings.SceneFringeIntensity =
        CaptureSettings.chromatic_aberration_intensity;
  }

  // We use Depth of Field where things that are too far are out of focus.
  // There is a different Mobile Depth of Field Region which seems to
  // have a region that completely in focus. Maybe we should expose that?
  if (!std::isnan(CaptureSettings.depth_of_field_focal_meters)) {
    PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
    PostProcessSettings.DepthOfFieldFocalDistance =
        CaptureSettings.depth_of_field_focal_meters * 100.0;

    // TODO: expose this too? (seems confusing)
    // PostProcessSettings.bOverride_DepthOfFieldFocalRegion= true;
    // PostProcessSettings.DepthOfFieldFocalRegion =
    //     CaptureSettings.depth_of_field_focal_meters * 100.0;
    if (!std::isnan(CaptureSettings.depth_of_field_transition_region)) {
      PostProcessSettings.bOverride_DepthOfFieldNearTransitionRegion = true;
      PostProcessSettings.bOverride_DepthOfFieldFarTransitionRegion = true;
      PostProcessSettings.DepthOfFieldNearTransitionRegion =
          CaptureSettings.depth_of_field_transition_region * 100.0;
      PostProcessSettings.DepthOfFieldFarTransitionRegion =
          CaptureSettings.depth_of_field_transition_region * 100.0;
    }
  }
}

void UUnrealCamera::SetNoiseMaterial(
    int ImageType, UObject* CaptureComponent,
    FPostProcessSettings& PostProcessSettings,
    const projectairsim::NoiseSettings& NoiseSettings) {
  if (!NoiseSettings.Enabled) return;

  UMaterialInstanceDynamic* NoiseMaterial =
      UMaterialInstanceDynamic::Create(NoiseMaterialStatic, CaptureComponent);
  NoiseMaterials[ImageType] = NoiseMaterial;

  NoiseMaterial->SetScalarParameterValue("HorzWaveStrength",
                                         NoiseSettings.HorzWaveStrength);
  NoiseMaterial->SetScalarParameterValue("RandSpeed", NoiseSettings.RandSpeed);
  NoiseMaterial->SetScalarParameterValue("RandSize", NoiseSettings.RandSize);
  NoiseMaterial->SetScalarParameterValue("RandDensity",
                                         NoiseSettings.RandDensity);
  NoiseMaterial->SetScalarParameterValue("RandContrib",
                                         NoiseSettings.RandContrib);
  NoiseMaterial->SetScalarParameterValue("HorzWaveContrib",
                                         NoiseSettings.HorzWaveContrib);
  NoiseMaterial->SetScalarParameterValue("HorzWaveVertSize",
                                         NoiseSettings.HorzWaveVertSize);
  NoiseMaterial->SetScalarParameterValue("HorzWaveScreenSize",
                                         NoiseSettings.HorzWaveScreenSize);
  NoiseMaterial->SetScalarParameterValue("HorzNoiseLinesContrib",
                                         NoiseSettings.HorzNoiseLinesContrib);
  NoiseMaterial->SetScalarParameterValue("HorzNoiseLinesDensityY",
                                         NoiseSettings.HorzNoiseLinesDensityY);
  NoiseMaterial->SetScalarParameterValue("HorzNoiseLinesDensityXY",
                                         NoiseSettings.HorzNoiseLinesDensityXY);
  NoiseMaterial->SetScalarParameterValue("HorzDistortionStrength",
                                         NoiseSettings.HorzDistortionStrength);
  NoiseMaterial->SetScalarParameterValue("HorzDistortionContrib",
                                         NoiseSettings.HorzDistortionContrib);
  // TODO: is it possible to remove/disable noise after being enabled?
  // If so, we should expose this as a toggle.
  PostProcessSettings.AddBlendable(NoiseMaterial, 1.0f);
}

void UUnrealCamera::SetDepthVisMaterial(
    UObject* CaptureComponent, FPostProcessSettings& PostProcessSettings,
    const projectairsim::CaptureSettings& CaptureSettings) {
  UMaterialInstanceDynamic* DepthVisMaterial = UMaterialInstanceDynamic::Create(
      DepthVisMaterialStatic, CaptureComponent);

  DepthVisMaterial->SetScalarParameterValue("MaxDepthMeters",
                                            CaptureSettings.max_depth_meters);

  PostProcessSettings.AddBlendable(DepthVisMaterial, 1.0f);
}

UTextureRenderTarget2D* UUnrealCamera::GetRenderTarget(
    const projectairsim::ImageType Type, bool bIfActive) {
  int ImageType = projectairsim::MathUtils::ToNumeric(Type);

  return (!bIfActive || CameraTypeEnabled.at(ImageType))
             ? RenderTargets[ImageType]
             : nullptr;
}

USceneCaptureComponent2D* UUnrealCamera::GetCaptureComponent(
    const projectairsim::ImageType Type, bool bIfActive) {
  int ImageTypeInt = projectairsim::MathUtils::ToNumeric(Type);

  return (!bIfActive || CameraTypeEnabled.at(ImageTypeInt))
             ? Captures[ImageTypeInt]
             : nullptr;
}

USceneCaptureComponent2D* UUnrealCamera::GetActiveStreamingCapture() {
  if (StreamingCaptures.Num() == 0) return nullptr;

  return StreamingCaptures[StreamingCaptureActiveIdx];
}

// Increments and returns the active streaming capture index, wrapping back to 0
// after reaching the end. Returns -1 if there is no valid streaming capture.
int32 UUnrealCamera::GetNextStreamingCaptureIdx() {
  if (StreamingCaptures.Num() == 0) return -1;

  // Increment index to cycle through the active streaming captures
  StreamingCaptureActiveIdx =
      (StreamingCaptureActiveIdx + 1) % StreamingCaptures.Num();

  return StreamingCaptureActiveIdx;
}

bool UUnrealCamera::GetCameraTypeEnabled(projectairsim::ImageType Type) const {
  return CameraTypeEnabled[projectairsim::MathUtils::ToNumeric(Type)];
}

bool UUnrealCamera::GetCameraTypeEnabled(const int typeInt) const {
  return CameraTypeEnabled[typeInt];
}

void UUnrealCamera::SetCameraTypeEnabled(projectairsim::ImageType Type,
                                         bool bIsEnabled) {
  USceneCaptureComponent2D* Capture = GetCaptureComponent(Type, false);
  if (Capture != nullptr) {
    if (bIsEnabled) {
      // do not make unnecessary calls to Activate() which otherwise causes
      // crash in Unreal
      if (!Capture->IsActive() || Capture->TextureTarget == nullptr) {
        Capture->Activate();
      }
    } else {
      if (Capture->IsActive() || Capture->TextureTarget != nullptr) {
        Capture->Deactivate();
      }
    }
    CameraTypeEnabled[projectairsim::MathUtils::ToNumeric(Type)] = bIsEnabled;
  }
  // else nothing to enable
}

void UUnrealCamera::BeginPlay() {
  Super::BeginPlay();

  projectairsim::Transform BeginPlayPose = UnrealTransform::GetPoseNed(this);
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[UnrealCamera] Camera '%S': BeginPlay(). Location (%f,%f,%f)"),
      SimCamera.GetId().c_str(), BeginPlayPose.translation_.x(),
      BeginPlayPose.translation_.y(),
      BeginPlayPose.translation_.z());  // NED

  UnrealCameraRenderRequest::OnBeginPlay();

  auto& CameraSetting = SimCamera.GetCameraSettings();
  this->CaptureIntervalSec = CameraSetting.capture_interval;
  bool bAnyCaptureEnabled = false;

  for (int ImageTypeInt = 0; ImageTypeInt < ImageTypeCount(); ++ImageTypeInt) {
    projectairsim::ImageType ImageTypeEnum =
        projectairsim::MathUtils::ToEnum<projectairsim::ImageType>(
            ImageTypeInt);
    const auto& CaptureSetting =
        CameraSetting.capture_settings.at(ImageTypeInt);

    if (CaptureSetting.capture_enabled) {
      CaptureRequests.push_back(ImageRequest(SimCamera.GetId(), ImageTypeEnum,
                                             CaptureSetting.pixels_as_float,
                                             CaptureSetting.compress));
      SetCameraTypeEnabled(ImageTypeEnum, true);
      bAnyCaptureEnabled = true;

      if (ImageTypeEnum == projectairsim::ImageType::kScene) {
        CalculateProjectionMatrix();  // precalc for bbox generation
      }
    } else {
      SetCameraTypeEnabled(ImageTypeEnum, false);
    }
  }

  UpdateCameraSettings();
}

void UUnrealCamera::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  std::string ObjectName = SimCamera.GetLookAtObject();
  if (!ObjectName.empty()) {
    AActor* Object = UnrealScene->FindActor(ObjectName);
    if (Object) {
      FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
          this->GetComponentTransform().GetLocation(),
          Object->GetActorLocation());
      this->SetWorldRotation(LookAtRotation);
    } else {
      UnrealLogger::Log(
          projectairsim::LogLevel::kWarning,
          TEXT("[UnrealCamera] Cannot look at '%hs' because it is "
               "not in the scene!"),
          ObjectName.c_str());
      SimCamera.ResetCameraPose(false);
    }
    SimCamera.MarkPoseUpdateAsCompleted();
  } else if (SimCamera.IsPoseUpdatePending()) {
    auto NewPose = SimCamera.GetDesiredPose();
    SetRelativePoseFromNed(NewPose);
    SimCamera.MarkPoseUpdateAsCompleted();
  }

  if (SimCamera.IsSettingsUpdatePending()) {
    UpdateCameraSettings();
    SimCamera.MarkSettingsUpdateAsCompleted();
  }

  // -----------------------------------------------------------------------
  // Start a new set of image captures if ready to take the next one.

  const TimeSec SimTimeElapsedSec = projectairsim::SimClock::Get()->NanosToSec(
      PoseUpdatedTimeStamp - CapturedTime);

  if (SimTimeElapsedSec > CaptureIntervalSec && bHasNewState &&
      SimCamera.IsUpdating() && !SimCamera.IsCaptureQueueFull()) {
    bool bCaptureIsActive = false;

    for (int ImageTypeInt = 0; ImageTypeInt < ImageTypeCount();
         ++ImageTypeInt) {
      // Only capture the scene for the image type if someone wants it
      bool bEnable = SimCamera.HasSubscribers(ImageTypeInt);
      // Initialize the capture component if it hasn't been enabled yet
      USceneCaptureComponent2D* Capture;

      if (bEnable != GetCameraTypeEnabled(ImageTypeInt)) {
        // The desired capture state and capture component state differ--
        // sync the component
        SetCameraTypeEnabled(microsoft::projectairsim::MathUtils::ToEnum<
                                 projectairsim::ImageType>(ImageTypeInt),
                             bEnable);

        // When the capture component is just activated, CaptureSceneDeferred()
        // doesn't seem to capture an image until the next tick so we'll
        // manually capture the scene now
        if (bEnable) {
          Capture = GetCaptureComponent(
              static_cast<projectairsim::ImageType>(ImageTypeInt), false);
          Capture->CaptureScene();
        }
      }

      if (bEnable) {
        Capture = GetCaptureComponent(
            static_cast<projectairsim::ImageType>(ImageTypeInt), true);
        Capture->CaptureSceneDeferred();
        bCaptureIsActive = true;
      }
    }

    if (bCaptureIsActive) {
      CapturedTime = PoseUpdatedTimeStamp;
      bCapturePending = true;
      bHasNewState = false;

      // Add time stamp to queue of capture requests in sim camera for later
      // publishing the results in the correct order since the image result
      // processing is done asynchronously
      SimCamera.AddRequestToCaptureQueue(CapturedTime);

      // Save the current sim time and pose to go with this rendered image data
      projectairsim::Transform CapturedCameraTransform =
          UnrealTransform::GetPoseNed(this);
      CapturedCameraTransform.timestamp_ = PoseUpdatedTimeStamp;

      auto AnnotationSettings =
          SimCamera.GetCameraSettings().annotation_settings;
      auto Annotations = std::vector<projectairsim::Annotation>();
      if (AnnotationSettings.enabled) {
        GetBoundingBoxes(AnnotationSettings, Annotations);
      }

      // Lambda capture a copy of CapturedCameraTransform to pass to callback.
      // When this render command completes, UUnrealCamera::OnRendered() will be
      // triggered to process and publish the image data
      ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)
      ([this, CapturedCameraTransform, Annotations = std::move(Annotations)](
           FRHICommandListImmediate& RHICmdList) {
        this->OnRendered(CapturedCameraTransform, Annotations);
      });
    }
  }

  // Draw camera frustum if enabled
  if (SimCamera.GetFrustumEnabled()) DrawViewFrustum();
}

void UUnrealCamera::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  if (EEndPlayReason::Type::EndPlayInEditor == EndPlayReason ||
      EEndPlayReason::Type::Quit == EndPlayReason) {
    // This will block game thread and physics engine!!!
    while (bCapturePending) {
      FPlatformProcess::Sleep(0.1f);
    }
  } else {
    // TODO - validate GarbageCollection honors IsReadyForFinishDestroy in all
    // other cases
  }

  Super::EndPlay(EndPlayReason);
  UnrealCameraRenderRequest::OnEndPlay();
}

bool UUnrealCamera::IsReadyForFinishDestroy() {
  // tell GC to retry later if the object is still being used by rendering
  // thread
  return !bCapturePending;
}

// static void WritePfmFile(const float* const image_data, int width, int
// height,
//                          std::string path, float scalef = 1.0f) {
//   std::fstream file(path.c_str(), std::ios::out | std::ios::binary);

//   std::string bands;
//   float fvalue;  // scale factor and temp value to hold pixel value
//   bands = "Pf";  // grayscale

//   // sign of scalefact indicates endianness, see pfm specs
//   // if (isLittleEndian())
//   int intval = 1;
//   unsigned char* uval = reinterpret_cast<unsigned char*>(&intval);
//   if (uval[0] == 1) scalef = -scalef;

//   // insert header information
//   file << bands << "\n";
//   file << width << " ";
//   file << height << "\n";
//   file << scalef << "\n";

//   if (bands == "Pf") {  // handle 1-band image
//     for (int i = 0; i < height; i++) {
//       for (int j = 0; j < width; ++j) {
//         fvalue = image_data[i * width + j];
//         file.write(reinterpret_cast<char*>(&fvalue), sizeof(fvalue));
//       }
//     }
//   }
// }

// static void WriteToFile(const std::vector<ImageResponse>& responses) {
//   bool save_success = false;
//   std::stringstream image_file_names;

//   for (auto i = 0; i < responses.size(); ++i) {
//     const auto& response = responses.at(i);

//     // build image file name
//     // Note: If universal timestamps based on 1970 Linux epoch are needed for
//     // filestamps, this GetTimeSinceEpochNanos() may need to switch to a
//     // separate chrono::system_clock method since sim switched to
//     // steady_clock that references epoch from computer boot.
//     // TODO should sim timestamp also be included in filename for
//     tracability? std::stringstream image_file_name; image_file_name
//         << "img_" << response.CameraName << "_"
//         << projectairsim::MathUtils::ToNumeric(response.ImageType) << "_"
//         << projectairsim::SimClock::Get()->GetRealTimeSinceEpochNanos()
//         << (response.bPixelsAsFloat ? ".pfm" : ".png");

//     if (i > 0) image_file_names << ";";
//     image_file_names << image_file_name.str();
//     // std::string image_full_file_path =
//     // utils::FileSystem::combine(image_path_, image_file_name.str());
//     std::string image_full_file_path =
//         "c:\\temp\\sim\\images\\" + image_file_name.str();

//     // write image file
//     try {
//       if (response.bPixelsAsFloat) {
//         WritePfmFile(response.ImageDataFloat.data(), response.Width,
//                      response.Height, image_full_file_path);
//       } else {
//         std::ofstream file(image_full_file_path, std::ios::binary);
//         file.write(
//             reinterpret_cast<const char*>(response.ImageDataUInt8.data()),
//             response.ImageDataUInt8.size());
//         file.close();
//       }

//       save_success = true;
//     } catch (std::exception&) {
//       save_success = false;
//     } catch (...) {
//       save_success = false;
//     }
//   }
// }

void UUnrealCamera::OnRendered(
    projectairsim::Transform CapturedCameraTransform,
    std::vector<projectairsim::Annotation> Annotations) {
  std::vector<std::pair<ImageRequest, UnrealCameraRenderRequest::RenderResult>>
      CaptureResults;

  FString RHIName = GDynamicRHI->GetName();
  if (RHIName != TEXT("NULL")) {  // skip pixel reads if running with '-nullrhi'
    // Process each image type requested for this captured frame
    for (const auto& ImageRequest : CaptureRequests) {
      USceneCaptureComponent2D* CaptureComponent =
          GetCaptureComponent(ImageRequest.ImageType, true);

      if (CaptureComponent == nullptr) {
        // Image type is capture-enabled but is not currently active (may not
        // have any subscribers, for instance.)
        continue;
      } else if (CaptureComponent->TextureTarget == nullptr) {
        UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                          TEXT("[UnrealCamera] Can't take screenshot because "
                               "texture target is null."));
      } else {
        FTextureRenderTargetResource* RtResource =
            CaptureComponent->TextureTarget->GetRenderTargetResource();
        if (RtResource) {
          FRHITexture2D* Texture = RtResource->GetRenderTargetTexture();
          // Copy pixel values out of Unreal's RHI
          UnrealCameraRenderRequest::RenderResult ImageResult;
          auto bIsDepthImage =
              (ImageRequest.ImageType ==
                   projectairsim::ImageType::kDepthPerspective ||
               ImageRequest.ImageType ==
                   projectairsim::ImageType::kDepthPlanar);

          auto bPixelsAsFloatRequired =
              ImageRequest.bPixelsAsFloat || bIsDepthImage;

          UnrealCameraRenderRequest::ReadPixels(Texture, bPixelsAsFloatRequired,
                                                &ImageResult);

#if TEST_DEBUG_IMAGE_INTEGRITY
          // If testing for image pixel integrity through the capture pipeline
          // to the client, overwrite all pixels in each image to a debug
          // counter value to check that every image has all matching pixels
          // when received at the client.
          for (auto& Pixel : ImageResult.UnrealImage) {
            Pixel.B = DebugImagePixelValue;
            Pixel.G = DebugImagePixelValue;
            Pixel.R = DebugImagePixelValue;
          }
          DebugImagePixelValue = (DebugImagePixelValue + 1) % 255;
#endif  // TEST_DEBUG_IMAGE_INTEGRITY

          CaptureResults.emplace_back(ImageRequest, std::move(ImageResult));
        } else {
          UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                            TEXT("[UnrealCamera] Could not get Render Target "
                                 "Resource to read the image pixels."));
        }
      }
    }
  }

  // Offload image message packing and publishing to a background task.
  // This passes a copy of the sim Camera, which takes a copy of the
  // shared_ptr to the sensor impl to use for publishing the image
  // message.
  (new FAutoDeleteAsyncTask<FImagePackingAsyncTask>(
          std::move(CaptureResults), CapturedCameraTransform, SimCamera,
          std::move(Annotations)))
          ->StartBackgroundTask();

  // Reset flag to allow next frame capture to start
  bCapturePending = false;
}

void UUnrealCamera::DrawViewFrustum() {
  int CaptureType = SimCamera.GetFrustumCaptureType();
  USceneCaptureComponent2D* CaptureComp = Captures[CaptureType];

  FVector CamLocation = CaptureComp->GetComponentTransform().GetLocation();
  FRotator CamRotation =
      CaptureComp->GetComponentTransform().GetRotation().Rotator();

  // Rotate the forward vector that represents how far the box is ahead of the
  // camera
  FVector DistVecFar =
      CamRotation.RotateVector(FVector(FrustumData[CaptureType].DistFar, 0, 0));
  FVector DistVecNear = CamRotation.RotateVector(
      FVector(FrustumData[CaptureType].DistNear, 0, 0));

  // Rotate the clipping planes about the origin then translate back to camera
  // location
  FVector UpperRightFar =
      CamRotation.RotateVector(FrustumData[CaptureType].UpperRightFar) +
      CamLocation + DistVecFar;
  FVector BottomRightFar =
      CamRotation.RotateVector(FrustumData[CaptureType].BottomRightFar) +
      CamLocation + DistVecFar;
  FVector UpperLeftFar =
      CamRotation.RotateVector(FrustumData[CaptureType].UpperLeftFar) +
      CamLocation + DistVecFar;
  FVector BottomLeftFar =
      CamRotation.RotateVector(FrustumData[CaptureType].BottomLeftFar) +
      CamLocation + DistVecFar;

  FVector UpperRightNear =
      CamRotation.RotateVector(FrustumData[CaptureType].UpperRightNear) +
      CamLocation + DistVecNear;
  FVector BottomRightNear =
      CamRotation.RotateVector(FrustumData[CaptureType].BottomRightNear) +
      CamLocation + DistVecNear;
  FVector UpperLeftNear =
      CamRotation.RotateVector(FrustumData[CaptureType].UpperLeftNear) +
      CamLocation + DistVecNear;
  FVector BottomLeftNear =
      CamRotation.RotateVector(FrustumData[CaptureType].BottomLeftNear) +
      CamLocation + DistVecNear;

  float RenderDuration = 0.05;
  FColor Color = FColor::Purple;
  float NearThickness = 2;
  float FarThickness = 25;

  // Far clipping plane
  DrawDebugLine(UnrealWorld, UpperRightFar, BottomRightFar, Color, false,
                RenderDuration, 0, FarThickness);
  DrawDebugLine(UnrealWorld, UpperLeftFar, BottomLeftFar, Color, false,
                RenderDuration, 0, FarThickness);
  DrawDebugLine(UnrealWorld, UpperLeftFar, UpperRightFar, Color, false,
                RenderDuration, 0, FarThickness);
  DrawDebugLine(UnrealWorld, BottomLeftFar, BottomRightFar, Color, false,
                RenderDuration, 0, FarThickness);

  // Near clipping plane
  DrawDebugLine(UnrealWorld, UpperRightNear, BottomRightNear, Color, false,
                RenderDuration, 0, NearThickness);
  DrawDebugLine(UnrealWorld, UpperLeftNear, BottomLeftNear, Color, false,
                RenderDuration, 0, NearThickness);
  DrawDebugLine(UnrealWorld, UpperLeftNear, UpperRightNear, Color, false,
                RenderDuration, 0, NearThickness);
  DrawDebugLine(UnrealWorld, BottomLeftNear, BottomRightNear, Color, false,
                RenderDuration, 0, NearThickness);

  // Connect the two planes
  DrawDebugLine(UnrealWorld, UpperRightNear, UpperRightFar, Color, false,
                RenderDuration, 0, NearThickness);
  DrawDebugLine(UnrealWorld, UpperLeftNear, UpperLeftFar, Color, false,
                RenderDuration, 0, NearThickness);
  DrawDebugLine(UnrealWorld, BottomRightNear, BottomRightFar, Color, false,
                RenderDuration, 0, NearThickness);
  DrawDebugLine(UnrealWorld, BottomLeftNear, BottomLeftFar, Color, false,
                RenderDuration, 0, NearThickness);
}

void UUnrealCamera::CalculateProjectionMatrix() {
  USceneCaptureComponent2D* CaptureComp =
      Captures[projectairsim::MathUtils::ToNumeric(
          projectairsim::ImageType::kScene)];
  auto RenderTexture = CaptureComp->TextureTarget;

  // Taken from:
  // https://answers.unrealengine.com/questions/158008/get-fsceneview-for-inactive-camera.html
  FMinimalViewInfo ViewInfo;
  ViewInfo.Location = CaptureComp->GetComponentTransform().GetLocation();
  ViewInfo.Rotation =
      CaptureComp->GetComponentTransform().GetRotation().Rotator();
  ViewInfo.FOV = CaptureComp->FOVAngle;
  ViewInfo.ProjectionMode = CaptureComp->ProjectionType;
  ViewInfo.AspectRatio =
      float(RenderTexture->SizeX) / float(RenderTexture->SizeY);
  ViewInfo.OrthoNearClipPlane = GNearClippingPlane;
  ViewInfo.OrthoFarClipPlane = 10000;  // TODO
  ViewInfo.bConstrainAspectRatio = true;

  if (CaptureComp->bUseCustomProjectionMatrix == true) {
    ProjectionMatrix = CaptureComp->CustomProjectionMatrix;
  } else {
    ProjectionMatrix = ViewInfo.CalculateProjectionMatrix();
  }
}

void UUnrealCamera::ComputeFrustumVertices() {
  // Clear to ensure type indices are unchanged upon camera setting change
  FrustumData.clear();
  for (int ImageType = 0; ImageType < ImageTypeCount(); ++ImageType) {
    FrustumInfo Info;
    USceneCaptureComponent2D* CaptureComp = Captures[ImageType];

    float Angle = FMath::DegreesToRadians(CaptureComp->FOVAngle / 2);
    float AspectRatio = float(CaptureComp->TextureTarget->SizeX) /
                        float(CaptureComp->TextureTarget->SizeY);

    // Compute vertices for near clipping plane centered at origin
    Info.DistNear = GNearClippingPlane;  // 10 or 0.1m
    float HalfWidthNear = FMath::Tan(Angle) * Info.DistNear;
    float HalfHeightNear = HalfWidthNear / AspectRatio;

    Info.UpperRightNear = FVector(0, HalfWidthNear, HalfHeightNear);
    Info.BottomRightNear = FVector(0, HalfWidthNear, -HalfHeightNear);
    Info.UpperLeftNear = FVector(0, -HalfWidthNear, HalfHeightNear);
    Info.BottomLeftNear = FVector(0, -HalfWidthNear, -HalfHeightNear);

    // Compute vertices for far clipping plane centered at origin
    Info.DistFar = 10000;  // parameterize
    float HalfWidthFar = FMath::Tan(Angle) * Info.DistFar;
    float HalfHeightFar = HalfWidthFar / AspectRatio;

    Info.UpperRightFar = FVector(0, HalfWidthFar, HalfHeightFar);
    Info.BottomRightFar = FVector(0, HalfWidthFar, -HalfHeightFar);
    Info.UpperLeftFar = FVector(0, -HalfWidthFar, HalfHeightFar);
    Info.BottomLeftFar = FVector(0, -HalfWidthFar, -HalfHeightFar);
    FrustumData.push_back(Info);
  }
}

bool UUnrealCamera::ProjectPoint(const FVector& WorldPosition,
                                 FVector2D& OutScreenPos,
                                 FVector& ProjectedPos) const {
  USceneCaptureComponent2D* CaptureComp =
      Captures[projectairsim::MathUtils::ToNumeric(
          projectairsim::ImageType::kScene)];

  auto RenderTexture = CaptureComp->TextureTarget;
  FIntRect ScreenRect(0, 0, RenderTexture->SizeX, RenderTexture->SizeY);
  auto ProjectionData = GetProjectionData();
  auto ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
  FPlane Result =
      ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
  if (Result.W > 0.0f) {
    const float RHW = 1.0f / Result.W;
    FPlane PosInScreenSpace =
        FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

    // Move from projection space to normalized 0..1 UI space
    const float NormalizedX = (PosInScreenSpace.X / 2.f) + 0.5f;
    const float NormalizedY = 1.f - (PosInScreenSpace.Y / 2.f) - 0.5f;

    FVector2D RayStartViewRectSpace((NormalizedX * (float)ScreenRect.Width()),
                                    (NormalizedY * (float)ScreenRect.Height()));

    OutScreenPos =
        RayStartViewRectSpace + FVector2D(static_cast<float>(ScreenRect.Min.X),
                                          static_cast<float>(ScreenRect.Min.Y));

    OutScreenPos.X = FMath::Clamp(static_cast<int>(OutScreenPos.X),
                                  ScreenRect.Min.X, ScreenRect.Max.X);
    OutScreenPos.Y = FMath::Clamp(static_cast<int>(OutScreenPos.Y),
                                  ScreenRect.Min.Y, ScreenRect.Max.Y);

    ProjectedPos.X =
        FMath::Clamp(static_cast<float>(Result.X * RHW), -1.0f, 1.0f);
    ProjectedPos.Y =
        FMath::Clamp(static_cast<float>(Result.Y * RHW), -1.0f, 1.0f);
    ProjectedPos.Z =
        FMath::Clamp(static_cast<float>(Result.Z * RHW), -1.0f, 1.0f);

    return true;
  }

  return false;
}

FSceneViewProjectionData UUnrealCamera::GetProjectionData() const {
  USceneCaptureComponent2D* CaptureComp =
      Captures[projectairsim::MathUtils::ToNumeric(
          projectairsim::ImageType::kScene)];
  // initialize projection data for sceneview
  FSceneViewProjectionData ProjectionData;

  if (CaptureComp != nullptr) {
    auto RenderTexture = CaptureComp->TextureTarget;
    FIntRect ScreenRect(0, 0, RenderTexture->SizeX, RenderTexture->SizeY);

    ProjectionData.ViewOrigin =
        CaptureComp->GetComponentTransform().GetLocation();
    // Apply rotation matrix of camera's pose plus the rotation matrix for
    // converting Unreal's world axes to the camera's view axes (z forward,
    // etc).
    ProjectionData.ViewRotationMatrix =
        FInverseRotationMatrix(
            CaptureComp->GetComponentTransform().GetRotation().Rotator()) *
        FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0),
                FPlane(0, 0, 0, 1));
    ProjectionData.ProjectionMatrix = ProjectionMatrix;
    ProjectionData.SetConstrainedViewRectangle(ScreenRect);
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("[UnrealCamera] couldn't capture image"));
  }

  return ProjectionData;
}

/// <summary>
/// If enabled, computes projected_bbox3d, projected_bbox2d and a 2D bounding
/// box oriented as the settings specified it.
/// </summary>
/// <param name="BBox2DSettings"></param>
/// <param name="Annotation">Annotation containing object id of
/// interest</param> <param name="OrientedBox">3D bbox if
/// precalculated</param> <param name="IsBBox3DPrecomputed"></param> <param
/// name="ScreenBox">From (0,0) to (w,h) of view rectangle</param>
/// <returns>True if a valid 2D bbox was generated.</returns>
bool UUnrealCamera::GetBoundingBoxProjections(
    projectairsim::Annotation& Annotation, FOrientedBox& OrientedBox,
    bool bIsBBox3DPrecomputed, const projectairsim::Box2f& ScreenBox) const {
  // TODO: add support for oriented 2D bboxes
  FRotator Rot;
  if (!bIsBBox3DPrecomputed &&
      !(UnrealScene->GetUnrealBoundingBox3D(
          Annotation.object_id, projectairsim::BoxAlignment::kOriented,
          OrientedBox, Rot))) {
    return false;  // skip, no such actor
  }

  FVector BoxCorners[8];
  OrientedBox.CalcVertices(BoxCorners);
  Annotation.bbox3d_in_image_space = std::vector<projectairsim::Vector2>(8);
  Annotation.bbox3d_in_projection_space =
      std::vector<projectairsim::Vector3>(8);

  auto BoundingBox = FBox2D(ForceInit);
  for (int32 CornerIndex = 0; CornerIndex < 8; CornerIndex++) {
    auto ProjectedPoint2D = FVector2D(0.f);
    auto ProjectedPoint3D = FVector(0.f);
    if (ProjectPoint(BoxCorners[CornerIndex], ProjectedPoint2D,
                     ProjectedPoint3D)) {
      // in front of camera, not necessarily in view rectangle
      BoundingBox += ProjectedPoint2D;
    }
    Annotation.bbox3d_in_image_space[CornerIndex] =
        projectairsim::Vector2(ProjectedPoint2D.X, ProjectedPoint2D.Y);

    Annotation.bbox3d_in_projection_space[CornerIndex] = projectairsim::Vector3(
        ProjectedPoint3D.X, ProjectedPoint3D.Y, ProjectedPoint3D.Z);
  }

  // Take intersection so that bouding box is always within screen bounds
  auto IntersectingBox = ScreenBox.intersection(projectairsim::Box2f(
      projectairsim::Vector2(BoundingBox.Min.X, BoundingBox.Min.Y),
      projectairsim::Vector2(BoundingBox.Max.X, BoundingBox.Max.Y)));

  Annotation.bbox2D = projectairsim::BBox2D(IntersectingBox);

  return (IntersectingBox.volume() > 0.0001);
}
/// <summary>
/// Generates a list of annotations (with 3D and 2D bboxes, as requested)
/// of the current capture.
/// </summary>
/// <param name="AnnotationSettings"></param>
/// <param name="Annotations"></param>
void UUnrealCamera::GetBoundingBoxes(
    const projectairsim::AnnotationSettings& AnnotationSettings,
    std::vector<projectairsim::Annotation>& OutAnnotations) const {
  if (!AnnotationSettings.enabled) {
    return;
  }

  auto RenderTarget = RenderTargets[projectairsim::MathUtils::ToNumeric(
      projectairsim::ImageType::kScene)];
  auto ScreenBox = projectairsim::Box2f(
      projectairsim::Vector2(0.0, 0.0),
      projectairsim::Vector2(RenderTarget->SizeX, RenderTarget->SizeY));

  for (auto& ObjectID : AnnotationSettings.object_ids) {
    auto Annotation = projectairsim::Annotation(ObjectID);
    FOrientedBox OrientedBox;
    auto bIsBBox3DPrecomputed = UnrealScene->GetSimBoundingBox3D(
        Annotation.object_id, AnnotationSettings.bbox3D_settings.alignment,
        OrientedBox, Annotation.bbox3D);
    bIsBBox3DPrecomputed &= AnnotationSettings.bbox3D_settings.alignment ==
                            projectairsim::BoxAlignment::kOriented;

    bool bIsBBoxInDetectionRange = true;

    projectairsim::Transform Location = UnrealTransform::GetPoseNed(this);

    if (AnnotationSettings.distance_filter_enabled) {
      if ((Annotation.bbox3D.center - Location.translation_).norm() >
          AnnotationSettings.distance_filter_range) {
        // outside detection range
        bIsBBoxInDetectionRange = false;
      }
    }

    // If exists, computes projected_bbox3d, projected_bbox2d and bbox2d
    if (bIsBBoxInDetectionRange &&
        GetBoundingBoxProjections(Annotation, OrientedBox, bIsBBox3DPrecomputed,
                                  ScreenBox)) {
      OutAnnotations.push_back(
          Annotation);  // this will also exclude out-of-frustum 3D bboxes
    }
  }
}