#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"

// Exact supported versions
#define UE_IS_5_2 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 2)
#define UE_IS_5_7 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 7)

// Convenience macro
#define UE_IS_SUPPORTED (UE_IS_5_2 || UE_IS_5_7)
