// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_SRC_MESSAGE_MESSAGE_IMPL_HPP_
#define CORE_SIM_SRC_MESSAGE_MESSAGE_IMPL_HPP_

#include <stdexcept>
#include <string>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class MessageImpl {
 public:
  explicit MessageImpl(MessageType type_val) : type(type_val) {}

  virtual ~MessageImpl() {}

  MessageType GetType() { return type; }

  virtual std::string Serialize() {
    throw std::runtime_error("This method should not be called.");
  }

  virtual void Deserialize(const std::string& /*buffer*/) {
    throw std::runtime_error("This method should not be called.");
  }

 protected:
  MessageType type;
  // TODO Consider consolidating time_stamp here (for sim time data
  // synchronization and possibly also for realtime tracking of published data)
  // for all messages to remember to handle.
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_MESSAGE_MESSAGE_IMPL_HPP_
