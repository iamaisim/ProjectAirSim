// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actuators/control_mapper.hpp"

#include <algorithm>

#include "constant.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/json_utils.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

ControlMapper::ControlMapper(void) noexcept
    : ddomain_(2.0f),
      domain_min_(-1.0f),
      domain_max_(1.0f),
      drange_(2.0f),
      fclamp_input_(true),
      fclamp_output_(true),
      range_min_(-1.0f),
      range_max_(1.0f),
      scale_(1.0f) {}

void ControlMapper::GetClampInput(bool* pfclamp_input_ret) const noexcept {
  *pfclamp_input_ret = fclamp_input_;
}

void ControlMapper::GetClampOutput(bool* pfclamp_output_ret) const noexcept {
  *pfclamp_output_ret = fclamp_output_;
}

void ControlMapper::GetDomain(float* pmin_ret, float* pmax_ret) const noexcept {
  *pmin_ret = domain_min_;
  *pmax_ret = domain_max_;
}

void ControlMapper::GetRange(float* pmin_ret, float* pmax_ret) const noexcept {
  *pmin_ret = range_min_;
  *pmax_ret = range_min_ + drange_;
}

void ControlMapper::GetScale(float* pscale_ret) const noexcept {
  *pscale_ret = scale_;
}

void ControlMapper::Load(ConfigJson config_json) {
  SetClampInput(JsonUtils::GetBoolean(
      config_json, Constant::Config::clamp_input, fclamp_input_));
  SetClampOutput(JsonUtils::GetBoolean(
      config_json, Constant::Config::clamp_output, fclamp_output_));

  SetDomain(JsonUtils::GetNumber<float>(config_json, Constant::Config::input_min,
                                 domain_min_),
            JsonUtils::GetNumber<float>(config_json, Constant::Config::input_max,
                                 domain_max_));

  SetRange(JsonUtils::GetNumber<float>(config_json, Constant::Config::output_min,
                                range_min_),
           JsonUtils::GetNumber<float>(config_json, Constant::Config::output_max,
                                range_max_));

  SetScale(JsonUtils::GetNumber<float>(config_json, Constant::Config::scale, scale_));
}

void ControlMapper::SetClampInput(bool fclamp_input) noexcept {
  fclamp_input_ = fclamp_input;
}

void ControlMapper::SetClampOutput(bool fclamp_output) noexcept {
  fclamp_output_ = fclamp_output;
}

void ControlMapper::SetDomain(float min, float max) noexcept {
  domain_min_ = min;
  domain_max_ = max;
  ddomain_ = max - min;
}

void ControlMapper::SetRange(float min, float max) noexcept {
  range_min_ = min;
  range_max_ = max;
  drange_ = max - min;
}

void ControlMapper::SetScale(float scale) noexcept { scale_ = scale; }

float ControlMapper::operator()(float control_signal) const {
  float output;

  if (fclamp_input_)
    control_signal = std::clamp(control_signal, domain_min_, domain_max_);

  output = (control_signal - domain_min_) / ddomain_ * drange_ + range_min_;

  if (fclamp_output_) output = std::clamp(output, range_min_, range_max_);

  return (output);
}

}  // namespace projectairsim
}  // namespace microsoft
