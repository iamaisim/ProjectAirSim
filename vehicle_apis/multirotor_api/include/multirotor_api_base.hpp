// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MULTIROTOR_API_BASE_HPP_
#define MULTIROTOR_API_INCLUDE_MULTIROTOR_API_BASE_HPP_

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
#include "imultirotor_api.hpp"

namespace microsoft {
namespace projectairsim {

class MultirotorApiBase : public IController, public IMultirotorApi {
 public:
  MultirotorApiBase() {}
  MultirotorApiBase(const Robot& robot, TransformTree* ptransformtree);

  virtual ~MultirotorApiBase() = default;

  void LoadParams(const std::unordered_map<std::string, float>& params_map);

  //---------------------------------------------------------------------------
  // IController

  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override = 0;
  void SetKinematics(const Kinematics* kinematics) override = 0;
  void Update() override = 0;
  std::vector<float> GetControlSignals(
      const std::string& actuator_id) override = 0;
  const IController::GimbalState& GetGimbalSignal(
      const std::string& gimbal_id) override;

  //---------------------------------------------------------------------------
  // IMultirotorApi

  bool EnableApiControl() override = 0;
  bool DisableApiControl() override = 0;
  bool IsApiControlEnabled() override = 0;
  bool Arm(int64_t command_start_time_nanos) override = 0;
  bool Disarm() override = 0;
  bool CanArm() const override = 0;
  ReadyState GetReadyState() const override = 0;
  bool CancelLastTask() override;
  virtual Kinematics GetKinematicsEstimated() const override = 0;
  virtual LandedState GetLandedState() const override = 0;

  bool MoveByMotorPWMs(float front_right_pwm, float rear_left_pwm,
                       float front_left_pwm, float rear_right_pwm,
                       float duration,
                       int64_t command_start_time_nanos) override;

  bool MoveByAngleRatesZ(float roll_rate, float pitch_rate, float yaw_rate,
                         float z, float duration,
                         int64_t command_start_time_nanos) override;

  bool MoveByAngleRatesThrottle(float roll_rate, float pitch_rate,
                                float yaw_rate, float throttle, float duration,
                                int64_t command_start_time_nanos) override;

  bool MoveByRollPitchYawZ(float roll, float pitch, float yaw, float z,
                           float duration,
                           int64_t command_start_time_nanos) override;

  bool MoveByRollPitchYawThrottle(float roll, float pitch, float yaw,
                                  float throttle, float duration,
                                  int64_t command_start_time_nanos) override;

  bool MoveByRollPitchYawrateThrottle(
      float roll, float pitch, float yaw_rate, float throttle, float duration,
      int64_t command_start_time_nanos) override;

  bool MoveByRollPitchYawrateZ(float roll, float pitch, float yaw_rate, float z,
                               float duration,
                               int64_t command_start_time_nanos) override;

  bool MoveByVelocity(float vx, float vy, float vz, float duration,
                      DrivetrainType drivetrain, bool yaw_is_rate, float yaw,
                      int64_t command_start_time_nanos) override;

  bool MoveByVelocityBodyFrame(float vx, float vy, float vz, float duration,
                               DrivetrainType drivetrain, bool yaw_is_rate,
                               float yaw,
                               int64_t command_start_time_nanos) override;

  bool MoveByVelocityZ(float vx, float vy, float z, float duration,
                       DrivetrainType drivetrain, bool yaw_is_rate, float yaw,
                       int64_t command_start_time_nanos) override;

  bool MoveByVelocityBodyFrameZ(float vx, float vy, float z, float duration,
                                DrivetrainType drivetrain, bool yaw_is_rate,
                                float yaw,
                                int64_t command_start_time_nanos) override;

  // Switched to using service method request-response for all control commands,
  // but leaving the below pub-sub version commented out for reference in case
  // it's needed in the future.
  // void OnSetpointNEDvelocityYawrate(const Topic& topic,
  //                                   const Message& message) override = 0;

  bool MoveOnPath(std::vector<std::vector<float>> path, float velocity,
                  float timeout_sec, DrivetrainType drivetrain,
                  bool yaw_is_rate, float yaw, float lookahead,
                  float adaptive_lookahead,
                  int64_t command_start_time_nanos) override;

