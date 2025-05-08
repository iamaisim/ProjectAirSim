// Copyright (C) Microsoft Corporation. All rights reserved.

#include "unreal_physics.hpp"

#include <memory>
#include <vector>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/rotor.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class UnrealPhysicsBody

UnrealPhysicsBody::UnrealPhysicsBody(const Robot& robot) : sim_robot_(robot) {
  SetName(robot.GetID());
  SetPhysicsType(PhysicsType::kUnrealPhysics);
  InitializeUnrealPhysicsBody();
}

void UnrealPhysicsBody::InitializeUnrealPhysicsBody() {
  // Get robot physics-related data
  const auto& actuators = sim_robot_.GetActuators();

  // Process robot's actuators to store pointers to their wrench points
  external_wrench_points_.clear();
  for (const auto& actuator_ref : actuators) {
    const Actuator& actuator = actuator_ref.get();
    if (actuator.GetType() == ActuatorType::kRotor) {
      const auto& rotor_ref = static_cast<const Rotor&>(actuator);
      external_wrench_points_.emplace_back(&(rotor_ref.GetWrenchPoint()));
    }
  }
}

void UnrealPhysicsBody::CalculateExternalWrench() {
  // Aggregate external wrenches into the total external_wrench_ on body
  Wrench aggregate_wrench = Wrench::Zero();
  for (const WrenchPoint* wrench_pt : external_wrench_points_) {
    // Accumulate wrench itself
    aggregate_wrench += wrench_pt->wrench;

    // Add additional torque from force applied at radius to CG, tau = r X F
    aggregate_wrench.torque +=
        wrench_pt->position.cross(wrench_pt->wrench.force);
  }

  aggregate_wrench.force = PhysicsUtils::TransformVectorToWorldFrame(
      aggregate_wrench.force, kinematics_.pose.orientation);

  external_wrench_ = aggregate_wrench;

  // Use callback to pass external wrench to Unreal
  std::function<void(const Wrench&)> callback = nullptr;
  callback = set_external_wrench_callback_;
  if (callback != nullptr) {
    callback(external_wrench_);
  }
}

void UnrealPhysicsBody::WriteRobotData(const Kinematics& kinematics,
                                       TimeNano external_time_stamp) {
  sim_robot_.UpdateKinematics(kinematics, external_time_stamp);
}

void UnrealPhysicsBody::SetCallbackSetExternalWrench(
    const std::function<void(const Wrench&)>& callback) {
  set_external_wrench_callback_ = callback;
}

// -----------------------------------------------------------------------------
// class UnrealPhysicsModel

void UnrealPhysicsModel::SetWrenchesOnPhysicsBody(
    std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a UnrealPhysicsBody
  std::shared_ptr<UnrealPhysicsBody> unreal_body =
      std::dynamic_pointer_cast<UnrealPhysicsBody>(body);

  if (unreal_body != nullptr) {
    unreal_body->CalculateExternalWrench();
  }
}

}  // namespace projectairsim
}  // namespace microsoft
