// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "unreal_vehicle_api.hpp"

#include <stdexcept>

#include "json.hpp"

namespace microsoft {
namespace projectairsim {

UnrealVehicleApi::UnrealVehicleApi(const Robot& robot) : sim_robot_(robot) {
  // Parse actuators from controller settings JSON
  const auto& settings_str = robot.GetControllerSettings();
  if (!settings_str.empty()) {
    try {
      auto settings_json = nlohmann::json::parse(settings_str);
      if (settings_json.contains("unreal-vehicle-api-settings")) {
        auto& api_settings = settings_json["unreal-vehicle-api-settings"];
        if (api_settings.contains("actuators") && api_settings["actuators"].is_array()) {
          for (const auto& act : api_settings["actuators"]) {
            std::string name = act.value("name", "");
            float default_val = act.value("default-value", 0.0f);
            if (!name.empty()) {
              actuator_signals_[name] = default_val;
              actuator_order_.push_back(name);
            }
          }
        }
      }
    } catch (...) {
      // If parsing fails, start with empty actuator map
      GetLogger().LogWarning("UnrealVehicleApi",
                             "Failed to parse actuator settings: %s",
                             settings_str.c_str());
    }
  }
}

void UnrealVehicleApi::BeginUpdate() { RegisterServiceMethods(); }

void UnrealVehicleApi::EndUpdate() {}

void UnrealVehicleApi::Reset() {
  std::lock_guard<std::mutex> lock(actuator_mutex_);
  for (auto& kv : actuator_signals_) kv.second = 0.0f;
}

void UnrealVehicleApi::SetKinematics(const Kinematics* kinematics) {
  kinematics_ = kinematics;
}

void UnrealVehicleApi::Update() {
  // No-op: the unreal vehicle manages its own physics.
}

std::vector<float> UnrealVehicleApi::GetControlSignals(
    const std::string& actuator_id) {
  // Unreal vehicle actuators are not managed by ProjectAirSim's actuator system
  return {};
}

const IController::GimbalState& UnrealVehicleApi::GetGimbalSignal(
    const std::string& gimbal_id) {
  throw std::runtime_error(
      "UnrealVehicleApi does not support gimbal devices.");
}

void UnrealVehicleApi::SetActuatorSignal(const std::string& name,
                                         float signal) {
  std::lock_guard<std::mutex> lock(actuator_mutex_);
  actuator_signals_[name] = signal;
}

float UnrealVehicleApi::GetActuatorSignal(const std::string& name) const {
  std::lock_guard<std::mutex> lock(actuator_mutex_);
  auto it = actuator_signals_.find(name);
  if (it != actuator_signals_.end()) {
    return it->second;
  }
  return 0.0f;
}

std::unordered_map<std::string, float>
UnrealVehicleApi::GetAllActuatorSignals() const {
  std::lock_guard<std::mutex> lock(actuator_mutex_);
  return actuator_signals_;
}

std::vector<float> UnrealVehicleApi::GetOrderedSignals() const {
  std::lock_guard<std::mutex> lock(actuator_mutex_);
  std::vector<float> result;
  result.reserve(actuator_order_.size());
  for (const auto& name : actuator_order_) {
    auto it = actuator_signals_.find(name);
    result.push_back(it != actuator_signals_.end() ? it->second : 0.0f);
  }
  return result;
}

// Service method handlers
bool UnrealVehicleApi::SetParameter(const std::string& name, float value) {
  SetActuatorSignal(name, value);
  return true;
}

bool UnrealVehicleApi::SetActuator(const std::string& name, float signal) {
  SetActuatorSignal(name, signal);
  return true;
}

void UnrealVehicleApi::RegisterServiceMethods() {
  auto set_parameter = ServiceMethod("SetParameter", {"name", "value"});
  auto set_parameter_handler =
      set_parameter.CreateMethodHandler(&UnrealVehicleApi::SetParameter, *this);
  sim_robot_.RegisterServiceMethod(set_parameter, set_parameter_handler);

  auto set_actuator = ServiceMethod("SetActuator", {"name", "signal"});
  auto set_actuator_handler =
      set_actuator.CreateMethodHandler(&UnrealVehicleApi::SetActuator, *this);
  sim_robot_.RegisterServiceMethod(set_actuator, set_actuator_handler);
}

}  // namespace projectairsim
}  // namespace microsoft
