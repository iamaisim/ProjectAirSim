// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// The Unreal EnvActor implementation.

#include "UnrealEnvActor.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>
#include <exception>

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ProjectAirSim.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "UnrealHelpers.h"
#include "UnrealLogger.h"
#include "core_sim/clock.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"

namespace projectairsim = microsoft::projectairsim;

AUnrealEnvActor::AUnrealEnvActor(const FObjectInitializer& ObjectInitialize)
    : AActor(ObjectInitialize) {
  PrimaryActorTick.bCanEverTick = true;
}

void AUnrealEnvActor::Initialize(const projectairsim::EnvActor& InEnvActor) {
  // Store ptrs to other corresponding components for this env actor
  this->SimEnvActor = InEnvActor;

  // Detect which links are roots based on their joint attachments
  auto RootLinks = GetRootLinks(InEnvActor.GetLinks(), InEnvActor.GetJoints());

  PrimaryActorTick.TickGroup = TG_PrePhysics;

  // Initialize the configured env actor component structure
  InitializeId(InEnvActor.GetID());
  InitializeLinks(InEnvActor.GetLinks(), RootLinks);
  InitializeJoints(InEnvActor.GetJoints());

  LinkChildrenMap = InEnvActor.GetLinkChildrenMap();

  UpdateEnvActorLinkRotAngles(InEnvActor.GetLinkRotationAngles());
  UpdateEnvActorLinkRotRates(InEnvActor.GetLinkRotationRates());
  RotateEnvActorLinksToAngle();
  RotateEnvActorLinksAtRate();
}

void AUnrealEnvActor::InitializeId(const std::string& InId) {
  UnrealHelpers::SetActorName(this, InId);
}

void AUnrealEnvActor::InitializeLinks(
    const std::vector<projectairsim::Link>& InLinks,
    const std::set<std::string>& InRootLinks) {
  std::for_each(InLinks.begin(), InLinks.end(),
                [this, &InRootLinks](const projectairsim::Link& CurLink) {
                  bool bIsRootLink = false;
                  if (InRootLinks.find(CurLink.GetID()) != InRootLinks.end()) {
                    bIsRootLink = true;
                  }
                  EnvActorLinks.insert(CreateLink(CurLink, bIsRootLink));
                });
}

std::pair<std::string, UUnrealRobotLink*> AUnrealEnvActor::CreateLink(
    const projectairsim::Link& InLink, bool bIsRootLink) {
  auto Id = InLink.GetID();

  auto NewLink = NewObject<UUnrealRobotLink>(this, Id.c_str());
  NewLink->Initialize(InLink,
                      false);  // with_unreal_physics always false for EnvActors

  if (bIsRootLink) {
    EnvActorRootLink = NewLink;
    RootComponent = NewLink;  // Also set as the USceneComponent's RootComponent
  }
  return {Id, NewLink};
}

void AUnrealEnvActor::InitializeJoints(
    const std::vector<projectairsim::Joint>& InJoints) {
  std::for_each(InJoints.begin(), InJoints.end(),
                [this](const projectairsim::Joint& CurJoint) {
                  EnvActorJoints.insert(CreateJoint(CurJoint));
                });
}

std::pair<std::string, UUnrealRobotJoint*> AUnrealEnvActor::CreateJoint(
    const projectairsim::Joint& InJoint) {
  auto Id = InJoint.GetID();

  auto NewJoint = NewObject<UUnrealRobotJoint>(this, Id.c_str());
  NewJoint->Initialize(InJoint);

  auto Parent = EnvActorLinks.at(InJoint.GetParentLink().c_str());
  auto Child = EnvActorLinks.at(InJoint.GetChildLink().c_str());

  NewJoint->ConstraintActor1 = this;
  NewJoint->ConstraintActor2 = this;

  NewJoint->AttachToComponent(Parent,
                              FAttachmentTransformRules::KeepRelativeTransform);
  Child->AttachToComponent(NewJoint,
                           FAttachmentTransformRules::KeepRelativeTransform);
  NewJoint->SetConstrainedComponents(Parent, NAME_None, Child, NAME_None);

  return {Id, NewJoint};
}

std::set<std::string> AUnrealEnvActor::GetRootLinks(
    const std::vector<projectairsim::Link>& InLinks,
    const std::vector<projectairsim::Joint>& InJoints) {
  std::set<std::string> Roots;

  std::transform(
      InLinks.begin(), InLinks.end(), std::inserter(Roots, Roots.begin()),
      [Roots](const projectairsim::Link& CurLink) { return CurLink.GetID(); });

  std::for_each(InJoints.begin(), InJoints.end(),
                [&Roots](const projectairsim::Joint& CurJoint) {
                  Roots.erase(CurJoint.GetChildLink());
                });

  return Roots;
}

void AUnrealEnvActor::UpdateEnvActorLinkRotAngles(
    const std::unordered_map<std::string, microsoft::projectairsim::AngleAxis>&
        LinkRotAngles) {
  FScopeLock ScopeLock(&UpdateMutex);
  EnvActorLinkRotAngles = LinkRotAngles;
  bHasLinkRotAnglesUpdated = true;
}

