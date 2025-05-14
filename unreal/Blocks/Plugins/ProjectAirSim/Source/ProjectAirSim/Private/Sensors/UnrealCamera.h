// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// The main Unreal Camera implementation.

#pragma once

#include <string>
#include <vector>

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "CineCameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "UnrealSensor.h"
#include "core_sim/clock.hpp"
#include "core_sim/sensors/camera.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealCamera.generated.h"

// To check image data integrity for the client API GetImages() requesting
// captured image data from the sim camera sensor without getting corrupted
// by new image capture data being moved from UUnrealCamera::OnRendered() to
// the sim camera through Camera::Impl::PublishImages(),
// set TEST_DEBUG_IMAGE_INTEGRITY to 1 and run the corresponding client
// pytest test_api_services.py::test_debug_image_integrity.
#define TEST_DEBUG_IMAGE_INTEGRITY 0

class AUnrealScene;

struct ImageRequest {
  std::string CameraName;
  microsoft::projectairsim::ImageType ImageType =
      microsoft::projectairsim::ImageType::kScene;
  bool bPixelsAsFloat = false;
  bool bCompress = true;

  ImageRequest() {}

  ImageRequest(const std::string& CameraNameVal,
               microsoft::projectairsim::ImageType ImageTypeVal,
               bool bPixelsAsFloatVal = false, bool bCompressVal = true) {
    CameraName = CameraNameVal;
    ImageType = ImageTypeVal;
    bPixelsAsFloat = bPixelsAsFloatVal;
    bCompress = bCompressVal;
  }
};

struct FrustumInfo {
  float DistNear;
  float DistFar;

  FVector UpperRightFar;
  FVector BottomRightFar;
  FVector UpperLeftFar;
  FVector BottomLeftFar;

  FVector UpperRightNear;
  FVector BottomRightNear;
  FVector UpperLeftNear;
  FVector BottomLeftNear;
};

struct ImageResponse {
  std::vector<uint8_t> ImageDataUInt8;
  std::vector<float> ImageDataFloat;

  std::string CameraName;
  microsoft::projectairsim::Vector3 CameraPosition;
  microsoft::projectairsim::Quaternion CameraOrientation;
  TimeNano TimeStamp = 0;
  std::string Message;
  bool bPixelsAsFloat = false;
  bool bCompress = true;
  int Width = 0, Height = 0;
  microsoft::projectairsim::ImageType ImageType;
};

UCLASS()
class UUnrealCamera : public UUnrealSensor {
  GENERATED_BODY()

 public:
  explicit UUnrealCamera(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::Camera& InCamera,
                  AUnrealScene* InUnrealScene);

  void TickComponent(float DeltaTime, ELevelTick TickType,
                     FActorComponentTickFunction* ThisTickFunction) override;

  void SetCameraTypeEnabled(microsoft::projectairsim::ImageType Type,
                            bool bIsEnabled);

  bool GetCameraTypeEnabled(microsoft::projectairsim::ImageType Type) const;

  bool GetCameraTypeEnabled(const int typeInt) const;

  void SetupCameraFromSettings(
      const microsoft::projectairsim::CameraSettings& CameraSetting);

  void SetRelativePoseFromNed(
      const microsoft::projectairsim::Transform& PoseNed);

  USceneCaptureComponent2D* GetCaptureComponent(
      const microsoft::projectairsim::ImageType Type, bool bIfActive);

  UTextureRenderTarget2D* GetRenderTarget(
      const microsoft::projectairsim::ImageType Type, bool bIfActive);

  USceneCaptureComponent2D* GetActiveStreamingCapture();
  int32 GetNextStreamingCaptureIdx();

  static int ImageTypeCount();

 protected:
  void BeginPlay() override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  bool IsReadyForFinishDestroy() override;

 private:
  void LoadCameraMaterials();
  void CreateComponents();

