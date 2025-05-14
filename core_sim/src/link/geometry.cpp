// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/link/geometry.hpp"

#include <memory>

#include "geometry_impl.hpp"

namespace microsoft {
namespace projectairsim {

Geometry::Geometry(const std::shared_ptr<GeometryImpl>& pimpl)
    : pimpl_(pimpl) {}

bool Geometry::IsLoaded() { return pimpl_->IsLoaded(); }

GeometryType Geometry::GetType() { return pimpl_->GetType(); }

}  // namespace projectairsim
}  // namespace microsoft
