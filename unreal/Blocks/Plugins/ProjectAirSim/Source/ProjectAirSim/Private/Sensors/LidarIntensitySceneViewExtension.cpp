#include "LidarIntensitySceneViewExtension.h"

#include "RHI.h"
#include "SceneView.h"
#include "RenderGraph.h"
#include "Runtime/Renderer/Private/PostProcess/PostProcessing.h"
#include "CommonRenderResources.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SceneRendering.h"

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
    Parameters.ViewportSizeInverse = FVector2f(
        1.0f / Parameters.ViewportSize.X, 1.0f / Parameters.ViewportSize.Y);

    Parameters.UVViewportMin = ViewportMin * Parameters.ExtentInverse;
    Parameters.UVViewportMax = ViewportMax * Parameters.ExtentInverse;

    Parameters.UVViewportSize =
        Parameters.UVViewportMax - Parameters.UVViewportMin;
    Parameters.UVViewportSizeInverse = FVector2f(
        1.0f / Parameters.UVViewportSize.X, 1.0f / Parameters.UVViewportSize.Y);

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
      RenderTarget2D(InRenderTarget2D) {}

void FLidarIntensitySceneViewExtension::PrePostProcessPass_RenderThread(
    FRDGBuilder& GraphBuilder, const FSceneView& View,
    const FPostProcessingInputs& Inputs) {
  if (CSParamsQ.empty() || !IsValidForBoundRenderTarget(*View.Family)) {
    return;
  }

  Inputs.Validate();
  auto cachedParams = CSParamsQ.front();
  CSParamsQ.pop();

  // The following is adapted from the ColorCorrectRegionsSceneViewExtension.cpp
  // example from UE. Up until line 208, when the first pass is added.
  checkSlow(View.bIsViewInfo);  // can't do dynamic_cast because FViewInfo
                                // doesn't have any virtual functions.
  const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;

  FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture,
                                Viewport);

  // not sure of the implications of it being invalid
  if (!SceneColor.IsValid()) {
    return;
  }

  // Getting material data for the current view.
  FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

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

  FSceneTextureShaderParameters SceneTextures =
      CreateSceneTextureShaderParameters(
          GraphBuilder, ((const FViewInfo&)View).GetSceneTexturesChecked(),
          View.GetFeatureLevel(), ESceneTextureSetupMode::All);

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
       DepthStencilState, PostProcessMaterialParameters](FRHICommandListImmediate& RHICmdList) {
        DrawScreenPass(
            RHICmdList, View,
            TextureViewport,  // Output Viewport
            TextureViewport,  // Input Viewport
            FScreenPassPipelineState(VertexShader, PixelShader,
                                     DefaultBlendState, DepthStencilState),
            [&](FRHICommandListImmediate& RHICmdList) {
              VertexShader->SetParameters(RHICmdList, View);
              SetShaderParameters(RHICmdList, VertexShader,
                                  VertexShader.GetVertexShader(),
                                  *PostProcessMaterialParameters);

              PixelShader->SetParameters(RHICmdList, View);
              SetShaderParameters(RHICmdList, PixelShader,
                                  PixelShader.GetPixelShader(),
                                  *PostProcessMaterialParameters);
            });
      });

  // Now that we have computed the intensity texture, we can pass this to the
  // LidarPointCloud compute shader as input and add its pass.

  TShaderMapRef<FLidarPointCloudCS> LidarPointCloudShader(
      GetGlobalShaderMap(GMaxRHIFeatureLevel));

  auto NumPoints = cachedParams.NumCams * cachedParams.HorizontalResolution *
                  cachedParams.LaserNums;
  auto BufferSize = NumPoints * sizeof(float) * 4;

  float* InitialData = new float[NumPoints * 4];
  FRDGBufferRef PointCloudBufferRDG =
      CreateStructuredBuffer(GraphBuilder,  // Our FRDGBuilder
                             TEXT("FLidarPointCloudCS_PointCloudBuffer_"
                                  "StructuredBuffer"),  // The name of this
                                                        // buffer (for debug
                                                        // purposes)
                             sizeof(float),  // The size of a single element
                             NumPoints * 4, InitialData, BufferSize);
  FRDGBufferUAVRef PointCloudBufferUAV = GraphBuilder.CreateUAV(
      PointCloudBufferRDG, PF_FloatRGBA, ERDGUnorderedAccessViewFlags::None);

  FLidarPointCloudCS::FParameters* PassParameters =
      GraphBuilder.AllocParameters<FLidarPointCloudCS::FParameters>();
  PassParameters->PointCloudBuffer = PointCloudBufferUAV;
  PassParameters->HorizontalResolution = cachedParams.HorizontalResolution;
  PassParameters->LaserNums = cachedParams.LaserNums;
  PassParameters->LaserRange = cachedParams.LaserRange;
  PassParameters->CurrentHorizontalAngleDeg =
      cachedParams.CurrentHorizontalAngleDeg;
  PassParameters->HorizontalFOV = cachedParams.HorizontalFOV;
  PassParameters->VerticalFOV = cachedParams.VerticalFOV;
  PassParameters->CamFrustrumHeight = cachedParams.CamFrustrumHeight;
  PassParameters->CamFrustrumWidth = cachedParams.CamFrustrumWidth;
  PassParameters->ProjectionMatrixInv =
      cachedParams.ViewProjectionMatInv.GetTransposed();

  PassParameters->CamRotationMatrix1 = cachedParams.RotationMatCam1;
  PassParameters->CamRotationMatrix2 = cachedParams.RotationMatCam2;
  PassParameters->CamRotationMatrix3 = cachedParams.RotationMatCam3;
  PassParameters->CamRotationMatrix4 = cachedParams.RotationMatCam4;

  PassParameters->DepthImage1 = cachedParams.DepthTexture1;
  PassParameters->DepthImage2 = IntensityRenderTarget.Texture;
  PassParameters->DepthImage3 = cachedParams.DepthTexture3;
  PassParameters->DepthImage4 = cachedParams.DepthTexture4;

  FSceneViewProjectionData ProjData;
  ProjData.ViewOrigin =
      FVector(0.f);  // camera space, should always be the origin
  // Apply rotation matrix of camera's pose plus the rotation matrix for
  // converting Unreal's world axes to the camera's view axes (z forward, etc).
  ProjData.ViewRotationMatrix = FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0),
                                        FPlane(0, 1, 0, 0), FPlane(0, 0, 0, 1));
  ProjData.ProjectionMatrix = cachedParams.ProjectionMat;
  ProjData.SetConstrainedViewRectangle(FIntRect(
      0, 0, cachedParams.CamFrustrumWidth, cachedParams.CamFrustrumHeight));
  PassParameters->ProjectionMatrix =
      FMatrix44f(ProjData.ComputeViewProjectionMatrix().GetTransposed());

  FIntVector GroupContext(
      cachedParams.NumCams * cachedParams.HorizontalResolution *
          cachedParams.LaserNums / 1024,
      NUM_THREADS_PER_GROUP_DIMENSION_Y, NUM_THREADS_PER_GROUP_DIMENSION_Z);

  LidarPointCloudData =
      std::vector<FVector4>(NumPoints, FVector4(-1, -1, -1, -1));

  FComputeShaderUtils::AddPass(
      GraphBuilder, RDG_EVENT_NAME("LidarPointCloud Pass"),
      LidarPointCloudShader, PassParameters, GroupContext);

    FCopyBufferToCPUPass* CopyPassParameters =
        GraphBuilder.AllocParameters<FCopyBufferToCPUPass>();
    CopyPassParameters->Buffer = PointCloudBufferRDG;

    GraphBuilder.AddPass(
        RDG_EVENT_NAME("FCopyBufferToCPUPass"), CopyPassParameters,
        ERDGPassFlags::Readback,
        [this, &InitialData, PointCloudBufferRDG, BufferSize](FRHICommandList& RHICmdList) {
          InitialData = (float*)RHILockBuffer(PointCloudBufferRDG->GetRHI(), 0,
                                              BufferSize, RLM_ReadOnly);

          FMemory::Memcpy(LidarPointCloudData.data(), InitialData, BufferSize);

          RHIUnlockBuffer(PointCloudBufferRDG->GetRHI());
        });
}

void FLidarIntensitySceneViewExtension::UpdateParameters(
    FLidarPointCloudCSParameters& params) {
  CSParamsQ.push(params);
}

bool FLidarIntensitySceneViewExtension::IsActiveThisFrame_Internal(
    const FSceneViewExtensionContext& Context) const {
  return !CSParamsQ.empty();
}