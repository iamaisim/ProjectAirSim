// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_FILE_MESH_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_FILE_MESH_HPP_

#include <string>

#include "core_sim/link/geometry.hpp"
#include "core_sim/math_utils.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;

class FileMesh : public Geometry {
 public:
  ~FileMesh() override {}

  const std::string& GetFileName() const;

  const Vector3& GetScale() const;

 private:
  friend class Visual;

  explicit FileMesh(const Logger& logger);

  void Load(ConfigJson config_json) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_FILE_MESH_HPP_
