// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.
#ifndef CORE_SIM_SRC_MESSAGE_MESSAGE_UTILS_HPP_
#define CORE_SIM_SRC_MESSAGE_MESSAGE_UTILS_HPP_

#include <string>

#include "core_sim/message/message.hpp"
#include "core_sim/topic.hpp"

namespace microsoft {
namespace projectairsim {

class MessageUtils {
 public:
  static Message ToMessage(const Topic& topic, const std::string& buffer);
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_MESSAGE_MESSAGE_UTILS_HPP_
