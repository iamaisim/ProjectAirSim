// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include <cstdint>

#include "ASCDecl.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Status codes
enum class Status : std::uint32_t {
  OK = 0,            // No error
  Failed,            // Operation failed
  Canceled,          // Operation was canceled
  Closed,            // The object is closed
  InProgress,        // Operation is in progress
  TimedOut,          // Operation timed out
  NotConnected,      // There is no connection
  NotFound,          // Item not found
  RejectedByServer,  // Request was rejected by the server
  NoScene,           // A scene isn't loaded on the server

  // The following must be last
  STATUS_MAX,
};  // enum Status

// If this bit is set, the other status bits are the errno code
static constexpr std::uint32_t kStatusFlagErrno = 0x02000000;

// If this bit is set, the other status bits are the NNG error code
static constexpr std::uint32_t kStatusFlagNNG = 0x04000000;

// Get the human-compatible string for the status code
ASC_DECL size_t GetStatusString(Status status, size_t cch_buf_max,
                                char* buf_out);

namespace internal {

// Convert an errno code to a Status code
Status ErrnoToStatus(int errno_value);

// Convert an NNG status code to a Status code
Status ErrNNGToStatus(uint32_t err_nng);

}  // namespace internal

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
