// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_ACTUATOR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_ACTUATOR_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class ActuatorImpl;
class ConfigJson;

enum class ActuatorType {
  kRotor = 0,
  kLiftDragControlSurface = 1,
  kTilt = 2,
  kGimbal = 3,
  kWheel = 4
};

// Abstract base class
class Actuator {
 public:
  virtual ~Actuator() {}

  bool IsLoaded() const;

  const std::string& GetId() const;
  ActuatorType GetType() const;
  bool IsEnabled() const;
  const std::string& GetParentLink() const;
  const std::string& GetChildLink() const;
  virtual void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos) = 0;

  bool UpdateFaultInjectionEnabledState(bool enabled);

 protected:
  explicit Actuator(const std::shared_ptr<ActuatorImpl>& pimpl);

  virtual void Load(ConfigJson config_json) = 0;

  std::shared_ptr<ActuatorImpl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_ACTUATOR_HPP_
