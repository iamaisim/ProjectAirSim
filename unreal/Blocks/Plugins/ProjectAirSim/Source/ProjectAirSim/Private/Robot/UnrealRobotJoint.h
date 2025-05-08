// Copyright (C) Microsoft Corporation.  All rights reserved.
// The Unreal robot joint declaration.

#pragma once

#include <atomic>

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "UnrealRobotLink.h"
#include "core_sim/joint.hpp"

#include "UnrealRobotJoint.generated.h"

UCLASS()
class UUnrealRobotJoint : public UPhysicsConstraintComponent {
  GENERATED_BODY()

 public:
  explicit UUnrealRobotJoint(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::Joint& Joint);

  void TickComponent(float DeltaTime, ELevelTick TickType,
                     FActorComponentTickFunction* ThisTickFunction) override;

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void InitializePose(const microsoft::projectairsim::Transform& Pose);

  void InitializeConstraints(const microsoft::projectairsim::Joint& Joint);

  void PublishJointState();

  void UpdateJointState(const microsoft::projectairsim::JointStateMessage& message);

  microsoft::projectairsim::Joint joint;

  std::atomic<float> target_position;
  std::atomic<float> target_velocity;
};
