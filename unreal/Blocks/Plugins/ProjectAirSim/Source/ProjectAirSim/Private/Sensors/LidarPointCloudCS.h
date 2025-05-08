// Copyright (C) Microsoft Corporation.  All rights reserved.
#pragma once

#include <queue>

#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION == 5
  #if ENGINE_MINOR_VERSION >= 2
    #include "DataDrivenShaderPlatformInfo.h"
  #endif
#endif
#include "GlobalShader.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "ShaderParameterStruct.h"
#include "UnrealCameraRenderRequest.h"

#define NUM_THREADS_PER_GROUP_DIMENSION_X 1024
#define NUM_THREADS_PER_GROUP_DIMENSION_Y 1
#define NUM_THREADS_PER_GROUP_DIMENSION_Z 1

/// <summary>
/// Internal class that holds the parameters and connects the HLSL Shader to the
/// engine
/// </summary>
class FLidarPointCloudCS : public FGlobalShader {
 public:
  // Declare this class as a global shader
  DECLARE_GLOBAL_SHADER(FLidarPointCloudCS);
  // Tells the engine that this shader uses a structure for its parameters
  SHADER_USE_PARAMETER_STRUCT(FLidarPointCloudCS, FGlobalShader);
  /// <summary>
  /// DECLARATION OF THE PARAMETER STRUCTURE
  /// The parameters must match the parameters in the HLSL code
  /// For each parameter, provide the C++ type, and the name (Same name used in
  /// HLSL code)
  /// </summary>
  BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
  SHADER_PARAMETER_TEXTURE(Texture2D<float4>, DepthImage1)
  SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, DepthImage2)
  SHADER_PARAMETER_TEXTURE(Texture2D<float4>, DepthImage3)
  SHADER_PARAMETER_TEXTURE(Texture2D<float4>, DepthImage4)
  SHADER_PARAMETER(FMatrix44f, CamRotationMatrix1)
  SHADER_PARAMETER(FMatrix44f, CamRotationMatrix2)
  SHADER_PARAMETER(FMatrix44f, CamRotationMatrix3)
  SHADER_PARAMETER(FMatrix44f, CamRotationMatrix4)
  SHADER_PARAMETER(unsigned int, HorizontalResolution)
  SHADER_PARAMETER(unsigned int, LaserNums)
  SHADER_PARAMETER(float, LaserRange)
  SHADER_PARAMETER(float, HorizontalFOV)
  SHADER_PARAMETER(float, CurrentHorizontalAngleDeg)
  SHADER_PARAMETER(float, VerticalFOV)
  SHADER_PARAMETER(unsigned int, CamFrustrumWidth)
  SHADER_PARAMETER(unsigned int, CamFrustrumHeight)
  SHADER_PARAMETER(FMatrix44f, ProjectionMatrix)
  SHADER_PARAMETER(FMatrix44f, ProjectionMatrixInv)
  SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float>, PointCloudBuffer)
  // SHADER_PARAMETER_UAV(RWStructuredBuffer<float>, PointCloudBuffer)
  END_SHADER_PARAMETER_STRUCT()

 public:
  // Called by the engine to determine which permutations to compile for this
  // shader
  static bool ShouldCompilePermutation(
      const FGlobalShaderPermutationParameters& Parameters) {
    return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
  }

  // Modifies the compilations environment of the shader
  static inline void ModifyCompilationEnvironment(
      const FGlobalShaderPermutationParameters& Parameters,
      FShaderCompilerEnvironment& OutEnvironment) {
    FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

    // We're using it here to add some preprocessor defines. That way we don't
    // have to change both C++ and HLSL code when we change the value for
    // NUM_THREADS_PER_GROUP_DIMENSION
    OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"),
                             NUM_THREADS_PER_GROUP_DIMENSION_X);
    OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"),
                             NUM_THREADS_PER_GROUP_DIMENSION_Y);
    OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"),
                             NUM_THREADS_PER_GROUP_DIMENSION_Z);
  }
};

// This struct act as a container for all the parameters that the client needs
// to pass to the Compute Shader Manager.
struct FLidarPointCloudCSParameters {
  FLidarPointCloudCSParameters() {}

 private:
 public:
  FMatrix44f RotationMatCam1;
  FMatrix44f RotationMatCam2;
  FMatrix44f RotationMatCam3;
  FMatrix44f RotationMatCam4;

  FTextureRHIRef DepthTexture1;
  FRDGTextureRef DepthTexture2;
  FTextureRHIRef DepthTexture3;
  FTextureRHIRef DepthTexture4;

  uint32 HorizontalResolution;  // horizontal width
  uint32 LaserNums;             // vertical height
  float LaserRange;

  uint32 CamFrustrumWidth;
  uint32 CamFrustrumHeight;
  float CurrentHorizontalAngleDeg;
  float HorizontalFOV;
  float VerticalFOV;

  FMatrix ProjectionMat;
  FMatrix44f ViewProjectionMatInv;

  int NumCams = 1;
};