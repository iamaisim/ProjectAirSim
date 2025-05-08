// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef ROVER_API_INCLUDE_ILOGGER_HPP_
#define ROVER_API_INCLUDE_ILOGGER_HPP_

#include <core_sim/log_level.hpp>

#include <string>

namespace microsoft {
namespace projectairsim {
namespace rover_api {

// Interface for logging handler
class ILogger {
 public:
  virtual ~ILogger() {}

  virtual void LogFatal(const std::string& component, const char* format,
                        ...) const;

  virtual void LogError(const std::string& component, const char* format,
                        ...) const;

  virtual void LogWarning(const std::string& component, const char* format,
                          ...) const;

  virtual void LogTrace(const std::string& component, const char* format,
                        ...) const;

  virtual void LogVerbose(const std::string& component, const char* format,
                          ...) const;

  virtual void VLog(const std::string& component, LogLevel level,
                    const char* format, va_list arg_list) const = 0;
};  // interface ILogger

// Default logger that writes entries to standard or debugger output
class DefaultLogger : public ILogger {
 public:
  virtual void LogFatal(const std::string& component, const char* format,
                        ...) const;

  virtual void LogError(const std::string& component, const char* format,
                        ...) const;

  virtual void LogWarning(const std::string& component, const char* format,
                          ...) const;

  virtual void LogTrace(const std::string& component, const char* format,
                        ...) const;

  virtual void LogVerbose(const std::string& component, const char* format,
                          ...) const;

  virtual void VLog(const std::string& component, LogLevel level,
                    const char* format, va_list arg_list) const;

 protected:
  // Send log entry to the log destination
  void WriteLogOutput(const char* sz);

 protected:
  // Mapping from log level to prefix string
  const char* kMapLogLevelSz[5];
};  // class DefaultLogger

}  // namespace rover_api
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_ILOGGER_HPP_
