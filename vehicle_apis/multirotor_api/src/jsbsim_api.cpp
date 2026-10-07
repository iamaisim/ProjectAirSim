// Copyright (C) IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "jsbsim_api.hpp"

#include <mutex>

#include "json.hpp"

// JSBSim requires RTTI, so its headers must not be included from jsbsim_api.hpp.
#include "FGFDMExec.h"

namespace microsoft {
namespace projectairsim {

JSBSimApi::JSBSimApi(const Robot& robot, TransformTree* ptransformtree)
    : sim_robot_(robot) {
  static_cast<void>(ptransformtree);
  LoadSettings(robot);
}

void JSBSimApi::LoadSettings(const Robot& robot) {
  if (robot.GetPhysicsType() != PhysicsType::kJSBSimPhysics) {
    throw std::runtime_error(
        "JSBSimApi can only be used with a robot that has JSBSim physics type");
  }

  if (robot.GetJSBSimModel() == nullptr) {
    throw std::runtime_error(
        "JSBSimApi requires a robot with a valid JSBSim model");
  }

  // The legacy actuator getter treats each actuator ID as a JSBSim property.
  // Freeze that ordering at configuration time for indexed snapshot access.
  for (const auto& actuator_ref : robot.GetActuators()) {
    const auto& actuator = actuator_ref.get();
    if (actuator.GetType() == ActuatorType::kGimbal) continue;
    const auto& property = actuator.GetId();
    const int index = static_cast<int>(output_properties_.size());
    if (actuator_id_to_output_idx_map_.emplace(property, index).second) {
      output_properties_.push_back(property);
    }
  }
}

void JSBSimApi::BeginUpdate() {
  RegisterServiceMethods();
}

void JSBSimApi::EndUpdate() {}

void JSBSimApi::Reset() {}

void JSBSimApi::SetKinematics(const Kinematics* /*kinematics*/) {}

void JSBSimApi::Update() {}

const IController::GimbalState& JSBSimApi::GetGimbalSignal(
    const std::string& /*gimbal_id*/) {
  return gimbal_state_;
}

float JSBSimApi::GetJSBSimProperty(const std::string& property) {
  std::lock_guard<std::mutex> lock(jsbsim_property_mutex_);
  return GetJSBSimPropertyUnlocked(property);
}

bool JSBSimApi::SetJSBSimProperty(const std::string& property, float value) {
  std::lock_guard<std::mutex> lock(jsbsim_property_mutex_);
  return SetJSBSimPropertyUnlocked(property, value);
}

float JSBSimApi::GetJSBSimPropertyUnlocked(const std::string& property) {
  return sim_robot_.GetJSBSimModel()->GetPropertyValue(property);
}

bool JSBSimApi::SetJSBSimPropertyUnlocked(const std::string& property,
                                          float value) {
  sim_robot_.GetJSBSimModel()->SetPropertyValue(property, value);
  return true;
}

void JSBSimApi::RegisterServiceMethods() {
  auto method = ServiceMethod("GetJSBSimProperty", {"_property_name"});
  auto method_handler =
      method.CreateMethodHandler(&JSBSimApi::GetJSBSimProperty, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  method = ServiceMethod("SetJSBSimProperty", {"_property_name", "_value"});
  method_handler =
      method.CreateMethodHandler(&JSBSimApi::SetJSBSimProperty, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

int JSBSimApi::GetControlSignalIndex(const std::string& actuator_id) {
  const auto it = actuator_id_to_output_idx_map_.find(actuator_id);
  return it == actuator_id_to_output_idx_map_.end() ? -1 : it->second;
}

void JSBSimApi::GetControlSignalSnapshot(std::vector<float>& control_signals) {
  std::lock_guard<std::mutex> lock(jsbsim_property_mutex_);
  control_signals.resize(output_properties_.size());
  for (size_t index = 0; index < output_properties_.size(); ++index) {
    control_signals[index] = GetJSBSimPropertyUnlocked(output_properties_[index]);
  }
}

std::vector<float> JSBSimApi::GetControlSignals(int signal_index) {
  std::lock_guard<std::mutex> lock(jsbsim_property_mutex_);
  if (signal_index < 0 ||
      signal_index >= static_cast<int>(output_properties_.size())) {
    return {0.f};
  }
  return {GetJSBSimPropertyUnlocked(output_properties_[signal_index])};
}

std::vector<float> JSBSimApi::GetControlSignals(
    const std::string& actuator_id) {
  return {GetJSBSimProperty(actuator_id)};
}

}  // namespace projectairsim
}  // namespace microsoft
