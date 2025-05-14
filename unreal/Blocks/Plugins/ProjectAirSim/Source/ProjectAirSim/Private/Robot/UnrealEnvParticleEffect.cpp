// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// The Unreal EnvObject implementation.

#include "UnrealEnvParticleEffect.h"

#include <vector>

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "core_sim/actor/env_actor.hpp"
#include "core_sim/link.hpp"

namespace projectairsim = microsoft::projectairsim;

AUnrealEnvParticleEffect::AUnrealEnvParticleEffect(
    const FObjectInitializer& ObjectInitializer) {
  // Set this character to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  // Tick in PrePhysics to wake on every tick in case of using Unreal physics
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = true;

  TickCount = 0;

  ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(
      TEXT("ParticleComponent"));
  NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(
      TEXT("NiagaraParticleComponent"));

  BoundingBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundingBoxComponent"));
}

void AUnrealEnvParticleEffect::Initialize(
    const projectairsim::EnvObject& InEnvActor) {
  // Store ptrs to other corresponding components for this env object
  this->SimEnvParticles = InEnvActor;
  auto visuals = InEnvActor.GetVisual();
  auto particle_mesh = static_cast<const projectairsim::UnrealMesh*>(
      visuals.GetGeometry());

  InitializeMesh(particle_mesh);
  InitializeId(InEnvActor.GetID());

  // Scale for EnvParticles
  FVector fvector_scale_local_cur;
  FVector fvector_scale_world_cur;
  {
    // Verify if geometry is valid
    if (ParticleComponent) {
      fvector_scale_local_cur =
          ParticleComponent->GetRelativeScale3D();  // Get current local scaling
      fvector_scale_world_cur =
          ParticleComponent->GetComponentScale();  // Get current world scaling
    } else if (NiagaraComponent) {
      fvector_scale_local_cur =
          NiagaraComponent->GetRelativeScale3D();  // Get current local scaling
      fvector_scale_world_cur =
          NiagaraComponent->GetComponentScale();  // Get current world scaling
    }

    auto fvector_scale_world_new =
        (particle_mesh == nullptr)
            ? projectairsim::Vector3(1.0f, 1.0f, 1.0f)
            : particle_mesh->GetScale();  // Get desired world scaling

    float scale_x_local =
        projectairsim::MathUtils::IsApproximatelyZero(fvector_scale_world_cur.X)
            ? std::numeric_limits<float>::max()
            : fvector_scale_world_new.x() / fvector_scale_world_cur.X *
                  fvector_scale_local_cur.X;
    float scale_y_local =
        projectairsim::MathUtils::IsApproximatelyZero(fvector_scale_world_cur.Y)
            ? std::numeric_limits<float>::max()
            : fvector_scale_world_new.y() / fvector_scale_world_cur.Y *
                  fvector_scale_local_cur.Y;
    float scale_z_local =
        projectairsim::MathUtils::IsApproximatelyZero(fvector_scale_world_cur.Z)
            ? std::numeric_limits<float>::max()
            : fvector_scale_world_new.z() / fvector_scale_world_cur.Z *
                  fvector_scale_local_cur.Z;
    // Set the new local scaling
    if (ParticleComponent) {
      ParticleComponent->SetRelativeScale3D(
          FVector(scale_x_local, scale_y_local, scale_z_local));
    } else if (NiagaraComponent) {
      NiagaraComponent->SetRelativeScale3D(
          FVector(scale_x_local, scale_y_local, scale_z_local));
    }
  }

  projectairsim::Vector3 position = SimEnvParticles.GetOrigin().translation_;
  projectairsim::Quaternion orientation = SimEnvParticles.GetOrigin().rotation_;

  projectairsim::Pose pose = projectairsim::Pose(position, orientation);

  UpdateEnvActorTargetPose(pose, 0);      // timestamp=0
  MoveEnvActorToTargetPose(false);
}

