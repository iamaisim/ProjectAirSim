// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/logger.hpp"

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace microsoft {
namespace projectairsim {

class Logger::Impl {
 public:
  Impl(Logger::LoggerCallback callback, LogLevel(level))
      : callback_(callback), current_level_(level) {
    if (this->callback_ == nullptr) {
      throw std::invalid_argument("Invalid Argument : callback");
    }
  }

  void Log(const std::string& component, LogLevel level, const char* format,
           std::va_list arg_list) {
    if (level <= current_level_) {
      if (component.empty()) {
        throw std::invalid_argument("Invalid Argument : component");
      }

      if (format == nullptr) {
        throw std::invalid_argument("Invalid Argument : format");
      }

      auto message = FormatMessage(format, arg_list);
      callback_(component, level, message);
    }
  }

  void SetLogLevel(LogLevel level) { current_level_ = level; }

  LogLevel GetLogLevel() { return current_level_; }

  // private:
  std::string FormatMessage(const char* format, std::va_list arg_list) {
    std::va_list args2;
    va_copy(args2, arg_list);

    auto size = std::vsnprintf(nullptr, 0, format, arg_list);
    if (size == -1) {
      va_end(args2);
      return std::string("!!! MESSAGE FORMATTING FAILURE !!!");
    }

    size += 1U;  // increment the size by 1 character to make space for null
                 // termiantor.
    auto buffer = std::make_unique<char[]>(size);
    std::vsnprintf(buffer.get(), size, format, args2);
    va_end(args2);
    return std::string(buffer.release());
  }

  LoggerCallback callback_;
  LogLevel current_level_;
};

Logger::Logger(LoggerCallback callback, LogLevel level)
    : pimpl_(std::make_shared<Impl>(callback, level)) {}

void Logger::Log(const std::string& component, LogLevel level,
                 const char* format, ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  pimpl_->Log(component, level, format, arg_list);
  va_end(arg_list);
}

void Logger::LogFatal(const std::string& component, const char* format,
                      ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  pimpl_->Log(component, LogLevel::kFatal, format, arg_list);
  va_end(arg_list);
}

void Logger::LogError(const std::string& component, const char* format,
                      ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  pimpl_->Log(component, LogLevel::kError, format, arg_list);
  va_end(arg_list);
}

void Logger::LogWarning(const std::string& component, const char* format,
                        ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  pimpl_->Log(component, LogLevel::kWarning, format, arg_list);
  va_end(arg_list);
}

void Logger::LogTrace(const std::string& component, const char* format,
                      ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  pimpl_->Log(component, LogLevel::kTrace, format, arg_list);
  va_end(arg_list);
}

void Logger::LogVerbose(const std::string& component, const char* format,
                        ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  pimpl_->Log(component, LogLevel::kVerbose, format, arg_list);
  va_end(arg_list);
}

void Logger::VLog(const std::string& component, LogLevel level,
                  const char* format, va_list arg_list) const {
  pimpl_->Log(component, level, format, arg_list);
}

void Logger::SetLogLevel(LogLevel level) { pimpl_->SetLogLevel(level); }

LogLevel Logger::GetLogLevel() { return pimpl_->GetLogLevel(); }

std::string Logger::FormatMessage(const char* format, ...) const {
  std::va_list arg_list;

  va_start(arg_list, format);
  std::string msg = pimpl_->FormatMessage(format, arg_list);
  va_end(arg_list);

  return msg;
}

}  // namespace projectairsim
}  // namespace microsoft
