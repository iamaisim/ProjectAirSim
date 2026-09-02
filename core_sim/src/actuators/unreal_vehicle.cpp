// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "core_sim/actuators/unreal_vehicle.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <utility>

#include "actuator_impl.hpp"
#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class UnrealVehicleActuator::Loader {
 public:
  explicit Loader(UnrealVehicleActuator::Impl& impl) : impl_(impl) {}

  void Load(const json& json);

 private:
  UnrealVehicleActuator::Impl& impl_;
};

class UnrealVehicleActuator::Impl : public ActuatorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const std::string& child_link, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager, const StateManager& state_manager)
      : ActuatorImpl(ActuatorType::kUnrealVehicle, id, is_enabled, parent_link,
                     child_link, Constant::Component::unreal_vehicle_actuator,
                     logger, topic_manager, parent_topic_path, service_manager,
                     state_manager),
        loader_(*this) {
    SetTopicPath();
  }

  void Load(ConfigJson config_json) { loader_.Load(config_json); }

  void OnBeginUpdate() override {}
  void OnEndUpdate() override {}

  int GetParameterIndex() const { return parameter_index_; }
  int GetControlSignalIndex() const { return control_signal_index_; }
  float GetControlSignal() const { return control_signal_.load(); }

  void UpdateActuatorOutput(std::vector<float>&& control_signals,
                            const TimeNano /*sim_dt_nanos*/) {
    if (!enabled_) return;

    if (control_signal_index_ >= static_cast<int>(control_signals.size())) {
      logger_.LogError(
          name_,
          "[%s] Controller returned %d signals, but Unreal vehicle actuator "
          "requires control signal index %d.",
          id_.c_str(), static_cast<int>(control_signals.size()),
          control_signal_index_);
      throw Error(
          "Unreal vehicle actuator control signal index is out of range.");
    }

    control_signal_.store(control_signals[control_signal_index_]);
  }

 private:
  friend class UnrealVehicleActuator::Loader;

  UnrealVehicleActuator::Loader loader_;
  int parameter_index_ = -1;
  int control_signal_index_ = 0;
  std::atomic<float> control_signal_{0.0f};
};

UnrealVehicleActuator::UnrealVehicleActuator()
    : Actuator(std::shared_ptr<ActuatorImpl>(nullptr)) {}

UnrealVehicleActuator::UnrealVehicleActuator(
    const std::string& id, bool is_enabled, const std::string& parent_link,
    const std::string& child_link, const Logger& logger,
    const TopicManager& topic_manager, const std::string& parent_topic_path,
    const ServiceManager& service_manager, const StateManager& state_manager)
    : Actuator(std::shared_ptr<ActuatorImpl>(new UnrealVehicleActuator::Impl(
          id, is_enabled, parent_link, child_link, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

void UnrealVehicleActuator::Load(ConfigJson config_json) {
  static_cast<UnrealVehicleActuator::Impl*>(pimpl_.get())->Load(config_json);
}

int UnrealVehicleActuator::GetParameterIndex() const {
  return static_cast<UnrealVehicleActuator::Impl*>(pimpl_.get())
      ->GetParameterIndex();
}

int UnrealVehicleActuator::GetControlSignalIndex() const {
  return static_cast<UnrealVehicleActuator::Impl*>(pimpl_.get())
      ->GetControlSignalIndex();
}

float UnrealVehicleActuator::GetControlSignal() const {
  return static_cast<UnrealVehicleActuator::Impl*>(pimpl_.get())
      ->GetControlSignal();
}

void UnrealVehicleActuator::UpdateActuatorOutput(
    std::vector<float>&& control_signals, const TimeNano sim_dt_nanos) {
  static_cast<UnrealVehicleActuator::Impl*>(pimpl_.get())
      ->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
}

void UnrealVehicleActuator::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_,
                           "[%s] Loading 'unreal-vehicle' actuator.",
                           impl_.id_.c_str());

  const auto settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::unreal_vehicle_settings);
  impl_.parameter_index_ = JsonUtils::GetInteger(
      settings_json, Constant::Config::parameter_index, -1);
  impl_.control_signal_index_ = JsonUtils::GetInteger(
      settings_json, Constant::Config::control_signal_index, 0);

  if (impl_.parameter_index_ < 0) {
    throw Error(
        "Unreal vehicle actuator parameter index must be non-negative.");
  }
  if (impl_.control_signal_index_ < 0) {
    throw Error(
        "Unreal vehicle actuator control signal index must be non-negative.");
  }

  impl_.is_loaded_ = true;
  impl_.logger_.LogVerbose(
      impl_.name_, "[%s] 'unreal-vehicle' actuator loaded.", impl_.id_.c_str());
}

}  // namespace projectairsim
}  // namespace microsoft
