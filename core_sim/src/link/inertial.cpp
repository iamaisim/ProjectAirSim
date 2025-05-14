// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/link/inertial.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class Inertial::Loader {
 public:
  Loader(Inertial::Impl& impl);

  void Load(const json& json);

 private:
  void LoadOrigin(const json& json);

  void LoadMass(const json& json);

  void LoadInertia(const json& json);

  void LoadAerodynamics(const json& json);

  // void LoadPhysicsEnabled(const json& json);

  // void LoadGravityEnabled(const json& json);

  void LoadInertiaTensorScale(const json& json);

  void LoadStablizationThresholdMultiplier(const json& json);

  void LoadVelocitySolverIterationCount(const json& json);

  void LoadPositionSolverIterationCount(const json& json);

  Inertial::Impl& impl_;
};

class Inertial::Impl : public Component {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

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

  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Inertial::Loader;

  Inertial::Loader loader_;
  Transform origin_;

  // Common physics params
  float mass_;
  Matrix3x3 inertia_;

  // FastPhysics aerodynamics params
  Vector3 body_box_;
  float drag_coefficient_;
  LiftDrag lift_drag_params_;
  Vector3 cross_section_areas_;

  // Unreal physics params
  bool physics_enabled_;
  bool gravity_enabled_;
  Vector3 inertia_tensor_scale_;
  float stablization_threshold_multiplier_;
  int velocity_solver_iteration_count_;
  int position_solver_iteration_count_;

  // Transform tree
  TransformTree::TransformRefFrame
      transformrefframe_;  // Inertial reference frame's transform tree node
};

// -----------------------------------------------------------------------------
// class Inertial

Inertial::Inertial(const Logger& logger)
    : pimpl_(std::shared_ptr<Inertial::Impl>(new Inertial::Impl(logger))) {}

void Inertial::Load(ConfigJson config_json) {
  return pimpl_->Load(config_json);
}

bool Inertial::IsLoaded() const { return pimpl_->IsLoaded(); }

const Transform& Inertial::GetOrigin() const { return pimpl_->GetOrigin(); }

float Inertial::GetMass() const { return pimpl_->GetMass(); }

const Matrix3x3& Inertial::GetInertia() const { return pimpl_->GetInertia(); }

const Vector3& Inertial::GetBodyBox() const { return pimpl_->GetBodyBox(); }

float Inertial::GetDragCoefficient() const {
  return pimpl_->GetDragCoefficient();
}

const LiftDrag& Inertial::GetLiftDrag() const { return pimpl_->GetLiftDrag(); }

const Vector3& Inertial::GetCrossSectionAreas() const {
  return pimpl_->GetCrossSectionAreas();
}

bool Inertial::IsPhysicsEnabled() const { return pimpl_->IsPhysicsEnabled(); }

bool Inertial::IsGravityEnabled() const { return pimpl_->IsGravityEnabled(); }

const Vector3& Inertial::GetInertiaTensorScale() const {
  return pimpl_->GetInertiaTensorScale();
}

float Inertial::GetStablizationThresholdMultiplier() const {
  return pimpl_->GetStablizationThresholdMultiplier();
}

int Inertial::GetVelocitySolverIterationCount() const {
  return pimpl_->GetVelocitySolverIterationCount();
}

int Inertial::GetPositionSolverIterationCount() const {
  return pimpl_->GetPositionSolverIterationCount();
}

Inertial::operator TransformTree::RefFrame&(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(
      pimpl_->operator const TransformTree::RefFrame&()));
}

Inertial::operator const TransformTree::RefFrame&(void) const {
  return (pimpl_->operator const TransformTree::RefFrame&());
}

// -----------------------------------------------------------------------------
// class Inertial::Impl

Inertial::Impl::Impl(const Logger& logger)
    : Component(Constant::Component::inertial, logger),
      loader_(*this),
      transformrefframe_("Inertial", &origin_) {}

void Inertial::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

const Transform& Inertial::Impl::GetOrigin() const { return origin_; }

float Inertial::Impl::GetMass() const { return mass_; }

const Matrix3x3& Inertial::Impl::GetInertia() const { return inertia_; }

