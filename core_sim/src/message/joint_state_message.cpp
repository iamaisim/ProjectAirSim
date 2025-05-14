// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/joint_state_message.hpp"

#include <memory>
#include <sstream>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class JointStateMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(float position_val, float velocity_val, float effort_val);

  ~Impl() override {}

  float GetPosition();

  float GetVelocity();

  float GetEffort();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(position, velocity, effort);

 private:
  float position;
  float velocity;
  float effort;
};

// -----------------------------------------------------------------------------
// class JointStateMessage

JointStateMessage::JointStateMessage()
    : Message(std::make_shared<JointStateMessage::Impl>()) {}

JointStateMessage::JointStateMessage(float position_val, float velocity_val,
                                     float effort_val)
    : Message(std::make_shared<JointStateMessage::Impl>(
          position_val, velocity_val, effort_val)) {}

JointStateMessage::~JointStateMessage() {}

float JointStateMessage::GetPosition() const {
  return static_cast<JointStateMessage::Impl*>(pimpl_.get())->GetPosition();
}

float JointStateMessage::GetVelocity() const {
  return static_cast<JointStateMessage::Impl*>(pimpl_.get())->GetVelocity();
}

float JointStateMessage::GetEffort() const {
  return static_cast<JointStateMessage::Impl*>(pimpl_.get())->GetEffort();
}

std::string JointStateMessage::Serialize() const {
  return static_cast<JointStateMessage::Impl*>(pimpl_.get())->Serialize();
}

void JointStateMessage::Deserialize(const std::string& buffer) {
  static_cast<JointStateMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class JointStateMessage::Impl

JointStateMessage::Impl::Impl()
    : MessageImpl(MessageType::kJointState),
      position(0),
      velocity(0),
      effort(0) {}

JointStateMessage::Impl::Impl(float position_val, float velocity_val,
                              float effort_val)
    : MessageImpl(MessageType::kJointState),
      position(position_val),
      velocity(velocity_val),
      effort(effort_val) {}

float JointStateMessage::Impl::GetPosition() { return position; }

float JointStateMessage::Impl::GetVelocity() { return velocity; }

float JointStateMessage::Impl::GetEffort() { return effort; }

std::string JointStateMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void JointStateMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
