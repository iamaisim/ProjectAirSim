// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_COLLISION_INFO_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_COLLISION_INFO_MESSAGE_HPP_

#include <memory>
#include <string>

#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class CollisionInfoMessage : public Message {
 public:
  CollisionInfoMessage(const CollisionInfo collision_info);

  CollisionInfoMessage();

  ~CollisionInfoMessage() override;

  CollisionInfo GetCollisionInfo() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_COLLISION_INFO_MESSAGE_HPP_
