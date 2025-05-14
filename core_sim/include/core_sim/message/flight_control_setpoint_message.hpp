// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_FLIGHT_CONTROL_SETPOINT_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_FLIGHT_CONTROL_SETPOINT_MESSAGE_HPP_

#include <memory>
#include <vector>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class FlightControlSetpointMessage : public Message {
 public:
  FlightControlSetpointMessage(float axes_0_val, float axes_1_val,
                               float axes_2_val, float axes_3_val);

  FlightControlSetpointMessage();

  ~FlightControlSetpointMessage() override;

  std::vector<float> GetAxesVec() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_FLIGHT_CONTROL_SETPOINT_MESSAGE_HPP_
