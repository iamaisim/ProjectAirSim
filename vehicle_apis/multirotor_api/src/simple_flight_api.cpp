// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "simple_flight_api.hpp"

#include <LVMon/lvmon.h>
#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include <mutex>

#include "core_sim/message/flight_control_rc_input_message.hpp"
#include "core_sim/message/flight_control_setpoint_message.hpp"
#include "core_sim/transforms/transform_tree.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class SimpleFlightApi

SimpleFlightApi::SimpleFlightApi(const Robot& robot,
                                 TransformTree* ptransformtree)
    : VTOLFWApiBase(robot, ptransformtree) {
  LoadSettings(robot);
}

void SimpleFlightApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  const std::string airframe_setup =
      controller_settings_json.value("airframe-setup", "");

  if (airframe_setup == "quadrotor-x") {
    num_motors_ = 4;
    vehicle_kind_ = VehicleKind::Multirotor;
  } else if (airframe_setup == "hexarotor-x") {
    num_motors_ = 6;
    vehicle_kind_ = VehicleKind::Multirotor;
  } else if (airframe_setup == "vtol-quad-x-tailsitter") {
    num_motors_ = 7;
    vehicle_kind_ = VehicleKind::VTOLTailsitter;
  } else if (airframe_setup == "vtol-quad-tiltrotor") {
    num_motors_ = 12;
    vehicle_kind_ = VehicleKind::VTOLTiltrotor;
  }

  // GetJsonObject
  const json& simple_flight_api_settings_json =
      controller_settings_json.value("simple-flight-api-settings", "{ }"_json);

  // Get SimpleFlight parameters and store in map
  const json& params_json =
      simple_flight_api_settings_json.value("parameters", "{ }"_json);
  for (json::const_iterator it = params_json.begin(); it != params_json.end();
       ++it) {
    params_map_[it.key()] = it.value();
  }

  // GetArray
  const json& actuator_order_json =
      simple_flight_api_settings_json.value("actuator-order", "[ ]"_json);

  try {
    int output_idx = 0;
    for (auto& actuator_json : actuator_order_json) {
      std::string id = actuator_json.value("id", "");
      actuator_id_to_output_idx_map_.insert({id, output_idx});
      output_idx++;
    }
  } catch (...) {
    throw;
  }
}

//---------------------------------------------------------------------------
// IController overrides

void SimpleFlightApi::BeginUpdate() {
  MultirotorApiBase::BeginUpdate();

  params_.reset(new simple_flight::Params);
  params_->motor.motor_count = num_motors_;

  if (!params_map_.empty()) {
    params_->LoadParams(params_map_);
    MultirotorApiBase::LoadParams(params_map_);
    safety_params_.LoadParams(params_map_);
  }

  if (vehicle_kind_ == VehicleKind::VTOLTailsitter)
    params_->controller_type =
        simple_flight::Params::ControllerType::kVFWTCascade;
  else if (vehicle_kind_ == VehicleKind::VTOLTiltrotor)
    params_->controller_type =
        simple_flight::Params::ControllerType::kVTRCascade;

  board_.reset(new AirSimSimpleFlightBoard(params_.get()));
  comm_link_.reset(new AirSimSimpleFlightCommLink());
  estimator_.reset(new AirSimSimpleFlightEstimator());
  estimator_fw_.reset(new AirSimSimpleFlightEstimatorFW());
  estimator_fw_->Initialize(estimator_.get());
  firmware_.reset(
      new simple_flight::Firmware(params_.get(), board_.get(), comm_link_.get(),
                                  estimator_.get(), estimator_fw_.get()));

  // Reset RC input timestamp--use uint64_t so wrap-around delta calculations
  // work
  {
    auto millis_cur = board_->Millis();

    millis_rc_input_last_update_ = *reinterpret_cast<uint64_t*>(&millis_cur);
  }

  Reset();

#ifdef LVMON_REPORTING
  LVMon::Set("Params/vtol/enable_fixed_wing",
             params_->vtol.enable_fixed_wing_mode ? "true" : "false");
#endif  // LVMON_REPORTING
}

