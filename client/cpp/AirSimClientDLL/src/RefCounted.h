// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include <atomic>

#include "IRefCounted.h"

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

// Reference counted class.  When the reference count drops
// to zero, the object deletes itelf.
template <typename I>
class TRefCounted : public I {
 public:
  TRefCounted(void) : I(), ref_count_(1) {}

  // Add a reference to this object
  void AddRef(void) { ref_count_++; }

  // Release a reference to this object.  Callers must stop using
  // this object immediately after calling this method.  If the
  // reference count drops below zero, the object deletes itself.
  void Release(void) {
    assert(ref_count_ > 0);

    if (ref_count_-- == 1) delete this;
  }

 protected:
  // Prohibit explicit delete
  ~TRefCounted() {}

 protected:
  std::atomic<unsigned int>
      ref_count_;  // Number of outstanding references to this object
};                 // class TRefCounted

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
