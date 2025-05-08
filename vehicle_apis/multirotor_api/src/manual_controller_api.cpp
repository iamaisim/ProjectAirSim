// Copyright (C) Microsoft Corporation. All rights reserved.

#include "manual_controller_api.hpp"

#include "json.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class ManualControllerApi

ManualControllerApi::ManualControllerApi(const Robot& robot)
    : sim_robot_(robot) {
  LoadSettings(robot);
}

//---------------------------------------------------------------------------
// IController overrides

void ManualControllerApi::BeginUpdate() { RegisterServiceMethods(); }

void ManualControllerApi::EndUpdate() {}

void ManualControllerApi::Reset() {
  std::fill(motor_output_.begin(), motor_output_.end(), 0.0f);
}

void ManualControllerApi::SetKinematics(const Kinematics* kinematics) {}

void ManualControllerApi::Update() {}

std::vector<float> ManualControllerApi::GetControlSignals(const std::string& actuator_id) {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);
  if (actuator_map_itr == actuator_id_to_output_idx_map_.end()) {
    GetLogger().LogWarning("ManualControllerApi",
                           "ManualControllerApi::GetControlSignal() called for "
                           "invalid actuator: %s",
                           actuator_id.c_str());
    return std::vector<float>(1,0.0f);
  }

  float output = motor_output_.at(actuator_map_itr->second);

  return std::vector<float>(1, output);
}

const IController::GimbalState& ManualControllerApi::GetGimbalSignal(
    const std::string& gimbal_id) {
  throw std::runtime_error(
      "This flight controller does not support externally-controlled gimbal "
      "devices.");
}

//---------------------------------------------------------------------------
// ManualControllerApi methods

bool ManualControllerApi::SetControlSignals(
    const std::unordered_map<std::string, float>& control_signal_map) {
  std::lock_guard<std::mutex> lock(update_lock_);
  bool is_success = true;

  for (auto& [actuator_id, control_val] : control_signal_map) {
    auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);
    if (actuator_map_itr == actuator_id_to_output_idx_map_.end()) {
      GetLogger().LogWarning(
          "ManualControllerApi",
          "ManualControllerApi::SetControlSignals() called for "
          "invalid actuator: %s",
          actuator_id.c_str());
      is_success = false;
      continue;
    }

    motor_output_.at(actuator_map_itr->second) = control_val;
  }

  return is_success;
}

void ManualControllerApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  // GetJsonObject
  const json& manual_controller_api_settings_json =
      controller_settings_json.value("manual-controller-api-settings",
                                     "{ }"_json);

  // GetArray
  const json& actuator_order_json =
      manual_controller_api_settings_json.value("actuator-order", "[ ]"_json);

  motor_output_.assign(actuator_order_json.size(), 0.0f);

  try {
    int output_idx = 0;
    for (auto& actuator_json : actuator_order_json) {
      // Add actuator ID to map for this output index
      std::string id = actuator_json.value("id", "");
      actuator_id_to_output_idx_map_.insert({id, output_idx});

      // Set initial control value if specified
      if (actuator_json.contains("initial-value")) {
        float init_val = actuator_json.value("initial-value", 0.0f);
        motor_output_.at(output_idx) = init_val;
      }

      output_idx++;
    }
  } catch (...) {
    throw;
  }
}

void ManualControllerApi::RegisterServiceMethods() {
  // Register SetControlSignal
  auto method = ServiceMethod("SetControlSignals", {"control_signal_map"});
  auto method_handler = method.CreateMethodHandler(
      &ManualControllerApi::SetControlSignals, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

}  // namespace projectairsim
}  // namespace microsoft
