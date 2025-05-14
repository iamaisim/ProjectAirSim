// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef PHYSICS_INCLUDE_MESSAGE_PHYSICS_MODEL_INPUT_MESSAGE_HPP_
#define PHYSICS_INCLUDE_MESSAGE_PHYSICS_MODEL_INPUT_MESSAGE_HPP_

#include <sstream>
#include <string>
#include <vector>

#include "common_message_utils.hpp"
#include "msgpack.hpp"
#include "physics_message.hpp"

namespace microsoft {
namespace projectairsim {

class PhysicsModelInputMessage : PhysicsMessage {
 public:
  PhysicsModelInputMessage()
      : PhysicsMessage(PhysicsMessageType::kPhysicsModelInput) {}

  PhysicsModelInputMessage(
      uint64_t time_stamp_val,
      const std::vector<WrenchPointFlatMsgpack>& external_wrench_points_val,
      const std::vector<float> lift_drag_control_angles_val,
      const EnvironmentInfoFlatMsgpack& environment_info_val,
      const CollisionInfoFlatMsgpack& collision_info_val)
      : PhysicsMessage(PhysicsMessageType::kPhysicsModelInput),
        time_stamp(time_stamp_val),
        external_wrench_points(external_wrench_points_val),
        lift_drag_control_angles(lift_drag_control_angles_val),
        environment_info(environment_info_val),
        collision_info(collision_info_val) {}

  ~PhysicsModelInputMessage() override {}

  std::string Serialize() const override {
    std::stringstream stream;
    msgpack::packer<std::stringstream> packer(stream);
    this->msgpack_pack(packer);
    return stream.str();
  }

  void Deserialize(const std::string& buffer) override {
    auto handle = msgpack::unpack(buffer.data(), buffer.size());
    auto object = handle.get();
    this->msgpack_unpack(object);
  }

  MSGPACK_DEFINE_MAP(time_stamp, external_wrench_points,
                     lift_drag_control_angles, environment_info,
                     collision_info);

  // TODO Encapsulate with get/set methods?
  uint64_t time_stamp;
  std::vector<WrenchPointFlatMsgpack> external_wrench_points;
  std::vector<float> lift_drag_control_angles;
  EnvironmentInfoFlatMsgpack environment_info;
  CollisionInfoFlatMsgpack collision_info;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_MESSAGE_PHYSICS_MODEL_INPUT_MESSAGE_HPP_
