// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef PHYSICS_INCLUDE_MESSAGE_PHYSICS_MODEL_OUTPUT_MESSAGE_HPP_
#define PHYSICS_INCLUDE_MESSAGE_PHYSICS_MODEL_OUTPUT_MESSAGE_HPP_

#include <sstream>
#include <string>

#include "common_message_utils.hpp"
#include "msgpack.hpp"
#include "physics_message.hpp"

namespace microsoft {
namespace projectairsim {

class PhysicsModelOutputMessage : PhysicsMessage {
 public:
  PhysicsModelOutputMessage()
      : PhysicsMessage(PhysicsMessageType::kPhysicsModelOutput) {}

  PhysicsModelOutputMessage(
      uint64_t time_stamp_val, float pose_position_x_val,
      float pose_position_y_val, float pose_position_z_val,
      float pose_orientation_x_val, float pose_orientation_y_val,
      float pose_orientation_z_val, float pose_orientation_w_val,
      float twist_linear_x_val, float twist_linear_y_val,
      float twist_linear_z_val, float twist_angular_x_val,
      float twist_angular_y_val, float twist_angular_z_val,
      float accels_linear_x_val, float accels_linear_y_val,
      float accels_linear_z_val, float accels_angular_x_val,
      float accels_angular_y_val, float accels_angular_z_val)
      : PhysicsMessage(PhysicsMessageType::kPhysicsModelOutput),
        time_stamp(time_stamp_val),
        kinematics(
            pose_position_x_val, pose_position_y_val, pose_position_z_val,
            pose_orientation_x_val, pose_orientation_y_val,
            pose_orientation_z_val, pose_orientation_w_val, twist_linear_x_val,
            twist_linear_y_val, twist_linear_z_val, twist_angular_x_val,
            twist_angular_y_val, twist_angular_z_val, accels_linear_x_val,
            accels_linear_y_val, accels_linear_z_val, accels_angular_x_val,
            accels_angular_y_val, accels_angular_z_val) {}

  ~PhysicsModelOutputMessage() override {}

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

  MSGPACK_DEFINE_MAP(time_stamp, kinematics);

  // TODO Encapsulate with get/set methods?
  uint64_t time_stamp;
  KinematicsFlatMsgpack kinematics;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_MESSAGE_PHYSICS_MODEL_OUTPUT_MESSAGE_HPP_
