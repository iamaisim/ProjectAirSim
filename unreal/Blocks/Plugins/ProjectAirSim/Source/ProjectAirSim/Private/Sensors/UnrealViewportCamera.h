// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include <map>     // for std::map
#include <memory>  // for shared_ptr
#include <set>     // for std::set
#include <vector>  // for std::vector

#include "CoreMinimal.h"
// #include "core_sim/clock.hpp"
#include "CineCameraActor.h"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/viewport_camera.hpp"
// comment so that generated.h is always the last include file with clang-format
#include "UnrealViewportCamera.generated.h"

UCLASS()
class AUnrealViewportCamera : public ACineCameraActor {
  GENERATED_BODY()

 public:
  explicit AUnrealViewportCamera(const FObjectInitializer& ObjectInitialize);

  void Initialize(microsoft::projectairsim::ViewportCamera& InViewportCamera);

  void Tick(float DeltaTime) override;

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void UpdateViewportCameraTargetPose(
      const microsoft::projectairsim::Pose& InPose);
  void UpdateViewportCameraAspectRatio(float);
  void UpdateViewPortCameraZoom(float zoom);
  void MoveViewportCameraToTargetPose();
  void MoveViewportCameraToAspectRatio();
  void MoveViewportCameraToZoom();

  microsoft::projectairsim::Pose ViewportCameraTargetPose;
  float aspect_ratio = 0;
  float zoom_ = 0.0f;
  bool bHasTargetPoseUpdated = false;
  bool bHasAspectRatioUpdated = false;
  bool bHasZoomUpdated = false;
};