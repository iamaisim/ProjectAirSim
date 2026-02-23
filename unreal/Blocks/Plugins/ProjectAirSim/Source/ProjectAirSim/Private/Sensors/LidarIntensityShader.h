#pragma once

#include "CoreMinimal.h"
#include "UnrealCameraRenderRequest.h"
#include "Engine/TextureRenderTarget2D.h"

#include "RHI.h"
#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include "RenderResource.h"
#include "MaterialShader.h"
#include "RenderGraphResources.h"

// FScreenPassTextureViewportParameters and FScreenPassTextureInput
#include "ScreenPass.h"
#include "SceneTextureParameters.h"

BEGIN_SHADER_PARAMETER_STRUCT(FLidarIntensityShaderInputParameters, )
  SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
  SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
  SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, PostProcessOutput)
  RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

/// <summary>
/// Class that holds the parameters to and declares the Lidar Intensity HLSL
/// shader.
/// </summary>
class FLidarIntensityShader : public FGlobalShader {

 public:
  using FParameters = FLidarIntensityShaderInputParameters;
  SHADER_USE_PARAMETER_STRUCT_WITH_LEGACY_BASE(FLidarIntensityShader,
                                               FGlobalShader);

  /// Makes sure only shaders for post processing materials get compiled into
  /// the shader cache.
  static bool ShouldCompilePermutation(
      const FGlobalShaderPermutationParameters& Parameters) {
    // return Parameters.MaterialParameters.MaterialDomain == MD_PostProcess; // TODO ?
    return !IsMobilePlatform(Parameters.Platform);
  }
};

class FLidarIntensityPS : public FLidarIntensityShader {
  DECLARE_GLOBAL_SHADER(FLidarIntensityPS);

 public:
  FLidarIntensityPS() {}
  FLidarIntensityPS(
      const ShaderMetaType::CompiledShaderInitializerType& Initializer)
      : FLidarIntensityShader(Initializer) {}

  void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FSceneView& View) {
    FGlobalShader::SetParameters<FViewUniformShaderParameters>(
        BatchedParameters, View.ViewUniformBuffer);
  }

  static void ModifyCompilationEnvironment(
      const FGlobalShaderPermutationParameters& Parameters,
      FShaderCompilerEnvironment& OutEnvironment) {
    FLidarIntensityShader::ModifyCompilationEnvironment(Parameters,
                                                        OutEnvironment);
  }
};

class FLidarIntensityVS : public FLidarIntensityShader {
  DECLARE_GLOBAL_SHADER(FLidarIntensityVS);

 public:
  FLidarIntensityVS() = default;
  FLidarIntensityVS(
      const ShaderMetaType::CompiledShaderInitializerType& Initializer)
      : FLidarIntensityShader(Initializer) {}

  void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FSceneView& View) {
    FGlobalShader::SetParameters<FViewUniformShaderParameters>(
        BatchedParameters, View.ViewUniformBuffer);
  }
};