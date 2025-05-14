// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/link/geometry/unreal_mesh.hpp"

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

class UnrealMesh::Loader {
 public:
  Loader(UnrealMesh::Impl& impl);

  void Load(const json& json);

 private:
  void LoadName(const json& json);

  void LoadScale(const json& json);

  UnrealMesh::Impl& impl_;
};

class UnrealMesh::Impl : public GeometryImpl {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  const std::string& GetName();

  const Vector3& GetScale();

 private:
  friend class UnrealMesh::Loader;

  UnrealMesh::Loader loader_;
  std::string mesh_name_;
  Vector3 scale_;
};

// -----------------------------------------------------------------------------
// class UnrealMesh

UnrealMesh::UnrealMesh(const Logger& logger)
    : Geometry(std::make_shared<UnrealMesh::Impl>(logger)) {}

void UnrealMesh::Load(ConfigJson config_json) {
  return static_cast<UnrealMesh::Impl*>(pimpl_.get())->Load(config_json);
}

const std::string& UnrealMesh::GetName() const {
  return static_cast<UnrealMesh::Impl*>(pimpl_.get())->GetName();
}

const Vector3& UnrealMesh::GetScale() const {
  return static_cast<UnrealMesh::Impl*>(pimpl_.get())->GetScale();
}

// -----------------------------------------------------------------------------
// class UnrealMesh::Impl

UnrealMesh::Impl::Impl(const Logger& logger)
    : GeometryImpl(GeometryType::kUnrealMesh, Constant::Component::unreal_mesh,
                   logger),
      loader_(*this),
      scale_(Vector3::Ones()) {}

void UnrealMesh::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

const std::string& UnrealMesh::Impl::GetName() { return mesh_name_; }

const Vector3& UnrealMesh::Impl::GetScale() { return scale_; }

// -----------------------------------------------------------------------------
// class UnrealMesh::Loader

UnrealMesh::Loader::Loader(UnrealMesh::Impl& impl) : impl_(impl) {}

void UnrealMesh::Loader::Load(const json& json) {
  LoadName(json);
  LoadScale(json);

  impl_.is_loaded_ = true;
}

void UnrealMesh::Loader::LoadName(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'name'.");

  if (!JsonUtils::HasKey(json, Constant::Config::name)) {
    impl_.logger_.LogError(impl_.name_, "'file_name' missing.");
    throw Error("Mesh file name missing.");
  }

  impl_.mesh_name_ = JsonUtils::GetString(json, Constant::Config::name);

  impl_.logger_.LogVerbose(impl_.name_, "'name' loaded.");
}

void UnrealMesh::Loader::LoadScale(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'scale'.");

  if (JsonUtils::HasKey(json, Constant::Config::scale)) {
    impl_.scale_ = JsonUtils::GetVector3(json, Constant::Config::scale);
  }

  impl_.logger_.LogVerbose(impl_.name_, "'scale' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
