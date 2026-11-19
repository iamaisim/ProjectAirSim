// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_UNREAL_VEHICLE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_UNREAL_VEHICLE_HPP_

#include <initializer_list>
#include <string>
#include <vector>

#include "core_sim/actuators/actuator.hpp"

namespace microsoft {
namespace projectairsim {

class ActuatorImpl;
class ConfigJson;
class Logger;
class ServiceManager;
class StateManager;
class TopicManager;

// Bridges a controller output to an indexed IProjectAirSimVehicle parameter.
// Unreal consumes the latest value on its game thread.
class UnrealVehicleActuator : public Actuator {
 public:
  UnrealVehicleActuator();

  int GetParameterIndex() const;
  int GetControlSignalIndex() const;
  float GetControlSignal() const;

  // Robot dispatch supplies the single channel already selected by its cache.
  void UpdateActuatorOutput(const ControlSignals& control_signals,
                            const TimeNano sim_dt_nanos) override;

  // Legacy callers supply the full controller output and select by configuration.
  void UpdateActuatorOutput(std::vector<float>&& control_signals,
                            const TimeNano sim_dt_nanos);
  void UpdateActuatorOutput(std::initializer_list<float> control_signals,
                            const TimeNano sim_dt_nanos);

 private:
  friend class ActuatorImpl;

  UnrealVehicleActuator(const std::string& id, bool is_enabled,
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

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_UNREAL_VEHICLE_HPP_
