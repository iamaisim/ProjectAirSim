// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/transform_message.hpp"

#include <array>
#include <memory>
#include <sstream>
#include <string>

#include "core_sim/math_utils.hpp"
#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

class TransformMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp, std::string parent_frame_id,
       std::string child_frame_id, Vector3 translation, Quaternion rotation);

  ~Impl() override {}

  const Vector3 GetTranslation() const;
  const Quaternion GetRotation() const;
  const std::string GetParentFrameId() const;
  const std::string GetChildFrameId() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, parent_frame_id, child_frame_id, translation,
                     rotation);

 private:
  TimeNano time_stamp;
  std::string parent_frame_id;
  std::string child_frame_id;
  Vector3Msgpack translation;
  QuaternionMsgpack rotation;
};

TransformMessage::TransformMessage()
    : Message(std::make_shared<TransformMessage::Impl>()) {}

TransformMessage::TransformMessage(TimeNano time_stamp,
                                   std::string parent_frame_id,
                                   std::string child_frame_id,
                                   Vector3 translation, Quaternion rotation)
    : Message(std::make_shared<TransformMessage::Impl>(
          time_stamp, parent_frame_id, child_frame_id, translation, rotation)) {
}

TransformMessage::~TransformMessage() {}

const Vector3 TransformMessage::GetTranslation() const {
  return static_cast<TransformMessage::Impl*>(pimpl_.get())->GetTranslation();
}

const Quaternion TransformMessage::GetRotation() const {
  return static_cast<TransformMessage::Impl*>(pimpl_.get())->GetRotation();
}

const std::string TransformMessage::GetParentFrameId() const {
  return static_cast<TransformMessage::Impl*>(pimpl_.get())->GetParentFrameId();
}

const std::string TransformMessage::GetChildFrameId() const {
  return static_cast<TransformMessage::Impl*>(pimpl_.get())->GetChildFrameId();
}

std::string TransformMessage::Serialize() const {
  return static_cast<TransformMessage::Impl*>(pimpl_.get())->Serialize();
}

void TransformMessage::Deserialize(const std::string& buffer) {
  static_cast<TransformMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

TransformMessage::Impl::Impl() : MessageImpl(MessageType::kTransform) {}

TransformMessage::Impl::Impl(TimeNano time_stamp, std::string parent_frame_id,
                             std::string child_frame_id, Vector3 translation,
                             Quaternion rotation)
    : MessageImpl(MessageType::kTransform),
      time_stamp(time_stamp),
      parent_frame_id(parent_frame_id),
      child_frame_id(child_frame_id),
      translation(translation),
      rotation(rotation) {}

const Vector3 TransformMessage::Impl::GetTranslation() const {
  return translation.ToVector3();
}

const Quaternion TransformMessage::Impl::GetRotation() const {
  return rotation.ToQuaternion();
}

const std::string TransformMessage::Impl::GetParentFrameId() const {
  return parent_frame_id;
}

const std::string TransformMessage::Impl::GetChildFrameId() const {
  return child_frame_id;
}

std::string TransformMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void TransformMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
