// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_BAROMETER_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_BAROMETER_MESSAGE_HPP_

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class BarometerMessage : public Message {
 public:
  BarometerMessage();

  BarometerMessage(TimeNano time_stamp, float altitude, float pressure,
                   float qnh);

  ~BarometerMessage() override;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

  json getData() const;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_BAROMETER_MESSAGE_HPP_
