// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "Message.h"

#include "IMessageBuffer.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

ASC_DECL Message::Message(void) noexcept : RefCountedContainer() {}

ASC_DECL Message::Message(const Message& message_other) noexcept
    : RefCountedContainer(message_other) {}

ASC_DECL Message::Message(Message&& message_other) noexcept
    : RefCountedContainer(std::move(message_other)) {}

ASC_DECL Message::~Message() { Clear(); }

ASC_DECL void Message::Clear(void) { RefCountedContainer::Clear(); }

ASC_DECL const std::uint8_t* Message::Data(void) const {
  if (piref_counted_ == nullptr) return (nullptr);

  return (static_cast<IMessageBuffer*>(piref_counted_)->Data());
}

ASC_DECL size_t Message::Cb(void) const {
  if (piref_counted_ == nullptr) return (0);

  return (static_cast<IMessageBuffer*>(piref_counted_)->Cb());
}

ASC_DECL void Message::SetMessageBuffer(IMessageBuffer* pimessage_buffer) {
  RefCountedContainer::SetPIRefCounted(pimessage_buffer);
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
