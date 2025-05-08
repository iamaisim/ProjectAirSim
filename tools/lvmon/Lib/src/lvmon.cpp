// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef NDEBUG

#define _CRT_SECURE_NO_WARNINGS  // Don't warn about sprintf()

#include "lvmon.h"

#include <stdio.h>

#include "lvmonserver.h"

namespace {

struct HPTiming {
  uint64_t hptickPerSecond;  // Number of high-performance ticks per second

#ifdef _WIN32

  HPTiming(void) : hptickPerSecond(0) {
    LARGE_INTEGER li;

    if (QueryPerformanceFrequency(&li)) hptickPerSecond = li.QuadPart;
  }

#else  // _WIN32

  HPTiming(void)
      : hptickPerSecond(1000 * 1000 * 1000) {}  // 1 nanosecond per tick

#endif  // _WIN32
} const c_hptiming;

LVMon::CLVMonServer glvmonserver;  // LVMon server singleton

}  // namespace

namespace LVMon {

uint64_t CalculateHNS(uint64_t hptimestamp) {
  return (hptimestamp * 1000 * 1000 * 10 / c_hptiming.hptickPerSecond);
}

uint64_t CalculateMS(uint64_t hptimestamp) {
  return (hptimestamp * 1000 / c_hptiming.hptickPerSecond);
}

uint64_t CalculateNS(uint64_t hptimestamp) {
  return (hptimestamp * 1000 * 1000 * 1000 / c_hptiming.hptickPerSecond);
}

uint64_t CalculateUS(uint64_t hptimestamp) {
  return (hptimestamp * 1000 * 1000 / c_hptiming.hptickPerSecond);
}

uint64_t GetHPTimestamp(void) {
#ifdef _WIN32

  LARGE_INTEGER li;

  if (!QueryPerformanceCounter(&li))
    return (0);
  else
    return (li.QuadPart);

#else  //_WIN32

  timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec);  // Return nanoseconds

#endif  // _WIN32
}

void Get(const char* szName, int64_t* pi64Ret) {
  glvmonserver.Get(szName, pi64Ret);
}

void Get(const char* szName, uint64_t* pui64Ret) {
  glvmonserver.Get(szName, pui64Ret);
}

void Get(const char* szName, double* prRet) { glvmonserver.Get(szName, prRet); }

void Get(const char* szName, std::string* pstrRet) {
  glvmonserver.Get(szName, pstrRet);
}

void Set(const char* szName, int32_t i32) { Set(szName, (int64_t)i32); }

void Set(const char* szName, uint32_t ui32) { Set(szName, (uint64_t)ui32); }

void Set(const char* szName, int64_t i64) { glvmonserver.Set(szName, i64); }

void Set(const char* szName, uint64_t ui64) { glvmonserver.Set(szName, ui64); }

void Set(const char* szName, double r) { glvmonserver.Set(szName, r); }

void Set(const char* szName, const std::string& str) {
  glvmonserver.Set(szName, str);
}

void Set(const char* szName, const char* sz) { glvmonserver.Set(szName, sz); }

}  // namespace LVMon

#endif  // NDEBUG