void AUnrealEnvActor::RotateEnvActorLinksToAngle() {
  if (!bHasLinkRotAnglesUpdated) return;

  for (auto& [LinkName, AxisAngle] : EnvActorLinkRotAngles) {
    FVector Axis = UnrealHelpers::ToFVector(
        projectairsim::TransformUtils::NedToNeuAngular(AxisAngle.axis()));
    EnvActorLinks.at(LinkName)->SetRelativeRotation(
        FQuat(Axis, AxisAngle.angle()));

    // Rotate children links.
    // TODO: Set rotation according to computed transform tree once implemented
    auto ChildrenMapItr = LinkChildrenMap.find(LinkName);
    if (ChildrenMapItr != LinkChildrenMap.end()) {
      for (auto& ChildLink : ChildrenMapItr->second) {
        EnvActorLinks.at(ChildLink)->SetRelativeRotation(
            FQuat(Axis, AxisAngle.angle()));
      }
    }
  }
  bHasLinkRotAnglesUpdated = false;
}

void AUnrealEnvActor::UpdateEnvActorLinkRotRates(
    const std::unordered_map<std::string, microsoft::projectairsim::AngleAxis>&
        LinkRotRates) {
  FScopeLock ScopeLock(&UpdateMutex);
  EnvActorLinkRotRates = LinkRotRates;
}

void AUnrealEnvActor::RotateEnvActorLinksAtRate() {
  FScopeLock ScopeLock(&UpdateMutex);

  TimeNano NowNanos = projectairsim::SimClock::Get()->NowSimNanos();
  float dt = (NowNanos - PrevSimTimeNanos) / 1.0e9f;
  PrevSimTimeNanos = NowNanos;

  for (auto& [LinkName, AxisRotRate] : EnvActorLinkRotRates) {
    FVector Axis = UnrealHelpers::ToFVector(
        projectairsim::TransformUtils::NedToNeuAngular(AxisRotRate.axis()));
//
//  try/catch Added to address out of bounds errors from EnvActorLinks.at(LinkName)
//  Adding the FScopeLock above suppresses the exceptions, but unknown root cause 
//
    try {
      EnvActorLinks.at(LinkName)->AddLocalRotation(FQuat(Axis, dt * AxisRotRate.angle()));
    }
    catch (const std::exception& e) {
        FString exceptionCause(e.what());
        FString linkName(LinkName.c_str());
        UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                          TEXT("Container At Error rotating link <%s>, container size %d: %s"), *linkName, EnvActorLinks.size(), *exceptionCause);
        UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                          TEXT("EnvActor object %x"), this);
    }
  }
}

void AUnrealEnvActor::UpdateEnvActorTargetPose(const projectairsim::Pose InPose,
                                               TimeNano InTimestamp) {
  EnvActorTargetPose = InPose;
  TargetPoseUpdatedTimeStamp = InTimestamp;
  bHasTargetPoseUpdated = true;

  // UnrealLogger::Log(projectairsim::LogLevel::kTrace,
  //                   TEXT("UpdateEnvActorTargetPose xyz=%f, %f, %f"),
  //                   EnvActorTargetPose.position.x(),
  //                   EnvActorTargetPose.position.y(),
  //                   EnvActorTargetPose.position.z());
}

void AUnrealEnvActor::MoveEnvActorToTargetPose(bool bUseCollisionSweep) {
  if (EnvActorRootLink == nullptr || bHasTargetPoseUpdated == false) return;

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
  EnvActorRootLink->SetWorldLocationAndRotation(TgtLocNEU, TgtRot,
                                                bUseCollisionSweep, nullptr,
                                                ETeleportType::TeleportPhysics);
  // If 'bUseCollisionSweep' is true, collision hits during
  // SetWorldLocationAndRotation will be handled by the
  // callback AUnrealRobot::OnCollisionHit() with the FHitResult info

  bHasEnvActorPoseUpdated = true;

  // UnrealLogger::Log(projectairsim::LogLevel::kTrace,
  //                   TEXT("SetWorldLocationAndRotation xyz=%f, %f, %f"),
  //                   TgtLocNEU.X, TgtLocNEU.Y, TgtLocNEU.Z);
}

void AUnrealEnvActor::BeginPlay() {
  Super::BeginPlay();
  SimEnvActor.SetCallbackLinkRotAnglesUpdated(
      [this](const std::unordered_map<
             std::string, microsoft::projectairsim::AngleAxis>& LinkRotAngles) {
        this->UpdateEnvActorLinkRotAngles(LinkRotAngles);
      });

  SimEnvActor.SetCallbackLinkRotRatesUpdated(
      [this](const std::unordered_map<
             std::string, microsoft::projectairsim::AngleAxis>& LinkRotRates) {
        this->UpdateEnvActorLinkRotRates(LinkRotRates);
      });
}

void AUnrealEnvActor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
}

void AUnrealEnvActor::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  // In case of a tick coming through while it should be paused, just return
  if (UGameplayStatics::IsGamePaused(this->GetWorld())) return;

  // At every tick, get the updated pose and move env_actor to that pose
  UpdateEnvActorTargetPose(SimEnvActor.GetKinematicsThreadSafe().pose,
                           0);      // timestamp=0
  MoveEnvActorToTargetPose(false);  // move without collision sweep
  RotateEnvActorLinksToAngle();
  RotateEnvActorLinksAtRate();
}

const microsoft::projectairsim::Kinematics& AUnrealEnvActor::GetKinematics()
    const {
  return SimEnvActor.GetKinematics();
}
