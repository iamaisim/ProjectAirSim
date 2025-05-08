// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef PHYSICS_INCLUDE_BASE_PHYSICS_HPP_
#define PHYSICS_INCLUDE_BASE_PHYSICS_HPP_

#include <memory>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class BasePhysicsBody {
 public:
  BasePhysicsBody() {}
  virtual ~BasePhysicsBody() {}

  // Aggregate all externally applied wrenches on body CG into wrench_
  virtual void CalculateExternalWrench() = 0;

  // Placeholder function to be overloaded by derived physics bodies to
  // calculate the inertia for its specific input params and configuration
  Matrix3x3 CalculateInertia() { return Matrix3x3::Identity(); }

  const std::string& GetName() const { return name_; }
  const PhysicsType& GetPhysicsType() const { return physics_type_; }
  float GetMass() const { return mass_; }
  float GetMassInv() const { return mass_inv_; }
  const Matrix3x3& GetInertia() const { return inertia_; }
  const Matrix3x3& GetInertiaInv() const { return inertia_inv_; }
  const Wrench& GetExternalWrench() const { return external_wrench_; }
  const Kinematics& GetKinematics() const { return kinematics_; }
  float GetFriction() const { return friction_; }
  float GetRestitution() const { return restitution_; }

  void SetName(const std::string& name) { name_ = name; }

  void SetPhysicsType(PhysicsType physics_type) {
    physics_type_ = physics_type;
  }

  void SetMass(float mass) {
    if (mass < kMinMass) {
      mass = kMinMass;
      // TODO Warn user about near-zero mass?
    }
    mass_ = mass;
    mass_inv_ = 1.0f / mass;
  }

  void SetInertia(const Matrix3x3& inertia) {
    inertia_ = inertia;
    inertia_inv_ = inertia.inverse();
  }

  void SetKinematics(const Kinematics& kinematics) { kinematics_ = kinematics; }
  void SetFriction(float friction) { friction_ = friction; }
  void SetRestitution(float restitution) { restitution_ = restitution; }

 protected:
  std::string name_ = "";
  PhysicsType physics_type_ = PhysicsType::kNonPhysics;
  float mass_ = 0.0f;
  float mass_inv_ = 0.0f;
  Matrix3x3 inertia_ = Matrix3x3::Zero(3, 3);
  Matrix3x3 inertia_inv_ = Matrix3x3::Zero(3, 3);
  float friction_ = 0.0f;
  float restitution_ = 0.0f;

  Kinematics kinematics_;
  Wrench external_wrench_;

  static constexpr float kMinMass = 1.0E-6f;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_BASE_PHYSICS_HPP_
