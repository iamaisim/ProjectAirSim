// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_IMU_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_IMU_MESSAGE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;
class ImuMessage : public Message {
 public:
  ImuMessage(TimeNano time_stamp, Quaternion orientation,
             Vector3 angular_velocity, Vector3 linear_acceleration);
  ImuMessage();

  ~ImuMessage() override;

  const Quaternion GetOrientation() const;
  const Vector3 GetAngularVelocity() const;
  const Vector3 GetLinearAcceleration() const;

  const std::vector<uint8_t>& GetData() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

  json getData() const;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_IMU_MESSAGE_HPP_
