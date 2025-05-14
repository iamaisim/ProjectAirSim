// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MESSAGE_HPP_
#define MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MESSAGE_HPP_

#include <memory>
#include <string>

namespace microsoft {
namespace projectairsim {

enum class ControlMessageType {
  kControlModelStart = 0,
  kControlModelInput = 1,
  kControlModelOutput = 2,
  kControlModelStop = 3
};

class ControlMessage {
 public:
  explicit ControlMessage(ControlMessageType type_val) : type_(type_val) {}
  virtual ~ControlMessage() {}

  ControlMessageType GetType() const { return type_; }

  virtual std::string Serialize() const = 0;

  virtual void Deserialize(const std::string& buffer) = 0;

 protected:
  ControlMessageType type_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MESSAGE_HPP_
