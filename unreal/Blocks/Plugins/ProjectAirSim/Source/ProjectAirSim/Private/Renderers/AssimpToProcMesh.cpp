// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "AssimpToProcMesh.h"

#include <queue>

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "ProcMeshActor.h"
#include "assimp/GltfMaterial.h"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "core_sim/math_utils.hpp"

UTexture2D* GetUnrealTexture(const aiTexture* embeddedTexture) {
  int totalSize = 0;

  if (embeddedTexture->mHeight == 0) {
    totalSize = embeddedTexture->mWidth;

  } else {
    totalSize =
        embeddedTexture->mWidth * embeddedTexture->mHeight * sizeof(aiTexel);
  }

  TArray<uint8> RawData;
  RawData.Reset(totalSize);

  // TODO Fix using FMemory::Memcpy() to do this with UE5 instead of manually
  // adding each element.
  auto PcDataPtr = reinterpret_cast<const uint8*>(embeddedTexture->pcData);
  for (int Idx = 0; Idx < totalSize; ++Idx) {
    RawData.Add(*PcDataPtr);
    PcDataPtr++;
  }
  // FMemory::Memcpy(RawData.GetData(), (uint8*)embeddedTexture->pcData,
  //                 totalSize);

  return FImageUtils::ImportBufferAsTexture2D(RawData);
}

UTexture2D* GetUnrealTexture(const FString& ImagePath) {
  TArray<uint8> FileData;
  if (!FFileHelper::LoadFileToArray(FileData, *ImagePath)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *ImagePath);
    return nullptr;
  }

  // Detect the image type using the ImageWrapper module
  IImageWrapperModule& ImageWrapperModule =
      FModuleManager::LoadModuleChecked<IImageWrapperModule>(
          FName("ImageWrapper"));

  EImageFormat ImageFormat =
      ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
  if (ImageFormat == EImageFormat::Invalid) {
    UE_LOG(LogTemp, Error, TEXT("Unrecognized image file format: %s"),
           *ImagePath);
    return nullptr;
  }

  // Create an image wrapper for the detected image format
  TSharedPtr<IImageWrapper> ImageWrapper =
      ImageWrapperModule.CreateImageWrapper(ImageFormat);
  if (!ImageWrapper.IsValid()) {
    UE_LOG(LogTemp, Error, TEXT("Failed to create image wrapper for file: %s"),
           *ImagePath);
    return nullptr;
  }

  // Decompress the image data
  TArray<uint8> RawData;
  ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num());
  ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData);

  if (RawData.Num() != 0) {
    UTexture2D* Texture = UTexture2D::CreateTransient(
        ImageWrapper->GetWidth(), ImageWrapper->GetHeight(),
        EPixelFormat::PF_R8G8B8A8);
    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];

    void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(MipData, RawData.GetData(), RawData.Num());
    Mip.BulkData.Unlock();

#undef UpdateResource
    Texture->UpdateResource();
#ifdef UNICODE
#define UpdateResource UpdateResourceW
#else
#define UpdateResource UpdateResourceA
#endif  // !UNICODE
    return Texture;
  }

  return nullptr;
}

