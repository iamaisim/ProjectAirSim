#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"

// Exact supported engine versions.
#define UE_IS_5_2 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 2)
#define UE_IS_5_7 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 7)
#define UE_IS_5_8 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 8)

// UE 5.7 and 5.8 share the modern compatibility path.
#define UE_IS_5_7_OR_5_8 (UE_IS_5_7 || UE_IS_5_8)

// Convenience macro.
#define UE_IS_SUPPORTED (UE_IS_5_2 || UE_IS_5_7_OR_5_8)

// The engine supports Lumen in scene captures from UE 5.5 (it maintains a
// dedicated Lumen scene per opted-in capture). Older engines hard-disable
// Lumen for capture views, so the lumen-* capture settings are compiled out
// below 5.5 and captures keep the exact legacy behavior there.
#define UE_SUPPORTS_LUMEN_SCENE_CAPTURES \
  (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)
