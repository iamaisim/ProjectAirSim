// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_FILE_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_FILE_UTILS_HPP_

#include <string>

namespace microsoft {
namespace projectairsim {

constexpr char kPathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_FILE_UTILS_HPP_
