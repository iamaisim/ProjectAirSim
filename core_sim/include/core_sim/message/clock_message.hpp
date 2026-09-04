// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_CLOCK_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_CLOCK_MESSAGE_HPP_

#include <memory>

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class ClockMessage : public Message {
 public:
  explicit ClockMessage(TimeNano time_stamp_val);
  ClockMessage();
  ~ClockMessage() override;

  TimeNano GetTimeStamp() const;

  std::string Serialize() const override;
  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_CLOCK_MESSAGE_HPP_
