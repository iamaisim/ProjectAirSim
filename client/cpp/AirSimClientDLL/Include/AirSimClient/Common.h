// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace microsoft {
namespace projectairsim {
namespace client {

// Array of bytes
typedef std::vector<std::uint8_t> VecB;

// Array of string
typedef std::vector<std::string> VecStr;

// Macro to catch Status exceptions and return it
#define RETURN_CATCH_STATUS(s)                                 \
  do {                                                         \
    try {                                                      \
      return (s);                                              \
    } catch (Status status_caught) {                           \
      return (status_caught);                                  \
    } catch (std::exception e) {                               \
      log.Error(std::string("Caught exception: ") + e.what()); \
      return (Status::Failed);                                 \
    }                                                          \
  } while (true)

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
