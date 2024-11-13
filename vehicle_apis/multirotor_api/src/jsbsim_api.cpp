// Copyright (C) Microsoft Corporation. All rights reserved.

#include "jsbsim_api.hpp"

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
#include "core_sim/service_method.hpp"
#include "core_sim/geodetic_converter.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class JSBSimApi

JSBSimApi::JSBSimApi(const Robot& robot,
                                 TransformTree* ptransformtree)
    : LandingGearApiBase(robot, ptransformtree) {
  LoadSettings(robot);
}

void JSBSimApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  const std::string airframe_setup =
      controller_settings_json.value("airframe-setup", "");

  if (airframe_setup == "quadrotor-x") {
    vehicle_kind_ = VehicleKind::Multirotor;
  } else if (airframe_setup == "hexarotor-x") {
    vehicle_kind_ = VehicleKind::Multirotor;
  } else if (airframe_setup == "vtol-quad-x-tailsitter") {
    vehicle_kind_ = VehicleKind::VTOLTailsitter;
  } else if (airframe_setup == "vtol-quad-tiltrotor") {;
    vehicle_kind_ = VehicleKind::VTOLTiltrotor;
  } else if (airframe_setup == "fixed-wing") { //TODO: review if this variable is necesary, the number of motors is flexible
    vehicle_kind_ = VehicleKind::FixedWing;
  } else {
    vehicle_kind_ = VehicleKind::Other;
  } 

  // GetJsonObject
  //const json& jsbsim_api_settings_json =
  //    controller_settings_json.value("jsbsim-api-settings", "{ }"_json);

}

//---------------------------------------------------------------------------
// IController overrides

