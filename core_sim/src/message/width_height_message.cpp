// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/width_height_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class WidthHeightMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(int width, int height);

  ~Impl() override {}

  int GetWidth() const;
  int GetHeight() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(width, height);

 private:
  int width;
  int height;
};

// -----------------------------------------------------------------------------
// class WidthHeightMessage

WidthHeightMessage::WidthHeightMessage() : Message(std::make_shared<WidthHeightMessage::Impl>()) {}

WidthHeightMessage::WidthHeightMessage(int width, int height)
    : Message(std::make_shared<WidthHeightMessage::Impl>(width, height)) {}
                                                  
WidthHeightMessage::~WidthHeightMessage() {}

int WidthHeightMessage::GetWidth() const {
  return static_cast<WidthHeightMessage::Impl*>(pimpl_.get())->GetWidth();
}

int WidthHeightMessage::GetHeight() const {
  return static_cast<WidthHeightMessage::Impl*>(pimpl_.get())->GetHeight();
}

std::string WidthHeightMessage::Serialize() const {
  return static_cast<WidthHeightMessage::Impl*>(pimpl_.get())->Serialize();
}

void WidthHeightMessage::Deserialize(const std::string& buffer) {
  static_cast<WidthHeightMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class WidthHeightMessage::Impl

WidthHeightMessage::Impl::Impl() : MessageImpl(MessageType::kWidthHeight) {}

WidthHeightMessage::Impl::Impl(int width_val, int height_val)
    : MessageImpl(MessageType::kWidthHeight),
      width(width_val),
      height(height_val) {}

int WidthHeightMessage::Impl::GetWidth() const { return width; }

int WidthHeightMessage::Impl::GetHeight() const {
  return height;
}

std::string WidthHeightMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void WidthHeightMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
