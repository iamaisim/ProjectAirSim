// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef PHYSICS_INCLUDE_MESSAGE_PHYSICS_MESSAGE_HPP_
#define PHYSICS_INCLUDE_MESSAGE_PHYSICS_MESSAGE_HPP_

#include <memory>
#include <string>

namespace microsoft {
namespace projectairsim {

enum class PhysicsMessageType {
  kPhysicsModelStart = 0,
  kPhysicsModelInput = 1,
  kPhysicsModelOutput = 2,
  kPhysicsModelStop = 3
};

class PhysicsMessage {
 public:
  explicit PhysicsMessage(PhysicsMessageType type_val) : type_(type_val) {}
  virtual ~PhysicsMessage() {}

  PhysicsMessageType GetType() const { return type_; }

  virtual std::string Serialize() const = 0;

  virtual void Deserialize(const std::string& buffer) = 0;

 protected:
  PhysicsMessageType type_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_MESSAGE_PHYSICS_MESSAGE_HPP_
