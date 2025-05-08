// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actor.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

Actor::Actor(const std::shared_ptr<ActorImpl>& pimpl) : pimpl_(pimpl) {}

bool Actor::IsLoaded() const {
  return (pimpl_.get() == nullptr) ? false : pimpl_->IsLoaded();
}

ActorType Actor::GetType() const { return pimpl_->GetType(); }

const std::string& Actor::GetID() const { return pimpl_->GetID(); }

const Transform& Actor::GetOrigin() const { return pimpl_->GetOrigin(); }

}  // namespace projectairsim
}  // namespace microsoft
