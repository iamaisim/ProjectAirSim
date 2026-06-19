#include "LidarIntensitySceneViewExtension.h"

#include "RHI.h"
#include "UnrealCompatibility.h"
#include "SceneView.h"
#include "RenderGraph.h"
#include "ScreenPass.h"
#include "RenderGraphUtils.h"
#include "CommonRenderResources.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "PostProcess/SceneFilterRendering.h"
#include "RenderGraphUtils.h"
#include "RendererInterface.h"
#include "SceneTextureParameters.h"

// Access to FPostProcessingInputs definition from internal Renderer headers
#if UE_IS_5_7
    #include "Runtime/Renderer/Internal/PostProcess/PostProcessInputs.h"
#elif UE_IS_5_2
    #include "Runtime/Renderer/Private/PostProcess/PostProcessing.h"
#endif

#include "LidarIntensityShader.h"

static const bool DEBUG_RENDER_TO_VIEWPORT = false;

//////////////////////////////////////////////////////////////////////
/// Directly referenced from ColorCorrectRegionSceneViewExtension ///

FScreenPassTextureViewportParameters GetTextureViewportParameters(
    const FScreenPassTextureViewport& InViewport) {
  const FVector2f Extent(InViewport.Extent);
  const FVector2f ViewportMin(InViewport.Rect.Min.X, InViewport.Rect.Min.Y);
  const FVector2f ViewportMax(InViewport.Rect.Max.X, InViewport.Rect.Max.Y);
  const FVector2f ViewportSize = ViewportMax - ViewportMin;

  FScreenPassTextureViewportParameters Parameters;

  if (!InViewport.IsEmpty()) {
    Parameters.Extent = Extent;
    Parameters.ExtentInverse = FVector2f(1.0f / Extent.X, 1.0f / Extent.Y);

    Parameters.ScreenPosToViewportScale = FVector2f(0.5f, -0.5f) * ViewportSize;
    Parameters.ScreenPosToViewportBias = (0.5f * ViewportSize) + ViewportMin;

    Parameters.ViewportMin = InViewport.Rect.Min;
    Parameters.ViewportMax = InViewport.Rect.Max;

    Parameters.ViewportSize = ViewportSize;
    Parameters.ViewportSizeInverse =
        FVector2f(1.0f / Parameters.ViewportSize.X,
                  1.0f / Parameters.ViewportSize.Y);

    Parameters.UVViewportMin = ViewportMin * Parameters.ExtentInverse;
    Parameters.UVViewportMax = ViewportMax * Parameters.ExtentInverse;

    Parameters.UVViewportSize =
        Parameters.UVViewportMax - Parameters.UVViewportMin;
    Parameters.UVViewportSizeInverse =
        FVector2f(1.0f / Parameters.UVViewportSize.X,
                  1.0f / Parameters.UVViewportSize.Y);

    Parameters.UVViewportBilinearMin =
        Parameters.UVViewportMin + 0.5f * Parameters.ExtentInverse;
    Parameters.UVViewportBilinearMax =
        Parameters.UVViewportMax - 0.5f * Parameters.ExtentInverse;
  }

  return Parameters;
}

template <typename TSetupFunction>
void DrawScreenPass(FRHICommandListImmediate& RHICmdList,
                    const FSceneView& View,
                    const FScreenPassTextureViewport& OutputViewport,
                    const FScreenPassTextureViewport& InputViewport,
                    const FScreenPassPipelineState& PipelineState,
                    TSetupFunction SetupFunction) {
  PipelineState.Validate();

  const FIntRect InputRect = InputViewport.Rect;
  const FIntPoint InputSize = InputViewport.Extent;
  const FIntRect OutputRect = OutputViewport.Rect;
  const FIntPoint OutputSize = OutputRect.Size();

  RHICmdList.SetViewport(OutputRect.Min.X, OutputRect.Min.Y, 0.0f,
                         OutputRect.Max.X, OutputRect.Max.Y, 1.0f);

  SetScreenPassPipelineState(RHICmdList, PipelineState);

  // Setting up buffers.
  SetupFunction(RHICmdList);

  FIntPoint LocalOutputPos(FIntPoint::ZeroValue);
  FIntPoint LocalOutputSize(OutputSize);
  EDrawRectangleFlags DrawRectangleFlags = EDRF_UseTriangleOptimization;

  DrawPostProcessPass(RHICmdList, LocalOutputPos.X, LocalOutputPos.Y,
                      LocalOutputSize.X, LocalOutputSize.Y, InputRect.Min.X,
                      InputRect.Min.Y, InputRect.Width(), InputRect.Height(),
                      OutputSize, InputSize, PipelineState.VertexShader,
                      View.StereoViewIndex, false, DrawRectangleFlags);
}
///////////////////////////////////////////////////////////

