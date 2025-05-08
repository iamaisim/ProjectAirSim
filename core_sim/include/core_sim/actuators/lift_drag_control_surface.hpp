// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_LIFT_DRAG_CONTROL_SURFACE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_LIFT_DRAG_CONTROL_SURFACE_HPP_

#include <cmath>
#include <string>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class ActuatorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

struct LiftDragControlSurfaceSettings {
  float rotation_rate = 0.0f;  // rad per 1.0 control signal
  float smoothing_tc = 0.0f;   // time constant for low pass filter
};

//------------------------------------------------------------------------------
// class LiftDragControlSurface

class LiftDragControlSurface : public Actuator {
 public:
  LiftDragControlSurface();

  void BeginUpdate();

  void EndUpdate();

  const LiftDragControlSurfaceSettings& GetSettings() const;

  const float& GetControlAngle() const;

 void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos)override;

 private:
  friend class Robot;
  friend class ActuatorImpl;

  LiftDragControlSurface(const std::string& id, bool is_enabled,
                         const std::string& parent_link,
                         const std::string& child_link, const Logger& logger,
                         const TopicManager& topic_manager,
                         const std::string& parent_topic_path,
                         const ServiceManager& service_manager,
                         const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_LIFT_DRAG_CONTROL_SURFACE_HPP_
