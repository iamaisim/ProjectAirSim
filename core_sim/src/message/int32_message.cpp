// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/int32_message.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class Int32Message::Impl : public MessageImpl {
 public:
  Impl();

  explicit Impl(int32_t val);

  ~Impl() override {}

  int32_t GetValue();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(value);

 private:
  int32_t value;
};

// -----------------------------------------------------------------------------
// class Int32Message

Int32Message::Int32Message() : Message(std::make_shared<Int32Message::Impl>()) {}

Int32Message::Int32Message(int32_t val)
    : Message(std::make_shared<Int32Message::Impl>(val)) {}

Int32Message::~Int32Message() {}

int32_t Int32Message::GetValue() const {
  return static_cast<Int32Message::Impl*>(pimpl_.get())->GetValue();
}

std::string Int32Message::Serialize() const {
  return static_cast<Int32Message::Impl*>(pimpl_.get())->Serialize();
}

void Int32Message::Deserialize(const std::string& buffer) {
  static_cast<Int32Message::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class Int32Message::Impl

Int32Message::Impl::Impl() : MessageImpl(MessageType::kInt32), value(0) {}

Int32Message::Impl::Impl(int32_t val)
    : MessageImpl(MessageType::kInt32), value(val) {}

int32_t Int32Message::Impl::GetValue() { return value; }

std::string Int32Message::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void Int32Message::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
