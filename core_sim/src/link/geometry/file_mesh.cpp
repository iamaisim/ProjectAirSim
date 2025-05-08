// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/link/geometry/file_mesh.hpp"

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

class FileMesh::Loader {
 public:
  Loader(FileMesh::Impl& impl);

  void Load(const json& json);

 private:
  void LoadFileName(const json& json);

  void LoadScale(const json& json);

  FileMesh::Impl& impl_;
};

class FileMesh::Impl : public GeometryImpl {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  const std::string& GetFileName();

  const Vector3& GetScale();

 private:
  friend class FileMesh::Loader;

  FileMesh::Loader loader_;
  std::string file_name_;
  Vector3 scale_;
};

// -----------------------------------------------------------------------------
// class FileMesh

FileMesh::FileMesh(const Logger& logger)
    : Geometry(std::make_shared<FileMesh::Impl>(logger)) {}

void FileMesh::Load(ConfigJson config_json) {
  return static_cast<FileMesh::Impl*>(pimpl_.get())->Load(config_json);
}

const std::string& FileMesh::GetFileName() const {
  return static_cast<FileMesh::Impl*>(pimpl_.get())->GetFileName();
}

const Vector3& FileMesh::GetScale() const {
  return static_cast<FileMesh::Impl*>(pimpl_.get())->GetScale();
}

// -----------------------------------------------------------------------------
// class FileMesh::Impl

FileMesh::Impl::Impl(const Logger& logger)
    : GeometryImpl(GeometryType::kFileMesh, Constant::Component::file_mesh,
                   logger),
      loader_(*this),
      scale_(Vector3::Ones()) {}

void FileMesh::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

const std::string& FileMesh::Impl::GetFileName() { return file_name_; }

const Vector3& FileMesh::Impl::GetScale() { return scale_; }
// ---------------------------------------------------------------------------------------------
// class file_mesh::loader

FileMesh::Loader::Loader(FileMesh::Impl& impl) : impl_(impl) {}

void FileMesh::Loader::Load(const json& json) {
  LoadFileName(json);
  LoadScale(json);

  impl_.is_loaded_ = true;
}

void FileMesh::Loader::LoadFileName(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'file_name'.");

  if (!JsonUtils::HasKey(json, Constant::Config::file_name)) {
    impl_.logger_.LogError(impl_.name_, "'file_name' missing.");
    throw Error("Mesh file name missing.");
  }

  impl_.file_name_ = JsonUtils::GetString(json, Constant::Config::file_name);

  impl_.logger_.LogVerbose(impl_.name_, "'file_name' loaded.");
}

void FileMesh::Loader::LoadScale(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'scale'.");

  if (JsonUtils::HasKey(json, Constant::Config::scale)) {
    impl_.scale_ = JsonUtils::GetVector3(json, Constant::Config::scale);
  }

  impl_.logger_.LogVerbose(impl_.name_, "'scale' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
