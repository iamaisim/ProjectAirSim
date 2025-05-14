// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTCOMMLINK_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTCOMMLINK_HPP_

#include <exception>
#include <vector>

#include "firmware/SimpleFlightCommonUtils.hpp"
#include "firmware/interfaces/ICommLink.hpp"

namespace microsoft {
namespace projectairsim {

class AirSimSimpleFlightCommLink : public simple_flight::ICommLink {
 public:  // derived class specific methods
  void GetStatusMessages(std::vector<std::string>& messages) {
    if (messages_.size() > 0) {
      messages.insert(messages.end(), messages_.begin(), messages_.end());
      messages_.clear();
    }
  }

 public:  // implement CommLink interface
  virtual void Reset() {
    simple_flight::ICommLink::Reset();
    messages_.clear();
  }

  virtual void Update() { simple_flight::ICommLink::Update(); }

  virtual void Log(const std::string& message,
                   int32_t log_level = ICommLink::kLogLevelInfo) {
    unused_simpleflight(log_level);
    // if (log_level > 0)
    //    Utils::DebugBreak();
    messages_.push_back(std::string(message));
  }

 private:
  std::vector<std::string> messages_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTCOMMLINK_HPP_