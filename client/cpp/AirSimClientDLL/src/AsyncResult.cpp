// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "AsyncResult.h"

#include "pch.h"


namespace microsoft {
namespace projectairsim {
namespace client {

ASC_DECL AsyncResult::AsyncResult(IAsyncResultProvider* piasyncresultprovider)
    : RefCountedContainer(piasyncresultprovider) {}

ASC_DECL void AsyncResult::Cancel(void) {
  if (piref_counted_ != nullptr)
    static_cast<IAsyncResultProvider*>(piref_counted_)->Cancel();
}

ASC_DECL bool AsyncResult::FIsDone(void) const {
  return ((piref_counted_ == nullptr)
              ? true
              : static_cast<IAsyncResultProvider*>(piref_counted_)->FIsDone());
}

ASC_DECL Status AsyncResult::Wait(void) {
  return ((piref_counted_ == nullptr)
              ? Status::Canceled
              : static_cast<IAsyncResultProvider*>(piref_counted_)->Wait());
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
