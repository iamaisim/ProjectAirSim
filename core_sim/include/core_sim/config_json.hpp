// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_CONFIG_JSON_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_CONFIG_JSON_HPP_

#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class ConfigJson {
 public:
  ConfigJson(const json& wrapped_json) : wrapped_json_(wrapped_json) {}

  ConfigJson(json&& wrapped_json) = delete;

  operator const json&() { return wrapped_json_; }

 private:
  const json& wrapped_json_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_CONFIG_JSON_HPP_