void SimpleFlightApi::EndUpdate() {
  MultirotorApiBase::EndUpdate();
  // TODO: Do we need any clean-up of the board, firmware?
}

void SimpleFlightApi::Reset() { firmware_->Reset(); }

void SimpleFlightApi::SetKinematics(const Kinematics* kinematics) {
  board_->SetGroundTruthKinematics(kinematics);
  estimator_->SetGroundTruthKinematics(kinematics);
}

void SimpleFlightApi::Update() {
  // If RC is connected, mark as disconnected if there's no update
  // before the timeout
  if (board_->IsRcConnected()) {
    auto millis_cur = board_->Millis();

    if ((*reinterpret_cast<uint64_t*>(&millis_cur) -
         millis_rc_input_last_update_) >= params_->rc.connection_update_timeout)
      board_->SetIsRcConnected(false);
  }

  firmware_->Update();
};

std::vector<float> SimpleFlightApi::GetControlSignals(const std::string& actuator_id) {
  auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);
  if (actuator_map_itr == actuator_id_to_output_idx_map_.end()) {
    GetLogger().LogWarning(
        GetControllerName(),
        "SimpleFlightApi::GetControlSignal() called for invalid actuator: %s",
        actuator_id.c_str());
    return std::vector<float>(1,0.f);
  }

  return std::vector<float>(1, board_->GetMotorControlSignal(actuator_map_itr->second));
}

//---------------------------------------------------------------------------
// IMultirotorApi overrides

bool SimpleFlightApi::EnableApiControl() {
  std::string dummy_message;
  return firmware_->OffboardApi().RequestApiControl(dummy_message);
}

bool SimpleFlightApi::DisableApiControl() {
  firmware_->OffboardApi().ReleaseApiControl();
  return true;
}

bool SimpleFlightApi::IsApiControlEnabled() {
  return firmware_->OffboardApi().HasApiControl();
}

bool SimpleFlightApi::Arm(int64_t /*command_start_time_nanos*/) {
  std::string message;
  return firmware_->OffboardApi().Arm(message);
}

bool SimpleFlightApi::Disarm() {
  std::string message;
  return firmware_->OffboardApi().Disarm(message);
}

bool SimpleFlightApi::SetVTOLMode(VTOLMode vtolmode) {
  if ((vehicle_kind_ == VehicleKind::VTOLTailsitter) ||
      (vehicle_kind_ == VehicleKind::VTOLTiltrotor)) {
    params_->vtol.enable_fixed_wing_mode = (vtolmode == VTOLMode::FixedWing);

#ifdef LVMON_REPORTING
    LVMon::Set("Params/vtol/enable_fixed_wing",
               params_->vtol.enable_fixed_wing_mode ? "true" : "false");
#endif  // LVMON_REPORTING
    return (true);
  }

  return (false);
}

bool SimpleFlightApi::CanArm() const { return true; }

ReadyState SimpleFlightApi::GetReadyState() const {
  ReadyState state;
  state.ReadyVal = true;
  return state;
}

//---------------------------------------------------------------------------
// MultirotorApiBase overrides
void SimpleFlightApi::CreateTopics(void) {
  // Call superclass
  VTOLFWApiBase::CreateTopics();

  // Add rc_input topic for RC-vehicle-style remote control input
  rc_input_topic_ = sim_robot_.CreateTopic(
      "simple_flight/rc_input", TopicType::kSubscribed, 60,
      MessageType::kFlightControlRCInput,
      [this](const Topic& topic, const Message& message) {
        OnTopicRCInput(topic, message);
      });
  topics_.push_back(rc_input_topic_);
}

//---------------------------------------------------------------------------
// Implementation for MultirotorApiBase

