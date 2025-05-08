// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_INERTIAL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_INERTIAL_HPP_

#include <memory>

#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_tree.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;

class Inertial {
 public:
  bool IsLoaded() const;

  const Transform& GetOrigin() const;

  float GetMass() const;

  const Matrix3x3& GetInertia() const;

  const Vector3& GetBodyBox() const;

  float GetDragCoefficient() const;

  const LiftDrag& GetLiftDrag() const;

  const Vector3& GetCrossSectionAreas() const;

  bool IsPhysicsEnabled() const;

  bool IsGravityEnabled() const;

  const Vector3& GetInertiaTensorScale() const;

  float GetStablizationThresholdMultiplier() const;

  int GetVelocitySolverIterationCount() const;

  int GetPositionSolverIterationCount() const;

  // RefFrame accessors--the conversion operators allow this object to be passed
  // directly to TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Link;

  explicit Inertial(const Logger& logger);

  void Load(ConfigJson config_json);

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_INERTIAL_HPP_
