// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_KINEMATICS_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_KINEMATICS_MESSAGE_HPP_

#include <memory>
#include <string>

#include "core_sim/clock.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class KinematicsMessage : public Message {
 public:
  KinematicsMessage(const TimeNano time_stamp_val,
                    const Kinematics kinematics_val);

  KinematicsMessage();

  ~KinematicsMessage() override;

  TimeNano GetTimeStamp() const;
  Kinematics GetKinematics() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_KINEMATICS_MESSAGE_HPP_
