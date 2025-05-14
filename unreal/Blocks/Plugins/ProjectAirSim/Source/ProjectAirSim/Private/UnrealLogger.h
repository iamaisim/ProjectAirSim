// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once

#include <iostream>

#include "CoreMinimal.h"
#include "core_sim/log_level.hpp"

DECLARE_LOG_CATEGORY_EXTERN(SimPlugin, All, All);

class UnrealLogger {
 public:
  template <size_t N, typename... ArgTypes>
  static void Log(microsoft::projectairsim::LogLevel level,
                  const TCHAR (&format)[N], ArgTypes... args);

  static void LogSim(const std::string& module,
                     microsoft::projectairsim::LogLevel level,
                     const std::string& message);

 private:
  static std::string GetTimeStamp();
};

template <size_t N, typename... ArgTypes>
inline void UnrealLogger::Log(microsoft::projectairsim::LogLevel level,
                              const TCHAR (&format)[N], ArgTypes... args) {
  FString LogMessage = FString::Printf(format, args...);

  // Output to Unreal's native log file/console
  switch (level) {
    case microsoft::projectairsim::LogLevel::kFatal:
      UE_LOG(SimPlugin, Fatal, TEXT("%s"), *LogMessage);
      break;

    case microsoft::projectairsim::LogLevel::kError:
      UE_LOG(SimPlugin, Error, TEXT("%s"), *LogMessage);
      break;

    case microsoft::projectairsim::LogLevel::kWarning:
      UE_LOG(SimPlugin, Warning, TEXT("%s"), *LogMessage);
      break;

    case microsoft::projectairsim::LogLevel::kTrace:
      UE_LOG(SimPlugin, Log, TEXT("%s"), *LogMessage);
      break;

    case microsoft::projectairsim::LogLevel::kVerbose:
      UE_LOG(SimPlugin, Verbose, TEXT("%s"), *LogMessage);
      break;

    default:
      UE_LOG(SimPlugin, VeryVerbose, TEXT("%s"), *LogMessage);
      break;
  }

  // Output to an independent log file
  auto time_stamp = GetTimeStamp();
  std::clog << "[" << time_stamp << "] " << TCHAR_TO_UTF8(*LogMessage)
            << std::endl;
}

inline void UnrealLogger::LogSim(const std::string& module,
                                 microsoft::projectairsim::LogLevel level,
                                 const std::string& message) {
  Log(level, TEXT("[%hs] %hs"), module.c_str(), message.c_str());
}

inline std::string UnrealLogger::GetTimeStamp() {
  // Get standard date time
  char time_date[20];  // YYYY.MM.DD-HH.MM.SS is 19 char + 1 null
  auto now = std::chrono::system_clock::now();
  auto t_now = std::chrono::system_clock::to_time_t(now);
  struct tm tm_now;
  // Populate tm for UTC
#ifdef _WIN32
  gmtime_s(&tm_now, &t_now);
#else
  gmtime_r(&t_now, &tm_now);
#endif
  std::strftime(time_date, sizeof(time_date), "%Y.%m.%d-%H.%M.%S", &tm_now);

  // Get milliseconds and append
  auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto fraction = now - seconds;
  int milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();
  std::vector<char> time_stamp_buf(24);  // :mmm adds 4 char to time_date
  std::snprintf(&time_stamp_buf[0], time_stamp_buf.size(), "%s:%.3i", time_date,
                milliseconds);

  // Return chars as string without the null terminator
  return std::string(time_stamp_buf.begin(), time_stamp_buf.end() - 1);
}
