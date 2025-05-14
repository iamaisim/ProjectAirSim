// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_RUNTIME_COMPONENTS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_RUNTIME_COMPONENTS_HPP_

#include <string>

#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class IRuntimeComponent {
 public:
  // virtual void Load(ConfigJson config_json) = 0;

  virtual void BeginUpdate() = 0;

  virtual void EndUpdate() = 0;

  virtual void Reset() = 0;

  virtual ~IRuntimeComponent() = default;
};

class IController : public IRuntimeComponent {
 public:
  struct GimbalState {
    // Roll,pitch,yaw in radians.
    float roll = NAN;
    float pitch = NAN;
    float yaw = NAN;

    // Angular velocity in rad/s
    // NAN is default because it's supposed to be ignored until then (different from it being zero).
    float roll_vel = NAN;
    float pitch_vel = NAN;
    float yaw_vel = NAN;

    // gimbal lock (currently not changeable at runtime since we depend on unreal)
    bool roll_lock;
    bool pitch_lock;
    bool yaw_lock;
  };

 public:
  virtual void SetKinematics(const Kinematics* kinematics) = 0;

  // TODO: Should this be in the base IRuntimeComponent?
  virtual void Update() = 0;

  virtual std::vector<float> GetControlSignals(const std::string& actuator_id) = 0;

  virtual const GimbalState& GetGimbalSignal(const std::string& gimbal_id) = 0;

};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_RUNTIME_COMPONENTS_HPP_