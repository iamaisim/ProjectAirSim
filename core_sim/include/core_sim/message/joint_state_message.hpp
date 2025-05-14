// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_JOINT_STATE_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_JOINT_STATE_MESSAGE_HPP_

#include <string>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class JointStateMessage : public Message {
 public:
  JointStateMessage(float position_val, float velocity_val, float effort_val);

  JointStateMessage();

  ~JointStateMessage() override;

  float GetPosition() const;

  float GetVelocity() const;

  float GetEffort() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_JOINT_STATE_MESSAGE_HPP_
