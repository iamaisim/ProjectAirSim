// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/int_list_message.hpp"

#include <initializer_list>
#include <memory>
#include <sstream>
#include <vector>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class IntListMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(std::initializer_list<int> values);

  ~Impl() override {}

  std::vector<int> GetList(void);

  std::string Serialize(void) override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(values);

 private:
  std::vector<int> values;
};

// -----------------------------------------------------------------------------
// class IntListMessage

IntListMessage::IntListMessage(void)
    : Message(std::make_shared<IntListMessage::Impl>()) {}

IntListMessage::IntListMessage(
    std::initializer_list<int> values)
    : Message(
          std::make_shared<IntListMessage::Impl>(values)) {}

IntListMessage::~IntListMessage(void) {}

std::vector<int> IntListMessage::GetValues(void) const {
  return static_cast<IntListMessage::Impl*>(pimpl_.get())
      ->GetList();
}

std::string IntListMessage::Serialize(void) const {
  return static_cast<IntListMessage::Impl*>(pimpl_.get())
      ->Serialize();
}

void IntListMessage::Deserialize(const std::string& buffer) {
  static_cast<IntListMessage::Impl*>(pimpl_.get())
      ->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class IntListMessage::Impl

IntListMessage::Impl::Impl(void)
    : MessageImpl(MessageType::kIntList), values() {}

IntListMessage::Impl::Impl(
    std::initializer_list<int> _values)
    : MessageImpl(MessageType::kIntList), values() {
  values.assign(_values.begin(), _values.end());
}

std::vector<int> IntListMessage::Impl::GetList() {
  return values;
}

std::string IntListMessage::Impl::Serialize(void) {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void IntListMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
