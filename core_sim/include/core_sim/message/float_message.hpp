// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_FLOAT_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_FLOAT_MESSAGE_HPP_

#include <memory>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class FloatMessage : public Message {
 public:
  explicit FloatMessage(float val);

  FloatMessage();

  ~FloatMessage() override;

  float GetValue() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_FLOAT_MESSAGE_HPP_