bool FLidarIntensitySceneViewExtension::IsValidForBoundRenderTarget(
    const FSceneViewFamily& Family) const {
  return (RenderTarget2D.IsValid() &&
          Family.RenderTarget == RenderTarget2D->GetRenderTargetResource()) ||
         DEBUG_RENDER_TO_VIEWPORT;
}

FLidarIntensitySceneViewExtension::FLidarIntensitySceneViewExtension(
    const FAutoRegister& AutoRegister,
    TWeakObjectPtr<UTextureRenderTarget2D> InRenderTarget2D)
    : FSceneViewExtensionBase(AutoRegister),
      RenderTarget2D(InRenderTarget2D) {
  for (int i = 0; i < NumReadbackBuffers; ++i) {
    ReadbackBuffers[i] = MakeUnique<FRHIGPUBufferReadback>(FName(*FString::Printf(TEXT("LidarReadback_%d"), i)));
    ReadbackBuffersSizes[i] = 0;
  }
}

void FLidarIntensitySceneViewExtension::PrePostProcessPass_RenderThread(
    FRDGBuilder& GraphBuilder, const FSceneView& View,
    const FPostProcessingInputs& Inputs) {
  if (CSParamsQ.empty() || !IsValidForBoundRenderTarget(*View.Family)) {
    return;
  }

  auto cachedParams = CSParamsQ.front();
  CSParamsQ.pop();

  const FIntRect Viewport = View.UnscaledViewRect;

  // Access scene color from the new API
  const FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, Viewport);

  // not sure of the implications of it being invalid
  if (!SceneColor.IsValid()) {
    return;
  }

  // Getting material data for the current view.
  FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());

  // Reusing the same output description for our back buffer as SceneColor
  FRDGTextureDesc LidarIntensityOutputDesc = SceneColor.Texture->Desc;

  FRDGTexture* IntensityRenderTargetTexture = GraphBuilder.CreateTexture(
      LidarIntensityOutputDesc, TEXT("IntensityRenderTargetTexture"));
  FScreenPassRenderTarget IntensityRenderTarget = FScreenPassRenderTarget(
      IntensityRenderTargetTexture, SceneColor.ViewRect,
      ERenderTargetLoadAction::EClear);
  FScreenPassRenderTarget SceneColorRenderTarget(
      SceneColor, ERenderTargetLoadAction::ELoad);
  const FScreenPassTextureViewport SceneColorTextureViewport(SceneColor);

  // TODO: not sure what these different states entail.
  FRHIBlendState* DefaultBlendState =
      FScreenPassPipelineState::FDefaultBlendState::GetRHI();
  FRHIDepthStencilState* DepthStencilState =
      FScreenPassPipelineState::FDefaultDepthStencilState::GetRHI();

  const FScreenPassTextureViewportParameters SceneTextureViewportParams =
      GetTextureViewportParameters(SceneColorTextureViewport);

#if UE_IS_5_7
  FSceneTextureShaderParameters SceneTextures =
      CreateSceneTextureShaderParameters(
          GraphBuilder, View, ESceneTextureSetupMode::All);
#elif UE_IS_5_2
  FSceneTextureShaderParameters SceneTextures =
      CreateSceneTextureShaderParameters(
          GraphBuilder, ((const FViewInfo&)View).GetSceneTexturesChecked(),
          View.GetFeatureLevel(), ESceneTextureSetupMode::All);
