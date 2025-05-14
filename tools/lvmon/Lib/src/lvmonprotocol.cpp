// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "lvmonprotocol.h"

#include <assert.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>

#include <string>

#if defined(_WIN32)

#include <WinSock2.h>

#elif defined(__linux__)

#include <arpa/inet.h>

#endif

namespace LVMon {

uint64_t NToH(uint64_t ui64) {
#ifdef WIN32
  return (ntohll(ui64));
#else   // WIN32
  return (be64toh(ui64));
#endif  // WIN32
}

MsgNewName* MsgNewName::Create(ValueType valuetype, uint32_t valueid,
                               const char* sz, std::vector<uint8_t>* pvecbRet) {
  MsgNewName* pmsgnewNameRet = nullptr;
  size_t cch = strlen(sz);
  size_t cbData =
      sizeof(MsgNewName) - sizeof(MsgHeader) + ((cch > 1) ? (cch - 1) : 0);
  size_t cbMsg = sizeof(MsgHeader) + cbData;

  assert(cbData < 256);  // Maximum message data size
  if (cbData > 255) return (nullptr);

  pvecbRet->resize(cbMsg);
  pmsgnewNameRet =
      new (pvecbRet->data()) MsgNewName(valuetype, valueid, (uint8_t)cch);
#ifdef _WIN32
  memcpy_s(pmsgnewNameRet->rgch, cbMsg - sizeof(MsgNewName) + 1, sz, cch);
#else   //_WIN32
  memcpy(pmsgnewNameRet->rgch, sz, cch);
#endif  //_WIN32
  return (pmsgnewNameRet);
}

size_t MsgNewName::GetCch(void) const {
  static const uint8_t c_cbDataOverhead = sizeof(*this) - sizeof(MsgHeader);

  return ((cbData < c_cbDataOverhead) ? 0 : (cbData - c_cbDataOverhead + 1));
}

void MsgNewName::NToH(void) { valueid = ntohl(valueid); }

MsgSetValue* MsgSetValue::Create(ValueType valuetype, uint32_t valueid,
                                 const char* rgch, size_t cch,
                                 std::vector<uint8_t>* pvecbRet) {
  MsgSetValue* pmsgsetvalueRet = nullptr;
  size_t cbMessage =
      cch + reinterpret_cast<size_t>(
                &reinterpret_cast<MsgSetValue*>(0)->value.str.rgch[0]);

  if (cbMessage <= 255) {
    pmsgsetvalueRet = CreateCore(valuetype, valueid, cch, pvecbRet);

    if (pmsgsetvalueRet != nullptr) {
      pmsgsetvalueRet->value.str.cch = (uint8_t)cch;
      memcpy(pmsgsetvalueRet->value.str.rgch, rgch, cch);
    }
  }

  return (pmsgsetvalueRet);
}

MsgSetValue* MsgSetValue::Create(ValueType valuetype, uint32_t valueid,
                                 const std::string& str,
                                 std::vector<uint8_t>* pvecbRet) {
  return (Create(valuetype, valueid, str.data(), str.size(), pvecbRet));
}

template <>
MsgSetValue* MsgSetValue::Create<int64_t>(ValueType valuetype, uint32_t valueid,
                                          int64_t i64,
                                          std::vector<uint8_t>* pvecbRet) {
  auto pMsgSetValue = CreateCore(valuetype, valueid, sizeof(int64_t), pvecbRet);

  if (pMsgSetValue != nullptr) pMsgSetValue->value.i64 = i64;

  return (pMsgSetValue);
}

template <>
MsgSetValue* MsgSetValue::Create<uint64_t>(ValueType valuetype,
                                           uint32_t valueid, uint64_t ui64,
                                           std::vector<uint8_t>* pvecbRet) {
  auto pMsgSetValue =
      CreateCore(valuetype, valueid, sizeof(uint64_t), pvecbRet);

  if (pMsgSetValue != nullptr) pMsgSetValue->value.ui64 = ui64;

  return (pMsgSetValue);
}

template <>
MsgSetValue* MsgSetValue::Create<double>(ValueType valuetype, uint32_t valueid,
                                         double r,
                                         std::vector<uint8_t>* pvecbRet) {
  auto pMsgSetValue = CreateCore(valuetype, valueid, sizeof(double), pvecbRet);

  if (pMsgSetValue != nullptr) pMsgSetValue->value.r = r;

  return (pMsgSetValue);
}

template <>
MsgSetValue* MsgSetValue::Create<const char*>(ValueType valuetype,
                                              uint32_t valueid, const char* sz,
                                              std::vector<uint8_t>* pvecbRet) {
  size_t cch = strlen(sz);
  return (Create(valuetype, valueid, sz, cch, pvecbRet));
}

MsgSetValue* MsgSetValue::CreateCore(ValueType valuetype, uint32_t valueid,
                                     size_t cbValue,
                                     std::vector<uint8_t>* pvecbRet) {
  MsgSetValue* pmsgsetvalueRet = nullptr;
  size_t cbMsg = sizeof(MsgSetValue);

  if (cbValue > sizeof(uint64_t)) cbMsg += cbValue - sizeof(uint64_t);

  assert(cbMsg < 256);  // Maximum data payload size of a message
  if (cbMsg > 255) return (nullptr);

  pvecbRet->resize(cbMsg);
  pmsgsetvalueRet =
      new (pvecbRet->data()) MsgSetValue(valueid, valuetype, (uint8_t)cbMsg);

  return (pmsgsetvalueRet);
}

void MsgSetValue::NToH(void) {
  switch ((ValueType)valuetype) {
    default:
    case ValueType::Double:
    case ValueType::String:
      // Swap no needed
      break;

    case ValueType::Int64: {
      uint64_t ui64 = *reinterpret_cast<uint64_t*>(&value.i64);

      ui64 = LVMon::NToH(ui64);
      value.i64 = *reinterpret_cast<int64_t*>(&ui64);
    } break;

    case ValueType::UInt64:
      value.ui64 = LVMon::NToH(value.ui64);
      break;
  }
}

void MsgSubscribe::NToH(void) { valueid = ntohl(valueid); }

void MsgUnsubscribe::NToH(void) { valueid = ntohl(valueid); }

MsgValueUpdate* MsgValueUpdate::Create(ValueType valuetype, uint32_t valueid,
                                       uint64_t usec, const char* rgch,
                                       size_t cch,
                                       std::vector<uint8_t>* pvecbRet) {
  MsgValueUpdate* pmsgvalueupdateRet = nullptr;
  size_t cbMessage =
      cch + reinterpret_cast<size_t>(
                &reinterpret_cast<MsgValueUpdate*>(0)->value.str.rgch[0]);

  if (cbMessage <= 255) {
    pmsgvalueupdateRet = CreateCore(valuetype, valueid, usec, cch, pvecbRet);

    if (pmsgvalueupdateRet != nullptr) {
      pmsgvalueupdateRet->value.str.cch = (uint8_t)cch;
      memcpy(pmsgvalueupdateRet->value.str.rgch, rgch, cch);
    }
  }

  return (pmsgvalueupdateRet);
}

MsgValueUpdate* MsgValueUpdate::Create(ValueType valuetype, uint32_t valueid,
                                       uint64_t usec, const std::string& str,
                                       std::vector<uint8_t>* pvecbRet) {
  return (Create(valuetype, valueid, usec, str.data(), str.size(), pvecbRet));
}

template <>
MsgValueUpdate* MsgValueUpdate::Create<int64_t>(
    ValueType valuetype, uint32_t valueid, uint64_t usec, int64_t i64,
    std::vector<uint8_t>* pvecbRet) {
  auto pmsgvalueupdate =
      CreateCore(valuetype, valueid, usec, sizeof(int64_t), pvecbRet);

  if (pmsgvalueupdate != nullptr) pmsgvalueupdate->value.i64 = i64;

  return (pmsgvalueupdate);
}

template <>
MsgValueUpdate* MsgValueUpdate::Create<uint64_t>(
    ValueType valuetype, uint32_t valueid, uint64_t usec, uint64_t ui64,
    std::vector<uint8_t>* pvecbRet) {
  auto pmsgvalueupdate =
      CreateCore(valuetype, valueid, usec, sizeof(uint64_t), pvecbRet);

  if (pmsgvalueupdate != nullptr) pmsgvalueupdate->value.ui64 = ui64;

  return (pmsgvalueupdate);
}

template <>
MsgValueUpdate* MsgValueUpdate::Create<double>(ValueType valuetype,
                                               uint32_t valueid, uint64_t usec,
                                               double r,
                                               std::vector<uint8_t>* pvecbRet) {
  auto pmsgvalueupdate =
      CreateCore(valuetype, valueid, usec, sizeof(double), pvecbRet);

  if (pmsgvalueupdate != nullptr) pmsgvalueupdate->value.r = r;

  return (pmsgvalueupdate);
}

template <>
MsgValueUpdate* MsgValueUpdate::Create<const char*>(
    ValueType valuetype, uint32_t valueid, uint64_t usec, const char* sz,
    std::vector<uint8_t>* pvecbRet) {
  size_t cch = strlen(sz);
  return (Create(valuetype, valueid, usec, sz, cch, pvecbRet));
}

MsgValueUpdate* MsgValueUpdate::CreateCore(ValueType valuetype,
                                           uint32_t valueid, uint64_t usec,
                                           size_t cbValue,
                                           std::vector<uint8_t>* pvecbRet) {
  MsgValueUpdate* pmsgvalueupdateRet = nullptr;
  size_t cbMsg = sizeof(MsgValueUpdate);

  if (cbValue > sizeof(uint64_t)) cbMsg += cbValue - sizeof(uint64_t);

  assert(cbMsg < 256);  // Maximum data payload size of a message
  if (cbMsg > 255) return (nullptr);

  pvecbRet->resize(cbMsg);
  pmsgvalueupdateRet = new (pvecbRet->data())
      MsgValueUpdate(valueid, usec, valuetype, (uint8_t)cbMsg);

  return (pmsgvalueupdateRet);
}

void MsgValueUpdate::NToH(void) {
  valueid = ntohl(valueid);
  usec = LVMon::NToH(usec);

  switch ((ValueType)valuetype) {
    default:
    case ValueType::Double:
    case ValueType::String:
      // Swap no needed
      break;

    case ValueType::Int64: {
      uint64_t ui64 = *reinterpret_cast<uint64_t*>(&value.i64);

      ui64 = LVMon::NToH(ui64);
      value.i64 = *reinterpret_cast<int64_t*>(&ui64);
    } break;

    case ValueType::UInt64:
      value.ui64 = LVMon::NToH(value.ui64);
      break;
  }
}

}  // namespace LVMon
