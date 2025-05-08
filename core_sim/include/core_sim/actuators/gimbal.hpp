// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_GIMBAL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_GIMBAL_HPP_

#include <cmath>
#include <string>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/control_mapper.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

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

 void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos) override;

  const std::string& GetTargetID(void) const;

  void UpdateGimbal(IController::GimbalState& new_state,
                    const TimeNano sim_dt_nanos);

  const IController::GimbalState& GetGimbalState() const;

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
