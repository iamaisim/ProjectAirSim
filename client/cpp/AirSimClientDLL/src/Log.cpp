// Copyright (C) Microsoft Corporation.  All rights reserved.

#include <assert.h>
#include <time.h>

#include <array>
#include <ctime>
#include <stdexcept>

#include "AirSimClient.h"
#include "pch.h"

#ifdef _WIN32
// extern "C" void OutputDebugStringA(const char *message);
#include <debugapi.h>
#else
#include <cstdio>
#endif  //_WIN32

namespace microsoft {
namespace projectairsim {
namespace client {

Log::LogSink* Log::glog_sink_ =
    Log::LogSinkDefault;  // Function to invoke to add the log message

size_t Log::AddPrefix(Severity severity, size_t cch_buf_max,
                      char* buf_out) noexcept {
  static const std::string c_mpseveritystr[4] = {"CRITICAL", "ERROR", "WARNING",
                                                 "INFO"};
  size_t cch_max;
  const std::string* pstrSeverity = nullptr;
  std::string strSeverityUnknown;

  if ((severity >= Severity::Critical) && (severity <= Severity::Info))
    pstrSeverity = &c_mpseveritystr[(int)severity];
  else {
    strSeverityUnknown = std::string("SEV") + std::to_string((int)severity);
    pstrSeverity = &strSeverityUnknown;
  }

  cch_max = 12 + 4 + pstrSeverity->size() +
            1;  // Timestamp + " [] " + severity_string + terminating_NUL

  assert(cch_buf_max > cch_max);
  if (cch_buf_max <= cch_max) {
    return (0);
  }

  GetTimestamp(buf_out);
  buf_out += 12;

  sprintf(buf_out, " [%s] ", pstrSeverity->c_str());

  return (cch_max - 1);  // Return number of characters without terminating NUL
}

ASC_DECL void Log::Critical(const std::string& str) noexcept {
  LogMessage(Severity::Critical, str.c_str());
}

ASC_DECL void Log::CriticalF(const char* fmt, ...) noexcept {
  va_list vl;

  va_start(vl, fmt);
  CriticalV(fmt, vl);
  va_end(vl);
}

ASC_DECL void Log::CriticalV(const char* fmt, va_list vl) noexcept {
  LogMessageV(Severity::Critical, fmt, vl);
}

ASC_DECL void Log::Error(const std::string& str) noexcept {
  LogMessage(Severity::Error, str.c_str());
}

ASC_DECL void Log::ErrorF(const char* fmt, ...) noexcept {
  va_list vl;

  va_start(vl, fmt);
  ErrorV(fmt, vl);
  va_end(vl);
}

ASC_DECL void Log::ErrorV(const char* fmt, va_list vl) noexcept {
  LogMessageV(Severity::Error, fmt, vl);
}

void Log::GetTimestamp(char(buf_out)[13]) noexcept {
  struct timespec ts;

  if (timespec_get(&ts, TIME_UTC) == 0)
    strcpy(buf_out, "unknown     ");
  else {
    auto milliseconds = ts.tv_nsec / 1000 / 1000;
    auto tm_cur = *localtime(&ts.tv_sec);

    sprintf(buf_out, "%02d:%02d:%02d:%03d", tm_cur.tm_hour, tm_cur.tm_min,
            tm_cur.tm_sec, milliseconds);
  }
}

ASC_DECL void Log::Info(const std::string& str) noexcept {
  LogMessage(Severity::Info, str.c_str());
}

ASC_DECL void Log::InfoF(const char* fmt, ...) noexcept {
  va_list vl;

  va_start(vl, fmt);
  InfoV(fmt, vl);
  va_end(vl);
}

ASC_DECL void Log::InfoV(const char* fmt, va_list vl) noexcept {
  LogMessageV(Severity::Info, fmt, vl);
}

ASC_DECL void Log::LogMessage(Severity severity, const char* sz) noexcept {
  constexpr size_t kCchBuf = 256;
  char sz_buf[kCchBuf];
  char* pch = sz_buf;

  pch += AddPrefix(severity, kCchBuf, sz_buf);
  strncpy(pch, sz, kCchBuf - (pch - sz_buf));
  sz_buf[kCchBuf - 1] = '\0';

  glog_sink_(severity, sz_buf);
}

ASC_DECL void Log::LogMessageF(Severity severity, const char* fmt,
                               ...) noexcept {
  va_list vl;

  va_start(vl, fmt);
  LogMessageV(severity, fmt, vl);
  va_end(vl);
}

ASC_DECL void Log::LogMessageV(Severity severity, const char* fmt,
                               va_list vl) noexcept {
  constexpr size_t kCchBuf = 256;
  char sz_buf[kCchBuf];
  char* pch = sz_buf;

  pch += AddPrefix(severity, kCchBuf, sz_buf);
  vsnprintf(pch, kCchBuf - (pch - sz_buf), fmt, vl);

  try {
    glog_sink_(severity, sz_buf);
  } catch (std::exception e) {
    pch = sz_buf;
    pch += AddPrefix(Severity::Critical, kCchBuf, sz_buf);
    snprintf(pch, kCchBuf - (pch - sz_buf), "Exception calling Log sink: %s",
             e.what());
  }
}

void Log::LogSinkDefault(Severity /*severity*/,
                         const char* sz_message) noexcept {
#ifdef _WIN32
  {
    constexpr size_t kCchBuf = 256;
    char sz_buf[kCchBuf];
    size_t cch = strlen(sz_message);

    if (cch >= kCchBuf - 2) cch = kCchBuf - 2;

    memcpy(sz_buf, sz_message, cch * sizeof(sz_buf[0]));
    sz_buf[cch] = '\n';
    sz_buf[cch + 1] = '\0';

    OutputDebugStringA(sz_buf);
  }
#else
  puts(sz_message);
#endif
}

ASC_DECL void Log::SetLogSink(LogSink* plogsink) noexcept {
  if (plogsink == nullptr)
    glog_sink_ = Log::LogSinkDefault;
  else
    glog_sink_ = plogsink;
}

ASC_DECL void Log::Warning(const std::string& str) noexcept {
  LogMessage(Severity::Warning, str.c_str());
}

ASC_DECL void Log::WarningF(const char* fmt, ...) noexcept {
  va_list vl;

  va_start(vl, fmt);
  WarningV(fmt, vl);
  va_end(vl);
}

ASC_DECL void Log::WarningV(const char* fmt, va_list vl) noexcept {
  LogMessageV(Severity::Warning, fmt, vl);
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
