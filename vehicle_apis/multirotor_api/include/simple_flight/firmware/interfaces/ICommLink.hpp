// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_ICOMMLINK_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_ICOMMLINK_HPP_

#include <cstdint>
#include <string>

#include "IUpdatable.hpp"

namespace simple_flight {

class ICommLink : public IUpdatable {
 public:
  static constexpr int kLogLevelInfo = 0;
  static constexpr int kLogLevelWarn = 1;
  static constexpr int kLogLevelError = 2;

  virtual void Log(const std::string& message,
                   int32_t log_level = ICommLink::kLogLevelInfo) = 0;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_ICOMMLINK_HPP_