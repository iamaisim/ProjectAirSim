// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_GIMBAL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_GIMBAL_HPP_

#include <cmath>
#include <string>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/control_mapper.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

// Protocol-independent command consumed by a gimbal actuator. A non-finite
// angle or rate means that field is not commanded. If both angle and rate are
// finite, the rate limits motion toward the target angle.
struct GimbalCommand {
  float roll = NAN;
  float pitch = NAN;
  float yaw = NAN;
  float roll_rate = NAN;
  float pitch_rate = NAN;
  float yaw_rate = NAN;
  bool roll_lock = false;
  bool pitch_lock = false;
  bool yaw_lock = false;
};

// Actual simulated gimbal state. Unlike GimbalCommand, all numeric values are
// finite and angular rates describe motion achieved during the latest update.
struct GimbalState {
  float roll = 0.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;
  float roll_rate = 0.0f;
  float pitch_rate = 0.0f;
  float yaw_rate = 0.0f;
  bool roll_lock = false;
  bool pitch_lock = false;
  bool yaw_lock = false;
};

class ConfigJson;
class Logger;
class ActuatorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

//------------------------------------------------------------------------------

class Gimbal : public Actuator {
 public:
  Gimbal(void);

  void BeginUpdate(void);

  void EndUpdate(void);

  const ActuatedTransforms& GetActuatedTransforms() const;

  void UpdateActuatorOutput(std::vector<float>&& control_signals,
                            const TimeNano sim_dt_nanos) override;

  const std::string& GetTargetID(void) const;

  void SetCommand(const GimbalCommand& command);

  void UpdateGimbal(const TimeNano sim_dt_nanos);

  GimbalState GetGimbalState() const;

 private:
  friend class Robot;
  friend class ActuatorImpl;

  Gimbal(const std::string& id, bool is_enabled, const std::string& parent_link,
         const std::string& child_link, const Logger& logger,
         const TopicManager& topic_manager,
         const std::string& parent_topic_path,
         const ServiceManager& service_manager,
         const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  class Impl;
  class Loader;
};  // class Gimbal

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_GIMBAL_HPP_
