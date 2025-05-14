// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// Unreal RenderCommand

#pragma once

#include "CoreMinimal.h"

namespace UnrealCameraRenderRequest {

struct RenderResult {
  EPixelFormat Format;

  TArray<FColor> UnrealImage;
  TArray<FFloat16Color> UnrealImageFloat;

  int Width;
  int Height;

  TimeNano TimeStamp;
};

void OnEndPlay();

void OnBeginPlay();

void ReadPixels(FRHITexture2D* Texture, bool bPixelsAsFloat,
                RenderResult* ImageResult);

bool CompressUsingImageWrapper(const TArray<FColor>& UnCompressed,
                               const int32 Width, const int32 Height,
                               TArray<uint8>& Compressed);

}  // namespace UnrealCameraRenderRequest
