#ifndef CORE_SIM_ACTOR_ENV_OBJECT_IMPL_HPP_
#define CORE_SIM_ACTOR_ENV_OBJECT_IMPL_HPP_

#include "actor_impl.hpp"
#include "core_sim/actor/env_object.hpp"
#include "env_object_loader.hpp"

namespace microsoft {
namespace projectairsim {

class EnvObject::Impl : public ActorImpl {
 public:
  typedef microsoft::projectairsim::Transform Pose;

  Impl(const std::string& id, const Pose& origin,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  virtual void Load(ConfigJson config_json);

  const Visual& GetVisual() const;

  const std::unordered_map<std::string, std::vector<std::string>>&
  GetLinkChildrenMap() const;

 protected:
  friend class EnvObject;

  std::unordered_map<std::string, std::vector<std::string>> link_children_map_;

  Visual visual_settings_;
  EnvObject::Loader loader_;
  std::vector<std::reference_wrapper<Topic>> topics_;

};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_ACTOR_ENV_ACTOR_IMPL_HPP_
