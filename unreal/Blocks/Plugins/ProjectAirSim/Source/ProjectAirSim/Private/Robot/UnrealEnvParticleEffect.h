// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once

#include "CoreMinimal.h"
// include character class
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "core_sim/actor/env_actor.hpp"
#include "core_sim/link.hpp"
#include "core_sim/link/geometry/unreal_mesh.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealEnvParticleEffect.generated.h"

UCLASS()
class AUnrealEnvParticleEffect : public AActor {
  GENERATED_BODY()

 public:
  explicit AUnrealEnvParticleEffect(
      const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::EnvObject& InEnvObject);

  void InitializeMesh(
      const microsoft::projectairsim::UnrealMesh* particle_mesh);

  void Tick(float DeltaTime) override;

 protected:
  microsoft::projectairsim::EnvObject SimEnvParticles;

 private:
  void InitializeId(const std::string& InId);

  void UpdateEnvActorTargetPose(const microsoft::projectairsim::Pose InPose,
                                TimeNano InTimestamp);

  void MoveEnvActorToTargetPose(bool bUseCollisionSweep);

  bool bHasTargetPoseUpdated = false;
  microsoft::projectairsim::Pose EnvActorTargetPose;
  TimeNano TargetPoseUpdatedTimeStamp = 0;

  TimeNano PrevSimTimeNanos = 0;

  int TickCount;

  UPROPERTY(VisibleAnywhere, Category = "Particles")
  UParticleSystemComponent* ParticleComponent;

  UPROPERTY(VisibleAnywhere, Category = "Particles")
  UNiagaraComponent* NiagaraComponent;

  UPROPERTY(VisibleAnywhere, Category = "Components")
  UBoxComponent* BoundingBoxComponent;
};
