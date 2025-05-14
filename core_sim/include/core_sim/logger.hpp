// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LOGGER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LOGGER_HPP_

#include <functional>
#include <memory>
#include <string>
#include <cstdarg>

#include "core_sim/log_level.hpp"

namespace microsoft {
namespace projectairsim {

class Logger {
 public:
  using LoggerCallback =
      std::function<void(const std::string&, LogLevel, const std::string&)>;

  explicit Logger(LoggerCallback callback, LogLevel level = LogLevel::kError);

  void Log(const std::string& component, LogLevel level, const char* format,
           ...) const;

  void LogFatal(const std::string& component, const char* format, ...) const;

  void LogError(const std::string& component, const char* format, ...) const;

  void LogWarning(const std::string& component, const char* format, ...) const;

  void LogTrace(const std::string& component, const char* format, ...) const;

  void LogVerbose(const std::string& component, const char* format, ...) const;

  void VLog(const std::string& component, LogLevel level, const char* format,
            va_list arg_list) const;

  void SetLogLevel(LogLevel level);

  LogLevel GetLogLevel();

  std::string FormatMessage(const char* format, ...) const;

 private:
  class Impl;
  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LOGGER_HPP_
