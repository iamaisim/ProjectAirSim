// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.
#include "core_sim/link/geometry/skeletal_mesh.hpp"

#include <memory>

#include "../geometry_impl.hpp"
#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class SkeletalMesh::Loader {
 public:
  Loader(SkeletalMesh::Impl& impl);

  void Load(const json& json);

 private:
  void LoadName(const json& json);

  void LoadScale(const json& json);

  SkeletalMesh::Impl& impl_;
};

class SkeletalMesh::Impl : public GeometryImpl {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  const std::string& GetName();

  const Vector3& GetScale();

 private:
  friend class SkeletalMesh::Loader;

  SkeletalMesh::Loader loader_;
  std::string mesh_name_;
  Vector3 scale_;
};

// -----------------------------------------------------------------------------
// class SkeletalMesh

SkeletalMesh::SkeletalMesh(const Logger& logger)
    : Geometry(std::make_shared<SkeletalMesh::Impl>(logger)) {}

void SkeletalMesh::Load(ConfigJson config_json) {
  return static_cast<SkeletalMesh::Impl*>(pimpl_.get())->Load(config_json);
}

const std::string& SkeletalMesh::GetName() const {
  return static_cast<SkeletalMesh::Impl*>(pimpl_.get())->GetName();
}

const Vector3& SkeletalMesh::GetScale() const {
  return static_cast<SkeletalMesh::Impl*>(pimpl_.get())->GetScale();
}

// -----------------------------------------------------------------------------
// class SkeletalMesh::Impl

SkeletalMesh::Impl::Impl(const Logger& logger)
    : GeometryImpl(GeometryType::kSkeletalMesh, Constant::Component::skeletal_mesh,
                   logger),
      loader_(*this),
      scale_(Vector3::Ones()) {}

void SkeletalMesh::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

const std::string& SkeletalMesh::Impl::GetName() { return mesh_name_; }

const Vector3& SkeletalMesh::Impl::GetScale() { return scale_; }

// -----------------------------------------------------------------------------
// class SkeletalMesh::Loader

SkeletalMesh::Loader::Loader(SkeletalMesh::Impl& impl) : impl_(impl) {}

void SkeletalMesh::Loader::Load(const json& json) {
  LoadName(json);
  LoadScale(json);

  impl_.is_loaded_ = true;
}

void SkeletalMesh::Loader::LoadName(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'name'.");

  if (!JsonUtils::HasKey(json, Constant::Config::name)) {
    impl_.logger_.LogError(impl_.name_, "'file_name' missing.");
    throw Error("Mesh file name missing.");
  }

  impl_.mesh_name_ = JsonUtils::GetString(json, Constant::Config::name);

  impl_.logger_.LogVerbose(impl_.name_, "'name' loaded.");
}

void SkeletalMesh::Loader::LoadScale(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'scale'.");

  if (JsonUtils::HasKey(json, Constant::Config::scale)) {
    impl_.scale_ = JsonUtils::GetVector3(json, Constant::Config::scale);
  }

  impl_.logger_.LogVerbose(impl_.name_, "'scale' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
