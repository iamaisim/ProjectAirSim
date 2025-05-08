// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/link/collision.hpp"

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

class Collision::Loader {
 public:
  Loader(Collision::Impl& impl);

  void Load(const json& json);

 private:
  void LoadCollisionEnabled(const json& json);

  void LoadRestitution(const json& json);

  void LoadFriction(const json& json);

  Collision::Impl& impl_;
};

class Collision::Impl : public Component {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  bool IsCollisionEnabled() const;

  float GetRestitution() const;

  float GetFriction() const;

 private:
  friend class Collision::Loader;

  Collision::Loader loader_;
  float restitution_;
  float friction_;
  bool collision_enabled_;
};

// -----------------------------------------------------------------------------
// class Collision

Collision::Collision(const Logger& logger)
    : pimpl_(std::shared_ptr<Impl>(new Impl(logger))) {}

void Collision::Load(ConfigJson config_json) {
  return pimpl_->Load(config_json);
}

float Collision::GetRestitution() const { return pimpl_->GetRestitution(); }

float Collision::GetFriction() const { return pimpl_->GetFriction(); }

bool Collision::IsCollisionEnabled() const {
  return pimpl_->IsCollisionEnabled();
}

bool Collision::IsLoaded() { return pimpl_->IsLoaded(); }

// -----------------------------------------------------------------------------
// class Collision::Impl

Collision::Impl::Impl(const Logger& logger)
    : Component(Constant::Component::collision, logger), loader_(*this) {}

void Collision::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

bool Collision::Impl::IsCollisionEnabled() const { return collision_enabled_; }

float Collision::Impl::GetRestitution() const { return restitution_; }

float Collision::Impl::GetFriction() const { return friction_; }

// -----------------------------------------------------------------------------
// class Collision::Loader

Collision::Loader::Loader(Collision::Impl& impl) : impl_(impl) {}

void Collision::Loader::Load(const json& json) {
  LoadCollisionEnabled(json);
  LoadRestitution(json);
  LoadFriction(json);

  impl_.is_loaded_ = true;
}

void Collision::Loader::LoadCollisionEnabled(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'enabled'.");

  impl_.collision_enabled_ =
      JsonUtils::GetInteger(json, Constant::Config::enabled, 1 /*default*/);

  impl_.logger_.LogVerbose(impl_.name_, "'enabled' loaded.");
}

void Collision::Loader::LoadRestitution(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'restitution'.");

  impl_.restitution_ =
      JsonUtils::GetNumber<float>(json, Constant::Config::restitution);

  impl_.logger_.LogVerbose(impl_.name_, "'restitution' loaded.");
}

void Collision::Loader::LoadFriction(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'friction'.");

  impl_.friction_ =
      JsonUtils::GetNumber<float>(json, Constant::Config::friction);

  impl_.logger_.LogVerbose(impl_.name_, "'friction' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
