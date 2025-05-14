// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_TRANSFORM_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_TRANSFORM_MESSAGE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class TransformMessage : public Message {
 public:
  TransformMessage(TimeNano time_stamp, std::string parent_frame_id,
                   std::string child_frame_id, Vector3 translation,
                   Quaternion rotation);
  TransformMessage();

  ~TransformMessage() override;

  const Vector3 GetTranslation() const;
  const Quaternion GetRotation() const;
  const std::string GetParentFrameId() const;
  const std::string GetChildFrameId() const;

  const std::vector<uint8_t>& GetData() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_TRANSFORM_MESSAGE_HPP_
