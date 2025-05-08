// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IFWANGLECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IFWANGLECONTROLLER_HPP_

#include "IAxisController.hpp"

namespace simple_flight {

// The IFWAngleController interfaces adds a method to returns the desired angle
// for an axis rather than the desired torque.  In some fixed-wing goal
// scenarios a fixed-wing axis "aggregate" controller (like FWYTorqueController)
// needs to merge the output of several child controllers to produce a single
// torque value.  Merging torque values can cause instability as the PID
// controllers fight each other.  Merging the target axis angles of those child
// controllers instead avoids the PID tugs-of-war.
class IFWAngleController : public IAxisController {
 public:
  virtual TReal GetOutputAngle(void) = 0;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IFWANGLECONTROLLER_HPP_