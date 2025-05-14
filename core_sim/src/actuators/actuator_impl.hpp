// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_SRC_ACTUATORS_ACTUATOR_IMPL_HPP_
#define CORE_SIM_SRC_ACTUATORS_ACTUATOR_IMPL_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../topic_manager.hpp"
#include "component.hpp"
#include "constant.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/gimbal.hpp"
#include "core_sim/actuators/lift_drag_control_surface.hpp"
#include "core_sim/actuators/rotor.hpp"
#include "core_sim/actuators/wheel.hpp"
#include "core_sim/actuators/tilt.hpp"
#include "core_sim/error.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class ActuatorImpl : public ComponentWithTopicsAndServiceMethods {
 public:
  ActuatorImpl(ActuatorType type, const std::string& id, bool enabled,
               const std::string& parent_link, const std::string& child_link,
               const std::string& component, const Logger& logger,
               const TopicManager& topic_manager,
               const std::string& parent_topic_path,
               const ServiceManager& service_manager,
               const StateManager& state_manager)
      : ComponentWithTopicsAndServiceMethods(component, logger, id,
                                             topic_manager, parent_topic_path,
                                             service_manager, state_manager),
        type_(type),
        enabled_(enabled),
        parent_link_(parent_link),
        child_link_(child_link) {}

  const bool IsEnabled() const { return enabled_; }

  ActuatorType GetType() const { return type_; }

  const std::string& GetParentLink() const { return parent_link_; }

  const std::string& GetChildLink() const { return child_link_; }

  bool UpdateFaultInjectionEnabledState(bool enabled) {
    is_fault_injected_ = enabled;
    return true;
  };

  // static helpers

  static void LoadActuators(const json& json,
                            std::vector<std::unique_ptr<Actuator>>& actuators,
                            const Logger& actor_logger,
                            const std::string& actor_name,
                            const std::string& actor_id,
                            const TopicManager& topic_manager,
                            const std::string& parent_topic_path,
                            const ServiceManager& service_manager,
                            const StateManager& state_manager) {
    actor_logger.LogVerbose(actor_name, "[%s] Loading 'actuators'.",
                            actor_id.c_str());

    auto actuators_json =
        JsonUtils::GetArray(json, Constant::Config::actuators);

    if (JsonUtils::IsEmptyArray(actuators_json)) {
      actor_logger.LogVerbose(actor_name, "[%s] 'actuators' missing or empty.",
                              actor_id.c_str());
    }

    try {
      for (auto& actuator_json : actuators_json) {
        auto actuator = LoadActuator(actuator_json, actor_logger, actor_name,
                                     actor_id, topic_manager, parent_topic_path,
                                     service_manager, state_manager);
        actuators.emplace_back(std::move(actuator));
      }
    } catch (...) {
      actuators.clear();
      throw;
    }

    actor_logger.LogVerbose(actor_name, "[%s] 'actuators' loaded.",
                            actor_id.c_str());
  }

  static std::unique_ptr<Actuator> LoadActuator(
      const json& json, const Logger& actor_logger,
      const std::string& actor_name, const std::string& actor_id,
      const TopicManager& topic_manager, const std::string& parent_topic_path,
      const ServiceManager& service_manager,
      const StateManager& state_manager) {
    auto id = GetActuatorID(json, actor_logger, actor_name, actor_id);
    auto type = GetActuatorType(json, id, actor_logger, actor_name, actor_id);
    auto enabled =
        GetActuatorEnabled(json, id, actor_logger, actor_name, actor_id);
    auto parent_link =
        GetActuatorParentLink(json, id, actor_logger, actor_name, actor_id);
    auto child_link =
        GetActuatorChildLink(json, id, actor_logger, actor_name, actor_id);

    actor_logger.LogVerbose(actor_name, "[%s][%s] Loading 'actuator'.",
                            actor_id.c_str(), id.c_str());

    if (type == Constant::Config::rotor) {
      auto rotor = new Rotor(id, enabled, parent_link, child_link, actor_logger,
                             topic_manager, parent_topic_path, service_manager, state_manager);
      rotor->Load(json);
      return std::unique_ptr<Actuator>(rotor);
    } else if (type == Constant::Config::lift_drag_control_surface) {
      auto lift_drag_control_surface = new LiftDragControlSurface(
          id, enabled, parent_link, child_link, actor_logger, topic_manager,
          parent_topic_path, service_manager, state_manager);
      lift_drag_control_surface->Load(json);
      return std::unique_ptr<Actuator>(lift_drag_control_surface);
    } else if (type == Constant::Config::tilt) {
      auto tilt = new Tilt(id, enabled, parent_link, child_link, actor_logger,
                           topic_manager, parent_topic_path, service_manager,
                           state_manager);
      tilt->Load(json);
      return std::unique_ptr<Actuator>(tilt);
    } else if (type == Constant::Config::gimbal) {
      auto gimbal = new Gimbal(id, enabled, parent_link, child_link,
                               actor_logger, topic_manager, parent_topic_path,
                               service_manager, state_manager);
      gimbal->Load(json);
      return std::unique_ptr<Actuator>(gimbal);
    } 
    else if (type == Constant::Config::wheel) {
      auto wheel = new Wheel(id, enabled, parent_link, child_link,
                               actor_logger, topic_manager, parent_topic_path,
                               service_manager, state_manager);
      wheel->Load(json);
      return std::unique_ptr<Actuator>(wheel);
    } 
    else {
      actor_logger.LogError(actor_name, "[%s] Invalid actuator type '%s'.",
                            id.c_str(), type.c_str());
      throw Error("Invalid actuator type.");
    }

    actor_logger.LogVerbose(actor_name, "[%s][%s] 'actuator' loaded.",
                            actor_id.c_str(), id.c_str());
  }

  static std::string GetActuatorID(const json& json, const Logger& actor_logger,
                                   const std::string& actor_name,
                                   const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name, "[%s] Loading 'actuator.name'.",
                            actor_id.c_str());
    auto id = JsonUtils::GetIdentifier(json, Constant::Config::name);
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'actuator.name' loaded.",
                            actor_id.c_str(), id.c_str());

    return id;
  }

  static std::string GetActuatorType(const json& json,
                                     const std::string& actuator_id,
                                     const Logger& actor_logger,
                                     const std::string& actor_name,
                                     const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name, "[%s][%s] Loading 'actuator.type'.",
                            actor_id.c_str(), actuator_id.c_str());
    auto type = JsonUtils::GetIdentifier(json, Constant::Config::type);
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'actuator.type' loaded.",
                            actor_id.c_str(), actuator_id.c_str());

    return type;
  }

  static bool GetActuatorEnabled(const json& json,
                                 const std::string& actuator_id,
                                 const Logger& actor_logger,
                                 const std::string& actor_name,
                                 const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name, "[%s][%s] Loading 'actuator.enabled'.",
                            actor_id.c_str(), actuator_id.c_str());
    auto enabled =
        JsonUtils::GetInteger(json, Constant::Config::enabled, 1 /*default*/);
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'actuator.enabled' loaded.",
                            actor_id.c_str(), actuator_id.c_str());

    return enabled;
  }

  static std::string GetActuatorParentLink(const json& json,
                                           const std::string& actuator_id,
                                           const Logger& actor_logger,
                                           const std::string& actor_name,
                                           const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name,
                            "[%s][%s] Loading 'actuator.parent_link'.",
                            actor_id.c_str(), actuator_id.c_str());

    std::string parent_link =
        JsonUtils::GetString(json, Constant::Config::parent_link, "");
    if (parent_link == "") {
      actor_logger.LogVerbose(actor_name,
                              "'parent_link' missing or empty. Using default.");
    }
    actor_logger.LogVerbose(actor_name,
                            "[%s][%s] 'actuator.parent_link' loaded.",
                            actor_id.c_str(), actuator_id.c_str());

    return parent_link;
  }

  static std::string GetActuatorChildLink(const json& json,
                                          const std::string& actuator_id,
                                          const Logger& actor_logger,
                                          const std::string& actor_name,
                                          const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name,
                            "[%s][%s] Loading 'actuator.child_link'.",
                            actor_id.c_str(), actuator_id.c_str());

    std::string child_link =
        JsonUtils::GetString(json, Constant::Config::child_link, "");
    if (child_link == "") {
      actor_logger.LogVerbose(
          actor_name, "'child_link_link' missing or empty. Using default.");
    }
    actor_logger.LogVerbose(actor_name,
                            "[%s][%s] 'actuator.child_link' loaded.",
                            actor_id.c_str(), actuator_id.c_str());

    return child_link;
  }

  void RegisterServiceMethods() {
    auto toggle_fault_method_name = topic_path_ + "/" + "ToggleFault";
    auto toggle_fault_method = ServiceMethod(toggle_fault_method_name, {"enable"});
    auto toggle_fault_method_handler = toggle_fault_method.CreateMethodHandler(
        &ActuatorImpl::UpdateFaultInjectionEnabledState, *this);
    service_manager_.RegisterMethod(toggle_fault_method,
                                    toggle_fault_method_handler);
  }

 protected:
  ActuatorType type_;
  bool is_fault_injected_ = false;
  bool enabled_;
  std::string parent_link_;
  std::string child_link_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_ACTUATORS_ACTUATOR_IMPL_HPP_
