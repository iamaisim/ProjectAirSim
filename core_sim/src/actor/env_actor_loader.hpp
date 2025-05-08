// env_actor_loader.hpp

#ifndef CORE_SIM_ACTOR_ENV_ACTOR_LOADER_HPP_
#define CORE_SIM_ACTOR_ENV_ACTOR_LOADER_HPP_

#include "core_sim/actor/env_actor.hpp"

#include "json.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class EnvActor; // Forward declaration

class EnvActor::Loader {
 public:
  explicit Loader(EnvActor::Impl& impl);

  void Load(const json& json);

 private:
  void LoadLinks(const json& json);

  Link LoadLink(const json& json);

  void LoadLinkRotationSettings(const json& json, const std::string& link_name);

  void LoadJoints(const json& json);

  Joint LoadJoint(const json& json);

  void LoadEnvActorScript(const json& json);

  EnvActor::Impl& impl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_ACTOR_ENV_ACTOR_LOADER_HPP_
