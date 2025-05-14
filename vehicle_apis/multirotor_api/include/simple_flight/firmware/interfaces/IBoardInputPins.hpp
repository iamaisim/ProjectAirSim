// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDINPUTPINS_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDINPUTPINS_HPP_
#include <cstdint>

namespace simple_flight {

class IBoardInputPins {
 public:
  virtual float ReadChannel(uint16_t index) const = 0;  // output -1 to 1
  virtual bool IsRcConnected() const = 0;
  virtual float GetAvgMotorOutput() const = 0;

  virtual ~IBoardInputPins() = default;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDINPUTPINS_HPP_