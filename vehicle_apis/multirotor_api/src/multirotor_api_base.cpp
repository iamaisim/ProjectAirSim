// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "multirotor_api_base.hpp"

#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>

#include "core_sim/math_utils.hpp"
#include "core_sim/message/flight_control_setpoint_message.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "imultirotor_api.hpp"

namespace microsoft {
namespace projectairsim {

MultirotorApiBase::MultirotorApiBase(const Robot& robot,
                                     TransformTree* ptransformtree)
    : sim_robot_(robot), psim_transformtree_(ptransformtree) {}

void MultirotorApiBase::LoadParams(
    const std::unordered_map<std::string, float>& params_map) {
  if (params_map.find("MC_CP_VEL") != params_map.end())
    obs_avoidance_vel_ = params_map.at("MC_CP_VEL");

  if (params_map.find("MPC_LAND_SPEED") != params_map.end())
    landing_vel_ = params_map.at("MPC_LAND_SPEED");

  if (params_map.find("MC_APRX_ZERO_VEL") != params_map.end())
    approx_zero_vel_ = params_map.at("MC_APRX_ZERO_VEL");

  if (params_map.find("MC_APRX_ZERO_ANG_VEL") != params_map.end())
    approx_zero_angular_vel_ = params_map.at("MC_APRX_ZERO_ANG_VEL");
}

void MultirotorApiBase::BeginUpdate() {
  CreateTopics();
  RegisterServiceMethods();
}

void MultirotorApiBase::EndUpdate() {
  CancelLastTask();
  RemoveTopics();
}

void MultirotorApiBase::Reset() {}

void MultirotorApiBase::Update() {}

void MultirotorApiBase::CreateTopics() {
  // Switched to using service method request-response for all control commands,
  // but leaving the below pub-sub version commented out for reference in case
  // it's needed in the future.
  // setpoint_NEDvelocity_yawrate_topic_ = sim_robot_.CreateTopic(
  //     "multirotor_api/setpoint_NEDvelocity_yawrate", TopicType::kSubscribed,
  //     60, MessageType::kFlightControlSetpoint, [this](const Topic&
  //     topic, const Message& message) {
  //       OnSetpointNEDvelocityYawrate(topic, message);
  //     });
  // topics_.push_back(setpoint_NEDvelocity_yawrate_topic_);

  current_waypoint_number_topic_ = sim_robot_.CreateTopic(
      "multirotor_api/current_waypoint_number", TopicType::kPublished, 0,
      MessageType::kInt32, nullptr);

  topics_.push_back(current_waypoint_number_topic_);
}

void MultirotorApiBase::RemoveTopics() {
  for (const auto& topic : topics_) {
    sim_robot_.RemoveTopic(topic);
  }
}

void MultirotorApiBase::RegisterServiceMethods() {
  // TODO: is it useful to generalize this code.

  // Register EnableApiControl
  auto method = ServiceMethod("EnableApiControl", {""});
  auto method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::EnableApiControl, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register DisableApiControl
  method = ServiceMethod("DisableApiControl", {""});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::DisableApiControl, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register IsApiControlEnabled
  method = ServiceMethod("IsApiControlEnabled", {""});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::IsApiControlEnabled, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register Arm
  method = ServiceMethod("Arm", {"_service_method_start_time"});
  method_handler = method.CreateMethodHandler(&MultirotorApiBase::Arm, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register Disarm
  method = ServiceMethod("Disarm", {""});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::Disarm, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register CanArm
  method = ServiceMethod("CanArm", {""});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::CanArmServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register GetReadyState
  method = ServiceMethod("GetReadyState", {""});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::GetReadyStateServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register CancelLastTask
  method = ServiceMethod("CancelLastTask", {""});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::CancelLastTask, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // For methods below that need to track a timeout duration from the start time
  // of the command, the special parameter "_service_method_start_time" is
  // populated by ServiceManager with the simtime when it initiates the service
  // method request. This parameter is not exposed to the user API at the
  // client, so it is named with a prefix underscore to indicate it as internal.

  // Register Takeoff
  method =
      ServiceMethod("Takeoff", {"timeout_sec", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::TakeoffServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register Land
  method = ServiceMethod("Land", {"timeout_sec", "_service_method_start_time"});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::LandServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register GetEstimatedGeoLocation
  method = ServiceMethod("GetEstimatedGeoLocation", {""});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::GetEstimatedGeoLocationServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register LandedState
  method = ServiceMethod("GetLandedState", {""});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::GetLandedStateServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register GetEstimatedKinematics
  method = ServiceMethod("GetEstimatedKinematics", {""});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::GetKinematicsEstimatedServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register GoHome
  method = ServiceMethod(
      "GoHome", {"timeout_sec", "velocity", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::GoHomeServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register Hover
  method = ServiceMethod("Hover", {"_service_method_start_time"});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::HoverServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveByVelocity
  method = ServiceMethod("MoveByVelocity",
                         {"vx", "vy", "vz", "duration", "drivetrain",
                          "yaw_is_rate", "yaw", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveByVelocityServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveByVelocityZ
  method = ServiceMethod("MoveByVelocityZ",
                         {"vx", "vy", "z", "duration", "drivetrain",
                          "yaw_is_rate", "yaw", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveByVelocityZServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveByVelocity
  method = ServiceMethod("MoveByVelocityBodyFrame",
                         {"vx", "vy", "vz", "duration", "drivetrain",
                          "yaw_is_rate", "yaw", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveByVelocityBodyFrameServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveByVelocityZ
  method = ServiceMethod("MoveByVelocityBodyFrameZ",
                         {"vx", "vy", "z", "duration", "drivetrain",
                          "yaw_is_rate", "yaw", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveByVelocityBodyFrameZServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveToPosition
  method = ServiceMethod(
      "MoveToPosition",
      {"x", "y", "z", "velocity", "timeout_sec", "drivetrain", "yaw_is_rate",
       "yaw", "lookahead", "adaptive_lookahead", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveToPositionServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveToZ
  method = ServiceMethod(
      "MoveToZ",
      {"z", "velocity", "timeout_sec", "drivetrain", "yaw_is_rate", "yaw",
       "lookahead", "adaptive_lookahead", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveToZServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveOnPath
  method = ServiceMethod(
      "MoveOnPath",
      {"path", "velocity", "timeout_sec", "drivetrain", "yaw_is_rate", "yaw",
       "lookahead", "adaptive_lookahead", "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::MoveOnPathServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register RotateToYaw
  method =
      ServiceMethod("RotateToYaw", {"yaw", "timeout_sec", "margin", "yaw_rate",
                                    "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::RotateToYawServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register RotateByYawRate
  method = ServiceMethod("RotateByYawRate", {"yaw_rate", "duration",
                                             "_service_method_start_time"});
  method_handler = method.CreateMethodHandler(
      &MultirotorApiBase::RotateByYawRateServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register RequestControl
  method = ServiceMethod("RequestControl", {"_service_method_start_time"});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::RequestControl, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register SetMissionMode
  method = ServiceMethod("SetMissionMode", {"_service_method_start_time"});
  method_handler =
      method.CreateMethodHandler(&MultirotorApiBase::SetMissionMode, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

bool MultirotorApiBase::TakeoffServiceMethod(
    float timeout_sec, TimeNano _service_method_start_time) {
  return Takeoff(timeout_sec, _service_method_start_time);
}

bool MultirotorApiBase::Takeoff(float timeout_sec,
                                int64_t command_start_time_nanos) {
  // TODO Parameterize take off altitude?
  SingleTaskCall lock(this);

  auto kinematics = GetKinematicsEstimated();
  if (kinematics.twist.linear.norm() > approx_zero_vel_) {
    return false;
    // TODO: Should we introduce such exceptions in V2?
    /*throw VehicleMoveException(
        Utils::FormatMessage("Cannot perform takeoff because vehicle is already
       " "moving with velocity %f m/s", kinematics.twist.linear.norm()));*/
  }

  bool ret =
      MoveToPosition(kinematics.pose.position.x(), kinematics.pose.position.y(),
                     kinematics.pose.position.z() + GetTakeoffZ(), 0.5f,
                     timeout_sec, DrivetrainType::MaxDegreeOfFreedom, true, 0.0,
                     -1.0, 1.0, command_start_time_nanos);

  // last command is to hold on to position
  // CommandPosition(0, 0, GetTakeoffZ(), YawMode::Zero());

  return ret;
}

bool MultirotorApiBase::LandServiceMethod(float timeout_sec,
                                          TimeNano _service_method_start_time) {
  return Land(timeout_sec, _service_method_start_time);
}

const IController::GimbalState& MultirotorApiBase::GetGimbalSignal(
    const std::string& gimbal_id) {
  throw std::runtime_error(
      "This flight controller does not support following gimbal devices");
}

bool MultirotorApiBase::Land(float timeout_sec,
                             int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  // Start moving down
  MoveByVelocityInternal(0, 0, landing_vel_, true, 0.0);

  // Wait for vertical movement to start otherwise we can get a false detection
  // of zero Z-velocity and a false landing detection.  If we've already landed,
  // we won't move; if we're moving up, allow a little extra time to reverse
  // direction.
  {
    float vel_landing_started =
        (landing_vel_ > 1.0f) ? 0.1f : (landing_vel_ / 10.0f);

    (void)RunFlightCommand(
        [&]() { return (GetVelocity().z() > vel_landing_started); },
        (GetVelocity().z() < 0.5) ? 10 : 1, command_start_time_nanos);
  }

  // Wait until we stop moving vertically
  {
    static const float kSecNearZeroVelRequired =
        1.0f;  // Touchdown confirmed if near zero velocity for at least this
               // long (seconds)
    int near_zero_vel_count_required =
        (int)(kSecNearZeroVelRequired / GetCommandPeriod() +
              0.5f);  // Confirm touchdown if near zero velocity for this many
                      // counts
    int near_zero_vel_count =
        0;  // Count of consecutive times velocity is zero to detect if drone
            // has stopped moving and landed

    if (near_zero_vel_count_required < 1) near_zero_vel_count_required = 1;

    return RunFlightCommand(
               [&]() {
                 float z_vel = GetVelocity().z();
                 if (fabs(z_vel) <= approx_zero_vel_) {
                   ++near_zero_vel_count;
                 } else {
                   near_zero_vel_count = 0;
                 }
                 if (near_zero_vel_count > near_zero_vel_count_required) {
                   return true;  // Confirmed landing
                 } else {
                   MoveByVelocityInternal(0.0, 0.0, landing_vel_, true, 0.0);
                   return false;
                 }
               },
               timeout_sec, command_start_time_nanos)
        .IsComplete();
  }
}

bool MultirotorApiBase::GoHomeServiceMethod(
    float timeout_sec, float velocity, TimeNano _service_method_start_time) {
  return GoHome(timeout_sec, velocity, _service_method_start_time);
}

bool MultirotorApiBase::GoHome(float timeout_sec, float velocity,
                               int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  bool fRet = false;
  Pose poseHome;

  if (!psim_transformtree_->Convert(Pose::Zero(), sim_robot_.GetHomeRefFrame(),
                                    TransformTree::kRefFrameGlobal, &poseHome))
    throw std::logic_error(
        "Failed to map from vehicle home coordinates to global coordinates");
  else {
    auto& position = poseHome.position;
    auto radiansYawHome =
        poseHome.orientation.toRotationMatrix().eulerAngles(0, 1, 2).z();

    sim_robot_.GetLogger().LogTrace(
        GetControllerName(),
        "GoHome(): Moving to position (%f, %f, %f), %f degrees yaw",
        poseHome.position.x(), poseHome.position.y(), poseHome.position.z(),
        MathUtils::rad2Deg(radiansYawHome));

    fRet = RunFlightCommand(
               [=]() {
                 if (!MoveToPosition(position.x(), position.y(), position.z(),
                                     velocity, timeout_sec,
                                     DrivetrainType::MaxDegreeOfFreedom, true,
                                     0.0, -1.0, 1.0, command_start_time_nanos))
                   return (false);

                 return (RotateToYaw(radiansYawHome, timeout_sec, 0.05f, 0.0f,
                                     command_start_time_nanos));
               },
               timeout_sec, command_start_time_nanos)
               .IsComplete();
  }

  return (fRet);
}

bool MultirotorApiBase::HoverServiceMethod(
    TimeNano _service_method_start_time) {
  return Hover(_service_method_start_time);
}

bool MultirotorApiBase::Hover(int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  return MoveToZ(GetPosition().z(), 0.5f, MathUtils::Max<float>(), true, 0.0,
                 1.0f, false, command_start_time_nanos);
}

bool MultirotorApiBase::CancelLastTask() {
  try {
    token_.Cancel();
  } catch (...) {
    sim_robot_.GetLogger().LogError(GetControllerName(),
                                    "Error while cancelling last task.");
    return false;
  }
  return true;  // Have to return something to be able to expose ServiceMethod
}

bool MultirotorApiBase::RequestControl(int64_t /*command_start_time_nanos*/) {
  return false;
}

bool MultirotorApiBase::SetMissionMode(int64_t /*command_start_time_nanos*/) {
  return false;
}

bool MultirotorApiBase::MoveByMotorPWMs(float front_right_pwm,
                                        float rear_left_pwm,
                                        float front_left_pwm,
                                        float rear_right_pwm, float duration,
                                        int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               CommandMotorPWMs(front_right_pwm, rear_left_pwm, front_left_pwm,
                                rear_right_pwm);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByRollPitchYawZ(float roll, float pitch, float yaw,
                                            float z, float duration,
                                            int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               MoveByRollPitchYawZInternal(roll, pitch, yaw, z);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByRollPitchYawThrottle(
    float roll, float pitch, float yaw, float throttle, float duration,
    int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               MoveByRollPitchYawThrottleInternal(roll, pitch, yaw, throttle);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByRollPitchYawrateThrottle(
    float roll, float pitch, float yaw_rate, float throttle, float duration,
    int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               MoveByRollPitchYawrateThrottleInternal(roll, pitch, yaw_rate,
                                                      throttle);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByRollPitchYawrateZ(
    float roll, float pitch, float yaw_rate, float z, float duration,
    int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               MoveByRollPitchYawrateZInternal(roll, pitch, yaw_rate, z);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByAngleRatesZ(float roll_rate, float pitch_rate,
                                          float yaw_rate, float z,
                                          float duration,
                                          int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               MoveByAngleRatesZInternal(roll_rate, pitch_rate, yaw_rate, z);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByAngleRatesThrottle(
    float roll_rate, float pitch_rate, float yaw_rate, float throttle,
    float duration, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               MoveByAngleRatesThrottleInternal(roll_rate, pitch_rate, yaw_rate,
                                                throttle);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByVelocityServiceMethod(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, TimeNano _service_method_start_time) {
  return MoveByVelocity(vx, vy, vz, duration, drivetrain, yaw_is_rate, yaw,
                        _service_method_start_time);
}

bool MultirotorApiBase::MoveByVelocity(float vx, float vy, float vz,
                                       float duration,
                                       DrivetrainType drivetrain,
                                       bool yaw_is_rate, float yaw,
                                       int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  AdjustYaw(vx, vy, drivetrain, yaw_is_rate, yaw);

  return RunFlightCommand(
             [&]() {
               MoveByVelocityInternal(vx, vy, vz, yaw_is_rate, yaw);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByVelocityBodyFrameServiceMethod(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, TimeNano _service_method_start_time) {
  return MoveByVelocityBodyFrame(vx, vy, vz, duration, drivetrain, yaw_is_rate,
                                 yaw, _service_method_start_time);
}

bool MultirotorApiBase::MoveByVelocityBodyFrame(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  float vehicle_yaw =
      PhysicsUtils::GetYaw<float>(GetKinematicsEstimated().pose.orientation);
  float vx_adjusted =
      (vx * std::cosf(vehicle_yaw)) - (vy * std::sinf(vehicle_yaw));
  float vy_adjusted =
      (vx * std::sinf(vehicle_yaw)) + (vy * std::cosf(vehicle_yaw));

  if (duration <= 0) {
    return true;
  }

  if (!yaw_is_rate) {
    // adjust yaw to be absolute
    yaw += vehicle_yaw;
  }

  AdjustYaw(vx_adjusted, vy_adjusted, drivetrain, yaw_is_rate, yaw);

  return RunFlightCommand(
             [&]() {
               MoveByVelocityInternal(vx_adjusted, vy_adjusted, vz, yaw_is_rate,
                                      yaw);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByVelocityZServiceMethod(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, TimeNano _service_method_start_time) {
  return MoveByVelocityZ(vx, vy, vz, duration, drivetrain, yaw_is_rate, yaw,
                         _service_method_start_time);
}

bool MultirotorApiBase::MoveByVelocityZ(float vx, float vy, float z,
                                        float duration,
                                        DrivetrainType drivetrain,
                                        bool yaw_is_rate, float yaw,
                                        int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return false;
  }

  AdjustYaw(vx, vy, drivetrain, yaw_is_rate, yaw);

  return RunFlightCommand(
             [&]() {
               MoveByVelocityZInternal(vx, vy, z, yaw_is_rate, yaw);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveByVelocityBodyFrameZServiceMethod(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, TimeNano _service_method_start_time) {
  return MoveByVelocityBodyFrameZ(vx, vy, vz, duration, drivetrain, yaw_is_rate,
                                  yaw, _service_method_start_time);
}

bool MultirotorApiBase::MoveByVelocityBodyFrameZ(
    float vx, float vy, float z, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  float vehicle_yaw =
      PhysicsUtils::GetYaw<float>(GetKinematicsEstimated().pose.orientation);
  float vx_adjusted =
      (vx * std::cosf(vehicle_yaw)) - (vy * std::sinf(vehicle_yaw));
  float vy_adjusted =
      (vx * std::sinf(vehicle_yaw)) + (vy * std::cosf(vehicle_yaw));

  if (duration <= 0) {
    return false;
  }
  if (!yaw_is_rate) {
    // adjust yaw to be absolute
    yaw += vehicle_yaw;
  }

  AdjustYaw(vx_adjusted, vy_adjusted, drivetrain, yaw_is_rate, yaw);

  return RunFlightCommand(
             [&]() {
               MoveByVelocityZInternal(vx_adjusted, vy_adjusted, z, yaw_is_rate,
                                       yaw);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool MultirotorApiBase::MoveOnPathServiceMethod(
    std::vector<std::vector<float>> path, float velocity, float timeout_sec,
    DrivetrainType drivetrain, bool yaw_is_rate, float yaw, float lookahead,
    float adaptive_lookahead, TimeNano _service_method_start_time) {
  return MoveOnPath(path, velocity, timeout_sec, drivetrain, yaw_is_rate, yaw,
                    lookahead, adaptive_lookahead, _service_method_start_time);
}

bool MultirotorApiBase::MoveOnPath(std::vector<std::vector<float>> path,
                                   float velocity, float timeout_sec,
                                   DrivetrainType drivetrain, bool yaw_is_rate,
                                   float yaw, float lookahead,
                                   float adaptive_lookahead,
                                   int64_t command_start_time_nanos) {
  std::vector<Vector3> internal_path;
  for (unsigned int i = 0; i < path.size(); ++i) {
    std::vector<float> point = path.at(i);
    internal_path.push_back(Vector3(point[0], point[1], point[2]));
  }

  return MoveOnPathInternal(internal_path, velocity, timeout_sec, drivetrain,
                            yaw_is_rate, yaw, lookahead, adaptive_lookahead,
                            command_start_time_nanos);
}

bool MultirotorApiBase::MoveOnPathInternal(
    std::vector<Vector3> path, float velocity, float timeout_sec,
    DrivetrainType drivetrain, bool yaw_is_rate, float yaw, float lookahead,
    float adaptive_lookahead, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  // validate path size
  if (path.size() == 0) {
    sim_robot_.GetLogger().LogWarning(
        GetControllerName(),
        "MoveOnPath terminated because path has no points");
    return true;
  }

  // validate yaw mode
  if (drivetrain == DrivetrainType::ForwardOnly && yaw_is_rate) {
    throw std::invalid_argument(
        "Yaw cannot be specified as rate if drivetrain is ForwardOnly");
  }

  // validate and set auto-lookahead value
  float command_period_dist = velocity * GetCommandPeriod();
  if (lookahead == 0) {
    throw std::invalid_argument(
        "lookahead distance cannot be 0");  // won't allow progress on path
  } else if (lookahead > 0) {
    if (command_period_dist > lookahead)
      throw std::invalid_argument(sim_robot_.GetLogger().FormatMessage(
          "lookahead value %f is too small for velocity %f. It "
          "must be at least %f",
          lookahead, velocity, command_period_dist));
    if (GetDistanceAccuracy() > lookahead) {
      throw std::invalid_argument(sim_robot_.GetLogger().FormatMessage(
          "lookahead value %f is smaller than drone's distance accuracy %f.",
          lookahead, GetDistanceAccuracy()));
    }
  } else {
    // if auto mode requested for lookahead then calculate based on velocity
    lookahead = GetAutoLookahead(velocity, adaptive_lookahead);
  }

  // add current position as starting point
  std::vector<Vector3> path3d;
  std::vector<PathSegment> path_segs;
  path3d.push_back(GetKinematicsEstimated().pose.position);

  Vector3 point;
  float path_length = 0;
  // append the input path and compute segments
  for (unsigned int i = 0; i < path.size(); ++i) {
    point = path.at(i);
    PathSegment path_seg(path3d.at(i), point, velocity, path_length);
    path_length += path_seg.seg_length;
    path_segs.push_back(path_seg);
    path3d.push_back(point);
  }
  // add last segment as zero length segment so we have equal number of segments
  // and points. path_segs[i] refers to segment that starts at point i
  path_segs.push_back(PathSegment(point, point, velocity, path_length));

  // when path ends, we want to slow down
  float braking_dist = 0;
  if (velocity > GetMultirotorApiParams().braking_vel) {
    braking_dist =
        MathUtils::Clip(velocity * GetMultirotorApiParams().vel_to_braking_dist,
                        GetMultirotorApiParams().min_braking_dist,
                        GetMultirotorApiParams().max_braking_dist);
  }
  // else no need to change velocities for last segments

  // setup current position on path to 0 offset
  PathPosition cur_path_loc, next_path_loc;
  cur_path_loc.seg_index = 0;
  cur_path_loc.offset = 0;
  cur_path_loc.position = path3d[0];

  float lookahead_error_increasing = 0;
  float lookahead_error = 0;
  vehicle_apis::FunctionCaller function_caller(GetCommandPeriod(), timeout_sec,
                                               GetCancelToken(),
                                               command_start_time_nanos);

  // initialize next path position
  sim_robot_.PublishTopic(current_waypoint_number_topic_, Int32Message(0));
  if (SetNextPathPosition(path3d, path_segs, cur_path_loc,
                          lookahead + lookahead_error, next_path_loc) > 0) {
    return true;  // Already at or past end of path
  }

  float overshoot = 0;
  float goal_dist = 0;
  constexpr float kGoalDistTolerance = 0.01;  // meters

  // until we are at the end of the path (last seg is always zero size)
  while (
      !function_caller.IsTimeout() &&
      (next_path_loc.seg_index < path_segs.size() - 1 ||
       goal_dist > kGoalDistTolerance)) {  // current position is approximately
                                           // at the last end point

    float seg_velocity = path_segs.at(next_path_loc.seg_index).seg_velocity;
    float path_length_remaining =
        path_length - path_segs.at(cur_path_loc.seg_index).seg_path_length -
        cur_path_loc.offset;
    if (seg_velocity > GetMultirotorApiParams().min_vel_for_braking &&
        path_length_remaining <= braking_dist) {
      seg_velocity = GetMultirotorApiParams().braking_vel;
    }

    // send drone command to get to next lookahead
    MoveToPathPosition(next_path_loc.position, seg_velocity, drivetrain,
                       yaw_is_rate, yaw,
                       path_segs.at(cur_path_loc.seg_index).start_z);

    // sleep for rest of the cycle
    if (!function_caller.WaitForInterval()) {
      return false;
    }

    /*  Below, P is previous position on path, N is next goal and C is our
    current position.

    N
    ^
    |
    |
    |
    C'|---C
    |  /
    | /
    |/
    P

    Note that PC could be at any angle relative to PN, including 0 or -ve. We
    increase lookahead distance by the amount of |PC|. For this, we project PC
    on to PN to get vector PC' and length of CC'is our adaptive lookahead error
    by which we will increase lookahead distance.

    For next iteration, we first update our current position by goal_dist and
    then set next goal by the amount lookahead + lookahead_error.

    We need to take care of following cases:

    1. |PN| == 0 => lookahead_error = |PC|, goal_dist = 0
    2. |PC| == 0 => lookahead_error = 0, goal_dist = 0
    3. PC in opposite direction => lookahead_error = |PC|, goal_dist = 0

    One good test case is if C just keeps moving perpendicular to the path
    (instead of along the path). In that case, we expect next goal to come up
    and down by the amount of lookahead_error. However under no circumstances we
    should go back on the path (i.e. current pos on path can only move forward).
    */

    // how much have we moved towards last goal?
    const Vector3& goal_vect = next_path_loc.position - cur_path_loc.position;

    if (!goal_vect.isZero()) {  // goal can only be zero if we are at the end
                                // of path
      const Vector3& actual_vect = GetPosition() - cur_path_loc.position;

      // project actual vector on goal vector
      const Vector3& goal_normalized = goal_vect.normalized();
      goal_dist =
          actual_vect.dot(goal_normalized);  // dist could be -ve if drone
                                             // moves away from goal

      // if adaptive lookahead is enabled the calculate lookahead error (see
      // above fig)
      if (adaptive_lookahead) {
        const Vector3& actual_on_goal = goal_normalized * goal_dist;
        float error =
            (actual_vect - actual_on_goal).norm() * adaptive_lookahead;
        if (error > lookahead_error) {
          lookahead_error_increasing++;
          // TODO: below should be lower than 1E3 and configurable
          // but lower values like 100 doesn't work for simple_flight +
          // ScalableClock
          if (lookahead_error_increasing > 1E5) {
            throw std::runtime_error(
                "lookahead error is continually increasing so we do not have "
                "safe control, aborting moveOnPath operation");
          }
        } else {
          lookahead_error_increasing = 0;
        }
        lookahead_error = error;
      }
    } else {
      lookahead_error_increasing = 0;
      goal_dist = 0;
      lookahead_error = 0;  // this is not really required because we will exit
      function_caller.Complete();
    }

    // log("PF: cur=%s, goal_dist=%f, cur_path_loc=%s,
    // next_path_loc=%s, lookahead_error=%f",
    //     VectorMath::toString(getPosition()).c_str(), goal_dist,
    //     VectorMath::toString(cur_path_loc.position).c_str(),
    //     VectorMath::toString(next_path_loc.position).c_str(),
    //     lookahead_error);

    // if drone moved backward, we don't want goal to move backward as well
    // so only climb forward on the path, never back. Also note >= which means
    // we climb path even if distance was 0 to take care of duplicated points on
    // path
    if (goal_dist >= 0) {
      overshoot = SetNextPathPosition(path3d, path_segs, cur_path_loc,
                                      goal_dist, cur_path_loc);
      if (overshoot) {
        sim_robot_.GetLogger().LogTrace(GetControllerName(), "overshoot=%f",
                                        overshoot);
      }
    }

    // compute next target on path
    overshoot = SetNextPathPosition(path3d, path_segs, cur_path_loc,
                                    lookahead + lookahead_error, next_path_loc);
  }

  return function_caller.IsComplete();
}

bool MultirotorApiBase::MoveToPositionServiceMethod(
    float x, float y, float z, float velocity, float timeout_sec,
    DrivetrainType drivetrain, bool yaw_is_rate, float yaw, float lookahead,
    float adaptive_lookahead, TimeNano _service_method_start_time) {
  return MoveToPosition(x, y, z, velocity, timeout_sec, drivetrain, yaw_is_rate,
                        yaw, lookahead, adaptive_lookahead,
                        _service_method_start_time);
}

bool MultirotorApiBase::MoveToPosition(
    float x, float y, float z, float velocity, float timeout_sec,
    DrivetrainType drivetrain, bool yaw_is_rate, float yaw, float lookahead,
    float adaptive_lookahead, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  std::vector<Vector3> path{Vector3(x, y, z)};
  return MoveOnPathInternal(path, velocity, timeout_sec, drivetrain,
                            yaw_is_rate, yaw, lookahead, adaptive_lookahead,
                            command_start_time_nanos);
}

bool MultirotorApiBase::MoveToZServiceMethod(
    float z, float velocity, float timeout_sec, bool yaw_is_rate, float yaw,
    float lookahead, float adaptive_lookahead,
    TimeNano _service_method_start_time) {
  return MoveToZ(z, velocity, timeout_sec, yaw_is_rate, yaw, lookahead,
                 adaptive_lookahead, _service_method_start_time);
}

bool MultirotorApiBase::MoveToZ(float z, float velocity, float timeout_sec,
                                bool yaw_is_rate, float yaw, float lookahead,
                                float adaptive_lookahead,
                                int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  std::vector<Vector3> path{Vector3(GetPosition().x(), GetPosition().y(), z)};
  return MoveOnPathInternal(path, velocity, timeout_sec,
                            DrivetrainType::MaxDegreeOfFreedom, yaw_is_rate,
                            yaw, lookahead, adaptive_lookahead,
                            command_start_time_nanos);
}

bool MultirotorApiBase::RotateToYawServiceMethod(
    float yaw, float timeout_sec, float margin, float yaw_rate,
    TimeNano _service_method_start_time) {
  return RotateToYaw(yaw, timeout_sec, margin, yaw_rate,
                     _service_method_start_time);
}

bool MultirotorApiBase::RotateToYaw(float yaw, float timeout_sec, float margin,
                                    float yaw_rate,
                                    int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (timeout_sec <= 0) {
    return true;
  }

  return RunFlightCommand(
             [&]() {
               auto start_pos = GetPosition();
               float yaw_target = MathUtils::NormalizeAngle<float>(yaw);

               static const float kPI = (float)M_PI;
               static const float k2PI = (float)(2.0 * M_PI);
               static const float kPIHalf = (float)(M_PI / 2.0);

               // Yaw to the heading at the specified rate
               if ((yaw_rate > 0) && !IsYawWithinMargin(yaw, margin)) {
                 float yaw_current = GetAngles()[2];
                 float dyaw = yaw - yaw_current;

                 // Pick the most direct rotation direction
                 if (dyaw > kPI)
                   dyaw -= k2PI;
                 else if (dyaw < -kPI)
                   dyaw += k2PI;
                 if (dyaw < 0) yaw_rate = -yaw_rate;

                 if (RunFlightCommand(
                         [&]() {
                           if (IsYawWithinMargin(yaw, margin))
                             return (true);
                           else {
                             float dyaw = yaw - GetAngles()[2];

                             if (dyaw > kPI)
                               dyaw -= k2PI;
                             else if (dyaw < -kPI)
                               dyaw += k2PI;
                             if (fabs(dyaw) < kPIHalf) {
                               if ((dyaw > 0) != (yaw_rate > 0))
                                 return (
                                     true);  // Direction of yaw to target is
                                             // opposite of yaw rate--we've
                                             // turned past the target heading
                             }

                             MoveToPositionInternal(start_pos, true, yaw_rate);
                           }

                           return false;
                         },
                         timeout_sec, command_start_time_nanos)
                         .IsTimeout()) {
                   return false;  // Let outer RunFlightCommand timeout
                 }
               }

               // Yaw to target at unlimited rate
               if (RunFlightCommand(
                       [&]() {
                         if (IsYawWithinMargin(yaw_target, margin)) {
                           //  sim_robot_.GetLogger().LogTrace(
                           //      GetControllerName(),
                           //      "Yaw is within margin, trying to stop
                           //      rotation");

                           // Set yaw rate=0 to try to stop rotation
                           MoveToPositionInternal(start_pos, true, 0);
                           auto yaw_rate =
                               GetKinematicsEstimated().twist.angular.z();

                           if (abs(yaw_rate) <= approx_zero_angular_vel_) {
                             return true;  // achieved stable yaw at target
                           } else {
                             //  sim_robot_.GetLogger().LogTrace(
                             //      GetControllerName(),
                             //      "Yaw_rate hasn't slowed down enough yet");
                           }
                         } else {
                           // Yaw is not within margin, keep rotating toward yaw
                           // target
                           MoveToPositionInternal(start_pos, false, yaw_target);
                           //  sim_robot_.GetLogger().LogTrace(
                           //      GetControllerName(),
                           //      "Yaw (%f) is not within margin yet",
                           //      PhysicsUtils::GetYaw<float>(GetOrientation()));
                         }

                         return false;  // keep moving until timeout
                       },
                       timeout_sec, command_start_time_nanos)
                       .IsComplete()) {
                 return true;  // acheived stable yaw at target, we're done
               }

               return false;  // Let outer RunFlightCommand timeout
             },
             timeout_sec, command_start_time_nanos)
      .IsComplete();
}

bool MultirotorApiBase::RotateByYawRateServiceMethod(
    float yaw_rate, float duration, TimeNano _service_method_start_time) {
  return RotateByYawRate(yaw_rate, duration, _service_method_start_time);
}

bool MultirotorApiBase::RotateByYawRate(float yaw_rate, float duration,
                                        int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  auto start_pos = GetPosition();

  return RunFlightCommand(
             [&]() {
               MoveToPositionInternal(start_pos, true, yaw_rate);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

GeoPoint MultirotorApiBase::GetEstimatedGeoLocationServiceMethod() {
  return GetGpsLocationEstimated();
}

bool MultirotorApiBase::CanArmServiceMethod() { return CanArm(); }

ReadyStateMessage MultirotorApiBase::GetReadyStateServiceMethod() {
  ReadyState state = GetReadyState();
  return ReadyStateMessage(SimClock::Get()->NowSimNanos(), state.ReadyVal,
                           state.ReadyMessage);
}

KinematicsMessage MultirotorApiBase::GetKinematicsEstimatedServiceMethod() {
  KinematicsMessage kin_msg(SimClock::Get()->NowSimNanos(),
                            GetKinematicsEstimated());
  return kin_msg;
}

LandedState MultirotorApiBase::GetLandedStateServiceMethod() {
  return GetLandedState();
}

void MultirotorApiBase::SetAngleLevelControllerGains(
    const std::vector<float>& kp, const std::vector<float>& ki,
    const std::vector<float>& kd) {
  uint8_t controller_type = 2;
  SetControllerGains(controller_type, kp, ki, kd);
}

void MultirotorApiBase::SetAngleRateControllerGains(
    const std::vector<float>& kp, const std::vector<float>& ki,
    const std::vector<float>& kd) {
  uint8_t controller_type = 3;
  SetControllerGains(controller_type, kp, ki, kd);
}

void MultirotorApiBase::SetVelocityControllerGains(
    const std::vector<float>& kp, const std::vector<float>& ki,
    const std::vector<float>& kd) {
  uint8_t controller_type = 4;
  SetControllerGains(controller_type, kp, ki, kd);
}

void MultirotorApiBase::SetPositionControllerGains(
    const std::vector<float>& kp, const std::vector<float>& ki,
    const std::vector<float>& kd) {
  uint8_t controller_type = 5;
  SetControllerGains(controller_type, kp, ki, kd);
}

void MultirotorApiBase::MoveByVelocityInternal(float vx, float vy, float vz,
                                               bool yaw_is_rate, float yaw) {
  if (SafetyCheckVelocity(Vector3(vx, vy, vz))) {
    CommandVelocity(vx, vy, vz, yaw_is_rate, yaw);
  }
}

void MultirotorApiBase::MoveByVelocityZInternal(float vx, float vy, float z,
                                                bool yaw_is_rate, float yaw) {
  if (SafetyCheckVelocityZ(vx, vy, z)) {
    CommandVelocityZ(vx, vy, z, yaw_is_rate, yaw);
  }
}

void MultirotorApiBase::MoveByVelocityZBodyInternal(float vx, float vy, float z,
                                                    bool yaw_is_rate,
                                                    float yaw_world) {
  if (SafetyCheckVelocityZ(vx, vy, z)) {
    CommandVelocityZBody(vx, vy, z, yaw_is_rate, yaw_world);
  }
}

void MultirotorApiBase::MoveByVelocityBodyInternal(float vx, float vy, float vz,
                                                   bool yaw_is_rate,
                                                   float yaw_world) {
  if (SafetyCheckVelocity(Vector3(vx, vy, vz))) {
    CommandVelocityBody(vx, vy, vz, yaw_is_rate, yaw_world);
  }
}

void MultirotorApiBase::MoveToPositionInternal(const Vector3& dest,
                                               bool yaw_is_rate, float yaw) {
  if (SafetyCheckDestination(dest)) {
    CommandPosition(dest.x(), dest.y(), dest.z(), yaw_is_rate, yaw);
  }
}

void MultirotorApiBase::MoveByRollPitchYawZInternal(float roll, float pitch,
                                                    float yaw, float z) {
  if (SafetyCheckVelocity(GetVelocity())) {
    CommandRollPitchYawZ(roll, pitch, yaw, z);
  }
}

void MultirotorApiBase::MoveByRollPitchYawThrottleInternal(float roll,
                                                           float pitch,
                                                           float yaw,
                                                           float throttle) {
  if (SafetyCheckVelocity(GetVelocity())) {
    CommandRollPitchYawThrottle(roll, pitch, yaw, throttle);
  }
}

void MultirotorApiBase::MoveByRollPitchYawrateThrottleInternal(float roll,
                                                               float pitch,
                                                               float yaw_rate,
                                                               float throttle) {
  if (SafetyCheckVelocity(GetVelocity())) {
    CommandRollPitchYawrateThrottle(roll, pitch, yaw_rate, throttle);
  }
}

void MultirotorApiBase::MoveByRollPitchYawrateZInternal(float roll, float pitch,
                                                        float yaw_rate,
                                                        float z) {
  if (SafetyCheckVelocity(GetVelocity())) {
    CommandRollPitchYawrateZ(roll, pitch, yaw_rate, z);
  }
}

void MultirotorApiBase::MoveByAngleRatesZInternal(float roll_rate,
                                                  float pitch_rate,
                                                  float yaw_rate, float z) {
  if (SafetyCheckVelocity(GetVelocity())) {
    CommandAngleRatesZ(roll_rate, pitch_rate, yaw_rate, z);
  }
}

void MultirotorApiBase::MoveByAngleRatesThrottleInternal(float roll_rate,
                                                         float pitch_rate,
                                                         float yaw_rate,
                                                         float throttle) {
  if (SafetyCheckVelocity(GetVelocity())) {
    CommandAngleRatesThrottle(roll_rate, pitch_rate, yaw_rate, throttle);
  }
}

// executes a given function until it returns true. Each execution is spaced
// apart at command period. return value is true if exit was due to given
// function returning true, otherwise false (due to timeout)
vehicle_apis::FunctionCaller MultirotorApiBase::RunFlightCommand(
    WaitFunction flight_controller_function, float flight_command_timeout_sec,
    int64_t command_start_time_nanos) {
  vehicle_apis::FunctionCaller function_caller(
      GetCommandPeriod(), flight_command_timeout_sec, GetCancelToken(),
      command_start_time_nanos);

  if (flight_command_timeout_sec <= 0) {
    return function_caller;
  }

  do {
    if (flight_controller_function()) {
      function_caller.Complete();
      break;
    }
  } while (function_caller.WaitForInterval());
  return function_caller;
}

bool MultirotorApiBase::WaitForZ(float timeout_sec, float z, float margin,
                                 int64_t command_start_time_nanos) {
  float cur_z = 100000;
  return RunFlightCommand(
             [&]() {
               cur_z = GetPosition().z();
               return (std::abs(cur_z - z) <= margin);
             },
             timeout_sec, command_start_time_nanos)
      .IsComplete();
}

void MultirotorApiBase::MoveToPathPosition(const Vector3& dest, float velocity,
                                           DrivetrainType drivetrain,
                                           bool yaw_is_rate, float yaw,
                                           float last_z) {
  // validate dest
  if (dest.hasNaN()) {
    throw std::invalid_argument(sim_robot_.GetLogger().FormatMessage(
        "%s[%f, %f, %f]", "dest vector cannot have NaN: ", dest[0], dest[1],
        dest[2]));
  }

  // what is the distance we will travel at this velocity?
  float expected_dist = velocity * GetCommandPeriod();

  // get velocity vector
  const Vector3 cur = GetPosition();
  const Vector3 cur_dest = dest - cur;
  float cur_dest_norm = cur_dest.norm();

  // yaw for the direction of travel
  AdjustYaw(cur_dest, drivetrain, yaw_is_rate, yaw);

  // find velocity vector
  Vector3 velocity_vect;
  if (cur_dest_norm <
      GetDistanceAccuracy())  // our dest is approximately same as current
  {
    velocity_vect = Vector3::Zero();
  } else if (cur_dest_norm >= expected_dist) {
    velocity_vect = (cur_dest / cur_dest_norm) * velocity;
  } else {  // cur dest is too close than the distance we would travel
            // generate velocity vector that is same size as cur_dest_norm /
            // command period this velocity vect when executed for command
            // period would yield cur_dest_norm
    sim_robot_.GetLogger().LogTrace(
        GetControllerName(),
        "Too close dest: cur_dest_norm=%f, expected_dist=%f", cur_dest_norm,
        expected_dist);
    velocity_vect =
        (cur_dest / cur_dest_norm) * (cur_dest_norm / GetCommandPeriod());
  }

  // send commands
  // try to maintain altitude if path was in XY plan only, velocity based
  // control is not as good
  if (std::abs(cur.z() - dest.z()) <=
      GetDistanceAccuracy())  // for paths in XY plan current code leaves z
                              // untouched, so we can compare with strict
                              // equality
  {
    // When close to target Z altitude, use MoveByVelocityInternal() with zero
    // vertical speed target instead of MoveByVelocityInternal() with the
    // absolute target destination altitude to avoid oscillations at high
    // altitude (this is the same patch as implemented in v1, but doesn't
    // explain why the effect is related to high altitude so it may just be a
    // workaround).
    MoveByVelocityInternal(velocity_vect.x(), velocity_vect.y(), 0, yaw_is_rate,
                           yaw);
    // MoveByVelocityZInternal(velocity_vect.x(), velocity_vect.y(), dest.z(),
    //                         yaw_is_rate, yaw);
    // TODO Investigate the root cause of the original oscillation when using
    // MoveByVelocityZInternal() in case there is some other issue with our
    // controller/environment/physics calculations at high altitudes.
  } else {
    MoveByVelocityInternal(velocity_vect.x(), velocity_vect.y(),
                           velocity_vect.z(), yaw_is_rate, yaw);
  }
}

bool MultirotorApiBase::SafetyCheckVelocity(const Vector3& velocity) {
  return true;
}
bool MultirotorApiBase::SafetyCheckVelocityZ(float vx, float vy, float z) {
  return true;
}
bool MultirotorApiBase::SafetyCheckDestination(const Vector3& dest_pos) {
  return true;
}

float MultirotorApiBase::SetNextPathPosition(
    const std::vector<Vector3>& path, const std::vector<PathSegment>& path_segs,
    const PathPosition& cur_path_loc, float next_dist,
    PathPosition& next_path_loc) {
  // note: cur_path_loc and next_path_loc may both point to same object
  unsigned int i = cur_path_loc.seg_index;
  float offset = cur_path_loc.offset;
  bool path_has_mutiple_segments = false;
  while (i < path.size() - 1) {
    const PathSegment& seg = path_segs.at(i);
    if (seg.seg_length > 0 &&  // protect against duplicate points in path,
                               // normalized seg will have NaN
        seg.seg_length >= next_dist + offset) {
      next_path_loc.seg_index = i;
      next_path_loc.offset =
          next_dist +
          offset;  // how much total distance we will travel on this segment
      next_path_loc.position =
          path.at(i) + seg.seg_normalized * next_path_loc.offset;
      return 0;
    }
    // otherwise use up this segment, move on to next one
    next_dist -= seg.seg_length - offset;
    offset = 0;

    if (&cur_path_loc == &next_path_loc) {
      sim_robot_.GetLogger().LogTrace(
          GetControllerName(), "segment %d done: x=%f, y=%f, z=%f", i,
          path.at(i).x(), path.at(i).y(), path.at(i).z());

      Int32Message msg = Int32Message(cur_path_loc.seg_index + 1);
      sim_robot_.PublishTopic(current_waypoint_number_topic_, msg);
      path_has_mutiple_segments = true;
    }

    ++i;
  }

  if (path_has_mutiple_segments) {
    sim_robot_.PublishTopic(current_waypoint_number_topic_, Int32Message(-1));
    path_has_mutiple_segments = false;
  }

  // if we are here then we ran out of segments
  // consider last segment as zero length segment
  next_path_loc.seg_index = i;
  next_path_loc.offset = 0;
  next_path_loc.position = path.at(i);
  return next_dist;
}

void MultirotorApiBase::AdjustYaw(const Vector3& heading,
                                  DrivetrainType drivetrain, bool& yaw_is_rate,
                                  float& yaw) {
  // adjust yaw for the direction of travel in forward-only mode
  if (drivetrain == DrivetrainType::ForwardOnly && !yaw_is_rate) {
    if (heading.norm() > GetDistanceAccuracy()) {
      yaw += std::atan2(heading.y(), heading.x());
      yaw = MathUtils::NormalizeAngle<float>(yaw);
    } else {
      yaw_is_rate = true;  // don't change existing yaw if heading is too
      yaw = 0.0;           // small because that can generate random result
    }
  }
  // else no adjustment needed
}

void MultirotorApiBase::AdjustYaw(float x, float y, DrivetrainType drivetrain,
                                  bool& yaw_is_rate, float& yaw) {
  AdjustYaw(Vector3(x, y, 0), drivetrain, yaw_is_rate, yaw);
}

bool MultirotorApiBase::IsYawWithinMargin(float yaw_target,
                                          float margin) const {
  const float yaw_current = GetAngles()[2];
  return std::abs(yaw_current - yaw_target) <= margin;
}

float MultirotorApiBase::GetAutoLookahead(float velocity,
                                          float adaptive_lookahead,
                                          float max_factor,
                                          float min_factor) const {
  // if auto mode requested for lookahead then calculate based on velocity
  float command_period_dist = velocity * GetCommandPeriod();
  float lookahead =
      command_period_dist * (adaptive_lookahead > 0 ? min_factor : max_factor);
  lookahead = std::max(lookahead, GetDistanceAccuracy() *
                                      1.5f);  // 50% more than distance accuracy
  return lookahead;
}

float MultirotorApiBase::GetObsAvoidanceVelocity(
    float risk_dist, float max_obs_avoidance_vel) const {
  return max_obs_avoidance_vel;
}

}  // namespace projectairsim
}  // namespace microsoft