const Vector3& Inertial::Impl::GetBodyBox() const { return body_box_; }

float Inertial::Impl::GetDragCoefficient() const { return drag_coefficient_; }

const LiftDrag& Inertial::Impl::GetLiftDrag() const {
  return lift_drag_params_;
}

const Vector3& Inertial::Impl::GetCrossSectionAreas() const {
  return cross_section_areas_;
}

bool Inertial::Impl::IsPhysicsEnabled() const { return physics_enabled_; }

bool Inertial::Impl::IsGravityEnabled() const { return gravity_enabled_; }

const Vector3& Inertial::Impl::GetInertiaTensorScale() const {
  return inertia_tensor_scale_;
}

float Inertial::Impl::GetStablizationThresholdMultiplier() const {
  return stablization_threshold_multiplier_;
}

int Inertial::Impl::GetVelocitySolverIterationCount() const {
  return velocity_solver_iteration_count_;
}

int Inertial::Impl::GetPositionSolverIterationCount() const {
  return position_solver_iteration_count_;
}

Inertial::Impl::operator const TransformTree::RefFrame&(void) const {
  return (this->transformrefframe_);
}

// -----------------------------------------------------------------------------
// class Inertial::Loader

Inertial::Loader::Loader(Inertial::Impl& impl) : impl_(impl) {}

void Inertial::Loader::Load(const json& json) {
  LoadOrigin(json);
  LoadMass(json);
  LoadInertia(json);

  LoadAerodynamics(json);  // Fast Physics

  // LoadPhysicsEnabled(json);                   // UE physics
  // LoadGravityEnabled(json);                   // UE physics
  LoadInertiaTensorScale(json);               // UE physics
  LoadStablizationThresholdMultiplier(json);  // UE physics
  LoadPositionSolverIterationCount(json);     // UE physics
  LoadVelocitySolverIterationCount(json);     // UE physics

  impl_.is_loaded_ = true;
}

void Inertial::Loader::LoadOrigin(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'origin'.");

  impl_.origin_ = JsonUtils::GetTransform(json, Constant::Config::origin);

  impl_.logger_.LogVerbose(impl_.name_, "'origin' loaded.");
}

void Inertial::Loader::LoadMass(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'mass'.");

  impl_.mass_ = JsonUtils::GetNumber<float>(json, Constant::Config::mass);

  impl_.logger_.LogVerbose(impl_.name_, "'mass' loaded.");
}

void Inertial::Loader::LoadInertia(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'inertia'.");

  auto inertia_json = JsonUtils::GetJsonObject(json, Constant::Config::inertia);
  auto inertia_type =
      JsonUtils::GetString(inertia_json, Constant::Config::type);

  Matrix3x3 inertia = Matrix3x3::Zero();  // Init default as zero matrix

  if (inertia_type == Constant::Config::geometry) {
    // https://en.wikipedia.org/wiki/List_of_moments_of_inertia
    auto geometry_json =
        JsonUtils::GetJsonObject(inertia_json, Constant::Config::geometry);

    if (JsonUtils::HasKey(geometry_json, Constant::Config::box)) {
      // Inertia of a solid cuboid around its CG
      auto box_json =
          JsonUtils::GetJsonObject(geometry_json, Constant::Config::box);
      auto box_xyz = JsonUtils::GetVector3(box_json, Constant::Config::size);

      inertia(0, 0) = impl_.mass_ / 12.0f *
                      (box_xyz.y() * box_xyz.y() + box_xyz.z() * box_xyz.z());

      inertia(1, 1) = impl_.mass_ / 12.0f *
                      (box_xyz.x() * box_xyz.x() + box_xyz.z() * box_xyz.z());

      inertia(2, 2) = impl_.mass_ / 12.0f *
                      (box_xyz.x() * box_xyz.x() + box_xyz.y() * box_xyz.y());
    } else {
      impl_.logger_.LogVerbose(impl_.name_,
                               "No supported inertia geometry found.");
    }
  } else if (inertia_type == Constant::Config::matrix) {
    auto ixx = JsonUtils::GetNumber<float>(inertia_json, Constant::Config::ixx);
    auto iyy = JsonUtils::GetNumber<float>(inertia_json, Constant::Config::iyy);
    auto izz = JsonUtils::GetNumber<float>(inertia_json, Constant::Config::izz);
    inertia(0, 0) = ixx;
    inertia(1, 1) = iyy;
    inertia(2, 2) = izz;
  } else {
    // For point-mass or invalid inertia type, leave as zero matrix
  }

  impl_.inertia_ = inertia;

  impl_.logger_.LogVerbose(impl_.name_, "'inertia' loaded.");
}

