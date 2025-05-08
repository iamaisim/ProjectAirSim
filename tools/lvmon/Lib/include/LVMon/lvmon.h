// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMON_H_
#define TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMON_H_

#include <stdint.h>

#include <string>

namespace LVMon {

#ifdef NDEBUG

__inline uint64_t CalculateHNS(uint64_t hptimestamp) { return (0); }
__inline uint64_t CalculateMS(uint64_t hptimestamp) { return (0); }
__inline uint64_t CalculateNS(uint64_t hptimestamp) { return (0); }
__inline uint64_t CalculateUS(uint64_t hptimestamp) { return (0); }
__inline uint64_t GetHPTimestamp(void) { return (0); }

__inline void Get(const char* szName, int64_t* pi64Ret) {}
__inline void Get(const char* szName, uint64_t* pui64Ret) {}
__inline void Get(const char* szName, double* prRet) {}
__inline void Get(const char* szName, std::string* pstrRet) {}

__inline void Set(const char* szName, int32_t i32) {}
__inline void Set(const char* szName, uint32_t ui32) {}
__inline void Set(const char* szName, int64_t i64) {}
__inline void Set(const char* szName, uint64_t ui64) {}
__inline void Set(const char* szName, double r) {}
__inline void Set(const char* szName, const std::string& str) {}
__inline void Set(const char* szName, const char* sz) {}

#else  // NDEBUG

extern uint64_t CalculateHNS(uint64_t hptimestamp);
extern uint64_t CalculateMS(uint64_t hptimestamp);
extern uint64_t CalculateNS(uint64_t hptimestamp);
extern uint64_t CalculateUS(uint64_t hptimestamp);
extern uint64_t GetHPTimestamp(void);

extern void Get(const char* szName, int64_t* pi64Ret);
extern void Get(const char* szName, uint64_t* pui64Ret);
extern void Get(const char* szName, double* prRet);
extern void Get(const char* szName, std::string* pstrRet);

extern void Set(const char* szName, int32_t i32);
extern void Set(const char* szName, uint32_t ui32);
extern void Set(const char* szName, int64_t i64);
extern void Set(const char* szName, uint64_t ui64);
extern void Set(const char* szName, double r);
extern void Set(const char* szName, const std::string& str);
extern void Set(const char* szName, const char* sz);

#endif  // NDEBUG

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMON_H_
