// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

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

size_t Actuator::GetSignalCount() const { return pimpl_->GetSignalCount(); }

int Actuator::GetSignalIndex(size_t signal_offset) const {
  return pimpl_->GetSignalIndex(signal_offset);
}

void Actuator::SetSignalIndex(int signal_index, size_t signal_offset) {
  pimpl_->SetSignalIndex(signal_index, signal_offset);
}

bool Actuator::IsEnabled() const { return pimpl_->IsEnabled(); }

const std::string& Actuator::GetParentLink() const {
  return pimpl_->GetParentLink();
}

const std::string& Actuator::GetChildLink() const {
  return pimpl_->GetChildLink();
}

}  // namespace projectairsim
}  // namespace microsoft