  bool MoveToPosition(float x, float y, float z, float velocity,
                      float timeout_sec, DrivetrainType drivetrain,
                      bool yaw_is_rate, float yaw, float lookahead,
                      float adaptive_lookahead,
                      int64_t command_start_time_nanos) override;

  bool MoveToZ(float z, float velocity, float timeout_sec, bool yaw_is_rate,
               float yaw, float lookahead, float adaptive_lookahead,
               int64_t command_start_time_nanos) override;

  bool RotateToYaw(float yaw, float timeout_sec, float margin, float yaw_rate,
                   int64_t command_start_time_nanos) override;

  bool RotateByYawRate(float yaw_rate, float duration,
                       int64_t command_start_time_nanos) override;

  // high level control APIs
  bool Takeoff(float timeout_sec, int64_t command_start_time_nanos) override;
  bool Land(float timeout_sec, int64_t command_start_time_nanos) override;
  bool GoHome(float timeout_sec, float velocity,
              int64_t command_start_time_nanos) override;
  bool Hover(int64_t command_start_time_nanos) override;
  bool RequestControl(int64_t /*command_start_time_nanos*/) override;
  bool SetMissionMode(int64_t command_start_time_nanos) override;

  // controller configs
  void SetAngleLevelControllerGains(const std::vector<float>& kp,
                                    const std::vector<float>& ki,
                                    const std::vector<float>& kd) override;

  void SetAngleRateControllerGains(const std::vector<float>& kp,
                                   const std::vector<float>& ki,
                                   const std::vector<float>& kd) override;

  void SetVelocityControllerGains(const std::vector<float>& kp,
                                  const std::vector<float>& ki,
                                  const std::vector<float>& kd) override;

  void SetPositionControllerGains(const std::vector<float>& kp,
                                  const std::vector<float>& ki,
                                  const std::vector<float>& kd) override;

 protected:  // must be implemented
  virtual std::string GetControllerName() const = 0;

  //---------------------------------------------------------------------------
  // Low level commands

  virtual void CommandMotorPWMs(float front_right_pwm, float rear_left_pwm,
                                float front_left_pwm, float rear_right_pwm) = 0;

  virtual void CommandRollPitchYawrateThrottle(float roll, float pitch,
                                               float yaw_rate,
                                               float throttle) = 0;

  virtual void CommandRollPitchYawZ(float roll, float pitch, float yaw,
                                    float z) = 0;

  virtual void CommandRollPitchYawThrottle(float roll, float pitch, float yaw,
                                           float throttle) = 0;

  virtual void CommandRollPitchYawrateZ(float roll, float pitch, float yaw_rate,
                                        float z) = 0;

  virtual void CommandAngleRatesZ(float roll_rate, float pitch_rate,
                                  float yaw_rate, float z) = 0;

  virtual void CommandAngleRatesThrottle(float roll_rate, float pitch_rate,
                                         float yaw_rate, float throttle) = 0;

  virtual void CommandVelocity(float vx, float vy, float vz, bool yaw_is_rate,
                               float yaw) = 0;

  virtual void CommandVelocityZ(float vx, float vy, float z, bool yaw_is_rate,
                                float yaw) = 0;

  virtual void CommandVelocityBody(float vx, float vy, float vz,
                                   bool yaw_is_rate, float yaw) = 0;

  virtual void CommandVelocityZBody(float vx, float vy, float z,
                                    bool yaw_is_rate, float yaw) = 0;

  virtual void CommandPosition(float x, float y, float z, bool yaw_is_rate,
                               float yaw) = 0;

  // controller configs
  virtual void SetControllerGains(uint8_t controllerType,
                                  const std::vector<float>& kp,
                                  const std::vector<float>& ki,
                                  const std::vector<float>& kd) = 0;

  /************************* State APIs *********************************/
  // properties of vehicle
  struct MultirotorApiParams {
    MultirotorApiParams(){};

