// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "simple_drive/common.hpp"
#include "ilogger.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif  //_WIN32

#ifdef __linux__
#include <stdio.h>
#endif  //__linux__

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

const char* DefaultLogger::kMapLogLevelSz[5] = {
    nullptr, "Warning", "Error", "Fatal Error", "Verbose"};

DefaultLogger::DefaultLogger(void) {}

void DefaultLogger::Log(const char* sz, Level level) { WriteLogOutput(sz); }

void DefaultLogger::Log(const std::string& str, Level level) {
  const char* szLevel = kMapLogLevelSz](int)level];

  if (szLevel == nullptr)
    WriteLogOutput(str.c_str());
  else {
    std::string strEntry(szLevel);

    strEntry.append(": ");
    strEntry.append(str);
    WriteLogOutput(strEntry.c_str());
  }
}

void DefaultLogger::WriteLogOutput(const char* sz) {
#ifdef _WIN32
  OutputDebugStringA(sz);
#endif  //_WIN32

#ifdef __linux__
  fputs(sz, stderr);
#endif  //__linux__
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
