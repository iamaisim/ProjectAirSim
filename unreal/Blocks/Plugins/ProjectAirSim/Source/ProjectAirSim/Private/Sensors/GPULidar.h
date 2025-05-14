// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// GPU-based implementation of a lidar.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "UnrealSensor.h"
#include "LidarIntensitySceneViewExtension.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Camera/CameraComponent.h"
#include "UnrealSensor.h"
#include "core_sim/clock.hpp"
#include "core_sim/sensors/lidar.hpp"
#include "core_sim/transforms/transform_utils.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "GPULidar.generated.h"

UCLASS() class UGPULidar : public UUnrealSensor {
  GENERATED_BODY()

 public:
  UPROPERTY(EditAnywhere)
  bool resetCams;

  explicit UGPULidar(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::Lidar& SimLidar);

  void TickComponent(float DeltaTime, ELevelTick TickType,
                     FActorComponentTickFunction* ThisTickFunction) override;

  void SetupLidarFromSettings(
      const microsoft::projectairsim::LidarSettings& LidarSettings);

 protected:
  void BeginPlay() override;
  void BeginDestroy() override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void InitializePose();

  void Simulate(const float DeltaTime);

 private:
  void BeginFrameCallback();
  void EndFrameCallback();
  void BeginFrameCallbackRT();
  void EndFrameCallbackRT();

 private:
  microsoft::projectairsim::Lidar Lidar;
  microsoft::projectairsim::LidarSettings Settings;
  float CurrentHorizontalAngleDeg = 0.0f;
  std::vector<float> PointCloud;
  std::vector<float> AzimuthElevationRangeCloud;
  std::vector<int> SegmentationCloud;
  std::vector<float> IntensityCloud;
  std::vector<int> LaserIndexCloud;
  TimeNano LastSimTime = 0;

  void SaveImage();
  void SetUpCams();
  void SetupSceneCapture(UCameraComponent* CameraComponent,
                         float HorizontalAngle, float Width, float Height,
                         USceneCaptureComponent2D* OutSceneCaptureComp);

 private:
  std::vector<UCameraComponent*> CameraComponents;
  std::vector<USceneCaptureComponent2D*> DepthSceneCaptures;
  std::vector<USceneCaptureComponent2D*> IntensityCaptureComponents;

  UPROPERTY() UMaterial* LidarIntensityMaterialStatic;

  uint32 HorizontalResolution;  // more like spherical horizontal width
  uint32 LaserNums;             // more like spherical vertical height

  // Need camfrustrum as well?
  uint32 CamFrustrumWidth;
  uint32 CamFrustrumHeight;

  FMatrix ProjectionMat;
  FVector cam1loc;
  FQuat cam1Rot;

  std::vector<FMatrix> CamRotationMats;
  FSceneViewProjectionData Cam1ProjData;

  TSharedPtr<FLidarIntensitySceneViewExtension, ESPMode::ThreadSafe>
      IntensityExtension;
};