    void LoadParams(const std::unordered_map<std::string, float>& params_map) {
      if (params_map.find("MC_VEL_TO_BRK_DIST") != params_map.end())
        vel_to_braking_dist = params_map.at("MC_VEL_TO_BRK_DIST");

      if (params_map.find("MC_MIN_BRK_DIST") != params_map.end())
        min_braking_dist = params_map.at("MC_MIN_BRK_DIST");

      if (params_map.find("MC_MAX_BRK_DIST") != params_map.end())
        max_braking_dist = params_map.at("MC_MAX_BRK_DIST");

      if (params_map.find("MC_BRK_VEL") != params_map.end())
        braking_vel = params_map.at("MC_BRK_VEL");

      if (params_map.find("MC_MIN_BRK_VEL") != params_map.end())
        min_vel_for_braking = params_map.at("MC_MIN_BRK_VEL");

      if (params_map.find("MC_DIST_ACC") != params_map.end())
        distance_accuracy = params_map.at("MC_DIST_ACC");

      if (params_map.find("MC_CP_DIST") != params_map.end())
        obs_clearance = params_map.at("MC_CP_DIST");

      if (params_map.find("MC_OBS_WINDOW") != params_map.end())
        obs_window = params_map.at("MC_OBS_WINDOW");
    }

    // what is the braking distance for given velocity?
    // Below is just proportionality constant to convert from velocity to
    // braking distance
    float vel_to_braking_dist =
        0.5f;  // ideally this should be 2X for very high speed but for testing
               // we are keeping it 0.5
    float min_braking_dist = 1;  // min braking distance
    float max_braking_dist = 3;  // min braking distance
    float braking_vel = 1.0f;
    float min_vel_for_braking = 3;

    // what is the differential positional accuracy of cur_loc?
    // this is not same as GPS accuracy because translational errors
    // usually cancel out. Typically this would be 0.2m or less
    float distance_accuracy = 0.1f;

    // what is the minimum clearance from obstacles?
    float obs_clearance = 2;

    // what is the +/-window we should check on obstacle map?
    // for example 2 means check from ticks -2 to 2
    int obs_window = 0;
  };

  virtual GeoPoint GetGpsLocationEstimated() const = 0;
  virtual bool CanArmServiceMethod();
  virtual ReadyStateMessage GetReadyStateServiceMethod();
  virtual KinematicsMessage GetKinematicsEstimatedServiceMethod();
  virtual LandedState GetLandedStateServiceMethod();
  virtual const MultirotorApiParams& GetMultirotorApiParams() const = 0;

  /************************* basic config APIs
   * *********************************/
  virtual float GetCommandPeriod()
      const = 0;  // time between two command required for drone in seconds
  virtual float GetTakeoffZ()
      const = 0;  // the height above ground for the drone after successful
                  // takeoff (Z above ground is negative due to NED coordinate
                  // system).
  // noise in difference of two position coordinates. This is not GPS or
  // position accuracy which can be very low such as 1m. the difference between
  // two position cancels out transitional errors. Typically this would be 0.1m
  // or lower.
  virtual float GetDistanceAccuracy() const = 0;

 protected:  // optional overrides but recommended, default values may work
  virtual float GetAutoLookahead(float velocity, float adaptive_lookahead,
                                 float max_factor = 40,
                                 float min_factor = 30) const;
  virtual float GetObsAvoidanceVelocity(float risk_dist,
                                        float max_obs_avoidance_vel) const;

  // below methods gets called by default implementations of move-related
  // commands that would use a long running loop. These can be used by derived
  // classes to do some init/cleanup.
  virtual void BeforeTask() {
    // default is do nothing
  }
  virtual void AfterTask() {
    // default is do nothing
  }

 protected:  // utility methods
  typedef std::function<bool()> WaitFunction;

