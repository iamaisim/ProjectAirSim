// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/pose_stamped_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class PoseStampedMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(const TimeNano time_stamp_val, const Vector3 position_val,
       const Quaternion orientation_val);

  ~Impl() override {}

  TimeNano GetTimeStamp() const;
  Vector3 GetPosition() const;
  Quaternion GetOrientation() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, position, orientation);

 private:
  TimeNano time_stamp;
  Vector3Msgpack position;
  QuaternionMsgpack orientation;
};

// -----------------------------------------------------------------------------
// class PoseStampedMessage

PoseStampedMessage::PoseStampedMessage() : Message(std::make_shared<PoseStampedMessage::Impl>()) {}

PoseStampedMessage::PoseStampedMessage(const TimeNano time_stamp_val,
                         const Vector3 position_val,
                         const Quaternion orientation_val)
    : Message(std::make_shared<PoseStampedMessage::Impl>(time_stamp_val, position_val,
                                                  orientation_val)) {}

PoseStampedMessage::~PoseStampedMessage() {}

TimeNano PoseStampedMessage::GetTimeStamp() const {
  return static_cast<PoseStampedMessage::Impl*>(pimpl_.get())->GetTimeStamp();
}

Vector3 PoseStampedMessage::GetPosition() const {
  return static_cast<PoseStampedMessage::Impl*>(pimpl_.get())->GetPosition();
}

Quaternion PoseStampedMessage::GetOrientation() const {
  return static_cast<PoseStampedMessage::Impl*>(pimpl_.get())->GetOrientation();
}

std::string PoseStampedMessage::Serialize() const {
  return static_cast<PoseStampedMessage::Impl*>(pimpl_.get())->Serialize();
}

void PoseStampedMessage::Deserialize(const std::string& buffer) {
  static_cast<PoseStampedMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class PoseStampedMessage::Impl

PoseStampedMessage::Impl::Impl() : MessageImpl(MessageType::kPosestamped) {}

PoseStampedMessage::Impl::Impl(const TimeNano time_stamp_val,
                        const Vector3 position_val,
                        const Quaternion orientation_val)
    : MessageImpl(MessageType::kPosestamped),
      time_stamp(time_stamp_val),
      position(position_val),
      orientation(orientation_val) {}

TimeNano PoseStampedMessage::Impl::GetTimeStamp() const { return time_stamp; }

Vector3 PoseStampedMessage::Impl::GetPosition() const { return position.ToVector3(); }

Quaternion PoseStampedMessage::Impl::GetOrientation() const {
  return orientation.ToQuaternion();
}

std::string PoseStampedMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void PoseStampedMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
