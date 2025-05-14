// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IOFFBOARDAPI_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IOFFBOARDAPI_HPP_

#include <string>

#include "CommonStructs.hpp"
#include "IGoal.hpp"
#include "IStateEstimator.hpp"

namespace simple_flight {

class IOffboardApi : public IGoal {
 public:
  virtual bool CanRequestApiControl(std::string& message) = 0;
  virtual bool HasApiControl() = 0;
  virtual bool RequestApiControl(std::string& message) = 0;
  virtual void ReleaseApiControl() = 0;
  virtual bool SetGoalAndMode(const Axis4r* goal, const GoalMode* goal_mode,
                              std::string& message) = 0;
  virtual bool Arm(std::string& message) = 0;
  virtual bool Disarm(std::string& message) = 0;
  virtual VehicleStateType GetVehicleState() const = 0;
  virtual bool GetLandedState() const = 0;
  virtual const IStateEstimator& GetStateEstimator() = 0;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IOFFBOARDAPI_HPP_