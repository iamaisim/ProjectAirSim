// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.
#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_ROTOR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_ROTOR_HPP_

#include <cmath>
#include <string>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/message/rotor_message.hpp"
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

// In NED system, +ve torque would generate clockwise rotation
enum class RotorTurningDirection : int {
  kRotorTurningDirectionCCW = -1,
  kRotorTurningDirectionCW = 1
};

// TODO NED v/s ENU
struct RotorSetting {
  Transform origin_setting;
  Vector3 normal_vector = Vector3(0, 0, -1);
  RotorTurningDirection turning_direction =
      RotorTurningDirection::kRotorTurningDirectionCW;

  float coeff_of_thrust = 0.0f;     // Thrust co-efficient (by UIUC)
  float coeff_of_torque = 0.0f;     // Torque co-efficient (by UIUC)
  float max_rpm = 0.0f;             // revolutions per minute
  float propeller_diameter = 0.0f;  // m, default is for DJI Phantom 2
  float smoothing_tc = 0.0f;        // time constant for low pass filter

  float max_speed_square = 0.0f;  // (rad/s)^2
  float max_thrust = 0.0f;
  float max_torque = 0.0f;  // computed from above formula

  RotorSetting() {
    origin_setting.translation_ = Vector3(0, 0, 0);
    origin_setting.rotation_ = Quaternion::Identity();
    CalcMaxThrustAndTorque();
  }

  // Ref: http://physics.stackexchange.com/a/32013/14061
  // force in Newton = coeff_of_thrust * \rho * n^2 * D^4
  // torque in N.m = coeff_of_torque * \rho * n^2 * D^5 / (2*pi)
  // where,
  // \rho = air density (1.225 kg/m^3)
  // n = radians per sec
  // D = propeller diameter in meters
  // coeff_of_thrust, coeff_of_torque = dimensionless constants available at
  // propeller performance database
  // http://m-selig.ae.illinois.edu/props/propDB.html
  // By default, we use values for GWS 9X5 propeller for which,
  // coeff_of_thrust = 0.109919, coeff_of_torque = 0.040164 @ 6396.667 RPM
  void CalcMaxThrustAndTorque() {
    constexpr float kBaseAirDensity = 1.225f;  // kg/m^3
    float revs_per_second = max_rpm / 60.0f;
    max_speed_square = (float)std::pow(revs_per_second * 2 * M_PI, 2);
    max_thrust =
        (float)(coeff_of_thrust * kBaseAirDensity * revs_per_second *
                revs_per_second *
                std::pow(propeller_diameter, 4));  // defaults to 4.179446268f
    max_torque = (float)(coeff_of_torque * kBaseAirDensity * revs_per_second *
                         revs_per_second * std::pow(propeller_diameter, 5) /
                         (2 * M_PI));  // defaults to 0.055562f
  }
};

//------------------------------------------------------------------------------
// class Rotor

class Rotor : public Actuator {
 public:
  Rotor();

  void BeginUpdate();

  void EndUpdate();

  const RotorSetting& GetRotorSettings() const;

  const WrenchPoint& GetWrenchPoint() const;

  float GetAngle() const;

  float GetRotatingSpeed() const;

  float GetThrust() const;

  float GetTorque() const;

  const ActuatedRotations& GetActuatedRotations() const;
  const ActuatedTransforms& GetActuatedTransforms() const;

  const float GetPowerConsumption() const;

  void SetAirDensityRatio(float air_density_ratio);

  void SetTilt(Quaternion quat);

 void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos)override;

  // These conversion operators allow this object to be passed directly to
  // TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Robot;
  friend class ActuatorImpl;

  Rotor(const std::string& id, bool is_enabled, const std::string& parent_link,
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
