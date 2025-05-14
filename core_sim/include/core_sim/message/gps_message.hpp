// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_GPS_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_GPS_MESSAGE_HPP_

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
class GpsMessage : public Message {
 public:
  GpsMessage();

  GpsMessage(TimeNano time_stamp, TimeMilli time_utc_millis, float latitude,
             float longitude, float altitude, float epv, float eph,
             int position_cov_type, int fix_type, Vector3 velocity);

  ~GpsMessage() override;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

  json getData() const;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_GPS_MESSAGE_HPP_
