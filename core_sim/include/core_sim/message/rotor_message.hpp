// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_ROTOR_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_ROTOR_MESSAGE_HPP_

#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

struct RotorInfo {
  std::string rotor_id;
  float speed;
  float angle;
  float torque;
  float thrust;

  RotorInfo() {}
  RotorInfo(const std::string& in_id, const float in_speed,
            const float in_angle, const float in_torque, const float in_thrust)
      : rotor_id(in_id),
        speed(in_speed),
        angle(in_angle),
        torque(in_torque),
        thrust(in_thrust) {}
};

class RotorMessage : public Message {
 public:
  RotorMessage();

  RotorMessage(TimeNano time_stamp,
               const std::vector<RotorInfo>& rotor_info_vec);

  ~RotorMessage() override;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_ROTOR_MESSAGE_HPP_