void JSBSimApi::BeginUpdate() {
  MultirotorApiBase::BeginUpdate();

  params_.reset(new simple_flight::Params);
  params_->motor.motor_count = 4;

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

void JSBSimApi::EndUpdate() {
  MultirotorApiBase::EndUpdate();
  // TODO: Do we need any clean-up of the board, firmware?
}

void JSBSimApi::Reset() { firmware_->Reset(); }

void JSBSimApi::Update() { firmware_->Update(); }

void JSBSimApi::SetKinematics(const Kinematics* kinematics) {
  board_->SetGroundTruthKinematics(kinematics);
  estimator_->SetGroundTruthKinematics(kinematics);
}

// GetJSBSimProperty
float JSBSimApi::GetJSBSimProperty(const std::string& property) {
  auto model = sim_robot_.GetJSBSimModel();
  auto value = model->GetPropertyValue(property);
  return value;
}

bool JSBSimApi::SetJSBSimProperty(const std::string& property, float value) {
  auto model = sim_robot_.GetJSBSimModel();
  model->SetPropertyValue(property, value);
  return true;
}

void JSBSimApi::RegisterServiceMethods() {
  LandingGearApiBase::RegisterServiceMethods();

  //Register GetJSBSimProperty
  auto method = ServiceMethod("GetJSBSimProperty", {"_property_name"});
  auto method_handler =
      method.CreateMethodHandler(&JSBSimApi::GetJSBSimProperty, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  //Register SetJSBSimProperty
  method = ServiceMethod("SetJSBSimProperty", {"_property_name","_value"});
  method_handler =
      method.CreateMethodHandler(&JSBSimApi::SetJSBSimProperty, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

std::vector<float> JSBSimApi::GetControlSignals(const std::string& actuator_id) {
  auto model = sim_robot_.GetJSBSimModel();
  //add fcs/ to actuator_id
  auto jsbsim_actuator_id = actuator_id;
  std::vector<float> return_vector{GetJSBSimProperty(jsbsim_actuator_id)};
  return return_vector;
}

//---------------------------------------------------------------------------

bool JSBSimApi::Takeoff(float timeout_sec, TimeNano _service_method_start_time) {
  // if this vehicle is not a fixed wing
  if (vehicle_kind_ != VehicleKind::FixedWing)
    return VTOLFWApiBase::Takeoff(timeout_sec, _service_method_start_time);

  // if this vehicle is a fixed wing
  auto model = sim_robot_.GetJSBSimModel();

  // A JSBSim vehicle must have the "airsim/takeoff" booleans defined
  // that triggers all the necessary takeoff procedures
  SetJSBSimProperty("airsim/takeoff", 1);

  auto success = RunFlightCommand(
            [&]() {
              float takeoff_ended = GetJSBSimProperty("airsim/takeoff-ended");
              return takeoff_ended > 0.99;
            },
            timeout_sec, _service_method_start_time)
    .IsComplete();

  if (!success)
    return false;

  success =  RunFlightCommand(
          [&]() {
            float takeoff_success = GetJSBSimProperty("airsim/takeoff-success");
            return takeoff_success > 0.99;
          },
          timeout_sec, _service_method_start_time)
  .IsComplete();
  return success;
}

bool JSBSimApi::Land(float timeout_sec, int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);
  
  auto model = sim_robot_.GetJSBSimModel();
  
  // Wait until we stop moving vertically
  {
    // A JSBSim vehicle must have the "airsim/takeoff" boolean defined
    // that triggers all the necessary takeoff procedures
    SetJSBSimProperty("airsim/land", 1);

    return RunFlightCommand(
               [&]() {
                  float land_ended = GetJSBSimProperty("airsim/land-ended");
                  return static_cast<bool>(land_ended);
               },
               timeout_sec, command_start_time_nanos)
        .IsComplete();
  }
}

bool JSBSimApi::EnableApiControl() {
  is_api_control_enabled_ = true;
  return true;
}

bool JSBSimApi::DisableApiControl() {
  firmware_->OffboardApi().ReleaseApiControl();
  is_api_control_enabled_ = false;
  return true;
}

bool JSBSimApi::IsApiControlEnabled() {
  return is_api_control_enabled_;
}

bool JSBSimApi::CanArm() const {
  return true;
}

void JSBSimApi::CommandVelocityBody(float vx, float vy, float vz,
                          bool yaw_is_rate, float yaw) {
}

void JSBSimApi::CommandVelocityZBody(float vx, float vy, float z,
                          bool yaw_is_rate, float yaw) {
}

GeoPoint JSBSimApi::GetGpsLocationEstimated() const
{
  return sim_robot_.GetEnvironment().home_geo_point.geo_point;
}

ReadyState JSBSimApi::GetReadyState() const {
  ReadyState state;
  state.ReadyVal = true;
  return state;
}

bool JSBSimApi::SetBrakes(float value) {
  //get jsbsim model
  auto model = sim_robot_.GetJSBSimModel();
  SetJSBSimProperty("fcs/left-brake-cmd-norm", value);
  SetJSBSimProperty("fcs/right-brake-cmd-norm", value);
  SetJSBSimProperty("fcs/center-brake-cmd-norm", value);
  return true;
}


bool JSBSimApi::Arm(int64_t /*command_start_time_nanos*/) {
  //std::string message;
  //return firmware_->OffboardApi().Arm(message);
  //get jsbsim model
  auto model = sim_robot_.GetJSBSimModel();
  SetJSBSimProperty("fcs/mixture-cmd-norm[0]", 1);
  SetJSBSimProperty("fcs/mixture-cmd-norm[1]", 1);
  SetJSBSimProperty("fcs/advance-cmd-norm[0]", 1);
  SetJSBSimProperty("fcs/advance-cmd-norm[1]", 1);
  SetJSBSimProperty("fcs/throttle-cmd-norm[0]", 0.01);
  SetJSBSimProperty("fcs/throttle-cmd-norm[1]", 0.01);
  SetJSBSimProperty("propulsion/magneto_cmd", 3);
  SetJSBSimProperty("propulsion/starter_cmd", 1);


  return true;
}

bool JSBSimApi::Disarm() {
  std::string message;
  return firmware_->OffboardApi().Disarm(message);
}

//---------------------------------------------------------------------------
// Implementation for MultirotorApiBase

void JSBSimApi::CommandMotorPWMs(float front_right_pwm,
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

void JSBSimApi::CommandRollPitchYawZ(float roll, float pitch, float yaw,
                                           float z) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleLevel, GoalModeType::kAngleLevel,
      GoalModeType::kAngleLevel, GoalModeType::kPositionWorld);

  simple_flight::Axis4r goal(roll, pitch, yaw, z);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void JSBSimApi::CommandRollPitchYawThrottle(float roll, float pitch,
                                                  float yaw, float throttle) {
  typedef simple_flight::GoalModeType GoalModeType;
  simple_flight::GoalMode mode(
      GoalModeType::kAngleLevel, GoalModeType::kAngleLevel,
      GoalModeType::kAngleLevel, GoalModeType::kPassthrough);

  simple_flight::Axis4r goal(roll, pitch, yaw, throttle);

  std::string message;
  firmware_->OffboardApi().SetGoalAndMode(&goal, &mode, message);
}

void JSBSimApi::CommandRollPitchYawrateThrottle(float roll, float pitch,
                                                      float yaw_rate,
                                                      float throttle) {
}

void JSBSimApi::CommandRollPitchYawrateZ(float roll, float pitch,
                                               float yaw_rate, float z) {
}

void JSBSimApi::CommandAngleRatesZ(float roll_rate, float pitch_rate,
                                         float yaw_rate, float z) {
}

void JSBSimApi::CommandAngleRatesThrottle(float roll_rate,
                                                float pitch_rate,
                                                float yaw_rate,
                                                float throttle) {
}

void JSBSimApi::CommandVelocity(float vx, float vy, float vz,
                                      bool yaw_is_rate, float yaw) {
}

void JSBSimApi::CommandVelocityZ(float vx, float vy, float z,
                                       bool yaw_is_rate, float yaw) {
}

void JSBSimApi::CommandPosition(float x, float y, float z,
                                      bool yaw_is_rate, float yaw) {
}


bool JSBSimApi::MoveToPosition(float x, float y, float z, float velocity,
                      float timeout_sec, DrivetrainType drivetrain,
                      bool yaw_is_rate, float yaw, float lookahead,
                      float adaptive_lookahead,
                      int64_t command_start_time_nanos){

  //get world coordinates from sim_robot_
  auto world_geopoint = sim_robot_.GetEnvironment().home_geo_point.geo_point;
  // get coordinates for x,y,z
  GeodeticConverter converter(world_geopoint.latitude, world_geopoint.longitude,
                              world_geopoint.altitude); 
  double lat, lon;
  float alt;
  converter.ned2Geodetic(x,y,z,&lat,&lon,&alt);
  
  SetJSBSimProperty("ap/altitude_setpoint", -z * MathUtils::meters_to_feets); // world is flat so z is altitude
  SetJSBSimProperty("guidance/target_wp_latitude_rad", MathUtils::deg2Rad(lat));
  SetJSBSimProperty("guidance/target_wp_longitude_rad", MathUtils::deg2Rad(lon));
  SetJSBSimProperty("ap/heading-setpoint-select", 1);
  SetJSBSimProperty("guidance/heading-selector-switch", 0);
  SetJSBSimProperty("ap/heading_hold", 1);

  auto just_started = true; // to avoid checking distance on first iteration

  return RunFlightCommand(
            [&]() {
              if (just_started) {
                just_started = false;
                return false;
              }
              if (GetJSBSimProperty("guidance/wp-distance")<= lookahead * MathUtils::meters_to_feets) {
                return true; 
              } else 
                return false;
            },
            timeout_sec, command_start_time_nanos)
    .IsComplete();

}

const MultirotorApiBase::MultirotorApiParams&
JSBSimApi::GetMultirotorApiParams() const {
  return safety_params_;
}

void JSBSimApi::SetControllerGains(uint8_t controller_type,
                                         const std::vector<float>& kp,
                                         const std::vector<float>& ki,
                                         const std::vector<float>& kd) {
}

Kinematics JSBSimApi::GetKinematicsEstimated() const {
  return AirSimSimpleFlightCommon::ToKinematicsState3r(
      firmware_->OffboardApi().GetStateEstimator().GetKinematicsEstimated());
}

Vector3 JSBSimApi::GetAngles() const {
  const auto& val = firmware_->OffboardApi().GetStateEstimator().GetAngles();
  return AirSimSimpleFlightCommon::ToVector3(val);
}

Vector3 JSBSimApi::GetPosition() const {
  const auto& val = firmware_->OffboardApi().GetStateEstimator().GetPosition();
  return AirSimSimpleFlightCommon::ToVector3(val);
}

Vector3 JSBSimApi::GetVelocity() const {
  const auto& val =
      firmware_->OffboardApi().GetStateEstimator().GetLinearVelocity();
  return AirSimSimpleFlightCommon::ToVector3(val);
}

Quaternion JSBSimApi::GetOrientation() const {
  const auto& val =
      firmware_->OffboardApi().GetStateEstimator().GetOrientation();
  return AirSimSimpleFlightCommon::ToQuaternion(val);
}

LandedState JSBSimApi::GetLandedState() const {
  return firmware_->OffboardApi().GetLandedState() ? LandedState::Landed
                                                   : LandedState::Flying;
}

float JSBSimApi::GetCommandPeriod() const {
  return 1.0f / 50;  // 50hz
}

float JSBSimApi::GetTakeOffZ() {
  return takeoff_z_;
}

bool  JSBSimApi::SetTakeOffZ(float z) {
  takeoff_z_ = z;
  return true;
}

float JSBSimApi::GetDistanceAccuracy() const {
  return 0.5f;  // measured in simulator by firing commands "MoveToLocation -x 0
                // -y 0" multiple times and looking at distance traveled
}

//---------------------------------------------------------------------------
// Implementation for VTOLFWApiBase
bool JSBSimApi::MoveByHeading(float heading, float speed, float vz,
                                  float duration, float heading_margin,
                                  float yaw_rate, float timeout_sec,
                                  int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  float yaw = MathUtils::NormalizeAngle<float>(heading);

  return (RunFlightCommand(
              [&]() {
                // Command the heading and wait until we've reached it
                if (RunFlightCommand(
                        [&]() {
                          if (IsYawWithinMargin(yaw, heading_margin))
                            return (true);
                          else
                            CommandHeading(yaw, speed, vz);

                          return (false);
                        },
                        timeout_sec, command_start_time_nanos)
                        .IsTimeout()) {
                  return (false);
                }

                // Keep commanding the heading for the requested duration
                RunFlightCommand(
                    [&]() {
                      CommandHeading(yaw, speed, vz);
                      return (false);
                    },
                    duration, SimClock::Get()->NowSimNanos());

                return (true);
              },
              timeout_sec, command_start_time_nanos)
              .IsComplete());
}

void JSBSimApi::CommandHeading(float heading, float speed, float vz) {
  auto setpoint_yaw_deg = MathUtils::rad2Deg(heading);
  SetJSBSimProperty("ap/heading_setpoint",  setpoint_yaw_deg);
  SetJSBSimProperty("ap/heading-setpoint-select", 0);
  SetJSBSimProperty("ap/heading_hold", 1);
}

bool JSBSimApi::MoveToZ(float z, float velocity, float timeout_sec, bool yaw_is_rate,
            float yaw, float lookahead, float adaptive_lookahead,
            int64_t command_start_time_nanos) {
  auto model = sim_robot_.GetJSBSimModel();
  
  auto success = RunFlightCommand(
            [&]() {
              auto vel = GetVelocity().norm();
              if (vel >= 25.0f) {
                SetJSBSimProperty("ap/altitude_setpoint", -z * MathUtils::meters_to_feets);
                SetJSBSimProperty("ap/altitude_hold", 1);
                return true; 
              } else 
                return false;
            },
            timeout_sec, command_start_time_nanos)
    .IsComplete();

    if (!success)
      return false;

    success =  RunFlightCommand(
            [&]() {
              auto z_ = GetPosition().z();
              if (z_ <= z) {
                return true; 
              } else 
                return false;
            },
            timeout_sec, command_start_time_nanos)
    .IsComplete();

    return success;
              
}

//---------------------------------------------------------------------------

}  // namespace projectairsim
}  // namespace microsoft