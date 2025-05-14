// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <cstdint>

#include "IRefCounted.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class IMessageBuffer : public IRefCounted {
 public:
  virtual ~IMessageBuffer(void) {}

  virtual size_t Cb(void) const = 0;
  virtual const std::uint8_t* Data(void) const = 0;
};  // class IMessageBuffer

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
