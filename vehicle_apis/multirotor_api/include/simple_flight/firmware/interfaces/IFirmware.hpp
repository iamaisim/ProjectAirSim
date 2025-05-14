// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IFIRMWARE_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IFIRMWARE_HPP_

#include "IOffboardApi.hpp"
#include "IStateEstimator.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IFirmware : public IUpdatable {
 public:
  virtual IOffboardApi& OffboardApi() = 0;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IFIRMWARE_HPP_