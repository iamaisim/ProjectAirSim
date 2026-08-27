// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "ImagePackingAsyncTask.h"

#include <map>

#include "core_sim/message/image_message.hpp"
#include "UnrealLogger.h"

namespace projectairsim = microsoft::projectairsim;

void FImagePackingAsyncTask::DoWork() {
  std::map<projectairsim::ImageType, projectairsim::ImageMessage> ImageMessages;

  for (auto& [ImageRequest, RenderResult] : CaptureResults) {
    // --- Make image response from the image request and render result ---
    ImageResponse ImgResponse;
    ImgResponse.CameraPosition = CapturedCameraTransform.translation_;
    ImgResponse.CameraOrientation = CapturedCameraTransform.rotation_;
    ImgResponse.TimeStamp = CapturedCameraTransform.timestamp_;

    bool bIsDepthImage =
        (ImageRequest.ImageType == projectairsim::ImageType::kDepthPerspective ||
         ImageRequest.ImageType == projectairsim::ImageType::kDepthPlanar);

    // Handle Depth image requests here.
    // 1. Currently, we do not support Compression for depth images
    // 2. Depth is transmitted as the render target's IEEE 754 half-precision
    // (binary16) METERS, bit-exact: each pixel's FFloat16 bit pattern is
    // packed little-endian into two uint8s (encoding "16FC1" below) and
    // reinterpreted as float16 on the client side. No value conversion
    // happens here, so no precision is lost beyond the fp16 render target
    // itself, and there is no range cap: sky / no-hit pixels arrive as +inf
    // for clients to map to their own invalid-depth convention. NOTE: This
    // case also handles PixelsAsFloat implicitly i.e. it ignores it and
    // always sends back float16 for depth
    if (bIsDepthImage && !ImageRequest.bCompress) {
      ImgResponse.ImageDataUInt8.resize(
          RenderResult.Width * RenderResult.Height * 2 * sizeof(uint8));

      uint8* DstPtr = ImgResponse.ImageDataUInt8.data();
      for (const auto& SrcPixel : RenderResult.UnrealImageFloat) {
        // The depth materials write METERS to R; transmit the fp16 bit
        // pattern as-is (see the encoding comment above).
        const uint16 DepthHalfBits = SrcPixel.R.Encoded;
        *DstPtr++ = static_cast<uint8>(DepthHalfBits & 0xFF);        // least significant byte
        *DstPtr++ = static_cast<uint8>((DepthHalfBits >> 8) & 0xFF);  // most significant byte
      }
    }
    // Normal RGB images without compression or PixelsAsFloat requested
    else if (!bIsDepthImage && !ImageRequest.bPixelsAsFloat &&
             !ImageRequest.bCompress) {
      // Copy pixels to std::vector response:
      //   RenderResult.UnrealImage (TArray<FColor> BGRA aligned struct) ->
      //     ImgResponse.ImageDataUInt8 (std::vector<uint8_t> BGR)

      ImgResponse.ImageDataUInt8.resize(
          RenderResult.Width * RenderResult.Height * 3 * sizeof(uint8));

      uint8* DstPtr = ImgResponse.ImageDataUInt8.data();
      for (const auto& SrcPixel : RenderResult.UnrealImage) {
        *DstPtr++ = SrcPixel.B;
        *DstPtr++ = SrcPixel.G;
        *DstPtr++ = SrcPixel.R;
      }
    }
    // Normal RGB images with compression requested but not PixelsAsFloat
    else if (!bIsDepthImage && !ImageRequest.bPixelsAsFloat &&
             ImageRequest.bCompress) {
      // Do compression and copy to std::vector response:
      //   RenderResult.UnrealImage (TArray<FColor> BGRA aligned struct) ->
      //     CompressedArray (TArray<uint8> RGBA PNG) ->
      //     ImgResponse.ImageDataUInt8 (std::vector<uint8_t> RGBA PNG)

      // TODO Move PNG compression to inside sim?
      TArray<uint8> CompressedArray;
      UnrealCameraRenderRequest::CompressUsingImageWrapper(
          RenderResult.UnrealImage, RenderResult.Width, RenderResult.Height,
          CompressedArray);

      ImgResponse.ImageDataUInt8 = std::vector<uint8_t>(
          CompressedArray.GetData(),
          CompressedArray.GetData() + CompressedArray.Num());

    }
    // TODO: Since this does not respect PixelsAsFloat anyway, should we delete
    // the PixelsAsFloat option completely? Normal RGB images with FloatsAsPixel
    // requested but not compression
    else if (!bIsDepthImage && ImageRequest.bPixelsAsFloat &&
             !ImageRequest.bCompress) {
      // TODO Send image as exponent/mantissa/sign to be converted back to
      // float at client instead of converting to BGR8 here?
      // TODO Support compression with pixels as float?

      // Convert pixels from float16 to uint8 std::vector response:
      //   RenderResult.UnrealImageFloat (TArray<FFloat16Color> RGBA struct)->
      //     ImgResponse.ImageDataUInt8 (std::vector<uint8_t> BGR)
      ImgResponse.ImageDataUInt8.resize(
          RenderResult.Width * RenderResult.Height * 3 * sizeof(uint8));

      uint8* DstPtr = ImgResponse.ImageDataUInt8.data();
      for (const auto& SrcPixel : RenderResult.UnrealImageFloat) {
        *DstPtr++ = static_cast<uint8_t>(SrcPixel.B.GetFloat() * 255);
        *DstPtr++ = static_cast<uint8_t>(SrcPixel.G.GetFloat() * 255);
        *DstPtr++ = static_cast<uint8_t>(SrcPixel.R.GetFloat() * 255);
      }
    }
    // Unsupported combos right now
    // 1. PixelsAsFloat and Compression together
    // 2. DepthImages and Compression
    else {
      UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                        TEXT("[FImagePackingAsyncTask] Unsupport combination "
                             "of camera options."));
    }

    ImgResponse.CameraName = ImageRequest.CameraName;
    ImgResponse.bPixelsAsFloat = ImageRequest.bPixelsAsFloat;
    ImgResponse.bCompress = ImageRequest.bCompress;
    ImgResponse.Width = RenderResult.Width;
    ImgResponse.Height = RenderResult.Height;
    ImgResponse.ImageType = ImageRequest.ImageType;

    // --- Make image message from the image response ---
    std::string ImgEncoding;
    if (!bIsDepthImage) {
      if (ImgResponse.bCompress) {
        ImgEncoding = "PNG";
      } else {
        ImgEncoding = "BGR";
      }
    } else {  // bIsDepthImage
      // IEEE 754 half-precision (binary16), 1 channel, depth in METERS,
      // little-endian — bit-exact with the fp16 render target
      ImgEncoding = "16FC1";
    }

    ImageMessages.emplace(
        ImgResponse.ImageType,
        projectairsim::ImageMessage(
            ImgResponse.TimeStamp, ImgResponse.Height, ImgResponse.Width,
            ImgEncoding, false, 1, std::move(ImgResponse.ImageDataUInt8),
            ImgResponse.CameraPosition.x(), ImgResponse.CameraPosition.y(),
            ImgResponse.CameraPosition.z(), ImgResponse.CameraOrientation.w(),
            ImgResponse.CameraOrientation.x(),
            ImgResponse.CameraOrientation.y(),
            ImgResponse.CameraOrientation.z(),
            Annotations));
  }

  // Publish the whole pack of image messages
  Camera.PublishImages(std::move(ImageMessages));
}