  static void UpdateCaptureComponentSetting(
      USceneCaptureComponent2D* Capture, UTextureRenderTarget2D* RenderTarget,
      const microsoft::projectairsim::CaptureSettings& Setting,
      UWorld* UnrealWorld);

  void SetNoiseMaterial(
      int ImageType, UObject* CaptureComponent,
      FPostProcessSettings& PostProcessSettings,
      const microsoft::projectairsim::NoiseSettings& NoiseSettings);

  void SetDepthVisMaterial(
      UObject* CaptureComponent, FPostProcessSettings& PostProcessSettings,
      const microsoft::projectairsim::CaptureSettings& CaptureSettings);

  void UpdateCameraSettings();

  static void UpdateCameraPostProcessingSetting(
      FPostProcessSettings& PostProcessSettings,
      const microsoft::projectairsim::CaptureSettings& CaptureSettings);

  void CopyCameraSettingsToAllSceneCaptures(
      const microsoft::projectairsim::CameraSettings& CameraSetting);

  void CopyCameraSettingsToSceneCapture(
      UCameraComponent* SourceCamComponent,
      USceneCaptureComponent2D* TargetCaptureComponent,
      const microsoft::projectairsim::CaptureSettings& CaptureSetting);

  void OnRendered(
      microsoft::projectairsim::Transform CapturedCameraTransform,
      std::vector<microsoft::projectairsim::Annotation> Annotations);

  // For bounding box calculations
  void CalculateProjectionMatrix();

  // compute frustum points at origin
  void ComputeFrustumVertices();

  void DrawViewFrustum();

  void GetBoundingBoxes(
      const microsoft::projectairsim::AnnotationSettings& AnnotationSettings,
      std::vector<microsoft::projectairsim::Annotation>& OutAnnotations) const;

  bool GetBoundingBoxProjections(
      microsoft::projectairsim::Annotation& Annotation,
      FOrientedBox& OrientedBox, bool bIsBBox3DPrecomputed,
      const microsoft::projectairsim::Box2f& ScreenBox) const;

  bool ProjectPoint(const FVector& WorldPosition, FVector2D& OutScreenPos,
                    FVector& ProjectedPos) const;

  FSceneViewProjectionData GetProjectionData() const;

  AUnrealScene* UnrealScene;

  // sim object
  microsoft::projectairsim::Camera SimCamera;

  // unreal materials used by camera
  UPROPERTY() TArray<UMaterialInstanceDynamic*> NoiseMaterials;
  UPROPERTY() UMaterial* NoiseMaterialStatic;
  UPROPERTY() UMaterial* DepthPlanarMaterialStatic;
  UPROPERTY() UMaterial* DepthPerspectiveMaterialStatic;
  UPROPERTY() UMaterial* SegmentationMaterialStatic;
  UPROPERTY() UMaterial* DepthVisMaterialStatic;
  UPROPERTY() UMaterial* DisparityNormalizedMaterialStatic;
  UPROPERTY() UMaterial* SurfaceNormalsMaterialStatic;

  // capture components
  UCineCameraComponent* CameraComponent;
  UPROPERTY() TArray<USceneCaptureComponent2D*> Captures;
  UPROPERTY() TArray<UTextureRenderTarget2D*> RenderTargets;
  UPROPERTY() TArray<USceneCaptureComponent2D*> StreamingCaptures;
  int32 StreamingCaptureActiveIdx = 0;

  // settings
  std::vector<bool> CameraTypeEnabled;
  float CaptureIntervalSec = 0.0f;
  std::vector<ImageRequest> CaptureRequests;

  // true when there is a pending image capture
  volatile bool bCapturePending = false;

  // Timestamp stored to limit capture rate for enqueuing next capture
  TimeNano CapturedTime = 0;

  FMatrix ProjectionMatrix;

  std::vector<FrustumInfo> FrustumData;

#if TEST_DEBUG_IMAGE_INTEGRITY
  uint8_t DebugImagePixelValue = 0;
#endif
};