void SimpleFlightApi::CommandMotorPWMs(float front_right_pwm,
                                       float rear_left_pwm,
                                       float front_left_pwm,
                                       float rear_right_pwm) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kPassthrough, GoalModeType::kPassthrough,
      GoalModeType::kPassthrough, GoalModeType::kPassthrough);

  simple_flight::Axis4r goal(front_right_pwm, rear_left_pwm, front_left_pwm,
                             rear_right_pwm);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandRollPitchYawZ(float roll, float pitch, float yaw,
                                           float z) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleLevel, GoalModeType::kAngleLevel,
      GoalModeType::kAngleLevel, GoalModeType::kPositionWorld);

  simple_flight::Axis4r goal(roll, pitch, yaw, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandRollPitchYawThrottle(float roll, float pitch,
                                                  float yaw, float throttle) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleLevel, GoalModeType::kAngleLevel,
      GoalModeType::kAngleLevel, GoalModeType::kPassthrough);

  simple_flight::Axis4r goal(roll, pitch, yaw, throttle);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandRollPitchYawrateThrottle(float roll, float pitch,
                                                      float yaw_rate,
                                                      float throttle) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleLevel, GoalModeType::kAngleLevel,
      GoalModeType::kAngleRate, GoalModeType::kPassthrough);

  simple_flight::Axis4r goal(roll, pitch, yaw_rate, throttle);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandRollPitchYawrateZ(float roll, float pitch,
                                               float yaw_rate, float z) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleLevel, GoalModeType::kAngleLevel,
      GoalModeType::kAngleRate, GoalModeType::kPositionWorld);

  simple_flight::Axis4r goal(roll, pitch, yaw_rate, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandAngleRatesZ(float roll_rate, float pitch_rate,
                                         float yaw_rate, float z) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleRate, GoalModeType::kAngleRate,
      GoalModeType::kAngleRate, GoalModeType::kPositionWorld);

  simple_flight::Axis4r goal(roll_rate, pitch_rate, yaw_rate, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandAngleRatesThrottle(float roll_rate,
                                                float pitch_rate,
                                                float yaw_rate,
                                                float throttle) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleRate, GoalModeType::kAngleRate,
      GoalModeType::kAngleRate, GoalModeType::kPassthrough);

  simple_flight::Axis4r goal(roll_rate, pitch_rate, yaw_rate, throttle);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandVelocity(float vx, float vy, float vz,
                                      bool yaw_is_rate, float yaw) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kVelocityWorld, GoalModeType::kVelocityWorld,
      yaw_is_rate ? GoalModeType::kAngleRate : GoalModeType::kAngleLevel,
      GoalModeType::kVelocityWorld);

  simple_flight::Axis4r goal(vy, vx, yaw, vz);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandVelocityZ(float vx, float vy, float z,
                                       bool yaw_is_rate, float yaw) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kVelocityWorld, GoalModeType::kVelocityWorld,
      yaw_is_rate ? GoalModeType::kAngleRate : GoalModeType::kAngleLevel,
      GoalModeType::kPositionWorld);

  simple_flight::Axis4r goal(vy, vx, yaw, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandVelocityBody(float vx, float vy, float vz,
                                          bool yaw_is_rate, float yaw) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kVelocityBody, GoalModeType::kVelocityBody,
      yaw_is_rate ? GoalModeType::kAngleRate : GoalModeType::kAngleLevel,
      GoalModeType::kVelocityBody);

  simple_flight::Axis4r goal(vy, vx, yaw, vz);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandVelocityZBody(float vx, float vy, float z,
                                           bool yaw_is_rate, float yaw) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kVelocityBody, GoalModeType::kVelocityBody,
      yaw_is_rate ? GoalModeType::kAngleRate : GoalModeType::kAngleLevel,
      GoalModeType::kPositionWorld);

  simple_flight::Axis4r goal(vy, vx, yaw, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void SimpleFlightApi::CommandPosition(float x, float y, float z,
                                      bool yaw_is_rate, float yaw) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kPositionWorld, GoalModeType::kPositionWorld,
      yaw_is_rate ? GoalModeType::kAngleRate : GoalModeType::kAngleLevel,
      GoalModeType::kPositionWorld);

  // user cmd : {x,y,z,yaw} in NED frame ->
  // simple_flight goal : {roll,pitch,yaw,z} in body FRD (FrontRightDown)frame
  // x -> pitching, y -> rolling ; swap (yaw, z).
  simple_flight::Axis4r goal(y, x, yaw, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

const MultirotorApiBase::MultirotorApiParams&
SimpleFlightApi::GetMultirotorApiParams() const {
  return safety_params_;
}

void SimpleFlightApi::SetControllerGains(uint8_t controller_type,
                                         const std::vector<float>& kp,
                                         const std::vector<float>& ki,
                                         const std::vector<float>& kd) {
  simple_flight::GoalModeType controller_type_enum =
      static_cast<simple_flight::GoalModeType>(controller_type);

  std::vector<float> kp_axis4(4);
  std::vector<float> ki_axis4(4);
  std::vector<float> kd_axis4(4);

  switch (controller_type_enum) {
    // roll gain, pitch gain, yaw gain, and no gains in throttle / z axis
    case simple_flight::GoalModeType::kAngleRate:
      kp_axis4 = {kp[0], kp[1], kp[2], 1.0};
      ki_axis4 = {ki[0], ki[1], ki[2], 0.0};
      kd_axis4 = {kd[0], kd[1], kd[2], 0.0};
      params_->angle_rate_pid.p.SetValues(kp_axis4);
      params_->angle_rate_pid.i.SetValues(ki_axis4);
      params_->angle_rate_pid.d.SetValues(kd_axis4);
      params_->gains_changed = true;
      break;
    case simple_flight::GoalModeType::kAngleLevel:
      kp_axis4 = {kp[0], kp[1], kp[2], 1.0};
      ki_axis4 = {ki[0], ki[1], ki[2], 0.0};
      kd_axis4 = {kd[0], kd[1], kd[2], 0.0};
      params_->angle_level_pid.p.SetValues(kp_axis4);
      params_->angle_level_pid.i.SetValues(ki_axis4);
      params_->angle_level_pid.d.SetValues(kd_axis4);
      params_->gains_changed = true;
      break;
    case simple_flight::GoalModeType::kVelocityWorld:
      kp_axis4 = {kp[1], kp[0], 0.0, kp[2]};
      ki_axis4 = {ki[1], ki[0], 0.0, ki[2]};
      kd_axis4 = {kd[1], kd[0], 0.0, kd[2]};
      params_->velocity_pid.p.SetValues(kp_axis4);
      params_->velocity_pid.i.SetValues(ki_axis4);
      params_->velocity_pid.d.SetValues(kd_axis4);
      params_->gains_changed = true;
      break;
    case simple_flight::GoalModeType::kPositionWorld:
      kp_axis4 = {kp[1], kp[0], 0.0, kp[2]};
      ki_axis4 = {ki[1], ki[0], 0.0, ki[2]};
      kd_axis4 = {kd[1], kd[0], 0.0, kd[2]};
      params_->position_pid.p.SetValues(kp_axis4);
      params_->position_pid.i.SetValues(ki_axis4);
      params_->position_pid.d.SetValues(kd_axis4);
      params_->gains_changed = true;
      break;
    default:
      // Utils::Log("Unimplemented controller type");
      sim_robot_.GetLogger().LogError(GetControllerName(),
                                      "Unimplemented controller type");
      break;
  }
}

Kinematics SimpleFlightApi::GetKinematicsEstimated() const {
  return AirSimSimpleFlightCommon::ToKinematicsState3r(
      firmware_->OffboardApi().GetStateEstimator().GetKinematicsEstimated());
}

Vector3 SimpleFlightApi::GetAngles() const {
  const auto& val = firmware_->OffboardApi().GetStateEstimator().GetAngles();
  return AirSimSimpleFlightCommon::ToVector3(val);
}

Vector3 SimpleFlightApi::GetPosition() const {
  const auto& val = firmware_->OffboardApi().GetStateEstimator().GetPosition();
  return AirSimSimpleFlightCommon::ToVector3(val);
}

Vector3 SimpleFlightApi::GetVelocity() const {
  const auto& val =
      firmware_->OffboardApi().GetStateEstimator().GetLinearVelocity();
  return AirSimSimpleFlightCommon::ToVector3(val);
}

Quaternion SimpleFlightApi::GetOrientation() const {
  const auto& val =
      firmware_->OffboardApi().GetStateEstimator().GetOrientation();
  return AirSimSimpleFlightCommon::ToQuaternion(val);
}

LandedState SimpleFlightApi::GetLandedState() const {
  return firmware_->OffboardApi().GetLandedState() ? LandedState::Landed
                                                   : LandedState::Flying;
}

GeoPoint SimpleFlightApi::GetGpsLocationEstimated() const {
  return sim_robot_.GetEnvironment().env_info.geo_point;
}

float SimpleFlightApi::GetCommandPeriod() const {
  return 1.0f / 50;  // 50hz
}

float SimpleFlightApi::GetTakeoffZ() const {
  return params_->takeoff.takeoff_z;
}

float SimpleFlightApi::GetDistanceAccuracy() const {
  return 0.5f;  // measured in simulator by firing commands "MoveToLocation -x 0
                // -y 0" multiple times and looking at distance traveled
}

bool SimpleFlightApi::MoveByVelocityBodyFrame(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, int64_t command_start_time_nanos) {
  // It is only correctly implemented for multirotor and simple flight, need to
  // be done for the rest of airframes and controllers
  if (vehicle_kind_ != VehicleKind::Multirotor) {
    return MultirotorApiBase::MoveByVelocityBodyFrame(
        vx, vy, vz, duration, drivetrain, yaw_is_rate, yaw,
        command_start_time_nanos);
  }
  SingleTaskCall lock(this);

  if (duration <= 0) {
    return true;
  }

  if (!yaw_is_rate) {
    // adjust yaw to be absolute
    float vehicle_yaw =
        PhysicsUtils::GetYaw<float>(GetKinematicsEstimated().pose.orientation);
    yaw += vehicle_yaw;
    if (drivetrain == DrivetrainType::ForwardOnly) {
      float vx_adjusted =
          (vx * std::cosf(vehicle_yaw)) - (vy * std::sinf(vehicle_yaw));
      float vy_adjusted =
          (vx * std::sinf(vehicle_yaw)) + (vy * std::cosf(vehicle_yaw));
      AdjustYaw(vx_adjusted, vy_adjusted, drivetrain, yaw_is_rate, yaw);
    }
  }

  return RunFlightCommand(
             [&]() {
               MoveByVelocityBodyInternal(vx, vy, vz, yaw_is_rate, yaw);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

bool SimpleFlightApi::MoveByVelocityBodyFrameZ(
    float vx, float vy, float z, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);
  // It is only correctly implemented for multirotor and simple flight, need to
  // be done for the rest of airframes and controllers
  if (vehicle_kind_ != VehicleKind::Multirotor) {
    return MultirotorApiBase::MoveByVelocityBodyFrameZ(
        vx, vy, z, duration, drivetrain, yaw_is_rate, yaw,
        command_start_time_nanos);
  }

  if (duration <= 0) {
    return false;
  }
  if (!yaw_is_rate) {
    // adjust yaw to be absolute
    float vehicle_yaw =
        PhysicsUtils::GetYaw<float>(GetKinematicsEstimated().pose.orientation);
    yaw += vehicle_yaw;
    if (drivetrain == DrivetrainType::ForwardOnly) {
      float vx_adjusted =
          (vx * std::cosf(vehicle_yaw)) - (vy * std::sinf(vehicle_yaw));
      float vy_adjusted =
          (vx * std::sinf(vehicle_yaw)) + (vy * std::cosf(vehicle_yaw));
      AdjustYaw(vx_adjusted, vy_adjusted, drivetrain, yaw_is_rate, yaw);
    }
  }

  return RunFlightCommand(
             [&]() {
               MoveByVelocityZBodyInternal(vx, vy, z, yaw_is_rate, yaw);
               return false;  // keep moving until timeout
             },
             duration, command_start_time_nanos)
      .IsTimeout();
}

//---------------------------------------------------------------------------
// Implementation for VTOLFWApiBase

void SimpleFlightApi::CommandHeading(float heading, float speed, float vz) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kVelocityWorld, GoalModeType::kVelocityWorld,
      GoalModeType::kAngleLevel, GoalModeType::kVelocityWorld);
  auto vx = cos(heading) * speed;
  auto vy = sin(heading) * speed;
  simple_flight::Axis4r goal(vy, vx, heading, vz);
  std::string message;

  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

//---------------------------------------------------------------------------
// Implementation for MultirotorApiBase

// Switched to using service method request-response for all control commands,
// but leaving the below pub-sub version commented out for reference in case
// it's needed in the future.
// void SimpleFlightApi::OnSetpointNEDvelocityYawrate(
//     const Topic& topic, const Message& message) {
//   std::lock_guard<std::mutex> lock(update_lock_);

//   typedef simple_flight::GoalModeType GoalModeType;
//   simple_flight::GoalMode mode(
//       GoalModeType::kVelocityWorld, GoalModeType::kVelocityWorld,
//       GoalModeType::kAngleRate, GoalModeType::kVelocityWorld);

//   auto axes_vec =
//       static_cast<const FlightControlSetpointMessage&>(message)
//           .GetAxesVec();
//   // user cmd : {xdot,ydot,zdot,yawrate} in NED frame ->
//   // simple_flight goal : {roll,pitch,yaw,z} in body FRD
//   (FrontRightDown) frame
//       // xdot -> pitching, ydot -> rolling ; swap (yawrate, z).
//       simple_flight::Axis4r goal(axes_vec[1], axes_vec[0], axes_vec[3],
//                                  axes_vec[2]);

//   std::string firmware_dummy_message;
//   firmware_->OffboardApi().SetGoalAndMode(&goal, &mode,
//   firmware_dummy_message);
// }

void SimpleFlightApi::OnTopicRCInput(const Topic& topic,
                                     const Message& message) {
#ifdef LVMON_REPORTING
  static const char* c_rgsz[8] = {
      "SFRC/Ch0", "SFRC/Ch1", "SFRC/Ch2", "SFRC/Ch3",
      "SFRC/Ch4", "SFRC/Ch5", "SFRC/Ch6", "SFRC/Ch7",
  };
#endif  // LVMON_REPORTING

  auto vec_channels =
      static_cast<const FlightControlRCInputMessage&>(message).GetChannelsVec();
  size_t cchannel =
      std::min(vec_channels.size(), (size_t)params_->rc.channel_count);
  auto it = vec_channels.cbegin();
  auto millis_cur = board_->Millis();
  std::lock_guard<std::mutex> lock(update_lock_);

  for (size_t ichannel = 0; ichannel < cchannel; ++ichannel, ++it) {
    board_->SetInputChannel(ichannel, *it);
#ifdef LVMON_REPORTING
    if (ichannel < std::size(c_rgsz)) LVMon::Set(c_rgsz[ichannel], *it);
#endif  // LVMON_REPORTING
  }

  millis_rc_input_last_update_ = *reinterpret_cast<uint64_t*>(&millis_cur);
  board_->SetIsRcConnected(true);
}

}  // namespace projectairsim
}  // namespace microsoft