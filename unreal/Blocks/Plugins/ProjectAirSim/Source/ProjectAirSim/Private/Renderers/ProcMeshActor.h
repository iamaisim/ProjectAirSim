// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

// comment so that generated.h is always the last include file with clang-format
#include "ProcMeshActor.generated.h"

UCLASS()
class AProcMeshActor : public AActor {
  GENERATED_BODY()

 public:
  // Sets default values for this actor's properties
  AProcMeshActor();

  UPROPERTY(VisibleAnywhere)
  int lod;

  UPROPERTY(VisibleAnywhere)
  int x;

  UPROPERTY(VisibleAnywhere)
  int y;

  UPROPERTY(VisibleAnywhere)
  FString tilekey;

  void Init(int numMeshes, const FTransform& actor_transform,
            const std::vector<float>& scale, bool enable_physics);

  void UpdateMesh(int Index, const TArray<FVector>& Vertices,
                  const TArray<int32>& Triangles, const TArray<FVector2D>& UV0,
                  const TArray<FVector>& Normals,
                  const TArray<FProcMeshTangent>& Tangents,
                  FLinearColor BaseColor, FLinearColor EmissiveColor,
                  float MetallicFactor, float RoughnessFactor,
                  float SpecularFactor, UTexture2D* BaseColorTexture = nullptr,
                  UTexture2D* SpecularTexture = nullptr,
                  UTexture2D* EmissiveTexture = nullptr,
                  UTexture2D* RoughnessTexture = nullptr,
                  UTexture2D* NormalsTexture = nullptr,
                  UTexture2D* MetallicTexture = nullptr,
                  UTexture2D* MetallicRoughnessTexture = nullptr,
                  bool bTranslucentMaterial = false,
                  bool bCreateNormalsAndTangents = false,
                  bool bCreateCollision = true,
                  const FTransform& MeshTransform = FTransform());

 protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

 public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

 private:
  std::vector<UProceduralMeshComponent*> ProcMeshes;
};
