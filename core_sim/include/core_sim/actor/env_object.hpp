// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_OBJECT_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_OBJECT_HPP_

#include <memory>
#include <unordered_map>
#include <vector>

#include "core_sim/actor.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/environment.hpp"
#include "core_sim/joint.hpp"
#include "core_sim/link.hpp"
#include "core_sim/link/visual.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/trajectory.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;
class ServiceManager;
class StateManager;

class  EnvObject : public Actor {
 public:
   EnvObject();

  typedef microsoft::projectairsim::Transform Pose;

  //---------------------------------------------------------------------------
  // Static config/model
  //

  const Visual& GetVisual() const;

  //---------------------------------------------------------------------------
  // Runtime
  //

  const std::unordered_map<std::string, std::vector<std::string>>&
  GetLinkChildrenMap() const;

 protected:
  friend class Scene;
  class Impl;
  class Loader;

   EnvObject(const std::string& id, const Pose& origin, const Logger& logger,
           const TopicManager& topic_manager,
           const std::string& parent_topic_path,
           const ServiceManager& service_manager,
           const StateManager& state_manager);
   EnvObject(std::shared_ptr<Impl> pimpl);

  void Load(ConfigJson config_json) override;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_OBJECT_HPP_