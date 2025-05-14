// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "Status.h"

#include <NNGI/NNGI.h>

#include <string>

#include "ASCDecl.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

ASC_DECL size_t GetStatusString(Status status, size_t cch_buf_max,
                                char* buf_out) {
  size_t cch_ret = 0;
  const size_t cch_msg_max =
      cch_buf_max -
      1;  // Maximum number of characters not including terminating NUL
  auto status_value = static_cast<uint32_t>(status);

  if (cch_msg_max < 1) return (0);

  if ((status_value & kStatusFlagNNG) != 0) {
    int err = static_cast<int>(status_value & ~kStatusFlagNNG);
    const char* msg = nngi::nng_strerror(err);

    cch_ret = std::min<size_t>(strlen(msg), cch_msg_max);
    strncpy(buf_out, msg, cch_ret);
    buf_out[cch_ret] = '\0';
  } else if ((status_value & kStatusFlagErrno) != 0) {
    int errno_value = static_cast<int>(status_value & ~kStatusFlagErrno);
    const char* msg = strerror(errno_value);

    cch_ret = std::min<size_t>(strlen(msg), cch_msg_max);
    strncpy(buf_out, msg, cch_ret);
    buf_out[cch_ret] = '\0';
  } else if (status_value > static_cast<uint32_t>(Status::STATUS_MAX)) {
    snprintf(buf_out, cch_buf_max, "Unknown status %u", status_value);
  } else {
    static const char* kMapStatusSz[static_cast<int>(Status::STATUS_MAX)] = {
        "OK",                                  // Status::OK
        "Operation failed",                    // Status::Failed
        "Operation was canceled",              // Status::Canceled
        "Object is closed",                    // Status::Closed
        "Operation in progress",               // InProgress
        "Operation timed out",                 // Status::TimedOut
        "There is no connection",              // Status::NotConnected
        "Item not found",                      // Status::NotFound
        "Request was rejected by the server",  // Status::RejectedByServer
        "A scene isn't loaded on the server",  // Status::NoScene
    };

    const char* msg = kMapStatusSz[status_value];

    cch_ret = std::min<size_t>(strlen(msg), cch_msg_max);
    strncpy(buf_out, msg, cch_ret);
    buf_out[cch_ret] = '\0';
  }

  return (cch_ret);
}

namespace internal {

// Convert an errno code to a Status code
Status ErrnoToStatus(int errno_value) {
  return ((errno_value == 0)
              ? Status::OK
              : static_cast<Status>(errno_value | kStatusFlagErrno));
}

// Convert an NNG status code to a Status code
Status ErrNNGToStatus(uint32_t err_nng) {
  return ((err_nng == 0) ? Status::OK
                         : static_cast<Status>(err_nng | kStatusFlagNNG));
}

}  // namespace internal

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
