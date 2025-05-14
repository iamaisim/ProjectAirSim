// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_DISTANCE_SENSOR_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_DISTANCE_SENSOR_MESSAGE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class DistanceSensorMessage : public Message {
 public:
  DistanceSensorMessage(TimeNano time_stamp_val, float current_distance, Pose pose_val);

  DistanceSensorMessage();

  ~DistanceSensorMessage() override;

  const float GetCurrentDistance() const;

  const Pose GetPose() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_DISTANCE_SENSOR_MESSAGE_HPP_
