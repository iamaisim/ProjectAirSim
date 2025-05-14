// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMONPROTOCOL_H_
#define TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMONPROTOCOL_H_

#include <stdint.h>

#include <string>
#include <vector>

namespace LVMon {

#pragma pack(push)
#pragma pack(1)

// Default TCP ports
static const int kPortDefault = 22433;
static const char* kSzPortDefault = "22433";

enum class MessageType {
  Null = 0,  // Ignored message, can be used to probe where connection is still
             // good
  GetNames = 1,  // Request list of available values, to be returned as a series
                 // of NewName messages
  NewName = 2,   // Reports the name of a new value
  Subscribe = 3,    // Subscribe to changes in a value
  Unsubscribe = 4,  // Unsubscribe from changes in a value
  ValueUpdate = 5,  // Value has changed
  SetValue = 6,     // Set a value
};                  // enum class MessageType

typedef uint64_t MicroSec;  // Microseconds

typedef uint32_t ValueID;  // Unique short ID for a value

enum class ValueType {
  Int64 = 0,  // 64-bit signed integer
  UInt64,     // 64-bit unsigned integer
  Double,     // 64-bit floating-point value
  String,     // 8-bit character string
};            // enum class ValueType

struct MsgHeader {
  uint8_t messagetype;  // Enum MessageType value
  uint8_t cbData;       // Number of data bytes following this message

  MsgHeader(MessageType messagetypeIn, uint8_t cbDataIn)
      : messagetype((uint8_t)messagetypeIn), cbData(cbDataIn) {}

  void HToN(void) {}
  void NToH(void) {}
};  // struct MsgHeader

struct MsgGetNames : public MsgHeader {
  MsgGetNames(void) : MsgHeader(MessageType::GetNames, 0) {}

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void) {}
};  // struct MsgGetNames

struct MsgNewName : public MsgHeader {
  uint32_t valueid;   // Unique ID of this value
  uint8_t valuetype;  // Enum ValueType value
  uint8_t rgch[1];    // First character of value name (length set by
                      // MsgHeader.cbData, not NUL terminated)

  static MsgNewName* Create(ValueType valuetype, uint32_t valueid,
                            const char* sz, std::vector<uint8_t>* pvecbRet);

  MsgNewName(ValueType valuetypeIn, uint32_t valueidIn, uint8_t cch)
      : MsgHeader(MessageType::NewName,
                  sizeof(MsgNewName) - sizeof(MsgHeader) + cch - 1),
        valueid(valueidIn),
        valuetype((uint8_t)valuetypeIn),
        rgch{0} {}

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void);

  size_t GetCch(void) const;
};  // struct MsgNewName

struct MsgNull : public MsgHeader {
  MsgNull(void) : MsgHeader(MessageType::Null, 0) {}

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void) {}
};  // struct MsgNull

struct MsgSetValue : public MsgHeader {
  uint32_t valueid;   // Unique ID of this value
  uint8_t valuetype;  // Enum ValueType value
  union {
    int64_t i64;    // ValueType::Int64 value
    uint64_t ui64;  // UInt64
    double r;       // Double
    struct {
      uint8_t cch;
      char rgch[1];
    } str;  // String
  } value;  // Value depending on valuetype

  static MsgSetValue* Create(ValueType valuetype, uint32_t valueid,
                             const std::string& str,
                             std::vector<uint8_t>* pvecbRet);
  static MsgSetValue* Create(ValueType valuetype, uint32_t valueid,
                             const char* rgch, size_t cch,
                             std::vector<uint8_t>* pvecbRet);
  template <typename T>
  static MsgSetValue* Create(ValueType valuetype, uint32_t valueid, T t,
                             std::vector<uint8_t>* pvecbRet);

  MsgSetValue(uint32_t valueidIn, ValueType valuetypeIn, uint8_t cbData)
      : MsgHeader(MessageType::SetValue, cbData),
        valueid(valueidIn),
        valuetype((uint8_t)valuetypeIn) {
    value.i64 = 0;
  }

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void);

 protected:
  static MsgSetValue* CreateCore(ValueType valuetype, uint32_t valueid,
                                 size_t cbValue,
                                 std::vector<uint8_t>* pvecbRet);
};  // struct MsgSetValue

struct MsgSubscribe : public MsgHeader {
  uint32_t valueid;  // Unique ID of the value to subscribe to

  MsgSubscribe(uint32_t valueidIn)
      : MsgHeader(MessageType::Subscribe,
                  sizeof(MsgSubscribe) - sizeof(MsgHeader)),
        valueid(valueidIn) {}

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void);
};  // struct MsgSubscribe

struct MsgUnsubscribe : public MsgHeader {
  uint32_t valueid;  // Unique ID of the value to unsubscribe from

  MsgUnsubscribe(uint32_t valueidIn)
      : MsgHeader(MessageType::Subscribe,
                  sizeof(MsgUnsubscribe) - sizeof(MsgHeader)),
        valueid(valueidIn) {}

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void);
};  // struct MsgUnsubscribe

struct MsgValueUpdate : public MsgHeader {
  uint32_t valueid;   // Unique ID of this value
  uint64_t usec;      // Timestamp in microseconds
  uint8_t valuetype;  // Enum ValueType value
  union {
    int64_t i64;    // ValueType::Int64 value
    uint64_t ui64;  // UInt64
    double r;       // Double
    struct {
      uint8_t cch;
      char rgch[1];
    } str;  // String
  } value;  // Value depending on valuetype

  static MsgValueUpdate* Create(ValueType valuetype, uint32_t valueid,
                                uint64_t usec, const std::string& str,
                                std::vector<uint8_t>* pvecbRet);
  static MsgValueUpdate* Create(ValueType valuetype, uint32_t valueid,
                                uint64_t usec, const char* rgch, size_t cch,
                                std::vector<uint8_t>* pvecbRet);
  template <typename T>
  static MsgValueUpdate* Create(ValueType valuetype, uint32_t valueid,
                                uint64_t usec, T t,
                                std::vector<uint8_t>* pvecbRet);

  MsgValueUpdate(uint32_t valueidIn, uint64_t usecIn, ValueType valuetypeIn,
                 uint8_t cbData)
      : MsgHeader(MessageType::ValueUpdate, cbData),
        valueid(valueidIn),
        usec(usecIn),
        valuetype((uint8_t)valuetypeIn) {
    value.i64 = 0;
  }

  void HToN(void) {
    MsgHeader::HToN();
    NToH();
  }
  void NToH(void);

 protected:
  static MsgValueUpdate* CreateCore(ValueType valuetype, uint32_t valueid,
                                    uint64_t usec, size_t cbValue,
                                    std::vector<uint8_t>* pvecbRet);
};  // struct MsgValueUpdate

#pragma pack(pop)

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMONPROTOCOL_H_
