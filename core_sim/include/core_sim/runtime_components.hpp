// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

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
  virtual void SetKinematics(const Kinematics* kinematics) = 0;

  // TODO: Should this be in the base IRuntimeComponent?
  virtual void Update() = 0;

  virtual std::vector<float> GetControlSignals(const std::string& actuator_id) = 0;

};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_RUNTIME_COMPONENTS_HPP_