  //*********************************safe wrapper around low level
  // commands***************************************************
  virtual void MoveByRollPitchYawZInternal(float roll, float pitch, float yaw,
                                           float z);
  virtual void MoveByRollPitchYawThrottleInternal(float roll, float pitch,
                                                  float yaw, float throttle);
  virtual void MoveByRollPitchYawrateThrottleInternal(float roll, float pitch,
                                                      float yaw_rate,
                                                      float throttle);
  virtual void MoveByRollPitchYawrateZInternal(float roll, float pitch,
                                               float yaw_rate, float z);
  virtual void MoveByAngleRatesZInternal(float roll_rate, float pitch_rate,
                                         float yaw_rate, float z);
  virtual void MoveByAngleRatesThrottleInternal(float roll_rate,
                                                float pitch_rate,
                                                float yaw_rate, float throttle);
  virtual void MoveByVelocityInternal(float vx, float vy, float vz,
                                      bool yaw_is_rate, float yaw);
  virtual void MoveByVelocityZInternal(float vx, float vy, float z,
                                       bool yaw_is_rate, float yaw);
  virtual void MoveByVelocityBodyInternal(float vx, float vy, float vz,
                                          bool yaw_is_rate, float yaw);
  virtual void MoveByVelocityZBodyInternal(float vx, float vy, float z,
                                           bool yaw_is_rate, float yaw);
  virtual void MoveToPositionInternal(const Vector3& dest, bool yaw_is_rate,
                                      float yaw);
  virtual bool MoveOnPathInternal(std::vector<Vector3> path, float velocity,
                                  float timeout_sec, DrivetrainType drivetrain,
                                  bool yaw_is_rate, float yaw, float lookahead,
                                  float adaptive_lookahead,
                                  int64_t command_start_time_nanos);

  /************* safety checks & emergency maneuvers ************/
  // TODO: virtual bool emergencyManeuverIfUnsafe(const SafetyEval::EvalResult&
  // result);
  virtual bool SafetyCheckVelocity(const Vector3& velocity);
  virtual bool SafetyCheckVelocityZ(float vx, float vy, float z);
  virtual bool SafetyCheckDestination(const Vector3& dest_loc);

  /************* wait helpers ************/
  // helper function can wait for anything (as defined by the given function) up
  // to the max_wait duration (in seconds). returns true if the wait function
  // succeeded, or false if timeout occurred or the timeout is invalid.
  vehicle_apis::FunctionCaller RunFlightCommand(
      WaitFunction flight_controller_function, float flight_command_timeout_sec,
      int64_t command_start_time_nanos);

  // useful for derived class to check after takeoff
  bool WaitForZ(float timeout_sec, float z, float margin,
                int64_t command_start_time_nanos);

  /************* other short hands ************/
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

  CancelToken& GetCancelToken() { return token_; }
  Logger GetLogger() const { return sim_robot_.GetLogger(); }

 protected:  // types
  class SingleCall {
   public:
    SingleCall(MultirotorApiBase* api) : api_(api) {
      auto& token = api->GetCancelToken();

      // if we can't get lock, cancel previous call
      if (!token.Try_lock()) {
        // TODO: should we worry about spurious failures in try_lock?
        token.Cancel();
        token.Lock();
      }

      if (IsRootCall()) {
        token.Reset();
      }
      // else this is not the start of the call
    }

    virtual ~SingleCall() {
      auto& token = api_->GetCancelToken();

      if (IsRootCall()) {
        token.Reset();
      }
      // else this is not the end of the call

      token.Unlock();
    }

   protected:
    MultirotorApiBase* GetVehicleApi() { return api_; }

    bool IsRootCall() {
      return api_->GetCancelToken().GetRecursionCount() == 1;
    }

   private:
    MultirotorApiBase* api_;
  };

  class SingleTaskCall : public SingleCall {
   public:
    SingleTaskCall(MultirotorApiBase* api) : SingleCall(api) {
      if (IsRootCall()) {
        api->BeforeTask();
      }
    }

    virtual ~SingleTaskCall() {
      if (IsRootCall()) {
        GetVehicleApi()->AfterTask();
      }
    }
  };

  // use this lock for vehicle status APIs
  struct StatusLock {
    // this const correctness gymnastic is required because most
    // status update APIs are const
    StatusLock(const MultirotorApiBase* api)
        : lock_(*const_cast<std::recursive_mutex*>(&api->status_mutex_)) {}

   private:
    // we need mutable here because status APIs are const and shouldn't change
    // data members
    mutable std::lock_guard<std::recursive_mutex> lock_;
  };

 private:  // types
  struct PathPosition {
    unsigned int seg_index;
    float offset;
    Vector3 position;
  };

  struct PathSegment {
    Vector3 seg_normalized;
    Vector3 seg;
    float seg_length;
    float seg_velocity;
    float start_z;
    float seg_path_length;

    PathSegment(const Vector3& start, const Vector3& end, float velocity,
                float path_length) {
      seg = end - start;
      seg_length = seg.norm();
      seg_normalized = seg.normalized();
      start_z = start.z();
      seg_path_length = path_length;

      seg_velocity = velocity;
    }
  };

