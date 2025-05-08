// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_PHYSICS_STATE_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_PHYSICS_STATE_MESSAGE_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class PhysicsStateMessage : public Message {
 public:
  PhysicsStateMessage(
      const TimeNano time_stamp_val,
      const std::unordered_map<std::string, Kinematics> physics_state_val,
      const std::unordered_map<std::string, ActuatedRotations>
          actuated_rot_state_val);

  PhysicsStateMessage();

  ~PhysicsStateMessage() override;

  TimeNano GetTimeStamp() const;

  // Physics state is a map of robot IDs to their rigid body kinematics
  std::unordered_map<std::string, Kinematics> GetPhysicsState() const;

  // Actuated rotations state is a map of robot IDs to the map of actuated
  // rotations for the robot's link IDs
  std::unordered_map<std::string, ActuatedRotations> GetActuatedRotationsState()
      const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_PHYSICS_STATE_MESSAGE_HPP_
