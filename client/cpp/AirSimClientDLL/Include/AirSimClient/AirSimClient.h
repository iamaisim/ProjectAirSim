// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <string>

#include "ASCDecl.h"
#include "Client.h"
#include "Drone.h"
#include "Log.h"
#include "Status.h"
#include "Types.h"
#include "World.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Client API version
constexpr int kClientAPIVersion = 1;

// Log singleton object.  Used by the library and may be used by the client app.
ASC_DECL extern Log log;

// Returns a descriptive string for a Status code
ASC_DECL size_t GetStatusString(Status status, size_t cch_buf_max,
                                char* buf_out);
template <int cch_msg_max>
size_t GetStatusString(Status status, char (&msg_out)[cch_msg_max]) {
  return (GetStatusString(status, cch_msg_max, msg_out));
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
