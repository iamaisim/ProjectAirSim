// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// The Unreal EnvHuman implementation.

#include "UnrealEnvHuman.h"

#include <vector>

#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSlotEvaluationPose.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EnvHumanAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealHelpers.h"
#include "core_sim/actor/env_actor_grounded.hpp"
#include "core_sim/link.hpp"
#include "core_sim/math_utils.hpp"

namespace projectairsim = microsoft::projectairsim;

AUnrealEnvHuman::AUnrealEnvHuman(const FObjectInitializer& ObjectInitializer) {
  // Set this character to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  // Tick in PrePhysics to wake on every tick in case of using Unreal physics
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = true;
}

void AUnrealEnvHuman::Initialize(
    const projectairsim::EnvActorGrounded& InEnvActor) {
  // Store ptrs to other corresponding components for this env actor
  this->SimEnvHuman = InEnvActor;
  auto links = InEnvActor.GetLinks();
  auto skeletal_mesh = static_cast<const projectairsim::SkeletalMesh*>(
      links[0].GetVisual().GetGeometry());

  InitializeSkeletalMesh(skeletal_mesh);
  InitializeId(InEnvActor.GetID());

  // Scale for EnvHuman is set in the skeletal mesh
  {
    auto fvector_scale_local_cur = SkeletalMeshComponent->GetRelativeScale3D();
    auto fvector_scale_world_new =
        (skeletal_mesh == nullptr)
            ? projectairsim::Vector3(1.0f, 1.0f, 1.0f)
            : skeletal_mesh->GetScale();  // Get desired world scaling
    auto fvector_scale_world_cur =
        SkeletalMeshComponent->GetComponentScale();  // Get current world
                                                     // scaling
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
    SkeletalMeshComponent->SetRelativeScale3D(
        FVector(scale_x_local, scale_y_local, scale_z_local));

    // Set the capsule size to match the mesh size
    FBoxSphereBounds Bounds = SkeletalMeshComponent->CalcBounds(SkeletalMeshComponent->GetComponentTransform());
    FVector BoxExtent = Bounds.BoxExtent;
    if (BoxExtent.X < BoxExtent.Y) {
      GetCapsuleComponent()->SetCapsuleSize(BoxExtent.X, BoxExtent.Z);
    } else {
      GetCapsuleComponent()->SetCapsuleSize(BoxExtent.Y, BoxExtent.Z);
    }
    // The pivot points of the SkeletalMeshComponent (at the feet) and the CapsuleComponent (at the center) do not align.
    // Adjust the location of the SkeletalMeshComponent to match the CapsuleComponent, which is the immovable root component.
    // Otherwise, either the mesh or the capsule will sink into the floor.
    SkeletalMeshComponent->SetRelativeLocation(-GetSkeletalMeshZOffset());
  }
}

FVector AUnrealEnvHuman::GetSkeletalMeshZOffset(){
    // Method to get the adjusted location of the SkeletalMeshComponent
    float capsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    return FVector(0.0f, 0.0f, capsuleHalfHeight);
}

void AUnrealEnvHuman::InitializeSkeletalMesh(
    const projectairsim::SkeletalMesh* skeletal_mesh) {
  if (skeletal_mesh == nullptr) {
    return;
  }

  auto meshPath = FString(skeletal_mesh->GetName().c_str());
  auto skeletalMesh = LoadObject<USkeletalMesh>(nullptr, *meshPath);
  if (skeletalMesh == nullptr) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("Failed to load mesh '%s'"), *meshPath);
    return;
  }

  SkeletalMeshComponent = GetMesh();
  SkeletalMeshComponent->SetSkeletalMesh(skeletalMesh);
  SkeletalMeshComponent->SetCollisionEnabled(
      ECollisionEnabled::QueryAndPhysics);
  SkeletalMeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
  SkeletalMeshComponent->RegisterComponent();
  SkeletalMeshComponent->SetupAttachment(RootComponent);

  FString AnimBPPath = TEXT(
      "Script/Engine.AnimBlueprint'/ProjectAirSim/Characters/Mannequins/"
      "Animations/ABP_EnvHuman.ABP_EnvHuman_C'");
  UClass* AnimBPClass = LoadClass<UAnimInstance>(nullptr, *AnimBPPath);
  if (AnimBPClass) {
    SkeletalMeshComponent->SetAnimInstanceClass(AnimBPClass);
  }
  std::unordered_map<std::string, FVector> offset_map = {
      {"claudia", FVector(0, 0, 0.09)}, {"carla", FVector(0, 0, 0.03)},
      {"eric", FVector(0, 0, 0.015)},   {"manuel", FVector(0, 0, -0.005)},
      {"nathan", FVector(0, 0, 0)},     {"sophia", FVector(0, 0, -0.049)}};

  // Search for the mesh name in the skeletal mesh
  for (const auto& pair : offset_map) {
    if (meshPath.Contains(pair.first.c_str())) {
      SimEnvHuman.SetPositionZOffset(pair.second.Z);
      break;  // Stop searching once the match is found
    }
  }
}

void AUnrealEnvHuman::InitializeId(const std::string& InId) {
  UnrealHelpers::SetActorName(this, InId);
}

