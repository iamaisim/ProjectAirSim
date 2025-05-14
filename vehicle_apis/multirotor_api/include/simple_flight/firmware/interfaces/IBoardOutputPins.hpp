// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDOUTPUTPINS_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDOUTPUTPINS_HPP_

#include <cstdint>

namespace simple_flight {

class IBoardOutputPins {
 public:
  virtual void WriteOutput(
      uint16_t index,
      float val) = 0;  // val = -1 to 1 for reversible motors otherwise 0 to 1

  virtual ~IBoardOutputPins() = default;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDOUTPUTPINS_HPP_