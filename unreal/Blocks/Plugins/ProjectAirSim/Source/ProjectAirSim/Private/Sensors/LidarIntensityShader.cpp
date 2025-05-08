#include "LidarIntensityShader.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "ShaderCore.h"

// This will tell the engine to create the shader and where the shader entry
// point is.
IMPLEMENT_GLOBAL_SHADER(FLidarIntensityVS,
                        "/CustomShaders/LidarIntensityPS.usf",
                        "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FLidarIntensityPS,
                        "/CustomShaders/LidarIntensityPS.usf",
                        "MainPS", SF_Pixel);