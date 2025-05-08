// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/ready_state_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class ReadyStateMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(const TimeNano time_stamp_val, const bool ready_val,
       const std::string ready_message);

  ~Impl() override {}

  TimeNano GetTimeStamp() const;
  bool GetReadyVal() const;
  std::string GetReadyMessage() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, ready_val, ready_message);

 private:
  TimeNano time_stamp;
  bool ready_val;
  std::string ready_message;
};

// -----------------------------------------------------------------------------
// class ReadyStateMessage

ReadyStateMessage::ReadyStateMessage()
    : Message(std::make_shared<ReadyStateMessage::Impl>()) {}

ReadyStateMessage::ReadyStateMessage(const TimeNano time_stamp_val,
                                     const bool ready_val,
                                     const std::string ready_message)
    : Message(std::make_shared<ReadyStateMessage::Impl>(
          time_stamp_val, ready_val, ready_message)) {}

ReadyStateMessage::~ReadyStateMessage() {}

TimeNano ReadyStateMessage::GetTimeStamp() const {
  return static_cast<ReadyStateMessage::Impl*>(pimpl_.get())->GetTimeStamp();
}

bool ReadyStateMessage::GetReadyVal() const {
  return static_cast<ReadyStateMessage::Impl*>(pimpl_.get())->GetReadyVal();
}

std::string ReadyStateMessage::GetReadyMessage() const {
  return static_cast<ReadyStateMessage::Impl*>(pimpl_.get())->GetReadyMessage();
}

std::string ReadyStateMessage::Serialize() const {
  return static_cast<ReadyStateMessage::Impl*>(pimpl_.get())->Serialize();
}

void ReadyStateMessage::Deserialize(const std::string& buffer) {
  static_cast<ReadyStateMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class ReadyStateMessage::Impl

ReadyStateMessage::Impl::Impl() : MessageImpl(MessageType::kReadyState) {}

ReadyStateMessage::Impl::Impl(const TimeNano time_stamp_val,
                              const bool ready_val,
                              const std::string ready_message)
    : MessageImpl(MessageType::kReadyState),
      time_stamp(time_stamp_val),
      ready_val(ready_val),
      ready_message(ready_message) {}

TimeNano ReadyStateMessage::Impl::GetTimeStamp() const { return time_stamp; }

bool ReadyStateMessage::Impl::GetReadyVal() const { return ready_val; }

std::string ReadyStateMessage::Impl::GetReadyMessage() const {
  return ready_message;
}

std::string ReadyStateMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void ReadyStateMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