void AUnrealEnvParticleEffect::InitializeMesh(
    const microsoft::projectairsim::UnrealMesh* particle_mesh) {
  if (particle_mesh)  // Ensure particle_mesh is valid
  {
    auto meshPath = FString(particle_mesh->GetName().c_str());

    // Load the particle system
    UParticleSystem* ParticleSystem =
        LoadObject<UParticleSystem>(nullptr, *meshPath);
    UNiagaraSystem* NiagaraSystem =
        LoadObject<UNiagaraSystem>(nullptr, *meshPath);

    if (ParticleSystem) {
      ParticleComponent->SetTemplate(ParticleSystem);
      ParticleComponent->Activate();  // Ensure the particle system is activated
      RootComponent = ParticleComponent;
      NiagaraComponent = nullptr;  // Ensure NiagaraComponent is null

    } else if (NiagaraSystem) {
      NiagaraComponent->SetAsset(NiagaraSystem);
      NiagaraComponent->Activate();  // Ensure the particle system is activated
      RootComponent = NiagaraComponent;
      ParticleComponent = nullptr;  // Ensure ParticleComponent is null
    } else {
      UE_LOG(LogTemp, Error, TEXT("Failed to load particle asset at path: %s"),
             *meshPath);
    }
    BoundingBoxComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    BoundingBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BoundingBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);  // Adjust collision as needed
    BoundingBoxComponent->RegisterComponent();
  } else {
    return;
  }
}

void AUnrealEnvParticleEffect::InitializeId(const std::string& InId) {
  UnrealHelpers::SetActorName(this, InId);
}

void AUnrealEnvParticleEffect::Tick(float Delta) {
  // In case of a tick coming through while it should be paused, just return
  if (UGameplayStatics::IsGamePaused(this->GetWorld())) return;

  TimeNano NowNanos = projectairsim::SimClock::Get()->NowSimNanos();
  float dt = (NowNanos - PrevSimTimeNanos) / 1.0e9f;
  PrevSimTimeNanos = NowNanos;

  if (TickCount < 2) { // Wait for 2 ticks for the bounds of the particle system to initialize
    TickCount++;  // Increment the tick counter
  }

  if (NiagaraComponent) {
    NiagaraComponent->AdvanceSimulation(dt, 1);

    if (TickCount == 2) {
      TickCount++; // Increment the tick counter to avoid re-entering this block
      BoundingBoxComponent->SetBoxExtent(NiagaraComponent->Bounds.BoxExtent);
      BoundingBoxComponent->SetRelativeLocation(FVector(0, 0, (NiagaraComponent->Bounds.BoxExtent).Z)); // Change the relative location of the bounding box relative to the pivot, to avoid a BoundingBox displacement
    }
   // Advance the simulation
  } else if (ParticleComponent) {
    ParticleComponent->SetFloatParameter(FName("Lifetime"), dt);
    if (!ParticleComponent->IsActive()) {
      ParticleComponent->ActivateSystem(true);
    }    
    if (TickCount == 2) {
      TickCount++;  // Increment the tick counter to avoid re-entering this block
      BoundingBoxComponent->SetBoxExtent(ParticleComponent->Bounds.BoxExtent);
      BoundingBoxComponent->SetRelativeLocation(FVector(0, 0, (ParticleComponent->Bounds.BoxExtent).Z));// Change the relative location of the bounding box relative to the pivot
    }
  }
}

void AUnrealEnvParticleEffect::UpdateEnvActorTargetPose(
    const projectairsim::Pose InPose, TimeNano InTimestamp) {
  EnvActorTargetPose = InPose;
  bHasTargetPoseUpdated = true;
}

void AUnrealEnvParticleEffect::MoveEnvActorToTargetPose(
    bool bUseCollisionSweep) {
  if (RootComponent == nullptr || bHasTargetPoseUpdated == false) return;

  bHasTargetPoseUpdated = false;  // done processing target pose, clear flag

  // Use local copy of target pose to do actual env actor pose update
  const FVector TgtLocNEU =
      UnrealHelpers::ToFVector(projectairsim::TransformUtils::NedToNeuLinear(
          projectairsim::TransformUtils::ToCentimeters(
              EnvActorTargetPose.position)));
  const FRotator TgtRot =
      UnrealHelpers::ToFRotator(EnvActorTargetPose.orientation);

  // Move UE position
  RootComponent->SetWorldLocationAndRotation(TgtLocNEU, TgtRot,
                                             bUseCollisionSweep, nullptr,
                                             ETeleportType::TeleportPhysics);
}