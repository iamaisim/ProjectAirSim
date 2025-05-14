// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/link/material.hpp"

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

class Material::Loader {
 public:
  Loader(Material::Impl& impl);

  void Load(const json& json);

 private:
  void LoadColor(const json& json);

  void LoadColoredTexture(const json& json);

  void LoadTexture(const json& json);

  Material::Impl& impl_;
};

class Material::Impl : public Component {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  const Vector4& GetColor();

  const std::string& GetColoredTexture();

  const std::string& GetTexture();

 private:
  friend class Material::Loader;

  Material::Loader loader_;
  Vector4 color_;
  std::string colored_texture_;
  std::string texture_;
};

// -----------------------------------------------------------------------------
// class Material

Material::Material(const Logger& logger)
    : pimpl_(std::shared_ptr<Impl>(new Impl(logger))) {}

void Material::Load(ConfigJson config_json) {
  return pimpl_->Load(config_json);
}

bool Material::IsLoaded() { return pimpl_->IsLoaded(); }

const Vector4& Material::GetColor() { return pimpl_->GetColor(); }

const std::string& Material::GetColoredTexture() {
  return pimpl_->GetColoredTexture();
}

const std::string& Material::GetTexture() { return pimpl_->GetTexture(); }

// -----------------------------------------------------------------------------
// class Material::Impl

Material::Impl::Impl(const Logger& logger)
    : Component(Constant::Component::material, logger), loader_(*this) {}

void Material::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

const Vector4& Material::Impl::GetColor() { return color_; }

const std::string& Material::Impl::GetColoredTexture() {
  return colored_texture_;
}

const std::string& Material::Impl::GetTexture() { return texture_; }

// -----------------------------------------------------------------------------
// class Material::Loader

Material::Loader::Loader(Material::Impl& impl) : impl_(impl) {}

void Material::Loader::Load(const json& json) {
  LoadColor(json);
  LoadColoredTexture(json);
  LoadTexture(json);

  impl_.is_loaded_ = true;
}

void Material::Loader::LoadColor(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'color'.");

  if (JsonUtils::HasKey(json, Constant::Config::color)) {
    impl_.color_ = JsonUtils::GetVector4(json, Constant::Config::color);
  }

  impl_.logger_.LogVerbose(impl_.name_, "'color' loaded.");
}

void Material::Loader::LoadColoredTexture(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'colored-texture'.");

  if (JsonUtils::HasKey(json, Constant::Config::colored_texture)) {
    impl_.colored_texture_ =
        JsonUtils::GetString(json, Constant::Config::colored_texture);
  }

  impl_.logger_.LogVerbose(impl_.name_, "'colored-texture' loaded.");
}

void Material::Loader::LoadTexture(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'texture'.");

  if (JsonUtils::HasKey(json, Constant::Config::texture)) {
    impl_.texture_ = JsonUtils::GetString(json, Constant::Config::texture);
  }

  impl_.logger_.LogVerbose(impl_.name_, "'texture' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
