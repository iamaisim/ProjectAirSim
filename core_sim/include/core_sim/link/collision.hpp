// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_COLLISION_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_COLLISION_HPP_

#include <memory>

#include "core_sim/math_utils.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;

class Collision {
 public:
  bool IsLoaded();

  bool IsCollisionEnabled() const;

  float GetRestitution() const;

  float GetFriction() const;

 private:
  friend class Link;

  explicit Collision(const Logger& logger);

  void Load(ConfigJson config_json);

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_COLLISION_HPP_
