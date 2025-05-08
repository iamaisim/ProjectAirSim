// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_MATERIAL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_MATERIAL_HPP_

#include <memory>

#include "core_sim/math_utils.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;

class Material {
 public:
  bool IsLoaded();

  const Vector4& GetColor();

  const std::string& GetColoredTexture();

  const std::string& GetTexture();

 private:
  friend class Visual;

  explicit Material(const Logger& logger);

  void Load(ConfigJson config_json);

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_MATERIAL_HPP_
