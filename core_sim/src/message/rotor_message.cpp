// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/rotor_message.hpp"

#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class RotorMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val, const std::vector<RotorInfo>& rotor_info_vec);

  ~Impl() override {}

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, rotor_info_vec);

 private:
  TimeNano time_stamp;
  std::vector<RotorInfoMsgPack> rotor_info_vec;
};

// -----------------------------------------------------------------------------
// class RotorMessage

RotorMessage::RotorMessage()
    : Message(std::make_shared<RotorMessage::Impl>()) {}

RotorMessage::RotorMessage(TimeNano time_stamp_val,
                           const std::vector<RotorInfo>& rotor_info_vec)
    : Message(std::make_shared<RotorMessage::Impl>(time_stamp_val,
                                                   rotor_info_vec)) {}

RotorMessage::~RotorMessage() {}

std::string RotorMessage::Serialize() const {
  return static_cast<RotorMessage::Impl*>(pimpl_.get())->Serialize();
}

void RotorMessage::Deserialize(const std::string& buffer) {
  static_cast<RotorMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class RotorMessage::Impl

RotorMessage::Impl::Impl() : MessageImpl(MessageType::kRotorInfo) {}

RotorMessage::Impl::Impl(TimeNano time_stamp_val,
                         const std::vector<RotorInfo>& rotor_info_vec_val)
    : MessageImpl(MessageType::kRotorInfo), time_stamp(time_stamp_val) {
  for (auto& rotor_info : rotor_info_vec_val)
    rotor_info_vec.emplace_back(rotor_info);
}

std::string RotorMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void RotorMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
