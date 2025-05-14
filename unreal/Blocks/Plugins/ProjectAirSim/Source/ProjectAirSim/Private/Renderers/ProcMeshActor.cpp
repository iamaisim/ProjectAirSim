// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "ProcMeshActor.h"

#include "KismetProceduralMeshLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AProcMeshActor::AProcMeshActor() {
  // Set this actor to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProcMeshRoot"));
}

void AProcMeshActor::Init(int numMeshes, const FTransform& actor_transform,
                          const std::vector<float>& scale,
                          bool enable_physics) {
  for (int i = 0; i < numMeshes; i++) {
    std::string name = std::string("ProcMesh") + std::to_string(i);
    UProceduralMeshComponent* ProcMesh =
        NewObject<UProceduralMeshComponent>(this, name.c_str());
    ProcMesh->AttachToComponent(
        this->RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    ProcMesh->SetRelativeLocation(FVector(0, 0, 0));
    ProcMesh->SetHiddenInGame(false, true);
    ProcMesh->SetMobility(EComponentMobility::Movable);
    ProcMesh->RegisterComponent();
    ProcMesh->SetSimulatePhysics(enable_physics);
    ProcMesh->bUseAsyncCooking = true;
    ProcMeshes.push_back(ProcMesh);
  }

  SetActorTransform(actor_transform);
  RootComponent->SetWorldScale3D(
      FVector(scale.at(0), scale.at(1), scale.at(2)));
}

void AProcMeshActor::UpdateMesh(
    int Index, const TArray<FVector>& Vertices, const TArray<int32>& Triangles,
    const TArray<FVector2D>& UV0, const TArray<FVector>& Normals,
    const TArray<FProcMeshTangent>& Tangents, FLinearColor BaseColor,
    FLinearColor EmissiveColor, float MetallicFactor, float RoughnessFactor,
    float SpecularFactor, UTexture2D* BaseColorTexture,
    UTexture2D* SpecularTexture, UTexture2D* EmissiveTexture,
    UTexture2D* RoughnessTexture, UTexture2D* NormalsTexture,
    UTexture2D* MetallicTexture, UTexture2D* MetallicRoughnessTexture,
    bool bTranslucentMaterial, bool bCreateNormalsAndTangents,
    bool bCreateCollision, const FTransform& MeshTransform) {
  // Create a proc mesh from the mesh data
  int32 SectionIndex = 0;
  TArray<FColor> VertexColors;

  if (bCreateNormalsAndTangents) {
    TArray<FVector> GeneratedNormals;
    TArray<FProcMeshTangent> GeneratedTangents;
    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
        Vertices, Triangles, UV0, GeneratedNormals, GeneratedTangents);

    ProcMeshes[Index]->CreateMeshSection(SectionIndex, Vertices, Triangles,
                                         GeneratedNormals, UV0, VertexColors,
                                         GeneratedTangents, bCreateCollision);
  } else {
    ProcMeshes[Index]->CreateMeshSection(SectionIndex, Vertices, Triangles,
                                         Normals, UV0, VertexColors, Tangents,
                                         bCreateCollision);
  }

  ProcMeshes[Index]->SetRelativeTransform(MeshTransform);

  // Load a dyynamic instance of a template parameterized material and set
  // it on the proc mesh
  UMaterial* MatTemplate;

  if (bTranslucentMaterial) {
    MatTemplate = Cast<UMaterial>(StaticLoadObject(
        UMaterial::StaticClass(), nullptr,
        TEXT("/ProjectAirSim/Runtime/"
             "RuntimeMeshTranslucentMaterial.RuntimeMeshTranslucentMaterial")));
  } else {
    MatTemplate = Cast<UMaterial>(StaticLoadObject(
        UMaterial::StaticClass(), nullptr,
        TEXT(
            "/ProjectAirSim/Runtime/RuntimeMeshMaterial.RuntimeMeshMaterial")));
  }

  if (MatTemplate != nullptr) {
    UMaterialInstanceDynamic* ProcMat =
        UMaterialInstanceDynamic::Create(MatTemplate, ProcMeshes[Index]);

    if (BaseColorTexture) {
      ProcMat->SetTextureParameterValue("BaseColorTex", BaseColorTexture);
      ProcMat->SetScalarParameterValue("BaseColorTexRatio", 1.0f);
    } else {
      ProcMat->SetVectorParameterValue("BaseColor", BaseColor);
      ProcMat->SetScalarParameterValue("BaseColorTexRatio", 0.0f);
    }

    if (MetallicRoughnessTexture) {
      ProcMat->SetTextureParameterValue("MetallicRoughnessTex",
                                        MetallicRoughnessTexture);
      ProcMat->SetScalarParameterValue("MetallicRoughnessTexRatio", 1.0f);
    } else {
      if (MetallicTexture) {
        ProcMat->SetTextureParameterValue("MetallicTex", MetallicTexture);
        ProcMat->SetScalarParameterValue("MetallicTexRatio", 1.0f);
      } else {
        ProcMat->SetScalarParameterValue("MetallicFactor", MetallicFactor);
        ProcMat->SetScalarParameterValue("MetallicTexRatio", 0.0f);
      }

      if (RoughnessTexture) {
        ProcMat->SetTextureParameterValue("RoughnessTex", RoughnessTexture);
        ProcMat->SetScalarParameterValue("RoughnessTexRatio", 1.0f);
      } else {
        ProcMat->SetScalarParameterValue("RoughnessFactor", RoughnessFactor);
        ProcMat->SetScalarParameterValue("RoughnessTexRatio", 0.0f);
      }
    }

    if (SpecularTexture) {
      ProcMat->SetTextureParameterValue("SpecularTex", SpecularTexture);
      ProcMat->SetScalarParameterValue("SpecularTexRatio", 1.0f);
    } else {
      ProcMat->SetScalarParameterValue("SpecularFactor", SpecularFactor);
      ProcMat->SetScalarParameterValue("SpecularTexRatio", 0.0f);
    }

    if (EmissiveTexture) {
      ProcMat->SetTextureParameterValue("EmissiveTex", EmissiveTexture);
      ProcMat->SetScalarParameterValue("EmissiveTexRatio", 1.0f);
    } else {
      ProcMat->SetVectorParameterValue("EmissiveColor", EmissiveColor);
      ProcMat->SetScalarParameterValue("EmissiveTexRatio", 0.0f);
    }

    if (NormalsTexture) {
      ProcMat->SetTextureParameterValue("NormalsTex", NormalsTexture);
      ProcMat->SetScalarParameterValue("NormalsTexRatio", 1.0f);
    } else {
      ProcMat->SetScalarParameterValue("NormalsTexRatio", 0.0f);
    }

    ProcMeshes[Index]->SetMaterial(SectionIndex, ProcMat);
  }
}

// Called when the game starts or when spawned
void AProcMeshActor::BeginPlay() { Super::BeginPlay(); }

// Called every frame
void AProcMeshActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
