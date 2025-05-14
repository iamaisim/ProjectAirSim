// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_COMMONSTRUCTS_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_COMMONSTRUCTS_HPP_

#include <cmath>
#include <common/common.hpp>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace simple_flight {

using microsoft::projectairsim::vehicle_apis::k2PI;
using microsoft::projectairsim::vehicle_apis::kPI;
using microsoft::projectairsim::vehicle_apis::kPI_2;
using microsoft::projectairsim::vehicle_apis::Axis3;
using microsoft::projectairsim::vehicle_apis::Axis3r;
using microsoft::projectairsim::vehicle_apis::Axis4;
using microsoft::projectairsim::vehicle_apis::Axis4r;
using microsoft::projectairsim::vehicle_apis::AxisN;
using microsoft::projectairsim::vehicle_apis::AxisNr;
using microsoft::projectairsim::vehicle_apis::KinematicsState;
using microsoft::projectairsim::vehicle_apis::PidConfig;
using microsoft::projectairsim::vehicle_apis::TReal;
using microsoft::projectairsim::vehicle_apis::VehicleState;
using microsoft::projectairsim::vehicle_apis::VehicleStateType;

enum class GoalModeType : int {
  kUnknown = 0,
  kPassthrough,
  kAngleLevel,
  kAngleRate,
  kVelocityWorld,
  kVelocityBody,  // Velocity expressed in the horizontal plane (Forward Right
                  // Down). The name "body" was kept since it is more intuitive
                  // for the user.
  kPositionWorld,
  kConstantOutput
};

class GoalMode
    : public microsoft::projectairsim::vehicle_apis::Axis4<GoalModeType> {
 public:
  GoalMode(GoalModeType x_val = GoalModeType::kAngleLevel,
           GoalModeType y_val = GoalModeType::kAngleLevel,
           GoalModeType z_val = GoalModeType::kAngleRate,
           GoalModeType val4_val = GoalModeType::kPassthrough)
      : Axis4<GoalModeType>(x_val, y_val, z_val, val4_val) {}

  static const GoalMode& GetStandardAngleMode() {
    static const GoalMode mode = GoalMode();
    return mode;
  }

  static const GoalMode& GetVelocityXYPosZMode() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kVelocityWorld, GoalModeType::kVelocityWorld,
                 GoalModeType::kAngleRate, GoalModeType::kPositionWorld);
    return mode;
  }

  static const GoalMode& GetVelocityBodyXYPosZMode() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kVelocityBody, GoalModeType::kVelocityBody,
                 GoalModeType::kAngleRate, GoalModeType::kPositionWorld);
    return mode;
  }

  static const GoalMode& GetVelocityMode() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kVelocityWorld, GoalModeType::kVelocityWorld,
                 GoalModeType::kAngleRate, GoalModeType::kVelocityWorld);
    return mode;
  }

  static const GoalMode& GetVelocityBodyMode() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kVelocityBody, GoalModeType::kVelocityBody,
                 GoalModeType::kAngleRate, GoalModeType::kVelocityBody);
    return mode;
  }

  static const GoalMode& GetPositionMode() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kPositionWorld, GoalModeType::kPositionWorld,
                 GoalModeType::kAngleRate, GoalModeType::kPositionWorld);
    return mode;
  }

  static const GoalMode& GetAllRateMode() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kAngleRate, GoalModeType::kAngleRate,
                 GoalModeType::kAngleRate, GoalModeType::kPassthrough);
    return mode;
  }

  static const GoalMode& GetUnknown() {
    static const GoalMode mode =
        GoalMode(GoalModeType::kUnknown, GoalModeType::kUnknown,
                 GoalModeType::kUnknown, GoalModeType::kUnknown);
    return mode;
  }
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_COMMONSTRUCTS_HPP_