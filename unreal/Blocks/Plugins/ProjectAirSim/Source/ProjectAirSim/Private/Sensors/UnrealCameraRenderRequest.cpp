// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "UnrealCameraRenderRequest.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

namespace UnrealCameraRenderRequest {
static IImageWrapperModule* ImageWrapperModule = nullptr;

void OnBeginPlay() {
  ImageWrapperModule = &FModuleManager::LoadModuleChecked<IImageWrapperModule>(
      FName("ImageWrapper"));
}

void OnEndPlay() {
  // nothing to do for now
  ImageWrapperModule = nullptr;
}

void ReadPixels(FRHITexture2D* Texture, bool bPixelsAsFloat,
                RenderResult* ImageResult) {
  FRHICommandListImmediate& RHICmdList =
      GetImmediateCommandList_ForRenderCommand();

  ImageResult->Format = Texture->GetFormat();
  ImageResult->Width = Texture->GetSizeX();
  ImageResult->Height = Texture->GetSizeY();

  if (bPixelsAsFloat == false) {
    FReadSurfaceDataFlags Flags(RCM_UNorm, CubeFace_MAX);
    Flags.SetLinearToGamma(false);
    RHICmdList.ReadSurfaceData(
        Texture, FIntRect(0, 0, ImageResult->Width, ImageResult->Height),
        ImageResult->UnrealImage, Flags);
  } else {
    RHICmdList.ReadSurfaceFloatData(
        Texture, FIntRect(0, 0, ImageResult->Width, ImageResult->Height),
        ImageResult->UnrealImageFloat, CubeFace_PosX, 0, 0);
  }
}

static IImageWrapperModule* GetImageWrapperModule() {
  return ImageWrapperModule;
}

bool CompressUsingImageWrapper(const TArray<FColor>& UnCompressed,
                               const int32 Width, const int32 Height,
                               TArray<uint8>& Compressed) {
  bool bSucceeded = false;
  Compressed.Reset();
  if (UnCompressed.Num() > 0) {
    IImageWrapperModule* LocalImageWrapperModule = GetImageWrapperModule();
    if (LocalImageWrapperModule) {
      TSharedPtr<IImageWrapper> ImageWrapper =
          LocalImageWrapperModule->CreateImageWrapper(EImageFormat::PNG);
      if (ImageWrapper.IsValid() &&
          ImageWrapper->SetRaw(&UnCompressed[0],
                               UnCompressed.Num() * sizeof(FColor), Width,
                               Height, ERGBFormat::BGRA, 8)) {
        Compressed = ImageWrapper->GetCompressed();
        bSucceeded = true;
      }
    }
  }

  return bSucceeded;
}

}  // namespace UnrealCameraRenderRequest
