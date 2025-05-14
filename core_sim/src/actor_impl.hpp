// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_SRC_ACTOR_IMPL_HPP_
#define CORE_SIM_SRC_ACTOR_IMPL_HPP_

#include <string>

#include "component.hpp"
#include "core_sim/actor.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "core_sim/transforms/transform.hpp"
#include "string_utils.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class ActorImpl : public ComponentWithTopicsAndServiceMethods {
 public:
  ActorImpl(ActorType type, const std::string& id, const Transform& origin,
            const std::string& component, const Logger& logger,
            const TopicManager& topic_manager,
            const std::string& parent_topic_path,
            const ServiceManager& service_manager,
            const StateManager& state_manager)
      : ComponentWithTopicsAndServiceMethods(component, logger, id,
                                             topic_manager, parent_topic_path,
                                             service_manager, state_manager),
        type_(type),
        origin_(origin) {
    SetTopicPath();
  }

  const Transform& GetOrigin() const { return origin_; }

  ActorType GetType() const { return type_; }

 protected:
  ActorType type_;
  Transform origin_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_ACTOR_IMPL_HPP_