void Inertial::Loader::LoadAerodynamics(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'aerodynamics'.");

  auto aero_json =
      JsonUtils::GetJsonObject(json, Constant::Config::aerodynamics);
  auto aero_type = JsonUtils::GetString(aero_json, Constant::Config::type);

  Vector3 cross_section_areas = Vector3::Zero();  // Init default as zero vector
  Vector3 body_box = Vector3::Zero();

  if (aero_type == Constant::Config::geometry) {
    // For geometry type aerodynamics, calculate the cross section areas based
    // on the specified geometric shape
    auto geometry_json =
        JsonUtils::GetJsonObject(aero_json, Constant::Config::geometry);

    if (JsonUtils::HasKey(geometry_json, Constant::Config::box)) {
      auto box_json =
          JsonUtils::GetJsonObject(geometry_json, Constant::Config::box);

      // TODO This assumes there is only one link with a box aerodynamics type
      // to set the body box. The concept of body box should be generalized to
      // avoid this assumption. What happens if there is no body box specified?
      body_box = JsonUtils::GetVector3(box_json, Constant::Config::size);

      cross_section_areas = {body_box.y() * body_box.z(),
                             body_box.x() * body_box.z(),
                             body_box.x() * body_box.y()};
    } else if (JsonUtils::HasKey(geometry_json, Constant::Config::cylinder)) {
      auto cylinder_json =
          JsonUtils::GetJsonObject(geometry_json, Constant::Config::cylinder);

      auto radius =
          JsonUtils::GetNumber<float>(cylinder_json, Constant::Config::radius);
      auto length =
          JsonUtils::GetNumber<float>(cylinder_json, Constant::Config::length);

      cross_section_areas = {2 * radius * length, 2 * radius * length,
                             static_cast<float>(M_PI) * radius * radius};
    } else {
      impl_.logger_.LogVerbose(impl_.name_,
                               "No supported aerodynamic geometry found.");
    }
  } else if (aero_type == Constant::Config::cross_section_areas_xyz) {
    // For cross-section-areas type aerodynamics, just set the cross section
    // areas directly
    cross_section_areas = JsonUtils::GetVector3(
        aero_json, Constant::Config::cross_section_areas_xyz);
  } else if (aero_type == Constant::Config::lift_drag) {
    // For lift-drag type aerodynamics, leave cross section areas as zero vector
    // because drag will be calculated based on the lift-drag formulas
    auto lift_drag_json =
        JsonUtils::GetJsonObject(aero_json, Constant::Config::lift_drag);

    impl_.lift_drag_params_.enabled = true;

    impl_.lift_drag_params_.alpha_0 =
        JsonUtils::GetNumber<float>(lift_drag_json, Constant::Config::alpha_0,
                                    impl_.lift_drag_params_.alpha_0);

    impl_.lift_drag_params_.alpha_stall = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::alpha_stall,
        impl_.lift_drag_params_.alpha_stall);

    impl_.lift_drag_params_.c_lift_alpha = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::c_lift_alpha,
        impl_.lift_drag_params_.c_lift_alpha);

    impl_.lift_drag_params_.c_lift_alpha_stall = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::c_lift_alpha_stall,
        impl_.lift_drag_params_.c_lift_alpha_stall);

    impl_.lift_drag_params_.c_drag_alpha = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::c_drag_alpha,
        impl_.lift_drag_params_.c_drag_alpha);

    impl_.lift_drag_params_.c_drag_alpha_stall = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::c_drag_alpha_stall,
        impl_.lift_drag_params_.c_drag_alpha_stall);

    impl_.lift_drag_params_.c_moment_alpha = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::c_moment_alpha,
        impl_.lift_drag_params_.c_moment_alpha);

    impl_.lift_drag_params_.c_moment_alpha_stall = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::c_moment_alpha_stall,
        impl_.lift_drag_params_.c_moment_alpha_stall);

    impl_.lift_drag_params_.area = JsonUtils::GetNumber<float>(
        lift_drag_json, Constant::Config::area, impl_.lift_drag_params_.area);

    impl_.lift_drag_params_.control_surface_cl_per_rad =
        JsonUtils::GetNumber<float>(
            lift_drag_json, Constant::Config::control_surface_cl_per_rad,
            impl_.lift_drag_params_.control_surface_cl_per_rad);

    impl_.lift_drag_params_.control_surface_cd_per_rad =
        JsonUtils::GetNumber<float>(
            lift_drag_json, Constant::Config::control_surface_cd_per_rad,
            impl_.lift_drag_params_.control_surface_cd_per_rad);

    impl_.lift_drag_params_.control_surface_cm_per_rad =
        JsonUtils::GetNumber<float>(
            lift_drag_json, Constant::Config::control_surface_cm_per_rad,
            impl_.lift_drag_params_.control_surface_cm_per_rad);

    impl_.lift_drag_params_.center_pressure_xyz = JsonUtils::GetVector3(
        lift_drag_json, Constant::Config::center_pressure_xyz);

    impl_.lift_drag_params_.forward_xyz =
        JsonUtils::GetVector3(lift_drag_json, Constant::Config::forward_xyz);

    impl_.lift_drag_params_.upward_xyz =
        JsonUtils::GetVector3(lift_drag_json, Constant::Config::upward_xyz);
  }

  impl_.cross_section_areas_ = cross_section_areas;
  impl_.body_box_ = body_box;

  impl_.drag_coefficient_ = JsonUtils::GetNumber<float>(
      aero_json, Constant::Config::drag_coefficient);

  impl_.logger_.LogVerbose(impl_.name_, "'aerodynamics' loaded.");
}

