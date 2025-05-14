// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_GEOMETRY_HPP_

#include <memory>

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class GeometryImpl;

enum class GeometryType { kFileMesh = 0, kUnrealMesh = 1, kSkeletalMesh = 2 };

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
