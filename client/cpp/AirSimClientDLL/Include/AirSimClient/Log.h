// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <cstdarg>
#include <string>

#include "ASCDecl.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Logging facility
class Log {
 public:
  // Message severity level
  enum class Severity {
    Critical = 0,  // Critical error, execution cannot continue
    Error = 1,     // Error, operation failed but execution can continue
    Warning = 2,   // Warning, operation can continue but may not have the
                  // expected results
    Information = 3,  // Information, operation is proceeding normally
    Info = 3,         // Same as Info
  };                  // enum class Severity

  // Prototype for functions that handle log messages
  typedef void LogSink(Severity severity, const char* sz_message) noexcept;

 public:
  // Log a critical error
  ASC_DECL static void Critical(const std::string& str) noexcept;
  ASC_DECL static void CriticalF(const char* fmt, ...) noexcept;
  ASC_DECL static void CriticalV(const char* fmt, va_list vl) noexcept;

  // Log an error
  ASC_DECL static void Error(const std::string& str) noexcept;
  ASC_DECL static void ErrorF(const char* fmt, ...) noexcept;
  ASC_DECL static void ErrorV(const char* fmt, va_list vl) noexcept;

  // Log an informational message
  ASC_DECL static void Info(const std::string& str) noexcept;
  ASC_DECL static void InfoF(const char* fmt, ...) noexcept;
  ASC_DECL static void InfoV(const char* fmt, va_list vl) noexcept;

  // Log a warning
  ASC_DECL static void Warning(const std::string& str) noexcept;
  ASC_DECL static void WarningF(const char* fmt, ...) noexcept;
  ASC_DECL static void WarningV(const char* fmt, va_list vl) noexcept;

  // Log a message
  ASC_DECL static void LogMessage(Severity severity, const char* sz) noexcept;
  ASC_DECL static void LogMessageF(Severity severity, const char* fmt,
                                   ...) noexcept;
  ASC_DECL static void LogMessageV(Severity severity, const char* fmt,
                                   va_list vl) noexcept;

  // Set the handler for log messages
  ASC_DECL static void SetLogSink(LogSink* plogsink) noexcept;

 protected:
  // Add the log message prefix to the buffer
  static size_t AddPrefix(Severity severity, size_t cch_buf_max,
                          char* buf_out) noexcept;

  // Get the log message timestamp
  static void GetTimestamp(char(buf_out)[13]) noexcept;

  // Default log message handler
  static LogSink LogSinkDefault;

 protected:
  static LogSink* glog_sink_;  // Function to invoke to add the log message
};                             // class Log

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
