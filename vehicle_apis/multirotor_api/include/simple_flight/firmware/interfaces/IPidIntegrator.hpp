// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IPIDINTEGRATOR_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IPIDINTEGRATOR_HPP_

#include <algorithm>

#include "CommonStructs.hpp"

namespace simple_flight {

template <typename T>
class IPidIntegrator {
 public:
  virtual ~IPidIntegrator() {}
  virtual void Reset() = 0;
  virtual void Set(T val) = 0;
  virtual void Update(float dt, T error) = 0;
  virtual T GetOutput() = 0;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IPIDINTEGRATOR_HPP_