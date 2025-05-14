// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_MAGNETOMETER_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_MAGNETOMETER_MESSAGE_HPP_

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class MagnetometerMessage : public Message {
 public:
  MagnetometerMessage();

  MagnetometerMessage(TimeNano time_stamp, Vector3 magnetic_field_body,
                      std::vector<float> magnetic_field_covariance);

  ~MagnetometerMessage() override;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

  json getData() const;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_MAGNETOMETER_MESSAGE_HPP_
