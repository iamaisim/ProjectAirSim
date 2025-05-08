// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "core_sim/link/geometry/skeletal_mesh.hpp"
#include "Engine/SkeletalMesh.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "UnrealEnvActor.h"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealEnvCar.generated.h"

UCLASS()
class AUnrealEnvActorCar : public AUnrealEnvActor {
  GENERATED_BODY()

 public:
  explicit AUnrealEnvActorCar(const FObjectInitializer& ObjectInitializer);

  std::pair<std::string, UUnrealRobotLink*> CreateLink(
      const microsoft::projectairsim::Link& InLink, bool bIsRootLink) override;

  void InitializeSkeletalMesh(USkeletalMesh* skeletalMesh);

  virtual void Tick(float DeltaTime) override;

 protected:
  float wheel_radius = 0.0f;
  float wheels_distance = 0.0f;
  bool has_skeletal_mesh = false;
  microsoft::projectairsim::Vector3 scale = microsoft::projectairsim::Vector3::Ones();

  private:
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* SkeletalMeshComponent;

    UCapsuleComponent* CapsuleComponent;

    UPoseableMeshComponent* PoseableMeshComponent; // PoseableMeshComponent is used to scale the skeletal mesh

    float capsuleHalfHeight;

    void DetectZPositionwithStaticMesh(float wheel_rad);

    void DetectZPositionwithSkeletalMesh(float wheel_rad);

    void UpdateWheelRotation();

    TimeNano PrevSimTimeNanos = 0;

    float CurrentAngle = 0;
    std::vector<int32> FrontWheels;
    std::vector<int32> BackWheels;
};