void AUnrealEnvHuman::NotifyHit(class UPrimitiveComponent* MyComp,
                                class AActor* Other,
                                class UPrimitiveComponent* OtherComp,
                                bool bSelfMoved, FVector HitLocation,
                                FVector HitNormal, FVector NormalImpulse,
                                const FHitResult& Hit) {
  Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal,
                   NormalImpulse, Hit);
}

void AUnrealEnvHuman::Tick(float Delta) {
  // In case of a tick coming through while it should be paused, just return
  if (UGameplayStatics::IsGamePaused(this->GetWorld())) return;

  TimeNano NowNanos = projectairsim::SimClock::Get()->NowSimNanos();
  float dt = (NowNanos - PrevSimTimeNanos) / 1.0e9f;
  PrevSimTimeNanos = NowNanos;

  // Update the current position
  curr_position_ = GetActorLocation();

  // At every tick, get the updated pose and move env_actor to that pose
  UpdateEnvActorTargetPose(SimEnvHuman.GetKinematicsThreadSafe().pose,
                           0);      // timestamp=0
  MoveEnvActorToTargetPose(false);  // move without collision sweep

  // The LineTrace is calculated from the center of the capsule component, 
  // so the start and end points are adjusted by the capsule half height (EnvHuman's feet)
  FVector start = GetCapsuleComponent()->GetComponentLocation() - GetSkeletalMeshZOffset();
  FVector endUnder =
      start - GetSkeletalMeshZOffset();  // Trace down by the capsule half height
  FVector endOver =
      start + GetSkeletalMeshZOffset();  // The end of the vector over the capsule

  FHitResult HitResultUnder;
  FHitResult HitResultOver;
  FCollisionQueryParams TraceParams;
  TraceParams.AddIgnoredActor(this);

  bool bHit = GetWorld()->LineTraceSingleByChannel(
      HitResultUnder, start, endUnder, ECC_Visibility, TraceParams);
  bool bHitOver = GetWorld()->LineTraceSingleByChannel(
      HitResultOver, endOver, start, ECC_Visibility, TraceParams);

  if (bHitOver) {
    // Ceiling detected
    FVector CeilingLocation = HitResultOver.Location;
    SimEnvHuman.SetGroundLevel(
        projectairsim::TransformUtils::NeuToNedLinear(
            projectairsim::TransformUtils::ToMeters(
                UnrealHelpers::ToVector3(CeilingLocation + GetSkeletalMeshZOffset())))
            .z());
  } else if (bHit) {
    // Floor detected
    FVector FloorLocation = HitResultUnder.Location;
    SimEnvHuman.SetGroundLevel(projectairsim::TransformUtils::NeuToNedLinear(
                                   projectairsim::TransformUtils::ToMeters(
                                       UnrealHelpers::ToVector3(FloorLocation + GetSkeletalMeshZOffset())))
                                   .z());  // NEU (cm) to NED (m)
  } else {
    // Floor not detected
    SimEnvHuman.SetGroundLevel(projectairsim::TransformUtils::NeuToNedLinear(
                                   projectairsim::TransformUtils::ToMeters(
                                       UnrealHelpers::ToVector3(endUnder + GetSkeletalMeshZOffset())))
                                   .z());  // NEU (cm) to NED (m)
  }

  updateAnimation(dt);
  prev_position_ = curr_position_;
}

void AUnrealEnvHuman::updateAnimation(float Delta) {
  if (SkeletalMeshComponent == nullptr) {
    return;
  }

  SkeletalMeshComponent->EnableExternalTickRateControl(true);
  SkeletalMeshComponent->EnableExternalUpdate(true);
  SkeletalMeshComponent->TickAnimation(Delta, false);
  SkeletalMeshComponent->RefreshBoneTransforms();
  UEnvHumanAnimInstance* anim_instance =
      Cast<UEnvHumanAnimInstance>(SkeletalMeshComponent->GetAnimInstance());
  if (anim_instance == nullptr) {
    return;
  }
  anim_instance->velocity_ = (prev_position_ - curr_position_) / Delta;
  anim_instance->TickAnimationBPEvent();
}

void AUnrealEnvHuman::UpdateEnvActorTargetPose(const projectairsim::Pose InPose,
                                               TimeNano InTimestamp) {
  EnvActorTargetPose = InPose;
  bHasTargetPoseUpdated = true;
}

void AUnrealEnvHuman::MoveEnvActorToTargetPose(bool bUseCollisionSweep) {
  if (RootComponent == nullptr || bHasTargetPoseUpdated == false) return;

  // Todo later if collisions are added to env_actors

  // Clear robot's has_collided flag before trying to move again
  // sim_robot.SetHasCollided(false);

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
  // If 'bUseCollisionSweep' is true, collision hits during
  // SetWorldLocationAndRotation will be handled by the
  // callback AUnrealRobot::OnCollisionHit() with the FHitResult info
  // UE_LOG(LogTemp, Warning,
  //       TEXT("Moving Character to Location: %s, Rotation: %s"),
  //       *TgtLocNEU.ToString(), *TgtRot.ToString());
}