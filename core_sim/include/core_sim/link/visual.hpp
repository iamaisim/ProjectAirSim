// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_VISUAL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_VISUAL_HPP_

#include <memory>

#include "core_sim/link/geometry.hpp"
#include "core_sim/link/material.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_tree.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;

class Visual {
 public:
  bool IsLoaded();

  const Transform& GetOrigin() const;

  const Geometry* GetGeometry() const;

  const Material& GetMaterial() const;

  // RefFrame accessors--the conversion operators allow this object to be passed
  // directly to TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Link;
  friend class EnvObject;

  explicit Visual(const Logger& logger);

  void Load(ConfigJson config_json);

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_VISUAL_HPP_
