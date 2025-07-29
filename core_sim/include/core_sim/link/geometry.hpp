// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_HPP_

#include <memory>

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class GeometryImpl;

enum class GeometryType { kFileMesh = 0, kUnrealMesh = 1 };

class Geometry {
 public:
  virtual ~Geometry() {}

  bool IsLoaded();

  GeometryType GetType();

 protected:
  friend class Visual;

  explicit Geometry(const std::shared_ptr<GeometryImpl>& pimpl);

  virtual void Load(ConfigJson config_json) = 0;

  std::shared_ptr<GeometryImpl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_HPP_
