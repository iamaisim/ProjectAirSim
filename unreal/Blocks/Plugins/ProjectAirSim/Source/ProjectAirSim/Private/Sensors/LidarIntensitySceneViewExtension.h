#pragma once

#include "RHI.h"
#include "RHIResources.h"
#include "RHIGPUReadback.h"
#include "SceneViewExtension.h"

#include "LidarPointCloudCS.h"

// Forward declarations
struct FPostProcessingInputs;

class FLidarIntensitySceneViewExtension : public FSceneViewExtensionBase {
 public:
  FLidarIntensitySceneViewExtension(const FAutoRegister& AutoRegister,
                                    TWeakObjectPtr<UTextureRenderTarget2D> InRenderTarget2D);

  //~ Begin FSceneViewExtensionBase Interface
  virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
  virtual void SetupView(FSceneViewFamily& InViewFamily,
                         FSceneView& InView) override {};
  virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
  virtual void PreRenderViewFamily_RenderThread(
      FRDGBuilder& GraphBuilder,
      FSceneViewFamily& InViewFamily) override {};
  virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder,
                                          FSceneView& InView) override {};

  // Only implement this, called right before post processing begins.
  virtual void PrePostProcessPass_RenderThread(
      FRDGBuilder& GraphBuilder, const FSceneView& View,
      const FPostProcessingInputs& Inputs) override;
  virtual bool IsActiveThisFrame_Internal(
      const FSceneViewExtensionContext& Context) const override;

  //~ End FSceneViewExtensionBase Interface

  bool IsValidForBoundRenderTarget(const FSceneViewFamily& Family) const;
  void UpdateParameters(FLidarPointCloudCSParameters& params);

  BEGIN_SHADER_PARAMETER_STRUCT(FCopyBufferToCPUPass, )
    RDG_BUFFER_ACCESS(Buffer, ERHIAccess::CopySrc)
  END_SHADER_PARAMETER_STRUCT()

public:
  // Must be the float (16-byte) variant — UE5's FVector4 is TVector4<double>
  // (32 bytes), which mismatches the 16-byte-per-point layout the compute
  // shader writes and would cause silent stride misalignment.
  std::vector<FVector4f> LidarPointCloudData;
  TimeNano LidarPointCloudTime = 0;
  microsoft::projectairsim::Pose LidarPointCloudPose;
  bool bHasUnreadLidarPointCloudData = false;

private:
  std::queue<FLidarPointCloudCSParameters> CSParamsQ;
  TWeakObjectPtr<UTextureRenderTarget2D> RenderTarget2D;

  static constexpr int NumReadbackBuffers = 2;
  TUniquePtr<FRHIGPUBufferReadback> ReadbackBuffers[NumReadbackBuffers];
  uint32 ReadbackBuffersSizes[NumReadbackBuffers] = {};
  FLidarPointCloudCSParameters ReadbackMetadata[NumReadbackBuffers];
  int CurrentReadbackIndex = 0;
};
