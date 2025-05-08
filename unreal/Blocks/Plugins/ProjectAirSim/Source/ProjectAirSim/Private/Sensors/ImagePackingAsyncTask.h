// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "UnrealCameraRenderRequest.h"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/transforms/transform_utils.hpp"

// This class follows the example template in Unreal's AsyncWork.h for
// FAutoDeleteAsyncTask that deletes itself after completion on a background
// thread.
class FImagePackingAsyncTask : public FNonAbandonableTask {
  friend class FAutoDeleteAsyncTask<FImagePackingAsyncTask>;

 public:
  FImagePackingAsyncTask(
      std::vector<
          std::pair<ImageRequest, UnrealCameraRenderRequest::RenderResult>>&&
          InCaptureResults,
      microsoft::projectairsim::Transform InCapturedCameraTransform,
      microsoft::projectairsim::Camera InCamera,
      std::vector<microsoft::projectairsim::Annotation>&& InAnnotations)
      : CaptureResults(std::move(InCaptureResults)),
        CapturedCameraTransform(InCapturedCameraTransform),
        Camera(InCamera),
        Annotations(InAnnotations) {}

  void DoWork();  // the method that will execute when thread is started

  FORCEINLINE TStatId GetStatId() const {
    RETURN_QUICK_DECLARE_CYCLE_STAT(FImagePackingAsyncTask,
                                    STATGROUP_ThreadPoolAsyncTasks);
  }

  std::vector<std::pair<ImageRequest, UnrealCameraRenderRequest::RenderResult>>
      CaptureResults;
  microsoft::projectairsim::Transform CapturedCameraTransform;
  microsoft::projectairsim::Camera Camera;
  std::vector<microsoft::projectairsim::Annotation> Annotations;
};