void AssimpToProcMesh::UpdateProcMesh(const aiScene* scene,
                                      AProcMeshActor* RuntimeActor) {
  std::queue<aiNode*> NodesToParse;
  NodesToParse.push(scene->mRootNode);

  std::queue<aiMatrix4x4t<float>> ParentTransforms;

  // Rotate to convert LH glTF +Y up to UE +Z up
  aiMatrix4x4t<float> RootTransform;
  aiMatrix4x4t<float>::RotationX(M_PI / 2.0, RootTransform);

  ParentTransforms.push(RootTransform);

  while (!NodesToParse.empty()) {
    const aiNode* CurNode = NodesToParse.front();
    NodesToParse.pop();
    const aiMatrix4x4t<float> CurTransform =
        ParentTransforms.front() * CurNode->mTransformation;
    ParentTransforms.pop();

    for (unsigned int i = 0; i < CurNode->mNumChildren; ++i) {
      NodesToParse.push(CurNode->mChildren[i]);
      ParentTransforms.push(CurTransform);
    }

    for (unsigned int i = 0; i < CurNode->mNumMeshes; ++i) {
      TArray<FVector> Vertices;
      TArray<FVector> Normals;
      TArray<FProcMeshTangent> Tangents;
      TArray<FVector2D> TextureCoordinates;
      TArray<int32> Triangles;
      TArray<FLinearColor> VertexColors;
      int SectionIndex = 0;

      auto mesh = scene->mMeshes[CurNode->mMeshes[i]];
      int trianglesCount = 0;
      for (int j = 0; j < (int)mesh->mNumFaces; ++j) {
        auto face = mesh->mFaces[j];
        trianglesCount += mesh->mNumFaces;
      }

      Vertices.Reserve((int)mesh->mNumVertices);
      Triangles.Reserve(trianglesCount);
      Normals.Reserve((int)mesh->mNumVertices);
      Tangents.Reserve((int)mesh->mNumVertices);
      TextureCoordinates.Reserve((int)mesh->mNumVertices);
      VertexColors.Reserve((int)mesh->mNumVertices);

      auto AssimpMat = scene->mMaterials[mesh->mMaterialIndex];

      // Process base PBR material data

      aiString MatName = AssimpMat->GetName();

      bool bDoubleSided = false;
      AssimpMat->Get(AI_MATKEY_TWOSIDED, bDoubleSided);

      aiBlendMode BlendMode;
      AssimpMat->Get(AI_MATKEY_BLEND_FUNC, BlendMode);

      aiString AlphaMode;
      AssimpMat->Get(AI_MATKEY_GLTF_ALPHAMODE, AlphaMode);
      bool bUseTranslucent = AlphaMode == aiString("BLEND");

      aiColor4D BaseColor;
      AssimpMat->Get(AI_MATKEY_BASE_COLOR, BaseColor);
      FLinearColor UnrealColorFactor(BaseColor.r, BaseColor.g, BaseColor.b,
                                     BaseColor.a);

      aiColor4D EmissiveColor;
      AssimpMat->Get(AI_MATKEY_COLOR_EMISSIVE, EmissiveColor);
      FLinearColor UnrealEmissiveColor(EmissiveColor.r, EmissiveColor.g,
                                       EmissiveColor.b, EmissiveColor.a);

      float MetallicFactor = 0.0f;
      AssimpMat->Get(AI_MATKEY_METALLIC_FACTOR, MetallicFactor);

      float RoughnessFactor = 1.0f;
      AssimpMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, RoughnessFactor);

      float SpecularFactor = 0.5f;
      AssimpMat->Get(AI_MATKEY_SPECULAR_FACTOR, SpecularFactor);

      // Process texture data

      UTexture2D* DiffuseTex = nullptr;
      UTexture2D* SpecularTex = nullptr;
      UTexture2D* EmissiveTex = nullptr;
      UTexture2D* RoughnessTex = nullptr;
      UTexture2D* NormalsTex = nullptr;
      UTexture2D* MetallicTex = nullptr;
      UTexture2D* MetallicRoughnessTex = nullptr;

      aiString reltexPath;
      for (aiTextureType type :
           {aiTextureType_BASE_COLOR, aiTextureType_SPECULAR,
            aiTextureType_EMISSIVE, aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_NORMALS, aiTextureType_METALNESS,
            aiTextureType_UNKNOWN}) {
        // has texture
        if (AssimpMat->GetTextureCount(type) > 0) {
          AssimpMat->GetTexture(type, 0, &reltexPath);

          UTexture2D* tex = nullptr;

          const aiTexture* embeddedTexture =
              scene->GetEmbeddedTexture(reltexPath.C_Str());

          if (embeddedTexture) {
            tex = GetUnrealTexture(embeddedTexture);
          }

          if (type == aiTextureType_BASE_COLOR) {
            DiffuseTex = tex;
          }
          if (type == aiTextureType_SPECULAR) {
            SpecularTex = tex;
          }
          if (type == aiTextureType_EMISSIVE) {
            EmissiveTex = tex;
          }
          if (type == aiTextureType_DIFFUSE_ROUGHNESS) {
            RoughnessTex = tex;
          }
          if (type == aiTextureType_NORMALS) {
            NormalsTex = tex;
          }
          if (type == aiTextureType_METALNESS) {
            MetallicTex = tex;
          }
          if (type == aiTextureType_UNKNOWN) {
            // glTF
            // AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE
            // aliases to aiTextureType_UNKNOWN
            MetallicRoughnessTex = tex;
          }
        }
      }

      // Create the vertex
      int32 NumVertex = mesh->mNumVertices;
      TMap<int32, FVector> VertexPositions;
      VertexPositions.Reserve(NumVertex);
      const bool bHasNormals = (mesh->mNormals != nullptr);
      const bool bHasTangents = (mesh->mTangents != nullptr);
      for (int32 VertexIndex = 0; VertexIndex < NumVertex; ++VertexIndex) {
        VertexPositions.Add(VertexIndex,
                            FVector(mesh->mVertices[VertexIndex].x,
                                    mesh->mVertices[VertexIndex].y,
                                    mesh->mVertices[VertexIndex].z));
        Vertices.Push(FVector(mesh->mVertices[VertexIndex].x,
                              mesh->mVertices[VertexIndex].y,
                              mesh->mVertices[VertexIndex].z));

        if (bHasNormals) {
          Normals.Push(FVector(mesh->mNormals[VertexIndex].x,
                               mesh->mNormals[VertexIndex].y,
                               mesh->mNormals[VertexIndex].z));
        }

        if (bHasTangents) {
          Tangents.Push(FProcMeshTangent(mesh->mTangents[VertexIndex].x,
                                         mesh->mTangents[VertexIndex].y,
                                         mesh->mTangents[VertexIndex].z));
        }

        if ((int)mesh->GetNumColorChannels() > 0) {
          VertexColors.Push(FLinearColor(mesh->mColors[0][VertexIndex].r,
                                         mesh->mColors[0][VertexIndex].g,
                                         mesh->mColors[0][VertexIndex].b,
                                         mesh->mColors[0][VertexIndex].a));
        } else {
          VertexColors.Push(FLinearColor::White);
        }
        if (mesh->mTextureCoords != nullptr &&
            mesh->mTextureCoords[0] != nullptr) {
          TextureCoordinates.Push(
              FVector2D(mesh->mTextureCoords[0][VertexIndex].x,
                        mesh->mTextureCoords[0][VertexIndex].y));
        }
      }
      for (int j = 0; j < (int)mesh->mNumFaces; ++j) {
        auto face = mesh->mFaces[j];
        for (int32 index = 0; index < (int)face.mNumIndices; index++) {
          Triangles.Push(face.mIndices[index]);
        }
      }

      // Convert assimp transform matrix to UE FTransform
      // (aiMatrix4x4 is row-major, FMatrix is column-major)
      FMatrix CurTransformMatrix;
      for (int m = 0; m < 4; ++m) {
        for (int n = 0; n < 4; ++n) {
          CurTransformMatrix.M[n][m] = CurTransform[m][n];
        }
      }
      const FTransform MeshTransform(CurTransformMatrix);

      const bool bNeedsNormalsOrTangents = (!bHasNormals || !bHasTangents);
      RuntimeActor->UpdateMesh(
          CurNode->mMeshes[i], Vertices, Triangles, TextureCoordinates, Normals,
          Tangents, UnrealColorFactor, UnrealEmissiveColor, MetallicFactor,
          RoughnessFactor, SpecularFactor, DiffuseTex, SpecularTex, EmissiveTex,
          RoughnessTex, NormalsTex, MetallicTex, MetallicRoughnessTex,
          bUseTranslucent, bNeedsNormalsOrTangents, true, MeshTransform);
    }
  }
}