#endif

  const FScreenPassTextureViewport TextureViewport(
      SceneColorRenderTarget.Texture, Viewport);

  FLidarIntensityShaderInputParameters* PostProcessMaterialParameters =
      GraphBuilder.AllocParameters<FLidarIntensityShaderInputParameters>();

  // Added this to render the intensity texture into the game viewport
  // for debugging
  if (DEBUG_RENDER_TO_VIEWPORT) {
    PostProcessMaterialParameters->RenderTargets[0] =
        SceneColorRenderTarget.GetRenderTargetBinding();
  } else {
    PostProcessMaterialParameters->RenderTargets[0] =
        IntensityRenderTarget.GetRenderTargetBinding();
  }

  PostProcessMaterialParameters->PostProcessOutput = SceneTextureViewportParams;
  PostProcessMaterialParameters->SceneTextures = SceneTextures;
  PostProcessMaterialParameters->View = View.ViewUniformBuffer;

  TShaderMapRef<FLidarIntensityVS> VertexShader(GlobalShaderMap);
  TShaderMapRef<FLidarIntensityPS> PixelShader(GlobalShaderMap);

  ClearUnusedGraphResources(VertexShader, PixelShader,
                            PostProcessMaterialParameters);

  GraphBuilder.AddPass(
      RDG_EVENT_NAME("LidarIntensityPass"), PostProcessMaterialParameters,
      ERDGPassFlags::Raster,
      [&View, TextureViewport, VertexShader, PixelShader, DefaultBlendState,
       DepthStencilState, PostProcessMaterialParameters](
          FRHICommandListImmediate& RHICmdList) {
        DrawScreenPass(
            RHICmdList, View,
            TextureViewport,  // Output Viewport
            TextureViewport,  // Input Viewport
            FScreenPassPipelineState(VertexShader, PixelShader,
                                     DefaultBlendState, DepthStencilState),
            [&](FRHICommandListImmediate& RHICmdList) {
                #if UE_IS_5_7
                    // View UB is already bound via PostProcessMaterialParameters->View;
                    // SetShaderParameters commits everything in one scratch batch.
                    // Do not call VS/PS->SetParameters here — it would dirty the
                    // scratch parameters and trip the !HasParameters() ensure on
                    // the next GetScratchShaderParameters() call inside SetShaderParameters.
                    SetShaderParameters(RHICmdList, VertexShader,
                                        VertexShader.GetVertexShader(),
                                        *PostProcessMaterialParameters);

                    SetShaderParameters(RHICmdList, PixelShader,
                                        PixelShader.GetPixelShader(),
                                        *PostProcessMaterialParameters);
                #elif UE_IS_5_2
                     VertexShader->SetParameters(RHICmdList, View);
                        SetShaderParameters(RHICmdList, VertexShader,
                                            VertexShader.GetVertexShader(),
                                            *PostProcessMaterialParameters);

                        PixelShader->SetParameters(RHICmdList, View);
                        SetShaderParameters(RHICmdList, PixelShader,
                                            PixelShader.GetPixelShader(),
                                            *PostProcessMaterialParameters);
                #endif
            });
      });

  // Now that we have computed the intensity texture, we can pass this to the
  // LidarPointCloud compute shader as input and add its pass.

  TShaderMapRef<FLidarPointCloudCS> LidarPointCloudShader(GlobalShaderMap);

  uint32 NumPoints =
      cachedParams.NumCams * cachedParams.HorizontalResolution * cachedParams.LaserNums;
  uint32 BufferSize = NumPoints * sizeof(float) * 4;

  // Readback from previous frames if available
  auto& CurrentReadback = ReadbackBuffers[CurrentReadbackIndex];
  uint32 CurrentSize = ReadbackBuffersSizes[CurrentReadbackIndex];

  if (CurrentSize > 0 && CurrentReadback && CurrentReadback->IsReady()) {
      void* BufferData = CurrentReadback->Lock(CurrentSize);
      if (BufferData) {
          // Resize vector to match the data we are reading back
          const int NumPointsRead = CurrentSize / (sizeof(float) * 4);
          if (LidarPointCloudData.size() != NumPointsRead) {
            LidarPointCloudData.resize(NumPointsRead);
          }

          FMemory::Memcpy(LidarPointCloudData.data(), BufferData, CurrentSize);
          CurrentReadback->Unlock();
      }
  }

  // Guard: nothing to dispatch if NumPoints is zero.
  if (NumPoints == 0) {
    return;
  }

  // Create an RDG structured buffer for the compute shader output.
  // We do NOT upload initial data here (no nullptr crash); the shader writes
  // every slot, and AddClearUAVFloatPass zeroes any slots it misses.
  FRDGBufferRef PointCloudBufferRDG =
      GraphBuilder.CreateBuffer(
          FRDGBufferDesc::CreateStructuredDesc(sizeof(float), NumPoints * 4),
          TEXT("FLidarPointCloudCS_PointCloudBuffer"));

  // Structured UAV — no pixel format; must match RWStructuredBuffer<float> in HLSL.
  FRDGBufferUAVRef PointCloudBufferUAV = GraphBuilder.CreateUAV(PointCloudBufferRDG);
  AddClearUAVFloatPass(GraphBuilder, PointCloudBufferUAV, -1.0f);

  FLidarPointCloudCS::FParameters* PassParameters =
      GraphBuilder.AllocParameters<FLidarPointCloudCS::FParameters>();
  PassParameters->PointCloudBuffer = PointCloudBufferUAV;
  PassParameters->HorizontalResolution = cachedParams.HorizontalResolution;
  PassParameters->LaserNums = cachedParams.LaserNums;
  PassParameters->LaserRange = cachedParams.LaserRange;
  PassParameters->CurrentHorizontalAngleDeg =
      cachedParams.CurrentHorizontalAngleDeg;
  PassParameters->HorizontalFOV = cachedParams.HorizontalFOV;
    // Forward the configured azimuth window so the compute pass uses the same
    // FOV limits as the CPU-side sensor settings.
    PassParameters->HorizontalFOVStartDeg = cachedParams.HorizontalFOVStartDeg;
    PassParameters->HorizontalFOVEndDeg = cachedParams.HorizontalFOVEndDeg;
  PassParameters->VerticalFOV = cachedParams.VerticalFOV;
  PassParameters->CamFrustrumHeight = cachedParams.CamFrustrumHeight;
  PassParameters->CamFrustrumWidth = cachedParams.CamFrustrumWidth;
  PassParameters->ProjectionMatrixInv =
      cachedParams.ViewProjectionMatInv.GetTransposed();

  // Forward projection matrix used by the shader's ProjectWorldToScreen
  // (spherical coords → screen). The spherical coord is camera-relative,
  // so ViewOrigin = 0; the axis-swap rotation orients Unreal's world axes
  // into the projection's view axes (z-forward). Transposed because HLSL
  // mul(matrix, vector) treats the matrix as column-major.
  {
    FSceneViewProjectionData FwdProjData;
    FwdProjData.ViewOrigin = FVector(0.f);
    FwdProjData.ViewRotationMatrix =
        FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0),
                FPlane(0, 0, 0, 1));
    FwdProjData.ProjectionMatrix = cachedParams.ProjectionMat;
    FwdProjData.SetConstrainedViewRectangle(FIntRect(
        0, 0, cachedParams.CamFrustrumWidth, cachedParams.CamFrustrumHeight));
    PassParameters->ProjectionMatrix =
        FMatrix44f(FwdProjData.ComputeViewProjectionMatrix().GetTransposed());
  }

  PassParameters->CamRotationMatrix1 = cachedParams.RotationMatCam1;
  PassParameters->CamRotationMatrix2 = cachedParams.RotationMatCam2;
  PassParameters->CamRotationMatrix3 = cachedParams.RotationMatCam3;
  PassParameters->CamRotationMatrix4 = cachedParams.RotationMatCam4;

  PassParameters->DepthImage1 = cachedParams.DepthTexture1;
    // Each texture corresponds to one 90-degree capture used by the 360-degree
    // GPU lidar sweep.
    PassParameters->DepthImage2 = cachedParams.DepthTexture2;
  PassParameters->DepthImage3 = cachedParams.DepthTexture3;
  PassParameters->DepthImage4 = cachedParams.DepthTexture4;

  FIntVector GroupContext(
      FMath::DivideAndRoundUp<uint32>(NumPoints, 1024u),
      NUM_THREADS_PER_GROUP_DIMENSION_Y,
      NUM_THREADS_PER_GROUP_DIMENSION_Z);

  FComputeShaderUtils::AddPass(
      GraphBuilder, RDG_EVENT_NAME("LidarPointCloud Pass"),
      LidarPointCloudShader, PassParameters, GroupContext);

  AddEnqueueCopyPass(GraphBuilder, CurrentReadback.Get(), PointCloudBufferRDG, BufferSize);

  // Store the size for the next time we encounter this buffer slot
  ReadbackBuffersSizes[CurrentReadbackIndex] = BufferSize;

  // Advance index for next frame
  CurrentReadbackIndex = (CurrentReadbackIndex + 1) % NumReadbackBuffers;
}

void FLidarIntensitySceneViewExtension::UpdateParameters(
    FLidarPointCloudCSParameters& params) {
  CSParamsQ.push(params);
}

bool FLidarIntensitySceneViewExtension::IsActiveThisFrame_Internal(
    const FSceneViewExtensionContext& Context) const {
  return !CSParamsQ.empty();
}