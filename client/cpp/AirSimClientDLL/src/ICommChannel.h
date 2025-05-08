// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once
#include "Message.h"
#include "Status.h"

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

class ICommChannel {
 public:
  // Special value for "no timeout"
  static const unsigned long kMSTimeoutInfinite;

 public:
  virtual Status Receive(client::Message* pmessage_inout,
                         unsigned long ms_timeout = kMSTimeoutInfinite) = 0;
  virtual Status Send(const uint8_t* rgb, size_t cb) = 0;
};  // class ICommChannel

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
