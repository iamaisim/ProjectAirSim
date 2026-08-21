// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_TILT_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_TILT_HPP_

#include <cmath>
#include <memory>
#include <string>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/control_mapper.hpp"
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

}  // namespace projectairsim
}  // namespace microsoft

namespace JSBSim {
class FGFDMExec;
}

namespace microsoft {
namespace projectairsim {

struct TiltSettings {
  float radians_min = 0.0f;                   // Minimum angle (radians)
  float dradians = static_cast<float>(M_PI);  // Rotation amount (radians)
  float smoothing_tc = 0.0f;  // Time constant for low pass filter
    std::string jsbsim_cmd;    // JSBSim property to write control signal to
    std::string jsbsim_state;  // JSBSim property to read actuator state from
  Vector3 vec3_axis = Vector3(
      0.0f, 1.0f, 0.0f);  // Direction axis around which rotation is done
  std::string target_id;  // ID of target to tilt
};                        // struct TiltSettings

//------------------------------------------------------------------------------
// class Tilt

class Tilt : public Actuator {
 public:
  Tilt(void);

  void BeginUpdate(void);

  void EndUpdate(void);

  const ActuatedTransforms& GetActuatedTransforms() const;

  const Quaternion& GetControlRotation(void) const;

  const TiltSettings& GetSettings(void) const;

  const std::string& GetTargetID(void) const;

  void SetJSBSimModel(std::shared_ptr<::JSBSim::FGFDMExec> model);

  float GetJSBSimState() const;

 void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos) override;

 private:
  friend class Robot;
  friend class ActuatorImpl;

  Tilt(const std::string& id, bool is_enabled, const std::string& parent_link,
       const std::string& child_link, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  class Impl;
  class Loader;
};  // class Tilt

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_TILT_HPP_
