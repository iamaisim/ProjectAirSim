// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "core_sim/sensors/sensor.hpp"

#include <memory>

#include "component.hpp"
#include "constant.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

Sensor::Sensor(const std::shared_ptr<SensorImpl>& pimpl) : pimpl(pimpl) {}

bool Sensor::IsLoaded() const { return pimpl->IsLoaded(); }

void Sensor::BeginUpdate() { pimpl->BeginUpdate(); }

void Sensor::EndUpdate() { pimpl->EndUpdate(); }

SensorType Sensor::GetType() const { return pimpl->GetType(); }

const std::string& Sensor::GetId() const { return pimpl->GetID(); }

bool Sensor::IsEnabled() const { return pimpl->IsEnabled(); }

void Sensor::SetEnabled(bool is_enabled) {
  return pimpl->SetEnabled(is_enabled);
}

bool Sensor::IsUpdating() const { return pimpl->IsUpdating(); }

const std::string& Sensor::GetParentLink() const {
  return pimpl->GetParentLink();
}

}  // namespace projectairsim
}  // namespace microsoft
