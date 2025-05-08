// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LOG_LEVEL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LOG_LEVEL_HPP_

namespace microsoft {
namespace projectairsim {

enum class LogLevel {
  kFatal = 0,
  kError = 1,
  kWarning = 2,
  kTrace = 3,
  kVerbose = 4
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LOG_LEVEL_HPP_
