// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_INT_LIST_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_INT_LIST_MESSAGE_HPP_

#include <initializer_list>
#include <memory>
#include <vector>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

// This message is used for transporting remote control input (such as from
// manual input via a physical remote control transmitter or game controller)
// to the flight controller.
class IntListMessage : public Message {
 public:
  IntListMessage(std::initializer_list<int> values);

  IntListMessage(void);

  ~IntListMessage() override;

  // Return the array of int values
  std::vector<int> GetValues(void) const;

  std::string Serialize(void) const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_INT_LIST_MESSAGE_HPP_
