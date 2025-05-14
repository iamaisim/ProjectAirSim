// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDCLOCK_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDCLOCK_HPP_

#include <cstdint>

namespace simple_flight {

class IBoardClock {
 public:
  virtual int64_t Micros() const = 0;
  virtual int64_t Millis() const = 0;

  virtual ~IBoardClock() = default;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDCLOCK_HPP_