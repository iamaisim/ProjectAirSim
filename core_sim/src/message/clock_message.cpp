// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "core_sim/message/clock_message.hpp"

#include <memory>
#include <sstream>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

class ClockMessage::Impl : public MessageImpl {
 public:
  Impl() : MessageImpl(MessageType::kClock), time_stamp(0) {}
  explicit Impl(TimeNano time_stamp_val)
      : MessageImpl(MessageType::kClock), time_stamp(time_stamp_val) {}

  TimeNano GetTimeStamp() const { return time_stamp; }

  std::string Serialize() override {
    std::stringstream stream;
    msgpack::packer<std::stringstream> packer(stream);
    this->msgpack_pack(packer);
    return stream.str();
  }

  void Deserialize(const std::string& buffer) override {
    auto handle = msgpack::unpack(buffer.data(), buffer.size());
    this->msgpack_unpack(handle.get());
  }

  MSGPACK_DEFINE_MAP(time_stamp);

 private:
  TimeNano time_stamp;
};

ClockMessage::ClockMessage() : Message(std::make_shared<ClockMessage::Impl>()) {}

ClockMessage::ClockMessage(TimeNano time_stamp_val)
    : Message(std::make_shared<ClockMessage::Impl>(time_stamp_val)) {}

ClockMessage::~ClockMessage() {}

TimeNano ClockMessage::GetTimeStamp() const {
  return static_cast<ClockMessage::Impl*>(pimpl_.get())->GetTimeStamp();
}

std::string ClockMessage::Serialize() const {
  return static_cast<ClockMessage::Impl*>(pimpl_.get())->Serialize();
}

void ClockMessage::Deserialize(const std::string& buffer) {
  static_cast<ClockMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

}  // namespace projectairsim
}  // namespace microsoft
