// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/actor/env_object.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/kinematics_message.hpp"
#include "core_sim/runtime_components.hpp"
#include "env_object_impl.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// class  EnvObject

 EnvObject:: EnvObject() : Actor(nullptr) {}

 EnvObject:: EnvObject(const std::string& id, const Pose& origin,
                   const Logger& logger, const TopicManager& topic_manager,
                   const std::string& parent_topic_path,
                   const ServiceManager& service_manager,
                   const StateManager& state_manager)
    : Actor(std::shared_ptr<ActorImpl>(new EnvObject::Impl(
          id, origin, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

 EnvObject:: EnvObject(std::shared_ptr<Impl> pimpl)
    : Actor(std::shared_ptr<ActorImpl>(pimpl)) {}

void  EnvObject::Load(ConfigJson config_json) {
  return static_cast< EnvObject::Impl*>(pimpl_.get())->Load(config_json);
}

const Visual& EnvObject::GetVisual() const { 
  return static_cast<EnvObject::Impl*>(pimpl_.get())->GetVisual(); }

const std::unordered_map<std::string, std::vector<std::string>>&
 EnvObject::GetLinkChildrenMap() const {
  return static_cast< EnvObject::Impl*>(pimpl_.get())->GetLinkChildrenMap();
}

// -----------------------------------------------------------------------------
// class  EnvObject::Impl

 EnvObject::Impl::Impl(const std::string& id,
                     const Pose& origin, const Logger& logger,
                     const TopicManager& topic_manager,
                     const std::string& parent_topic_path,
                     const ServiceManager& service_manager,
                     const StateManager& state_manager)
    : ActorImpl(ActorType::kEnvObject, id, origin,
                Constant::Component::env_object, logger, topic_manager,
                parent_topic_path, service_manager, state_manager),
                loader_(*this),
                visual_settings_(logger) {
}

void  EnvObject::Impl::Load(ConfigJson config_json) {
  // Load  EnvObject from JSON config
  json json = config_json;
  loader_.Load(json);
  static_cast<TransformTree::RefFrame&>(visual_settings_)
      .SetID(std::string("L ") + GetID() + " visual");
}

const Visual& EnvObject::Impl::GetVisual() const { return visual_settings_; }

const std::unordered_map<std::string, std::vector<std::string>>&
 EnvObject::Impl::GetLinkChildrenMap() const {
  return link_children_map_;
}

// class  EnvObject::Loader
 EnvObject::Loader::Loader( EnvObject::Impl& impl) : impl_(impl) {}

void  EnvObject::Loader::Load(const json& json) {
  LoadVisual(json);
  impl_.is_loaded_ = true;
}

void EnvObject::Loader::LoadVisual(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'visual'.",
                           impl_.id_.c_str());

  auto visual_json = JsonUtils::GetJsonObject(json, Constant::Config::visual);
  impl_.visual_settings_.Load(visual_json);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'visual' loaded.",
                           impl_.id_.c_str());
}

}  // namespace projectairsim
}  // namespace microsoft
