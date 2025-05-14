// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMONMESSAGE_H_
#define TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMONMESSAGE_H_

#include <vector>

namespace LVMon {

class CLVMonMessage {
 public:
  void Clear(void);
  bool Process(uint8_t** ppbReceivedInOut, uint8_t* pbReceivedMax,
               uint8_t** ppbMessageRet, size_t* pcbMessageRet);

 protected:
  size_t cbMessage_;                  // Number of expected bytes in the message
  std::vector<uint8_t> vecbBuffer_;   // Buffer for partially received message
  std::vector<uint8_t> vecbMessage_;  // Buffer for fully received message
};                                    // class CLVMonMessage

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_INCLUDE_LVMON_LVMONMESSAGE_H_
