// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IAXISCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IAXISCONTROLLER_HPP_

#include <cstdint>

#include "IBoardClock.hpp"
#include "IGoal.hpp"
#include "IStateEstimator.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IAxisController : public IUpdatable {
 public:
  enum AxisID {
    kXRoll = 0,
    kYPitch = 1,
    kZYaw = 2,
    kZHeight = 3,
  };  // enum AxisID

 public:
  virtual void Initialize(unsigned int axis, const IGoal* goal,
                          const IStateEstimator* state_estimator) = 0;
  virtual TReal GetOutput() = 0;

  virtual void Reset() override {
    // disable checks for reset/update sequence because
    // this object may get created but not used
    ClearResetUpdateAsserts();
    IUpdatable::Reset();
  }
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IAXISCONTROLLER_HPP_