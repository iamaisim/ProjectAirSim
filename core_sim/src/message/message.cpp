// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/message.hpp"

#include <memory>

#include "message_impl.hpp"

namespace microsoft {
namespace projectairsim {

Message::Message(const std::shared_ptr<MessageImpl>& pimpl) : pimpl_(pimpl) {}

Message::~Message() {}

MessageType Message::GetType() const { return pimpl_->GetType(); }

std::string Message::Serialize() const { return pimpl_->Serialize(); }

void Message::Deserialize(const std::string& buffer) {
  pimpl_->Deserialize(buffer);
}

}  // namespace projectairsim
}  // namespace microsoft
