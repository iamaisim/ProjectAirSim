// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_WHEEL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_WHEEL_HPP_

#include <cmath>
#include <string>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_tree.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class ActuatorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

struct WheelSetting {
  Transform origin_setting;
  Vector3 normal_vector = Vector3(0, 0, -1);

  float wheel_type = 0.0f;  // 0.0 is rear wheel, 1.0 is front wheel
  float coeff_of_friction = 0.0f;
  float coeff_of_torque = 1.0f;
  // TO-DO: Make this value configurable.
  float radius_ = 0.457f;     // Radius of the wheel in meters
  float smoothing_tc = 0.0f;  // Time constant for low pass filter
  float max_torque =
      0.0f;  // Maximum torque the wheel can generate in Newton meters
  float max_force = 0.0f;  // Maximum force the wheel can generate in Newtons
  float max_acceleration = 0.0f;  // Maximum acceleration the wheel can achieve
                                  // in meters per second squared
  bool engine_connected_ = true;
  bool steering_connected_ = true;
  bool brake_connected_ = true;

  WheelSetting() {
    origin_setting.translation_ = Vector3(0, 0, 0);
    origin_setting.rotation_ = Quaternion::Identity();
    CalcMaxTorqueAndForce();
  }

  void CalcMaxTorqueAndForce() {
    max_force = coeff_of_friction;
    max_torque = max_force * radius_;
    max_acceleration = max_force / 1.0f;  // Assuming mass of the wheel is 1 kg
  }
};

//------------------------------------------------------------------------------
// class Wheel

class Wheel : public Actuator {
 public:
  Wheel();

  void BeginUpdate();

  void EndUpdate();

  const WheelSetting& GetWheelSettings() const;

  const WrenchPoint& GetWrenchPoint() const;

  float GetSteering() const;

  bool IsEngineConnected() const;

  bool IsBrakeConnected() const;

  bool IsSteeringConnected() const;

  float GetAngle() const;

  float GetRotatingSpeed() const;

  float GetSteeringSpeed() const;

  float GetTorque() const;

  float GetForce() const;

  float GetRadius() const;

  const ActuatedRotations& GetActuatedRotations() const;
  const ActuatedTransforms& GetActuatedTransforms() const;

  const float GetPowerConsumption() const;

  void UpdateActuatorOutput(std::vector<float>&& control_signals,
                            const TimeNano sim_dt_nanos) override;

  // These conversion operators allow this object to be passed directly to
  // TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Robot;
  friend class ActuatorImpl;

  Wheel(const std::string& id, bool is_enabled, const std::string& parent_link,
        const std::string& child_link, const Logger& logger,
        const TopicManager& topic_manager, const std::string& parent_topic_path,
        const ServiceManager& service_manager,
        const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  class Impl;
  class Loader;
};
}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_ROTOR_HPP_
