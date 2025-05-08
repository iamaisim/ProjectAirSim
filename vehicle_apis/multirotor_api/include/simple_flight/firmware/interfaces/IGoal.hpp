// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IGOAL_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IGOAL_HPP_

#include "CommonStructs.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IGoal {
 public:
  virtual const Axis4r& GetGoalValue() const = 0;
  virtual const GoalMode& GetGoalMode() const = 0;

  virtual ~IGoal() = default;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IGOAL_HPP_