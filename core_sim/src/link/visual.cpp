// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/link/visual.hpp"

#include <memory>

#include "constant.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/error.hpp"
#include "core_sim/link/geometry/file_mesh.hpp"
#include "core_sim/link/geometry/unreal_mesh.hpp"
#include "core_sim/link/geometry/skeletal_mesh.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/transforms/transform.hpp"
#include "geometry_impl.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class Visual::Loader {
 public:
  Loader(Visual::Impl& impl);

  void Load(const json& json);

 private:
  void LoadOrigin(const json& json);

  void LoadGeometry(const json& json);

  void LoadMaterial(const json& json);

  Visual::Impl& impl_;
};

class Visual::Impl : public Component {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  const Transform& GetOrigin() const;

  const Geometry* GetGeometry() const;

  const Material& GetMaterial() const;

  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Visual::Loader;

  Visual::Loader loader_;
  Transform origin_;
  std::unique_ptr<Geometry> geo_;
  Material mat_;
  TransformTree::TransformRefFrame
      transformrefframe_;  // Inertial reference frame's transform tree node
};

// -----------------------------------------------------------------------------
// class Visual

Visual::Visual(const Logger& logger)
    : pimpl_(std::shared_ptr<Visual::Impl>(new Visual::Impl(logger))) {}

void Visual::Load(ConfigJson config_json) { return pimpl_->Load(config_json); }

bool Visual::IsLoaded() { return pimpl_->IsLoaded(); }

const Transform& Visual::GetOrigin() const { return pimpl_->GetOrigin(); }

const Geometry* Visual::GetGeometry() const { return pimpl_->GetGeometry(); }

const Material& Visual::GetMaterial() const { return pimpl_->GetMaterial(); }

Visual::operator TransformTree::RefFrame&(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(
      pimpl_->operator const TransformTree::RefFrame&()));
}

Visual::operator const TransformTree::RefFrame&(void) const {
  return (pimpl_->operator const TransformTree::RefFrame&());
}

// -----------------------------------------------------------------------------
// class Visual::Impl

Visual::Impl::Impl(const Logger& logger)
    : Component(Constant::Component::visual, logger),
      loader_(*this),
      mat_(logger),
      transformrefframe_("Visual", &origin_) {}

void Visual::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

const Transform& Visual::Impl::GetOrigin() const { return origin_; }

const Geometry* Visual::Impl::GetGeometry() const { return geo_.get(); }

const Material& Visual::Impl::GetMaterial() const { return mat_; }

Visual::Impl::operator const TransformTree::RefFrame&(void) const {
  return (transformrefframe_);
}

// -----------------------------------------------------------------------------
// class Visual::Loader

Visual::Loader::Loader(Visual::Impl& impl) : impl_(impl) {}

void Visual::Loader::Load(const json& json) {
  LoadOrigin(json);
  LoadGeometry(json);
  LoadMaterial(json);

  impl_.is_loaded_ = true;
}

void Visual::Loader::LoadOrigin(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'origin'.");

  impl_.origin_ = JsonUtils::GetTransform(json, Constant::Config::origin);

  impl_.logger_.LogVerbose(impl_.name_, "'origin' loaded.");
}

void Visual::Loader::LoadGeometry(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'geometry'.");

  auto geometry_json =
      JsonUtils::GetJsonObject(json, Constant::Config::geometry);
  if (JsonUtils::IsEmpty(geometry_json)) {
    impl_.logger_.LogWarning(impl_.name_, "'geometry' missing or empty.");
    return;
  }

  auto type = JsonUtils::GetString(geometry_json, Constant::Config::type);

  impl_.logger_.LogVerbose(impl_.name_, "Json: %s", geometry_json.dump().c_str());

  if (type == Constant::Config::file_mesh) {
    auto mesh = new FileMesh(impl_.logger_);
    mesh->Load(geometry_json);
    impl_.geo_.reset(mesh);
  } else if (type == Constant::Config::unreal_mesh) {
    auto mesh = new UnrealMesh(impl_.logger_);
    mesh->Load(geometry_json);
    impl_.geo_.reset(mesh);
  } else if (type == Constant::Config::skeletal_mesh) {
    auto mesh = new SkeletalMesh(impl_.logger_);
    mesh->Load(geometry_json);
    impl_.geo_.reset(mesh);
  } else {
    impl_.logger_.LogError(impl_.name_, "Invalid geometry type '%s'.",
                           type.c_str());
    throw Error("Invalid geometry type.");
  }

  impl_.logger_.LogVerbose(impl_.name_, "'geometry' loaded.");
}

void Visual::Loader::LoadMaterial(const json& json) {
  auto material_json =
      JsonUtils::GetJsonObject(json, Constant::Config::material);
  if (!JsonUtils::IsEmpty(material_json)) {
    impl_.logger_.LogVerbose(impl_.name_, "Loading 'material'.");

    impl_.mat_.Load(material_json);
    impl_.logger_.LogVerbose(impl_.name_, "'material' loaded.");
  }
}

}  // namespace projectairsim
}  // namespace microsoft