// void Inertial::Loader::LoadPhysicsEnabled(const json& json) {
//   impl_.logger_.LogVerbose(impl_.name_, "Loading 'physics-enabled'.");

//   impl_.physics_enabled_ =
//       JsonUtils::GetInteger(json, Constant::Config::physics_enabled);

//   impl_.logger_.LogVerbose(impl_.name_, "'physics-enabled' loaded.");
// }

// void Inertial::Loader::LoadGravityEnabled(const json& json) {
//   impl_.logger_.LogVerbose(impl_.name_, "Loading 'gravity-enabled'.");

//   impl_.gravity_enabled_ =
//       JsonUtils::GetInteger(json, Constant::Config::gravity_enabled);

//   impl_.logger_.LogVerbose(impl_.name_, "'gravity-enabled' loaded.");
// }

void Inertial::Loader::LoadInertiaTensorScale(const json& json) {
  if (JsonUtils::HasKey(json, Constant::Config::inertia_tensor_scale)) {
    impl_.inertia_tensor_scale_ =
        JsonUtils::GetVector3(json, Constant::Config::inertia_tensor_scale);
  } else {
    impl_.inertia_tensor_scale_ = Vector3(1.0f, 1.0f, 1.0f);
  }
}

void Inertial::Loader::LoadStablizationThresholdMultiplier(const json& json) {
  impl_.stablization_threshold_multiplier_ =
      static_cast<float>(JsonUtils::GetInteger(
          json, Constant::Config::stablization_threshold_multiplier, 1));
}

void Inertial::Loader::LoadVelocitySolverIterationCount(const json& json) {
  impl_.velocity_solver_iteration_count_ = JsonUtils::GetInteger(
      json, Constant::Config::velocity_solver_iteration_count, 1);
}

void Inertial::Loader::LoadPositionSolverIterationCount(const json& json) {
  impl_.position_solver_iteration_count_ = JsonUtils::GetInteger(
      json, Constant::Config::position_solver_iteration_count, 8);
}

}  // namespace projectairsim
}  // namespace microsoft
