// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "ImagePackingAsyncTask.h"

#include <map>

#include "core_sim/message/image_message.hpp"

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
        (ImageRequest.ImageType ==
             projectairsim::ImageType::kDepthPerspective ||
         ImageRequest.ImageType == projectairsim::ImageType::kDepthPlanar);

    // Handle Depth image requests here.
    // 1. Currently, we do not support Compression for depth images
    // 2. We also do not support sending back Floats since our ImageResponse
    // Message is limited to uint8 for now. Instead, we convert Float16 from
    // Unreal to uint16 and then pack it in two uint8s that have to be unpacked
    // properly on client side. NOTE: This case also handles PixelsAsFloat
    // implicitly i.e. it ignores it and always sends back uint16 for depth

    if (bIsDepthImage && !ImageRequest.bCompress) {
      static constexpr float MAX_DEPTH = UINT16_MAX;  // TODO move to config?

      ImgResponse.ImageDataFloat.resize(RenderResult.Width *
                                        RenderResult.Height);

      float* DstPtr = ImgResponse.ImageDataFloat.data();
      for (const auto& SrcPixel : RenderResult.UnrealImageFloat) {
        float DepthMilli = SrcPixel.R.GetFloat();
        *DstPtr++ = DepthMilli;
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
      // 32-bit unsigned, 1 channel for depth in mm
      ImgEncoding = "32FC1";
    }

    ImageMessages.emplace(
        ImgResponse.ImageType,
        projectairsim::ImageMessage(
            ImgResponse.TimeStamp, ImgResponse.Height, ImgResponse.Width,
            ImgEncoding, false, 1, std::move(ImgResponse.ImageDataUInt8),
            std::move(ImgResponse.ImageDataFloat),
            ImgResponse.CameraPosition.x(), ImgResponse.CameraPosition.y(),
            ImgResponse.CameraPosition.z(), ImgResponse.CameraOrientation.w(),
            ImgResponse.CameraOrientation.x(),
            ImgResponse.CameraOrientation.y(),
            ImgResponse.CameraOrientation.z(), Annotations
            ));
  }

  // Publish the whole pack of image messages
  Camera.PublishImages(std::move(ImageMessages));
}