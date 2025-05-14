// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_CONTROL_MAPPER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_CONTROL_MAPPER_HPP_

#include <memory>
#include <string>
#include <vector>

namespace microsoft {
namespace projectairsim {

class ConfigJson;

// Maps input control signal value to desired output
class ControlMapper {
 public:
  ControlMapper(void) noexcept; //Default mapping is a no-op, [-1, +1] --> [-1, +1]
  virtual ~ControlMapper() {}

  void GetClampInput(bool* pfclamp_input_ret) const noexcept;
  void GetClampOutput(bool* pfclamp_output_ret) const noexcept;
  void GetDomain(float* pmin_ret, float* pmax_ret) const noexcept;
  void GetRange(float* pmin_ret, float* pmax_ret) const noexcept;
  void GetScale(float* pscale_ret) const noexcept;
  void Load(ConfigJson config_json);
  void SetClampInput(bool fclamp_input) noexcept;
  void SetClampOutput(bool fclamp_output) noexcept;
  void SetDomain(float min, float max) noexcept;
  void SetRange(float min, float max) noexcept;
  void SetScale(float scale) noexcept;

  // Returns (control_signal - domain_min) / (domain_max - domain_min) * scale * (range_max - range_min) +
  // range_min
  float operator()(float control_signal) const;

 protected:
  float ddomain_;      // Input span
  float domain_min_;  // Input minimum
  float domain_max_;  // Input maximum
  float drange_;      // Output span
  bool fclamp_input_;  // If true, the control input signal is clamped (domain_min_, domain_max_)
  bool fclamp_output_;  // If true, the output signal is clamped to (range_min_, range_max_)
  float range_min_;    // Output minimum
  float range_max_;    // Output maximum
  float scale_;        // Scale factor
};                    // class ControlMapper

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTUATORS_CONTROL_MAPPER_HPP_
