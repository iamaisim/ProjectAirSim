// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/float_message.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class FloatMessage::Impl : public MessageImpl {
 public:
  Impl();

  explicit Impl(float val);

  ~Impl() override {}

  float GetValue();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(value);

 private:
  float value;
};

// -----------------------------------------------------------------------------
// class FloatMessage

FloatMessage::FloatMessage() : Message(std::make_shared<FloatMessage::Impl>()) {}

FloatMessage::FloatMessage(float val)
    : Message(std::make_shared<FloatMessage::Impl>(val)) {}

FloatMessage::~FloatMessage() {}

float FloatMessage::GetValue() const {
  return static_cast<FloatMessage::Impl*>(pimpl_.get())->GetValue();
}

std::string FloatMessage::Serialize() const {
  return static_cast<FloatMessage::Impl*>(pimpl_.get())->Serialize();
}

void FloatMessage::Deserialize(const std::string& buffer) {
  static_cast<FloatMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class FloatMessage::Impl

FloatMessage::Impl::Impl() : MessageImpl(MessageType::kFloat), value(0) {}

FloatMessage::Impl::Impl(float val)
    : MessageImpl(MessageType::kFloat), value(val) {}

float FloatMessage::Impl::GetValue() { return value; }

std::string FloatMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void FloatMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
