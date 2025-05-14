// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once

#include "CoreMinimal.h"
// include character class
#include "GameFramework/Character.h"
#include "core_sim/actor/env_actor.hpp"
#include "core_sim/actor/env_actor_grounded.hpp"
#include "core_sim/link.hpp"
#include "core_sim/link/geometry/skeletal_mesh.hpp"
// comment so that generated.h is always the last include file with clang-format
#include "UnrealEnvHuman.generated.h"

UCLASS()
class AUnrealEnvHuman : public ACharacter {
  GENERATED_BODY()

 public:
  explicit AUnrealEnvHuman(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::EnvActorGrounded& InEnvActor);

  void InitializeSkeletalMesh(const microsoft::projectairsim::SkeletalMesh* skeletal_mesh);

  void Tick(float DeltaTime) override;

  void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other,
                 class UPrimitiveComponent* OtherComp, bool bSelfMoved,
                 FVector HitLocation, FVector HitNormal, FVector NormalImpulse,
                 const FHitResult& Hit);

 protected:
  microsoft::projectairsim::EnvActorGrounded SimEnvHuman;

 private:
  void InitializeId(const std::string& InId);

  void UpdateEnvActorTargetPose(const microsoft::projectairsim::Pose InPose,
                                TimeNano InTimestamp);

  void MoveEnvActorToTargetPose(bool bUseCollisionSweep);

  FVector GetSkeletalMeshZOffset();

  bool bHasTargetPoseUpdated = false;
  std::mutex PoseMutex;
  microsoft::projectairsim::Pose EnvActorTargetPose;

  FVector curr_position_;
  FVector prev_position_;
  FVector scale_;
  TimeNano PrevSimTimeNanos = 0;

  void updateAnimation(float Delta);

  UPROPERTY(VisibleAnywhere, Category = "Mesh")
  USkeletalMeshComponent* SkeletalMeshComponent;
};
