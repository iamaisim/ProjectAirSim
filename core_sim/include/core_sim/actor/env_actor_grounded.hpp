// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_ACTOR_GROUNDED_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_ACTOR_GROUNDED_HPP_

#include <memory>
#include <unordered_map>
#include <vector>

#include "core_sim/trajectory.hpp"
#include "env_actor.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;
class ServiceManager;
class StateManager;

class EnvActorGrounded : public EnvActor {
 public:
  EnvActorGrounded();

  typedef microsoft::projectairsim::Transform Pose;

  void SetGroundLevel(float ground_level_);

  void SetPositionZOffset(float position_z_offset_);

 protected:
  friend class Scene;

  class Impl;

  EnvActorGrounded(const std::string& id, const Pose& origin, Logger& logger,
                   TopicManager& topic_manager,
                   const std::string& parent_topic_path,
                   ServiceManager& service_manager,
                   StateManager& state_manager);

  EnvActorGrounded(std::shared_ptr<Impl> pimpl);
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_ACTOR_GROUNDED_HPP_