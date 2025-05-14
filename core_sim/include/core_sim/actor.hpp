// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTOR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTOR_HPP_

#include <memory>
#include <string>

#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class ActorImpl;
class ConfigJson;

enum class ActorType { kRobot = 0, kEnvActor = 1, kEnvObject = 2 };

class Actor {
 public:
  virtual ~Actor() {}

  bool IsLoaded() const;

  const std::string& GetID() const;

  const Transform& GetOrigin() const;

  ActorType GetType() const;

  virtual void Load(ConfigJson config_json) = 0;

 protected:
  explicit Actor(const std::shared_ptr<ActorImpl>& pimpl);

  std::shared_ptr<ActorImpl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTOR_HPP_
