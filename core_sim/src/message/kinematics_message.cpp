// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/kinematics_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class KinematicsMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(const TimeNano time_stamp_val, const Kinematics kinematics_val);

  ~Impl() override {}

  TimeNano GetTimeStamp() const;
  Kinematics GetKinematics() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, kinematics);

 private:
  TimeNano time_stamp;
  KinematicsMsgpack kinematics;
};

// -----------------------------------------------------------------------------
// class KinematicsMessage

KinematicsMessage::KinematicsMessage()
    : Message(std::make_shared<KinematicsMessage::Impl>()) {}

KinematicsMessage::KinematicsMessage(const TimeNano time_stamp_val,
                                     const Kinematics kinematics_val)
    : Message(std::make_shared<KinematicsMessage::Impl>(time_stamp_val,
                                                        kinematics_val)) {}

KinematicsMessage::~KinematicsMessage() {}

TimeNano KinematicsMessage::GetTimeStamp() const {
  return static_cast<KinematicsMessage::Impl*>(pimpl_.get())->GetTimeStamp();
}

Kinematics KinematicsMessage::GetKinematics() const {
  return static_cast<KinematicsMessage::Impl*>(pimpl_.get())->GetKinematics();
}

std::string KinematicsMessage::Serialize() const {
  return static_cast<KinematicsMessage::Impl*>(pimpl_.get())->Serialize();
}

void KinematicsMessage::Deserialize(const std::string& buffer) {
  static_cast<KinematicsMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class KinematicsMessage::Impl

KinematicsMessage::Impl::Impl() : MessageImpl(MessageType::kKinematics) {}

KinematicsMessage::Impl::Impl(const TimeNano time_stamp_val,
                              const Kinematics kinematics_val)
    : MessageImpl(MessageType::kKinematics),
      time_stamp(time_stamp_val),
      kinematics(kinematics_val) {}

TimeNano KinematicsMessage::Impl::GetTimeStamp() const { return time_stamp; }

Kinematics KinematicsMessage::Impl::GetKinematics() const {
  return kinematics.ToKinematics();
}

std::string KinematicsMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void KinematicsMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
