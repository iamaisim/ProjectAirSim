// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_BATTERY_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_BATTERY_MESSAGE_HPP_

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class BatteryStateMessage : public Message {
 public:
  BatteryStateMessage();

  BatteryStateMessage(TimeNano time_stamp, float battery_pct_remaining,
                      uint32_t estimated_time_remaining,
                      const std::string& battery_charge_state);

  ~BatteryStateMessage() override;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

  json getData() const;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_BATTERY_MESSAGE_HPP_
