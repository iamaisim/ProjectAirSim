// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_INT32_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_INT32_MESSAGE_HPP_

#include <memory>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class Int32Message : public Message {
 public:
  explicit Int32Message(int32_t val);

  Int32Message();

  ~Int32Message() override;

  int32_t GetValue() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_INT32_MESSAGE_HPP_
