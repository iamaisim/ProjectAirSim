// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_READY_STATE_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_READY_STATE_MESSAGE_HPP_

#include <memory>
#include <string>

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class ReadyStateMessage : public Message {
 public:
  ReadyStateMessage(const TimeNano time_stamp_val, const bool ready_val,
                    const std::string ready_message);

  ReadyStateMessage();

  ~ReadyStateMessage() override;

  TimeNano GetTimeStamp() const;
  bool GetReadyVal() const;
  std::string GetReadyMessage() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_READY_STATE_MESSAGE_HPP_
