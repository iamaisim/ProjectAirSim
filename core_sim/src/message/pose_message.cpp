// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/pose_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class PoseMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(const Vector3 position_val,
       const Quaternion orientation_val);

  ~Impl() override {}

  Vector3 GetPosition() const;
  Quaternion GetOrientation() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(position, orientation);

 private:
  Vector3Msgpack position;
  QuaternionMsgpack orientation;
};

// -----------------------------------------------------------------------------
// class PoseMessage

PoseMessage::PoseMessage() : Message(std::make_shared<PoseMessage::Impl>()) {}

PoseMessage::PoseMessage(const Vector3 position_val,
                         const Quaternion orientation_val)
    : Message(std::make_shared<PoseMessage::Impl>(position_val,
                                                  orientation_val)) {}

PoseMessage::~PoseMessage() {}

Vector3 PoseMessage::GetPosition() const {
  return static_cast<PoseMessage::Impl*>(pimpl_.get())->GetPosition();
}

Quaternion PoseMessage::GetOrientation() const {
  return static_cast<PoseMessage::Impl*>(pimpl_.get())->GetOrientation();
}

std::string PoseMessage::Serialize() const {
  return static_cast<PoseMessage::Impl*>(pimpl_.get())->Serialize();
}

void PoseMessage::Deserialize(const std::string& buffer) {
  static_cast<PoseMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class PoseMessage::Impl

PoseMessage::Impl::Impl() : MessageImpl(MessageType::kPosestamped) {}

PoseMessage::Impl::Impl(const Vector3 position_val,
                        const Quaternion orientation_val)
    : MessageImpl(MessageType::kPosestamped),
      position(position_val),
      orientation(orientation_val) {}

Vector3 PoseMessage::Impl::GetPosition() const { return position.ToVector3(); }

Quaternion PoseMessage::Impl::GetOrientation() const {
  return orientation.ToQuaternion();
}

std::string PoseMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void PoseMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
