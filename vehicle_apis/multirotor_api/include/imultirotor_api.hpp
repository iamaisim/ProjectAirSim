// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_IMULTIROTOR_API_HPP_
#define MULTIROTOR_API_INCLUDE_IMULTIROTOR_API_HPP_

#include <string>

#include "core_sim/physics_common_types.hpp"
// #include "core_sim/topic.hpp"

namespace microsoft {
namespace projectairsim {

enum class DrivetrainType { MaxDegreeOfFreedom = 0, ForwardOnly };
enum class LandedState { Landed = 0, Flying = 1 };

struct ReadyState {
  bool ReadyVal;
  std::string ReadyMessage;
};

// Represents public Multirotor APIs intended for client
class IMultirotorApi {
 public:
  // return value of these functions is true if command was completed without
  // interruption or timeouts

  // common robot control APIs
  virtual bool EnableApiControl() = 0;
  virtual bool DisableApiControl() = 0;
  virtual bool IsApiControlEnabled() = 0;
  virtual bool Arm(int64_t command_start_time_nanos) = 0;
  virtual bool Disarm() = 0;
  virtual bool CanArm() const = 0;
  virtual ReadyState GetReadyState() const = 0;
  virtual bool CancelLastTask() = 0;
  virtual LandedState GetLandedState() const = 0;
  virtual Kinematics GetKinematicsEstimated() const = 0;

  // control directly by providing motor PWMs -- lowest level multirotor control
  virtual bool MoveByMotorPWMs(float front_right_pwm, float rear_left_pwm,
                               float front_left_pwm, float rear_right_pwm,
                               float duration,
                               int64_t command_start_time_nanos) = 0;

  // control by angle rate
  virtual bool MoveByAngleRatesZ(float roll_rate, float pitch_rate,
                                 float yaw_rate, float z, float duration,
                                 int64_t command_start_time_nanos) = 0;

  virtual bool MoveByAngleRatesThrottle(float roll_rate, float pitch_rate,
                                        float yaw_rate, float throttle,
                                        float duration,
                                        int64_t command_start_time_nanos) = 0;
  // control by angle
  virtual bool MoveByRollPitchYawZ(float roll, float pitch, float yaw, float z,
                                   float duration,
                                   int64_t command_start_time_nanos) = 0;

  virtual bool MoveByRollPitchYawThrottle(float roll, float pitch, float yaw,
                                          float throttle, float duration,
                                          int64_t command_start_time_nanos) = 0;

  // control by mix of angle-rate and angle
  virtual bool MoveByRollPitchYawrateThrottle(
      float roll, float pitch, float yaw_rate, float throttle, float duration,
      int64_t command_start_time_nanos) = 0;

  virtual bool MoveByRollPitchYawrateZ(float roll, float pitch, float yaw_rate,
                                       float z, float duration,
                                       int64_t command_start_time_nanos) = 0;

  // control by velocity
  virtual bool MoveByVelocity(float vx, float vy, float vz, float duration,
                              DrivetrainType drivetrain, bool yaw_is_rate,
                              float yaw, int64_t command_start_time_nanos) = 0;

  virtual bool MoveByVelocityBodyFrame(float vx, float vy, float vz,
                                       float duration,
                                       DrivetrainType drivetrain,
                                       bool yaw_is_rate, float yaw,
                                       int64_t command_start_time_nanos) = 0;

  virtual bool MoveByVelocityZ(float vx, float vy, float z, float duration,
                               DrivetrainType drivetrain, bool yaw_is_rate,
                               float yaw, int64_t command_start_time_nanos) = 0;

  virtual bool MoveByVelocityBodyFrameZ(float vx, float vy, float z,
                                        float duration,
                                        DrivetrainType drivetrain,
                                        bool yaw_is_rate, float yaw,
                                        int64_t command_start_time_nanos) = 0;

  // Switched to using service method request-response for all control commands,
  // but leaving the below pub-sub version commented out for reference in case
  // it's needed in the future.
  // virtual void OnSetpointNEDvelocityYawrate(
  //     const Topic& topic, const Message& message) = 0;

  // control by position
  virtual bool MoveToPosition(float x, float y, float z, float velocity,
                              float timeout_sec, DrivetrainType drivetrain,
                              bool yaw_is_rate, float yaw, float lookahead,
                              float adaptive_lookahead,
                              int64_t command_start_time_nanos) = 0;

  virtual bool MoveToZ(float z, float velocity, float timeout_sec,
                       bool yaw_is_rate, float yaw, float lookahead,
                       float adaptive_lookahead,
                       int64_t command_start_time_nanos) = 0;

  virtual bool MoveOnPath(std::vector<std::vector<float>> path, float velocity,
                          float timeout_sec, DrivetrainType drivetrain,
                          bool yaw_is_rate, float yaw, float lookahead,
                          float adaptive_lookahead,
                          int64_t command_start_time_nanos) = 0;

  virtual bool RotateToYaw(float yaw, float timeout_sec, float margin, float yaw_rate,
                           int64_t command_start_time_nanos) = 0;

  virtual bool RotateByYawRate(float yaw_rate, float duration,
                               int64_t command_start_time_nanos) = 0;

  // high level control APIs
  virtual bool Takeoff(float timeout_sec, int64_t command_start_time_nanos) = 0;
  virtual bool Land(float timeout_sec, int64_t command_start_time_nanos) = 0;
  virtual bool GoHome(float timeout_sec, float velocity,
                      int64_t command_start_time_nanos) = 0;
  virtual bool Hover(int64_t command_start_time_nanos) = 0;
  virtual bool RequestControl(int64_t command_start_time_nanos) = 0;
  virtual bool SetMissionMode(int64_t command_start_time_nanos) = 0;

  // controller configs
  virtual void SetAngleLevelControllerGains(const std::vector<float>& kp,
                                            const std::vector<float>& ki,
                                            const std::vector<float>& kd) = 0;

  virtual void SetAngleRateControllerGains(const std::vector<float>& kp,
                                           const std::vector<float>& ki,
                                           const std::vector<float>& kd) = 0;

  virtual void SetVelocityControllerGains(const std::vector<float>& kp,
                                          const std::vector<float>& ki,
                                          const std::vector<float>& kd) = 0;

  virtual void SetPositionControllerGains(const std::vector<float>& kp,
                                          const std::vector<float>& ki,
                                          const std::vector<float>& kd) = 0;

  // TODO:
  // Some v1 Control APIs not currently supported
  //    1) Manual Control APIs
  //    2) Safety APIs
  // v1 State APIs not currently supported
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_IMULTIROTOR_API_HPP_