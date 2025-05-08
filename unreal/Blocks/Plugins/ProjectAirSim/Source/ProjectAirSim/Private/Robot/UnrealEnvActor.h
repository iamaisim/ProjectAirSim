// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include <map>
#include <set>
#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "UnrealRobotJoint.h"
#include "UnrealRobotLink.h"
#include "core_sim/actor/env_actor.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/physics_common_types.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealEnvActor.generated.h"

UCLASS()
class AUnrealEnvActor : public AActor {
  GENERATED_BODY()

 public:
  explicit AUnrealEnvActor(const FObjectInitializer& ObjectInitialize);

  void Initialize(const microsoft::projectairsim::EnvActor& InEnvActor);

  void Tick(float DeltaTime) override;

  const microsoft::projectairsim::Kinematics& GetKinematics() const;

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  microsoft::projectairsim::EnvActor SimEnvActor;
  
  UUnrealRobotLink* EnvActorRootLink;

 private:
  void InitializeId(const std::string& InId);

  void InitializeLinks(
      const std::vector<microsoft::projectairsim::Link>& InLinks,
      const std::set<std::string>& InRootLinks);

  virtual std::pair<std::string, UUnrealRobotLink*> CreateLink(
      const microsoft::projectairsim::Link& InLink, bool bIsRootLink);

  void InitializeJoints(
      const std::vector<microsoft::projectairsim::Joint>& InJoints);

  std::pair<std::string, UUnrealRobotJoint*> CreateJoint(
      const microsoft::projectairsim::Joint& InJoint);

  void UpdateEnvActorLinkRotAngles(
      const std::unordered_map<
          std::string, microsoft::projectairsim::AngleAxis>& LinkRotAngles);

  void RotateEnvActorLinksToAngle();

  void UpdateEnvActorLinkRotRates(
      const std::unordered_map<
          std::string, microsoft::projectairsim::AngleAxis>& LinkRotRates);

  void RotateEnvActorLinksAtRate();

  void UpdateEnvActorTargetPose(const microsoft::projectairsim::Pose InPose,
                                TimeNano InTimestamp);

  void MoveEnvActorToTargetPose(bool bUseCollisionSweep);

  std::set<std::string> GetRootLinks(
      const std::vector<microsoft::projectairsim::Link>& InLinks,
      const std::vector<microsoft::projectairsim::Joint>& InJoints);

  FCriticalSection UpdateMutex;

  std::map<std::string, UUnrealRobotLink*> EnvActorLinks;
  std::map<std::string, UUnrealRobotJoint*> EnvActorJoints;
  std::unordered_map<std::string, std::vector<std::string>> LinkChildrenMap;

  std::unordered_map<std::string, microsoft::projectairsim::AngleAxis>
      EnvActorLinkRotAngles;
  std::unordered_map<std::string, microsoft::projectairsim::AngleAxis>
      EnvActorLinkRotRates;

  bool bHasLinkRotAnglesUpdated = false;

  bool bHasTargetPoseUpdated = false;
  microsoft::projectairsim::Pose EnvActorTargetPose;
  TimeNano TargetPoseUpdatedTimeStamp = 0;

  bool bHasEnvActorPoseUpdated = false;
  TimeNano EnvActorPoseUpdatedTimeStamp = 0;

  TimeNano PrevSimTimeNanos = 0;  // for dt calc to rotate propellers
};