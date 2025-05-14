// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_ICONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_ICONTROLLER_HPP_

#include <cstdint>

#include "IBoardClock.hpp"
#include "IGoal.hpp"
#include "IStateEstimator.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IController : public IUpdatable {
 public:
  virtual void Initialize(const IGoal* goal,
                          const IStateEstimator* state_estimator) = 0;
  virtual const AxisNr& GetOutput() = 0;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_ICONTROLLER_HPP_