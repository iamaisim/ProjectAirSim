// Copyright (C) Microsoft Corporation.  All rights reserved.
// The Unreal robot joint implementation.

#include "UnrealRobotJoint.h"

#include "ProjectAirSim.h"
#include "Async/Async.h"
#include "core_sim/transforms/transform_utils.hpp"

namespace projectairsim = microsoft::projectairsim;

UUnrealRobotJoint::UUnrealRobotJoint(
    const FObjectInitializer& ObjectInitializer)
    : UPhysicsConstraintComponent(ObjectInitializer) {
  bAutoActivate = true;
  PrimaryComponentTick.bCanEverTick = true;
  // Tick in PrePhysics to set joint targets in case of using Unreal Physics to
  // calculate the joint motion
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UUnrealRobotJoint::Initialize(const microsoft::projectairsim::Joint& Joint) {
  InitializePose(Joint.GetOrigin());
  InitializeConstraints(Joint);

  RegisterComponent();

  joint = Joint;
}

void UUnrealRobotJoint::InitializePose(
    const microsoft::projectairsim::Transform& Pose) {
  auto xyz_ned = Pose.translation_;
  SetRelativeLocation({xyz_ned.x() * 100, xyz_ned.y() * 100,
                       -xyz_ned.z() * 100});  // NED m -> NEU cm

  auto rpy = projectairsim::TransformUtils::ToRPY(Pose.rotation_);
  SetRelativeRotation({
      projectairsim::TransformUtils::ToDegrees<float>(rpy.y()),  // Pitch
      projectairsim::TransformUtils::ToDegrees<float>(rpy.z()),  // Yaw
      projectairsim::TransformUtils::ToDegrees<float>(rpy.x()),  // Roll
  });
}
void UUnrealRobotJoint::InitializeConstraints(
    const microsoft::projectairsim::Joint& Joint) {
  FConstraintInstance constraint;

  auto jointAxis = Joint.GetAxis();
  auto axis = FVector(jointAxis.x(), jointAxis.y(), jointAxis.z());

  constraint.ProfileInstance.LinearLimit.bSoftConstraint = 0;
  constraint.ProfileInstance.TwistLimit.bSoftConstraint = 0;
  constraint.ProfileInstance.ConeLimit.bSoftConstraint = 0;

  constraint.AngularRotationOffset = {0, 0, 0};

  if (FVector::ForwardVector == axis) {
    switch (Joint.GetJointType()) {
      case projectairsim::JointType::kFixed:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
      case projectairsim::JointType::kRevolute:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(
            EAngularConstraintMotion::ACM_Limited,
            projectairsim::TransformUtils::ToDegrees<float>(Joint.GetLimit()));
        break;
      case projectairsim::JointType::kContinuous:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Free,
                                        0.0f);
        break;
    }
  } else if (FVector::RightVector == axis) {
    switch (Joint.GetJointType()) {
      case projectairsim::JointType::kFixed:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
      case projectairsim::JointType::kRevolute:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(
            EAngularConstraintMotion::ACM_Limited,
            projectairsim::TransformUtils::ToDegrees<float>(Joint.GetLimit()));
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
      case projectairsim::JointType::kContinuous:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Free,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
    }

  } else if (FVector::UpVector == axis) {
    switch (Joint.GetJointType()) {
      case projectairsim::JointType::kFixed:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
      case projectairsim::JointType::kRevolute:
        constraint.SetAngularSwing1Limit(
            EAngularConstraintMotion::ACM_Limited,
            projectairsim::TransformUtils::ToDegrees(Joint.GetLimit()));
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
      case projectairsim::JointType::kContinuous:
        constraint.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Free,
                                         0.0f);
        constraint.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked,
                                         0.0f);
        constraint.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked,
                                        0.0f);
        break;
    }
  }

  if (Joint.GetJointType() == projectairsim::JointType::kContinuous) {
    constraint.SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
    constraint.SetOrientationDriveSLERP(false);
    constraint.SetOrientationDriveTwistAndSwing(false, true);
    constraint.SetAngularVelocityDriveSLERP(false);
    constraint.SetAngularVelocityDriveTwistAndSwing(false, true);
  } else if (Joint.GetJointType() == projectairsim::JointType::kRevolute) {
    constraint.SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
    constraint.SetOrientationDriveSLERP(false);
    constraint.SetOrientationDriveTwistAndSwing(false, true);
    constraint.SetAngularVelocityDriveSLERP(false);
    constraint.SetAngularVelocityDriveTwistAndSwing(false, true);
  }

  constraint.ProfileInstance.bParentDominates = Joint.GetParentDominates();
  constraint.ProfileInstance.bDisableCollision = true;

  constraint.SetAngularDriveParams(Joint.GetSpringConstant(),
                                   Joint.GetDampingConstant(),
                                   Joint.GetMaxForce());

  ConstraintInstance = constraint;
}

