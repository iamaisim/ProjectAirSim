// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef ROVER_API_INCLUDE_ROVER_API_BASE_HPP_
#define ROVER_API_INCLUDE_ROVER_API_BASE_HPP_

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "common/function_caller.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/service_method.hpp"
#include "irover_api.hpp"

namespace microsoft {
namespace projectairsim {

class RoverApiBase : public IController, public IRoverApi {
 public:
  RoverApiBase(void);
  RoverApiBase(const Robot& robot, TransformTree* ptransformtree);

  virtual ~RoverApiBase(void) = default;

  //---------------------------------------------------------------------------
  // IRoverApi Methods
  // (Redeclared here so we can attach service methods)

  virtual bool DisableApiControl(void) = 0;
  virtual bool EnableApiControl(void) = 0;
  virtual bool IsApiControlEnabled(void) = 0;

  virtual bool Arm(int64_t command_start_time_nanos) = 0;
  virtual bool Disarm(void) = 0;
  virtual bool CanArm(void) const = 0;

  virtual bool MoveToPosition(float x, float y, float velocity,
                              float timeout_sec, float yaw_rate_max,
                              float lookahead, float adaptive_lookahead,
                              int64_t command_start_time_nanos) = 0;

  virtual bool MoveByHeading(float heading, float speed,
                             float duration, float heading_margin,
                             float yaw_rate, float timeout_sec,
                             int64_t command_start_time_nanos) = 0;

 protected:
  typedef std::function<bool()> WaitFunction;

 protected:  // Usually or must be implemented by subclasses
  virtual bool CanArmServiceMethod(void);
  virtual float GetCommandPeriod()
      const = 0;  // Time interval between two commands required for drone (seconds)
  virtual std::string GetControllerName(void) const = 0;
  virtual Kinematics GetKinematicsEstimated(void) const = 0;
  virtual void LoadParams(
      const std::unordered_map<std::string, float>& params_map) {}
  virtual void RegisterServiceMethods(void);

 protected:
  bool CancelLastTask(void);
    Logger GetLogger(void) const { return sim_robot_.GetLogger(); }
  virtual Vector3 GetPosition() const {
    return GetKinematicsEstimated().pose.position;
  }
  virtual Vector3 GetVelocity() const {
    return GetKinematicsEstimated().twist.linear;
  }
  virtual Vector3 GetAngles() const {
    auto orientation = GetKinematicsEstimated().pose.orientation;
    return Vector3(PhysicsUtils::GetRoll<float>(orientation),
                   PhysicsUtils::GetPitch<float>(orientation),
                   PhysicsUtils::GetYaw<float>(orientation));
  }
  virtual Quaternion GetOrientation() const {
    return GetKinematicsEstimated().pose.orientation;
  }

  CancelToken& GetCancelToken() { return cancel_token_; }
  vehicle_apis::FunctionCaller RunTimedCommand(
      WaitFunction flight_controller_function, float flight_command_timeout_sec,
      int64_t command_start_time_nanos);

 protected:
  CancelToken cancel_token_; //Command cancellation request token
  Robot sim_robot_;  // Associated robot
  TransformTree*
      psim_transformtree_;  // The transform tree containing the robot
};                          // class RoverApiBase

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MULTIROTOR_API_BASE_HPP_
