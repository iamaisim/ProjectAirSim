// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/int8_message.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class Int8Message::Impl : public MessageImpl {
 public:
  Impl();

  explicit Impl(int8_t val);

  ~Impl() override {}

  int8_t GetValue();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(value);

 private:
  int8_t value;
};

// -----------------------------------------------------------------------------
// class Int8Message

Int8Message::Int8Message() : Message(std::make_shared<Int8Message::Impl>()) {}

Int8Message::Int8Message(int8_t val)
    : Message(std::make_shared<Int8Message::Impl>(val)) {}

Int8Message::~Int8Message() {}

int8_t Int8Message::GetValue() const {
  return static_cast<Int8Message::Impl*>(pimpl_.get())->GetValue();
}

std::string Int8Message::Serialize() const {
  return static_cast<Int8Message::Impl*>(pimpl_.get())->Serialize();
}

void Int8Message::Deserialize(const std::string& buffer) {
  static_cast<Int8Message::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class Int8Message::Impl

Int8Message::Impl::Impl() : MessageImpl(MessageType::kInt8), value(0) {}

Int8Message::Impl::Impl(int8_t val)
    : MessageImpl(MessageType::kInt8), value(val) {}

int8_t Int8Message::Impl::GetValue() { return value; }

std::string Int8Message::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void Int8Message::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
