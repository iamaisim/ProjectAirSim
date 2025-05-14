// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_SRC_LINK_GEOMETRY_IMPL_HPP_
#define CORE_SIM_SRC_LINK_GEOMETRY_IMPL_HPP_

#include <string>

#include "component.hpp"
#include "core_sim/link/geometry.hpp"
#include "core_sim/logger.hpp"

namespace microsoft {
namespace projectairsim {

class GeometryImpl : public Component {
 public:
  GeometryImpl(GeometryType type, const std::string& name, const Logger& logger)
      : Component(name, logger), type_(type) {}

  GeometryType GetType() { return type_; }

 protected:
  GeometryType type_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_LINK_GEOMETRY_IMPL_HPP_
