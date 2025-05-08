// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actuators/actuator.hpp"

#include <memory>

#include "actuator_impl.hpp"
#include "component.hpp"
#include "constant.hpp"

namespace microsoft {
namespace projectairsim {

Actuator::Actuator(const std::shared_ptr<ActuatorImpl>& pimpl)
    : pimpl_(pimpl) {}

bool Actuator::IsLoaded() const { return pimpl_->IsLoaded(); }

ActuatorType Actuator::GetType() const { return pimpl_->GetType(); }

const std::string& Actuator::GetId() const { return pimpl_->GetID(); }

bool Actuator::IsEnabled() const { return pimpl_->IsEnabled(); }

const std::string& Actuator::GetParentLink() const {
  return pimpl_->GetParentLink();
}

const std::string& Actuator::GetChildLink() const {
  return pimpl_->GetChildLink();
}

}  // namespace projectairsim
}  // namespace microsoft
