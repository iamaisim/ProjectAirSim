// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "IRefCounted.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

ASC_DECL RefCountedContainer::RefCountedContainer(
    IRefCounted* piref_counted) noexcept
    : piref_counted_(piref_counted) {}

ASC_DECL RefCountedContainer::RefCountedContainer(
    const RefCountedContainer& selfdelete_other) noexcept
    : piref_counted_(selfdelete_other.piref_counted_) {
  if (piref_counted_ != nullptr) piref_counted_->AddRef();
}

ASC_DECL RefCountedContainer::RefCountedContainer(
    RefCountedContainer&& selfdelete_other) noexcept
    : piref_counted_(selfdelete_other.piref_counted_) {
  selfdelete_other.piref_counted_ = nullptr;
}

ASC_DECL RefCountedContainer::~RefCountedContainer() { Clear(); }

ASC_DECL void RefCountedContainer::Clear(void) {
  if (piref_counted_ != nullptr) {
    piref_counted_->Release();
    piref_counted_ = nullptr;
  }
}

ASC_DECL void RefCountedContainer::SetPIRefCounted(IRefCounted* piref_counted) {
  Clear();
  piref_counted_ = piref_counted;
}

ASC_DECL RefCountedContainer& RefCountedContainer::operator=(
    const RefCountedContainer& selfdeletecontainer_other) noexcept {
  SetPIRefCounted(selfdeletecontainer_other.piref_counted_);
  piref_counted_->AddRef();

  return (*this);
}

ASC_DECL RefCountedContainer& RefCountedContainer::operator=(
    RefCountedContainer&& selfdeletecontainer_other) noexcept {
  SetPIRefCounted(selfdeletecontainer_other.piref_counted_);
  selfdeletecontainer_other.piref_counted_ = nullptr;

  return (*this);
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