 protected:  // methods
  virtual void CreateTopics(void);
  virtual bool IsYawWithinMargin(float yaw_target, float margin) const;
  virtual void RegisterServiceMethods();
  virtual void RemoveTopics(void);
  void AdjustYaw(float x, float y, DrivetrainType drivetrain, bool& yaw_is_rate,
                 float& yaw);

 private:  // methods
  float SetNextPathPosition(const std::vector<Vector3>& path,
                            const std::vector<PathSegment>& path_segs,
                            const PathPosition& cur_path_loc, float next_dist,
                            PathPosition& next_path_loc);
  void AdjustYaw(const Vector3& heading, DrivetrainType drivetrain,
                 bool& yaw_is_rate, float& yaw);
  void MoveToPathPosition(const Vector3& dest, float velocity,
                          DrivetrainType drivetrain, bool yaw_is_rate,
                          float yaw, float last_z);

  void LoadSettings();

  //---------------------------------------------------------------------------
  // Service Method Wrappers

  bool TakeoffServiceMethod(float timeout_sec,
                            TimeNano _service_method_start_time);

  bool LandServiceMethod(float timeout_sec,
                         TimeNano _service_method_start_time);

  bool GoHomeServiceMethod(float timeout_sec, float velocity,
                           TimeNano _service_method_start_time);

  bool HoverServiceMethod(TimeNano _service_method_start_time);

  bool MoveByVelocityServiceMethod(float vx, float vy, float vz, float duration,
                                   DrivetrainType drivetrain, bool yaw_is_rate,
                                   float yaw,
                                   TimeNano _service_method_start_time);

  bool MoveByVelocityZServiceMethod(float vx, float vy, float vz,
                                    float duration, DrivetrainType drivetrain,
                                    bool yaw_is_rate, float yaw,
                                    TimeNano _service_method_start_time);

  bool MoveByVelocityBodyFrameServiceMethod(
      float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
      bool yaw_is_rate, float yaw, TimeNano _service_method_start_time);

  bool MoveByVelocityBodyFrameZServiceMethod(
      float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
      bool yaw_is_rate, float yaw, TimeNano _service_method_start_time);

  bool MoveOnPathServiceMethod(std::vector<std::vector<float>> path,
                               float velocity, float timeout_sec,
                               DrivetrainType drivetrain, bool yaw_is_rate,
                               float yaw, float lookahead,
                               float adaptive_lookahead,
                               TimeNano _service_method_start_time);

  bool MoveToPositionServiceMethod(float x, float y, float z, float velocity,
                                   float timeout_sec, DrivetrainType drivetrain,
                                   bool yaw_is_rate, float yaw, float lookahead,
                                   float adaptive_lookahead,
                                   TimeNano _service_method_start_time);

  bool MoveToZServiceMethod(float z, float velocity, float timeout_sec,
                            bool yaw_is_rate, float yaw, float lookahead,
                            float adaptive_lookahead,
                            TimeNano _service_method_start_time);

  bool RotateToYawServiceMethod(float yaw, float timeout_sec, float margin,
                                float yaw_rate,
                                TimeNano _service_method_start_time);

  bool RotateByYawRateServiceMethod(float yaw_rate, float duration,
                                    TimeNano _service_method_start_time);

  GeoPoint GetEstimatedGeoLocationServiceMethod();

 protected:
  Robot sim_robot_;  // The robot we control
  TransformTree*
      psim_transformtree_;     // The transform tree containing the robot
  std::vector<Topic> topics_;  // Topics to advertise or subscribe to

 private:  // variables
  // TODO: Do we need pub/sub to publish any controller data?

  // Switched to using service method request-response for all control commands,
  // but leaving the below pub-sub version commented out for reference in case
  // it's needed in the future.
  // Topic setpoint_NEDvelocity_yawrate_topic_;

  Topic current_waypoint_number_topic_;

  CancelToken token_;
  std::recursive_mutex status_mutex_;

  float obs_avoidance_vel_ = 0.5f;

  // TODO: make this configurable?
  float landing_vel_ = 0.2f;  // velocity to use for landing
  float approx_zero_vel_ = 0.05f;
  float approx_zero_angular_vel_ = 0.01f;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MULTIROTOR_API_BASE_HPP_