void UUnrealRobotJoint::BeginPlay() {
  Super::BeginPlay();

  joint.SetCallbackJointStateUpdated(
      [this](const projectairsim::JointStateMessage& message) {
        this->UpdateJointState(message);
      });

  joint.BeginUpdate();
}

void UUnrealRobotJoint::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (joint.GetJointType() == projectairsim::JointType::kRevolute) {
    auto jointAxis = joint.GetAxis();
    auto axis = FVector(jointAxis.x(), jointAxis.y(), jointAxis.z());

    if (FVector::ForwardVector == axis) {
      SetAngularOrientationTarget(
          {0, 0,
           projectairsim::TransformUtils::ToDegrees<float>(target_position)});
    } else if (FVector::RightVector == axis) {
      SetAngularOrientationTarget(
          {projectairsim::TransformUtils::ToDegrees<float>(target_position), 0,
           0});
    } else if (FVector::UpVector == axis) {
      SetAngularOrientationTarget(
          {0, projectairsim::TransformUtils::ToDegrees<float>(target_position),
           0});
    }
  } else if (joint.GetJointType() == projectairsim::JointType::kContinuous) {
    auto jointAxis = joint.GetAxis();
    auto axis = FVector(jointAxis.x(), jointAxis.y(), jointAxis.z());

    if (FVector::ForwardVector == axis) {
      SetAngularVelocityTarget(
          {projectairsim::TransformUtils::ToDegrees<float>(target_velocity), 0,
           0});
    } else if (FVector::RightVector == axis) {
      SetAngularVelocityTarget(
          {0, projectairsim::TransformUtils::ToDegrees<float>(target_velocity),
           0});
    } else if (FVector::UpVector == axis) {
      SetAngularVelocityTarget(
          {0, 0,
           projectairsim::TransformUtils::ToDegrees<float>(target_velocity)});
    }
  }

  PublishJointState();
}

void UUnrealRobotJoint::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  joint.EndUpdate();

  Super::EndPlay(EndPlayReason);
}

void UUnrealRobotJoint::PublishJointState() {
  float position = 0;
  float velocity = 0;

  if (joint.GetJointType() == projectairsim::JointType::kFixed) {
    return;
  }

  if (joint.GetJointType() == projectairsim::JointType::kRevolute) {
    auto jointAxis = joint.GetAxis();
    auto axis = FVector(jointAxis.x(), jointAxis.y(), jointAxis.z());

    if (FVector::ForwardVector == axis) {
      position = projectairsim::TransformUtils::ToRadians(GetCurrentTwist());
    } else if (FVector::RightVector == axis) {
      position = projectairsim::TransformUtils::ToRadians(GetCurrentSwing2());
    } else if (FVector::UpVector == axis) {
      position = projectairsim::TransformUtils::ToRadians(GetCurrentSwing1());
    }
  }

  if (joint.GetJointType() == projectairsim::JointType::kContinuous) {
    auto jointAxis = joint.GetAxis();
    auto axis = FVector(jointAxis.x(), jointAxis.y(), jointAxis.z());
    auto angularVelocity =
        ConstraintInstance.ProfileInstance.AngularDrive.AngularVelocityTarget;

    if (FVector::ForwardVector == axis) {
      velocity = projectairsim::TransformUtils::ToRadians(angularVelocity.X);
    } else if (FVector::RightVector == axis) {
      velocity = projectairsim::TransformUtils::ToRadians(angularVelocity.Y);
    } else if (FVector::UpVector == axis) {
      velocity = projectairsim::TransformUtils::ToRadians(angularVelocity.Z);
    }
  }

  auto joint_state = projectairsim::JointStateMessage(position, velocity, 0);
  joint.PublishJointState(joint_state);
}

void UUnrealRobotJoint::UpdateJointState(
    const microsoft::projectairsim::JointStateMessage& message) {
  if (joint.GetJointType() == projectairsim::JointType::kRevolute) {
    target_position.store(message.GetPosition());
  } else if (joint.GetJointType() == projectairsim::JointType::kContinuous) {
    target_velocity.store(message.GetVelocity());
  }
}
