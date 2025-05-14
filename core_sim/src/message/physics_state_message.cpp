// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/physics_state_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class PhysicsStateMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(const TimeNano time_stamp_val,
       const std::unordered_map<std::string, Kinematics> physics_state_val,
       const std::unordered_map<std::string, ActuatedRotations>
           actuated_rot_state_val);

  ~Impl() override {}

  TimeNano GetTimeStamp() const;

  std::unordered_map<std::string, Kinematics> GetPhysicsState() const;

  std::unordered_map<std::string, ActuatedRotations> GetActuatedRotationsState()
      const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, physics_state, actuated_rotations_state);

 private:
  TimeNano time_stamp;

  // k:robot ID, v:kinematics
  std::unordered_map<std::string, KinematicsMsgpack> physics_state;

  // k:robot ID, v:{k:link ID, v:(rad/s x, rad/s y, rad/s z)}
  std::unordered_map<std::string, ActuatedRotationsMsgpack>
      actuated_rotations_state;
};

// -----------------------------------------------------------------------------
// class PhysicsStateMessage

PhysicsStateMessage::PhysicsStateMessage()
    : Message(std::make_shared<PhysicsStateMessage::Impl>()) {}

PhysicsStateMessage::PhysicsStateMessage(
    const TimeNano time_stamp_val,
    const std::unordered_map<std::string, Kinematics> physics_state_val,
    const std::unordered_map<std::string, ActuatedRotations>
        actuated_rot_state_val)
    : Message(std::make_shared<PhysicsStateMessage::Impl>(
          time_stamp_val, physics_state_val, actuated_rot_state_val)) {}

PhysicsStateMessage::~PhysicsStateMessage() {}

TimeNano PhysicsStateMessage::GetTimeStamp() const {
  return static_cast<PhysicsStateMessage::Impl*>(pimpl_.get())->GetTimeStamp();
}

std::unordered_map<std::string, Kinematics>
PhysicsStateMessage::GetPhysicsState() const {
  return static_cast<PhysicsStateMessage::Impl*>(pimpl_.get())
      ->GetPhysicsState();
}

std::unordered_map<std::string, ActuatedRotations>
PhysicsStateMessage::GetActuatedRotationsState() const {
  return static_cast<PhysicsStateMessage::Impl*>(pimpl_.get())
      ->GetActuatedRotationsState();
}

std::string PhysicsStateMessage::Serialize() const {
  return static_cast<PhysicsStateMessage::Impl*>(pimpl_.get())->Serialize();
}

void PhysicsStateMessage::Deserialize(const std::string& buffer) {
  static_cast<PhysicsStateMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class PhysicsStateMessage::Impl

PhysicsStateMessage::Impl::Impl() : MessageImpl(MessageType::kPhysicsState) {}

PhysicsStateMessage::Impl::Impl(
    const TimeNano time_stamp_val,
    const std::unordered_map<std::string, Kinematics> physics_state_val,
    const std::unordered_map<std::string, ActuatedRotations>
        actuated_rot_state_val)
    : MessageImpl(MessageType::kPhysicsState), time_stamp(time_stamp_val) {
  for (auto& [robot_id, kinematics] : physics_state_val) {
    // Convert Kinematics to KinematicsMsgpack
    physics_state.emplace(robot_id, kinematics);
  }

  for (auto& [robot_id, actuated_rots] : actuated_rot_state_val) {
    // Convert ActuatedRotations to ActuatedRotationsMsgpack
    actuated_rotations_state.emplace(robot_id, actuated_rots);
  }
}

TimeNano PhysicsStateMessage::Impl::GetTimeStamp() const { return time_stamp; }

std::unordered_map<std::string, Kinematics>
PhysicsStateMessage::Impl::GetPhysicsState() const {
  std::unordered_map<std::string, Kinematics> physics_state_out;
  for (auto& [robot_id, kinematics_msgpack] : physics_state) {
    // Convert KinematicsMsgpack to Kinematics
    physics_state_out.emplace(robot_id, kinematics_msgpack.ToKinematics());
  }
  return physics_state_out;
}

std::unordered_map<std::string, ActuatedRotations>
PhysicsStateMessage::Impl::GetActuatedRotationsState() const {
  std::unordered_map<std::string, ActuatedRotations> actuated_rot_state_out;
  for (auto& [robot_id, actuated_rot_msgpack] : actuated_rotations_state) {
    // Convert ActuatedRotationsMsgpack to ActuatedRotations
    actuated_rot_state_out.emplace(robot_id,
                                   actuated_rot_msgpack.ToActuatedRotations());
  }
  return actuated_rot_state_out;
}

std::string PhysicsStateMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void PhysicsStateMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
