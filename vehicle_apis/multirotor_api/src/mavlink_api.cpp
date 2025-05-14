// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "mavlink_api.hpp"

#include <core_sim/actuators/gimbal.hpp>
#include <json.hpp>

#include "core_sim/clock.hpp"
#include "core_sim/message/flight_control_setpoint_message.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class MavLinkApi

MavLinkApi::MavLinkApi(const Robot& robot, TransformTree* ptransformtree)
    : VTOLFWApiBase(robot, ptransformtree) {
  LoadSettings(robot);
}

MavLinkApi::~MavLinkApi() {
  // TODO: revisit for reload-scene / RL training
  CloseAllConnections();
  if (this->connect_thread_.joinable()) {
    this->connect_thread_.join();
  }
}

void MavLinkApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  // GetJsonObject
  const json& mavlink_api_settings_json =
      controller_settings_json.value("px4-settings", "{ }"_json);

  // GetArray
  const json& actuator_order_json =
      mavlink_api_settings_json.value("actuator-order", "[ ]"_json);

  try {
    int output_idx = 0;
    // Might be prudent to verify if these are actually actuators even if we
    // document that it shouldn't be part of the actuator order list? the order
    // corresponds to the control output pins of PX4 output and can lead to out
    // of index issues if there are more than expected.
    for (auto& actuator_json : actuator_order_json) {
      std::string id = actuator_json.value("id", "");
      actuator_id_to_output_idx_map_.insert({id, output_idx});
      output_idx++;
    }
  } catch (...) {
    throw;
  }

  // serial/hitl settings
  connection_info_.use_serial = mavlink_api_settings_json.value(
      "use-serial", connection_info_.use_serial);
  connection_info_.serial_port = mavlink_api_settings_json.value(
      "serial-port", connection_info_.serial_port);
  connection_info_.baud_rate = mavlink_api_settings_json.value(
      "serial-baud-rate", connection_info_.baud_rate);

  // sitl-tcp
  connection_info_.lock_step =
      mavlink_api_settings_json.value("lock-step", connection_info_.lock_step);
  connection_info_.use_tcp =
      mavlink_api_settings_json.value("use-tcp", connection_info_.use_tcp);
  connection_info_.tcp_port =
      mavlink_api_settings_json.value("tcp-port", connection_info_.tcp_port);

  // sitl-udp
  connection_info_.udp_address = mavlink_api_settings_json.value(
      "udp-address", connection_info_.udp_address);
  connection_info_.udp_port =
      mavlink_api_settings_json.value("udp-port", connection_info_.udp_port);

  // sitl-control-channel (both for tcp and udp)
  connection_info_.control_ip_address = mavlink_api_settings_json.value(
      "control-ip-address", connection_info_.control_ip_address);
  connection_info_.control_port_local = mavlink_api_settings_json.value(
      "control-port", connection_info_.control_port_local);
  connection_info_.control_port_local = mavlink_api_settings_json.value(
      "control-port-local", connection_info_.control_port_local);
  connection_info_.control_port_remote = mavlink_api_settings_json.value(
      "control-port-remote", connection_info_.control_port_remote);

  connection_info_.gimbal_port_local = mavlink_api_settings_json.value(
      "gimbal-port", connection_info_.gimbal_port_local);
  connection_info_.gimbal_port_remote = mavlink_api_settings_json.value(
      "gimbal-port-remote", connection_info_.gimbal_port_remote);

  // QGC
  connection_info_.qgc_ip_address = mavlink_api_settings_json.value(
      "qgc-host-ip", connection_info_.qgc_ip_address);
  connection_info_.qgc_ip_port =
      mavlink_api_settings_json.value("qgc-port", connection_info_.qgc_ip_port);

  // Lock step timeout thresholds
  connection_info_.timeout_lock_step_update_ms =
      mavlink_api_settings_json.value(
          "timeout-lock-step-update-ms",
          connection_info_.timeout_lock_step_update_ms);

  connection_info_.timeout_lock_step_actuator_ms =
      mavlink_api_settings_json.value(
          "timeout-lock-step-actuator-ms",
          connection_info_.timeout_lock_step_actuator_ms);

  connection_info_.timeout_lock_step_hil_actuator_control_ms =
      mavlink_api_settings_json.value(
          "timeout-lock-step-hil-actuator-control-ms",
          connection_info_.timeout_lock_step_hil_actuator_control_ms);

  // mavlink vehicle identifiers
  connection_info_.sim_sysid =
      mavlink_api_settings_json.value("sim-sys-id", connection_info_.sim_sysid);
  connection_info_.sim_compid = mavlink_api_settings_json.value(
      "sim-comp-id", connection_info_.sim_compid);
  connection_info_.offboard_sysid = mavlink_api_settings_json.value(
      "offboard-sys-id", connection_info_.offboard_sysid);
  connection_info_.offboard_compid = mavlink_api_settings_json.value(
      "offboard-comp-id", connection_info_.offboard_compid);
  connection_info_.vehicle_sysid = mavlink_api_settings_json.value(
      "vehicle-sys-id", connection_info_.vehicle_sysid);
  connection_info_.vehicle_compid = mavlink_api_settings_json.value(
      "vehicle-comp-id", connection_info_.vehicle_compid);

  connection_info_.local_host_ip = mavlink_api_settings_json.value(
      "local-host-ip", connection_info_.local_host_ip);

  connection_info_.model =
      mavlink_api_settings_json.value("model", connection_info_.model);

  const json& params_json =
      mavlink_api_settings_json.value("parameters", "{ }"_json);
  for (json::const_iterator it = params_json.begin(); it != params_json.end();
       ++it) {
    connection_info_.params[it.key()] = it.value();
    if (it.key() == "MNT_MODE_IN") {
      enable_gimbal_ = true;
    }
  }
}

void MavLinkApi::GetSensors(const Robot& robot) {
  const std::vector<std::reference_wrapper<Sensor>>& sensors =
      robot.GetSensors();

  airspeed_sensor_ = nullptr;
  imu_sensor_ = nullptr;
  magnetometer_sensor_ = nullptr;
  barometer_sensor_ = nullptr;
  gps_sensor_ = nullptr;
  distance_sensor_ = nullptr;

  std::for_each(
      sensors.begin(), sensors.end(),
      [this](const std::reference_wrapper<Sensor> sensor_wrapper) {
        Sensor& sensor = sensor_wrapper.get();
        if (sensor.IsEnabled()) {
          if (imu_sensor_ == nullptr && sensor.GetType() == SensorType::kImu) {
            imu_sensor_ = static_cast<Imu*>(&((Imu&)sensor));
            auto id = imu_sensor_->GetId();
          } else if (airspeed_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kAirspeed) {
            airspeed_sensor_ =
                static_cast<AirspeedSensor*>(&((AirspeedSensor&)sensor));
          } else if (magnetometer_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kMagnetometer) {
            magnetometer_sensor_ =
                static_cast<Magnetometer*>(&((Magnetometer&)sensor));
          } else if (barometer_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kBarometer) {
            barometer_sensor_ = static_cast<Barometer*>(&((Barometer&)sensor));
          } else if (gps_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kGps) {
            gps_sensor_ = static_cast<Gps*>(&((Gps&)sensor));
          } else if (distance_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kDistanceSensor) {
            distance_sensor_ =
                static_cast<DistanceSensor*>(&((DistanceSensor&)sensor));
          }
        }
      });

  if (imu_sensor_ == nullptr)
    throw Error(
        "MavLinkApi: IMU sensor is required but not specified in sensor "
        "configuration");
}

//---------------------------------------------------------------------------
// IController overrides

void MavLinkApi::BeginUpdate() {
  MultirotorApiBase::BeginUpdate();

  is_simulation_mode_ = true;  // currently only sim mode is supported

  // TODO: revisit for reload-scene / RL training
  Reset();
  GetSensors(sim_robot_);
  InitializeGimbalStatus(sim_robot_);
  InitializeConnections();
}

void MavLinkApi::EndUpdate() {
  MultirotorApiBase::EndUpdate();

  CloseAllConnections();
  if (this->connect_thread_.joinable()) {
    this->connect_thread_.join();
  }
}

void MavLinkApi::Reset() {
  MultirotorApiBase::Reset();

  ResetState();
  was_reset_ = true;
  SetNormalMode();
}

void MavLinkApi::SendSystemTime(void) {
  // SYSTEM TIME from host
  auto tu = GetSimTimeMicros();
  uint32_t ms = (uint32_t)(tu / 1000);
  if (ms != last_sys_time_) {
    last_sys_time_ = ms;
    mavlinkcom::MavLinkSystemTime msg_system_time;
    msg_system_time.time_unix_usec = tu;
    msg_system_time.time_boot_ms = last_sys_time_;
    if (hil_node_ != nullptr) {
      hil_node_->sendMessage(msg_system_time);
    }
  }
}

void MavLinkApi::SetKinematics(const Kinematics* kinematics) {}

void MavLinkApi::Update() {
  try {
    MultirotorApiBase::Update();

    // TODO: if (sensors_ == nullptr || !connected_ || connection_ == nullptr ||
    if (imu_sensor_ == nullptr || !connected_ || connection_ == nullptr ||
        !connection_->isOpen() || !got_first_heartbeat_) {
      // GetLogger().LogTrace(GetControllerName(),
      //                     "Update: Not ready to send sensor data.");
      return;
    }

    auto now = SimClock::Get()->NowSimMicros();
    if (lock_step_active_) {
      if ((last_update_time_ != 0) &&
          ((now - last_update_time_) >=
           (connection_info_.timeout_lock_step_update_ms * 1000))) {
        // if we haven't received an actuator control message within the
        // timeout then something is terribly wrong, reset lockstep mode
        lock_step_active_ = false;
        AddStatusMessage(
            "MavLinkApi::Update(): too long between calls--resetting lock step "
            "mode");
      }
    }

    last_update_time_ = now;

    AdvanceSimTime();

    // send sensor updates
    SendSensorData();

    // SendSystemTime() was ported from v1 but the reason for doing it is not
    // clear. For SITL, PX4 uses the timestamp from the HIL_SENSOR messages we
    // send to set its clock and lockstep seems to still work even if we don't
    // do SendSystemTime(). Not sure what this system time message affects.
    SendSystemTime();

    // Send according to a preset frequency?
    // Or every sim tick?
    if (((SimClock::Get()->NowSimMicros() - last_gimbal_time_) >
         gimbalStatusUpdateTimePeriod) &&
        setup_gimbal_ && enable_gimbal_) {
      SendGimbalState();
    }

    // wait for px4 to respond with actuator message
    if (lock_step_active_) {
      std::unique_lock<std::mutex> ul(mutex_received_actuator_controls_);

      if (!cv_received_actuator_controls_.wait_for(
              ul,
              std::chrono::milliseconds(
                  connection_info_.timeout_lock_step_hil_actuator_control_ms),
              [this] {
                return (received_actuator_controls_ || !connected_);
              })) {
        if ((SimClock::Get()->NowSimMicros() - last_actuator_time_) >=
            (connection_info_.timeout_lock_step_actuator_ms * 1000)) {
          // Timeout waiting for actuator control message--reset lockstep mode
          AddStatusMessage(
              "MavLinkApi::Update(): timeout waiting for actuator control "
              "message from controller--resetting lockstep");
          lock_step_active_ = false;
        } else {
          // Timeout waiting for actuator control message--note but proceed for
          // now
          AddStatusMessage(
              "MavLinkApi::Update(): Advancing sim another step without "
              "receiving a PX4 actuator update.");
        }
      }
    }
  } catch (std::exception& e) {
    AddStatusMessage("Exception sending messages to vehicle");
    AddStatusMessage(e.what());
    Disconnect();
    Connect();  // re-start a new connection so PX4 can be restarted and AirSim
                // will happily continue on.
  } catch (...) {
    AddStatusMessage("Unknown exception sending messages to vehicle.");
    Disconnect();
    Connect();  // re-start a new connection so PX4 can be restarted and AirSim
                // will happily continue on.
  }

  // must be done at the end
  if (was_reset_) was_reset_ = false;
}

void MavLinkApi::InitializeGimbalStatus(const Robot& robot) {
  auto& actuators = robot.GetActuators();
  static const mavlinkcom::MAV_COMPONENT gimbal_comp[6] = {
      mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL,
      mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL2,
      mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL3,
      mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL4,
      mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL5,
      mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL6};
  // Need to use more component ids if we want more gimbals
  // Gimbal compononent Ids

  int gimbal_idx = 0;

  // This for loop doesn't work as it is because one gimbal_node/connection can
  // only handle one gi peripheral (had to find out through testing - no
  // documentation). However, if we open more mavlink channels on PX4 - should
  // be easy make changes to make it work. There are currently limits on how mav
  // channels are allowed Supposed to be a max of 3 according to
  // https://docs.px4.io/main/en/peripherals/mavlink_peripherals.html, which we
  // are already at.
  for (auto& actuator_wrapper : actuators) {
    auto& actuator = actuator_wrapper.get();
    if (actuator.GetType() == ActuatorType::kGimbal) {
      auto& gimbal = static_cast<Gimbal&>(actuator);
      auto gimbal_id = gimbal.GetId();
      if (gimbal_idx >= 6) {
        throw std::runtime_error(
            "This flight controller does not support more than 6 gimbal "
            "devices.");
      }
      auto comp_id = static_cast<int>(gimbal_comp[gimbal_idx]);
      gimbal_id_to_component_id_.emplace(gimbal_id, comp_id);
      gimbal_component_id_to_state_.emplace(comp_id, GimbalState());
    }
  }
}

std::vector<float> MavLinkApi::GetControlSignals(const std::string& actuator_id) {
  if (!is_simulation_mode_) {
    throw std::logic_error(
        "Attempt to read motor controls while not in simulation mode");
  }

  auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);
  if (actuator_map_itr == actuator_id_to_output_idx_map_.end()) {
    GetLogger().LogWarning(
        GetControllerName(),
        "MavLinkApi::GetControlSignal() called for invalid actuator: %s",
        actuator_id.c_str());
    return std::vector<float>(1, 0.f);
  }

  std::lock_guard<std::mutex> guard(hil_controls_mutex_);
  return std::vector<float>(1, control_outputs_[actuator_map_itr->second]);
}

const IController::GimbalState& MavLinkApi::GetGimbalSignal(
    const std::string& gimbal_id) {
  auto gimbal_map_itr = gimbal_id_to_component_id_.find(gimbal_id);
  if (gimbal_map_itr == gimbal_id_to_component_id_.end()) {
    GetLogger().LogWarning(
        GetControllerName(),
        "MavLinkApi::GetGimbalSignal() called for invalid actuator: %s",
        gimbal_id.c_str());
    static auto gimbal = GimbalState();
    return gimbal;
  }
  auto comp_id = gimbal_map_itr->second;
  std::lock_guard<std::mutex> guard(gimbal_mutex_);
  auto gimbal_state_itr = gimbal_component_id_to_state_.find(comp_id);
  return gimbal_state_itr->second;
}

//---------------------------------------------------------------------------
// IMultirotorApi overrides

bool MavLinkApi::EnableApiControl() {
  GetLogger().LogTrace(GetControllerName(), "Api: EnableApiControl");

  // If we have a connection to the vehicle but not yet connected, give the
  // connection a chance to connect
  if (connecting_ || IsValidConnection()) {
    for (int secCountdown = 30;
         (secCountdown > 0) && (connecting_ || IsValidConnection()) &&
         !connected_;
         --secCountdown) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  CheckValidVehicle();
  mav_vehicle_->requestControl();
  is_api_control_enabled_ = true;
  return true;
}

bool MavLinkApi::DisableApiControl() {
  GetLogger().LogTrace(GetControllerName(), "Api: DisableApiControl");
  CheckValidVehicle();
  mav_vehicle_->releaseControl();
  is_api_control_enabled_ = false;
  return true;
}

bool MavLinkApi::IsApiControlEnabled() { return is_api_control_enabled_; }

bool MavLinkApi::Arm(int64_t command_start_time_nanos) {
  GetLogger().LogTrace(GetControllerName(), "Api: Arm");

  SingleCall lock(this);

  CheckValidVehicle();
  UpdateState();

  bool rc = false;
  float timeout_sec = 20;

  // Wait for PX4 to begin GPS fusion or vehicle movement will be denied and
  // require the vehicle to be disarmed and re-armed
  WaitForHomeLocation(timeout_sec, command_start_time_nanos);
  WaitForStableGroundPosition(timeout_sec, command_start_time_nanos);

  mav_vehicle_->armDisarm(true).wait(10000, &rc);
  return rc;
}

bool MavLinkApi::Disarm() {
  GetLogger().LogTrace(GetControllerName(), "Api: Disarm");
  SingleCall lock(this);

  CheckValidVehicle();
  bool rc = false;

  mav_vehicle_->armDisarm(false).wait(10000, &rc);
  return rc;
}

bool MavLinkApi::MoveToPosition(float x, float y, float z, float velocity,
                                float timeout_sec,
                                DrivetrainType /*drivetrain*/, bool yaw_is_rate,
                                float yaw, float /*lookahead*/,
                                float /*adaptive_lookahead*/,
                                int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);

  // unused(adaptive_lookahead);
  // unused(lookahead);
  // unused(drivetrain);

  // save current manual, cruise, and max velocity parameters before we or PX4
  // changes them during the move
  bool result = false;
  mavlinkcom::MavLinkParameter manual_velocity_parameter,
      cruise_velocity_parameter, max_velocity_parameter;
  result = mav_vehicle_->getParameter("MPC_VEL_MANUAL")
               .wait(1000, &manual_velocity_parameter);
  result = result && mav_vehicle_->getParameter("MPC_XY_CRUISE")
                         .wait(1000, &cruise_velocity_parameter);
  result = result && mav_vehicle_->getParameter("MPC_XY_VEL_MAX")
                         .wait(1000, &max_velocity_parameter);

  if (result) {
    // set velocity parameters
    mavlinkcom::MavLinkParameter p_manual;
    p_manual.name = "MPC_VEL_MANUAL";
    p_manual.value = velocity;
    mav_vehicle_->setParameter(p_manual).wait(1000, &result);
    mavlinkcom::MavLinkParameter p_cruise;
    p_cruise.name = "MPC_XY_CRUISE";
    p_cruise.value = velocity;
    mav_vehicle_->setParameter(p_cruise).wait(1000, &result);
    mavlinkcom::MavLinkParameter p_max;
    p_max.name = "MPC_XY_VEL_MAX";
    p_max.value = velocity;
    mav_vehicle_->setParameter(p_max).wait(1000, &result);

    if (result) {
      const Vector3f& goal_pos = Vector3f(x, y, z);
      Vector3f goal_dist_vect;
      float goal_dist;

      vehicle_apis::FunctionCaller function_caller(GetCommandPeriod(), timeout_sec,
                                     GetCancelToken(),
                                     command_start_time_nanos);

      while (!function_caller.IsComplete()) {
        goal_dist_vect = GetPosition() - goal_pos;
        const Vector3f& goal_normalized = goal_dist_vect.normalized();
        goal_dist = goal_dist_vect.dot(goal_normalized);

        if (goal_dist > GetDistanceAccuracy()) {
          MoveToPositionInternal(goal_pos, yaw_is_rate, yaw);

          // sleep for rest of the cycle
          if (!function_caller.WaitForInterval()) {
            GetLogger().LogWarning(GetControllerName(),
                                   "MoveToPosition: command timed out");
            result = false;
            break;
          }
        } else {
          function_caller.Complete();
        }
      }

      // restore manual, cruise, and max velocity parameters
      bool result_temp = false;
      mav_vehicle_->setParameter(manual_velocity_parameter).wait(1000, &result);
      mav_vehicle_->setParameter(cruise_velocity_parameter)
          .wait(1000, &result_temp);
      result = result && result_temp;
      mav_vehicle_->setParameter(max_velocity_parameter)
          .wait(1000, &result_temp);
      result = result && result_temp;

      return result && function_caller.IsComplete();
    }
  }

  return result;
}

bool MavLinkApi::Takeoff(float timeout_sec, int64_t command_start_time_nanos) {
  GetLogger().LogTrace(GetControllerName(), "Api: Takeoff");
  SingleCall lock(this);

  CheckValidVehicle();
  UpdateState();

  // TODO: Is this really needed to be done here?
  // WaitForHomeLocation(timeout_sec);
  WaitForStableGroundPosition(timeout_sec, command_start_time_nanos);

  bool rc = false;
  auto vec = GetPosition();
  auto yaw = current_state_.attitude.yaw;
  float z = vec.z() + GetTakeoffZ();
  if (!mav_vehicle_->takeoff(z, 0.0f /* pitch */, yaw)
           .wait(TimeoutToMilliseconds(timeout_sec), &rc)) {
    auto error_msg = "TakeOff command - timeout waiting for response";
    GetLogger().LogError(GetControllerName(), error_msg);
    throw std::logic_error(error_msg);
  }
  if (!rc) {
    auto error_msg = "TakeOff command rejected by drone";
    GetLogger().LogError(GetControllerName(), error_msg);
    throw std::logic_error(error_msg);
  }
  if (timeout_sec <= 0) return true;  // client doesn't want to wait.

  return WaitForZ(timeout_sec, z, GetDistanceAccuracy(),
                  command_start_time_nanos);
}

bool MavLinkApi::Land(float timeout_sec, int64_t command_start_time_nanos) {
  GetLogger().LogTrace(GetControllerName(), "Api: Land");
  SingleCall lock(this);

  // TODO: bugbug: really need a downward pointing distance to ground sensor to
  // do this properly, for now we assume the ground is relatively flat an we are
  // landing roughly at the home altitude.
  UpdateState();
  CheckValidVehicle();
  if (current_state_.home.is_set) {
    bool rc = false;
    if (!mav_vehicle_
             ->land(current_state_.global_est.pos.lat,
                    current_state_.global_est.pos.lon,
                    current_state_.home.global_pos.alt)
             .wait(10000, &rc)) {
      auto error_msg =
          "Landing command - timeout waiting for response from drone";
      GetLogger().LogError(GetControllerName(), error_msg);
      throw std::logic_error(error_msg);
    } else if (!rc) {
      auto error_msg = "Landing command rejected by drone";
      GetLogger().LogError(GetControllerName(), error_msg);
      throw std::logic_error(error_msg);
    }
  } else {
    throw std::logic_error(
        "Cannot land safely with out a home position that tells us the home "
        "altitude.  Could fix this if we hook up a distance to ground "
        "sensor...");
  }

  const auto& function_caller = RunFlightCommand(
      [&]() {
        UpdateState();
        return current_state_.controls.landed;
      },
      timeout_sec, command_start_time_nanos);

  // Wait for landed state (or user cancellation)
  if (!function_caller.IsComplete()) {
    throw std::logic_error("Drone hasn't reported a landing state");
  }
  return function_caller.IsComplete();
}

bool MavLinkApi::GoHome(float timeout_sec, float velocity,
                        int64_t command_start_time_nanos) {
  SingleCall lock(this);

  CheckValidVehicle();

  // save current manual, cruise, and max velocity parameters before we or PX4
  // changes them during the move
  bool result = false;
  mavlinkcom::MavLinkParameter manual_velocity_parameter,
      cruise_velocity_parameter, max_velocity_parameter;
  result = mav_vehicle_->getParameter("MPC_VEL_MANUAL")
               .wait(1000, &manual_velocity_parameter);
  result = result && mav_vehicle_->getParameter("MPC_XY_CRUISE")
                         .wait(1000, &cruise_velocity_parameter);
  result = result && mav_vehicle_->getParameter("MPC_XY_VEL_MAX")
                         .wait(1000, &max_velocity_parameter);

  if (result) {
    // set velocity parameters
    mavlinkcom::MavLinkParameter p_manual;
    p_manual.name = "MPC_VEL_MANUAL";
    p_manual.value = velocity;
    mav_vehicle_->setParameter(p_manual).wait(1000, &result);
    mavlinkcom::MavLinkParameter p_cruise;
    p_cruise.name = "MPC_XY_CRUISE";
    p_cruise.value = velocity;
    mav_vehicle_->setParameter(p_cruise).wait(1000, &result);
    mavlinkcom::MavLinkParameter p_max;
    p_max.name = "MPC_XY_VEL_MAX";
    p_max.value = velocity;
    mav_vehicle_->setParameter(p_max).wait(1000, &result);

    if (result) {
      bool rc = false;
      if (mav_vehicle_ != nullptr &&
          !mav_vehicle_->returnToHome().wait(TimeoutToMilliseconds(timeout_sec),
                                             &rc)) {
        throw std::logic_error(
            "goHome - timeout waiting for response from drone");
      }

      vehicle_apis::FunctionCaller function_caller(
          GetCommandPeriod(), timeout_sec,
                                     GetCancelToken(),
                                     command_start_time_nanos);

      UpdateState();
      const Vector3f& goal_pos = Vector3f(current_state_.home.local_pose.pos.x,
                                          current_state_.home.local_pose.pos.y,
                                          current_state_.home.local_pose.pos.z);
      Vector3f goal_dist_vect;
      float goal_dist;

      while (!function_caller.IsComplete()) {
        goal_dist_vect = GetPosition() - goal_pos;
        const Vector3f& goal_normalized = goal_dist_vect.normalized();
        goal_dist = goal_dist_vect.dot(goal_normalized);

        if (goal_dist > GetDistanceAccuracy()) {
          // sleep for rest of the cycle
          if (!function_caller.WaitForInterval()) {
            GetLogger().LogWarning(GetControllerName(),
                                   "GoHome: command timed out");
            result = false;
            break;
          }
        } else {
          function_caller.Complete();
        }
      }

      // restore manual, cruise, and max velocity parameters
      bool result_temp = false;
      mav_vehicle_->setParameter(manual_velocity_parameter).wait(1000, &result);
      mav_vehicle_->setParameter(cruise_velocity_parameter)
          .wait(1000, &result_temp);
      result = result && result_temp;
      mav_vehicle_->setParameter(max_velocity_parameter)
          .wait(1000, &result_temp);
      result = result && result_temp;
    }
  }

  return result;
}

bool MavLinkApi::Hover(int64_t /*command_start_time_nanos*/) {
  SingleCall lock(this);

  bool rc = false;
  CheckValidVehicle();
  mavlinkcom::AsyncResult<bool> result = mav_vehicle_->loiter();
  // auto start_time = std::chrono::system_clock::now();
  while (!GetCancelToken().IsCancelled()) {
    if (result.wait(100, &rc)) {
      break;
    }
  }
  return rc;
}

bool MavLinkApi::RequestControl(int64_t /*command_start_time_nanos*/) {
  SingleCall lock(this);

  CheckValidVehicle();
  mav_vehicle_->requestControl();

  return true;
}

bool MavLinkApi::SetMissionMode(int64_t /*command_start_time_nanos*/) {
  SingleCall lock(this);

  bool rc = false;
  CheckValidVehicle();
  mavlinkcom::AsyncResult<bool> result = mav_vehicle_->setMissionMode();
  while (!GetCancelToken().IsCancelled()) {
    if (result.wait(100, &rc)) {
      break;
    }
  }
  return rc;
}

bool MavLinkApi::SetVTOLMode(VTOLMode vtolmode) {
  SingleCall lock(this);

  bool rc = false;
  CheckValidVehicle();
  mavlinkcom::AsyncResult<bool> result =
      mav_vehicle_->setVTOLState(vtolmode == VTOLMode::FixedWing);
  while (!GetCancelToken().IsCancelled()) {
    if (result.wait(100, &rc)) {
      break;
    }
  }
  return rc;
}

// Switched to using service method request-response for all control commands,
// but leaving the below pub-sub version commented out for reference in case
// it's needed in the future.
// void MavLinkApi::OnSetpointNEDvelocityYawrate(const Topic& topic,
//                                               const Message&
//                                               message) {
//   throw std::logic_error("API is not supported for this controller type");
// }

//---------------------------------------------------------------------------
// Implementation for MultirotorApiBase

void MavLinkApi::CommandMotorPWMs(float front_right_pwm, float rear_left_pwm,
                                  float front_left_pwm, float rear_right_pwm) {
  throw std::logic_error("API is not supported by this controller type");
}

void MavLinkApi::CommandRollPitchYawZ(float roll, float pitch, float yaw,
                                      float z) {
  if (target_height_ != -z) {
    // these PID values were calculated experimentally using AltHoldCommand n
    // MavLinkTest, this provides the best control over thrust to achieve
    // minimal over/under shoot in a reasonable amount of time, but it has not
    // been tested on a real drone outside jMavSim, so it may need
    // recalibrating...
    thrust_controller_.setPoint(-z, .05f, .005f, 0.09f);
    target_height_ = -z;
  }
  CheckValidVehicle();
  auto state = mav_vehicle_->getVehicleState();
  float thrust = 0.21f + thrust_controller_.control(-state.local_est.pos.z);
  mav_vehicle_->moveByAttitude(roll, pitch, yaw, 0, 0, 0, thrust);
}

void MavLinkApi::CommandRollPitchYawThrottle(float roll, float pitch, float yaw,
                                             float throttle) {
  CheckValidVehicle();
  mav_vehicle_->moveByAttitude(roll, pitch, yaw, 0, 0, 0, throttle);
}

void MavLinkApi::CommandRollPitchYawrateThrottle(float roll, float pitch,
                                                 float yaw_rate,
                                                 float throttle) {
  CheckValidVehicle();
  mav_vehicle_->moveByAttitude(roll, pitch, 0, 0, 0, yaw_rate, throttle);
}

void MavLinkApi::CommandRollPitchYawrateZ(float roll, float pitch,
                                          float yaw_rate, float z) {
  if (target_height_ != -z) {
    thrust_controller_.setPoint(-z, .05f, .005f, 0.09f);
    target_height_ = -z;
  }
  CheckValidVehicle();
  auto state = mav_vehicle_->getVehicleState();
  float thrust = 0.21f + thrust_controller_.control(-state.local_est.pos.z);
  mav_vehicle_->moveByAttitude(roll, pitch, 0, 0, 0, yaw_rate, thrust);
}

void MavLinkApi::CommandAngleRatesZ(float roll_rate, float pitch_rate,
                                    float yaw_rate, float z) {
  if (target_height_ != -z) {
    thrust_controller_.setPoint(-z, .05f, .005f, 0.09f);
    target_height_ = -z;
  }
  CheckValidVehicle();
  auto state = mav_vehicle_->getVehicleState();
  float thrust = 0.21f + thrust_controller_.control(-state.local_est.pos.z);
  mav_vehicle_->moveByAttitude(0, 0, 0, roll_rate, pitch_rate, yaw_rate,
                               thrust);
}

void MavLinkApi::CommandAngleRatesThrottle(float roll_rate, float pitch_rate,
                                           float yaw_rate, float throttle) {
  CheckValidVehicle();
  mav_vehicle_->moveByAttitude(0, 0, 0, roll_rate, pitch_rate, yaw_rate,
                               throttle);
}

void MavLinkApi::CommandHeading(float heading, float speed, float vz) {
  CheckValidVehicle();
  auto vx = sin(heading) * speed;
  auto vy = cos(heading) * speed;
  mav_vehicle_->moveByLocalVelocity(vx, vy, vz, true, heading);
}

void MavLinkApi::CommandVelocity(float vx, float vy, float vz, bool yaw_is_rate,
                                 float yaw) {
  CheckValidVehicle();
  mav_vehicle_->moveByLocalVelocity(vx, vy, vz, !yaw_is_rate, yaw);
}

void MavLinkApi::CommandVelocityZ(float vx, float vy, float z, bool yaw_is_rate,
                                  float yaw) {
  CheckValidVehicle();
  mav_vehicle_->moveByLocalVelocityWithAltHold(vx, vy, z, !yaw_is_rate, yaw);
}

void MavLinkApi::CommandVelocityBody(float vx, float vy, float vz,
                                     bool yaw_is_rate, float yaw_world) {
  // TO-DO: Set body frame for PX4 controller
  CheckValidVehicle();
  mav_vehicle_->moveByLocalVelocity(vx, vy, vz, !yaw_is_rate, yaw_world);
}

void MavLinkApi::CommandVelocityZBody(float vx, float vy, float z,
                                      bool yaw_is_rate, float yaw_world) {
  // TO-DO: Set body frame for PX4 controller
  CheckValidVehicle();
  mav_vehicle_->moveByLocalVelocityWithAltHold(vx, vy, z, !yaw_is_rate,
                                               yaw_world);
}

void MavLinkApi::CommandPosition(float x, float y, float z, bool yaw_is_rate,
                                 float yaw) {
  CheckValidVehicle();
  mav_vehicle_->moveToLocalPosition(x, y, z, !yaw_is_rate, yaw);
}

const MultirotorApiBase::MultirotorApiParams&
MavLinkApi::GetMultirotorApiParams() const {
  // return safety_params_;

  // defaults are good for PX4 generic quadcopter.
  static const MultirotorApiParams vehicle_params;
  return vehicle_params;
}

void MavLinkApi::SetControllerGains(uint8_t controller_type,
                                    const std::vector<float>& kp,
                                    const std::vector<float>& ki,
                                    const std::vector<float>& kd) {
  throw std::logic_error("API is not supported by this controller type");
}

void MavLinkApi::BeforeTask() { StartOffboardMode(); }
void MavLinkApi::AfterTask() { EndOffboardMode(); }

Kinematics MavLinkApi::GetKinematicsEstimated() const {
  UpdateState();
  Kinematics state;
  state.pose.position =
      Vector3(current_state_.local_est.pos.x, current_state_.local_est.pos.y,
              current_state_.local_est.pos.z);
  // state.pose.orientation = VectorMath::toQuaternion(
  //    current_state_.attitude.pitch, current_state_.attitude.roll,
  //    current_state_.attitude.yaw);
  state.pose.orientation = TransformUtils::ToQuaternion(
      current_state_.attitude.roll, current_state_.attitude.pitch,
      current_state_.attitude.yaw);
  state.twist.linear = Vector3(current_state_.local_est.lin_vel.x,
                               current_state_.local_est.lin_vel.y,
                               current_state_.local_est.lin_vel.z);
  state.twist.angular = Vector3(current_state_.attitude.roll_rate,
                                current_state_.attitude.pitch_rate,
                                current_state_.attitude.yaw_rate);
  state.accels.linear =
      Vector3(current_state_.local_est.acc.x, current_state_.local_est.acc.y,
              current_state_.local_est.acc.z);
  // TODO: how do we get angular acceleration?
  return state;
}

Vector3 MavLinkApi::GetPosition() const {
  UpdateState();
  return Vector3(current_state_.local_est.pos.x, current_state_.local_est.pos.y,
                 current_state_.local_est.pos.z);
}

Vector3 MavLinkApi::GetVelocity() const {
  UpdateState();
  return Vector3(current_state_.local_est.lin_vel.x,
                 current_state_.local_est.lin_vel.y,
                 current_state_.local_est.lin_vel.z);
}

Quaternion MavLinkApi::GetOrientation() const {
  UpdateState();

  return TransformUtils::ToQuaternion(current_state_.attitude.roll,
                                      current_state_.attitude.pitch,
                                      current_state_.attitude.yaw);
}

LandedState MavLinkApi::GetLandedState() const {
  UpdateState();

  return current_state_.controls.landed ? LandedState::Landed
                                        : LandedState::Flying;
}

GeoPoint MavLinkApi::GetGpsLocationEstimated() const {
  UpdateState();

  GeoPoint location;
  location.altitude = current_state_.global_est.pos.alt;
  location.latitude = current_state_.global_est.pos.lat;
  location.longitude = current_state_.global_est.pos.lon;
  return location;
}

float MavLinkApi::GetCommandPeriod() const {
  return 1.0f / 50;  // 50hz
}

float MavLinkApi::GetTakeoffZ() const {
  // PX4's minimum take off altitude is 10.0m. Set negative due to NED
  // coordinate system.
  return -10.0f;
}

float MavLinkApi::GetDistanceAccuracy() const {
  return 0.5f;  // measured in simulator by firing commands "MoveToLocation -x 0
                // -y 0" multiple times and looking at distance traveled
}

//---------------------------------------------------------------------------
// Helper methods

void MavLinkApi::InitializeConnections() {
  try {
    OpenAllConnections();
    is_ready_ = true;
  } catch (std::exception& ex) {
    is_ready_ = false;
    is_ready_message_ =
        GetLogger().FormatMessage("Failed to connect: %s", ex.what());
  } catch (...) {
    is_ready_ = false;
    is_ready_message_ =
        GetLogger().FormatMessage("Failed to connect with unknown exception.");
  }
}

void MavLinkApi::OpenAllConnections() {
  Disconnect();  // just in case if connections were open
  ResetState();  // reset all variables we might have changed during last
                 // session

  Connect();
}

void MavLinkApi::CloseAllConnections() { Disconnect(); }

void MavLinkApi::Connect() {
  if (!connecting_) {
    connecting_ = true;
    if (this->connect_thread_.joinable()) {
      this->connect_thread_.join();
    }
    this->connect_thread_ = std::thread(&MavLinkApi::ConnectThread, this);
    GetLogger().LogTrace(GetControllerName(), "Created connect thread");
  }
}

void MavLinkApi::Disconnect() {
  AddStatusMessage("Disconnecting mavlink vehicle");
  connected_ = false;
  connected_vehicle_ = false;
  if (connection_ != nullptr) {
    if (is_hil_mode_set_ && mav_vehicle_ != nullptr) {
      SetNormalMode();
    }

    connection_->close();
  }

  if (hil_node_ != nullptr) {
    hil_node_->close();
  }

  if (mav_vehicle_ != nullptr) {
    mav_vehicle_->getConnection()->stopLoggingSendMessage();
    mav_vehicle_->close();
    mav_vehicle_ = nullptr;
  }

  if (gimbal_node_ != nullptr) {
    gimbal_node_->getConnection()->stopLoggingSendMessage();
    gimbal_node_->close();
    gimbal_node_ = nullptr;
  }

  // if (video_server_ != nullptr) video_server_->close();

  // if (logviewer_proxy_ != nullptr) {
  //   logviewer_proxy_->close();
  //   logviewer_proxy_ = nullptr;
  // }

  // if (logviewer_out_proxy_ != nullptr) {
  //   logviewer_out_proxy_->close();
  //   logviewer_out_proxy_ = nullptr;
  // }

  if (qgc_proxy_ != nullptr) {
    qgc_proxy_->close();
    qgc_proxy_ = nullptr;
  }
}

void MavLinkApi::ConnectThread() {
  GetLogger().LogTrace(GetControllerName(), "ConnectThread is running");
  AddStatusMessage("Waiting for mavlink vehicle...");
  CreateMavConnection(connection_info_);
  if (mav_vehicle_ != nullptr) {
    // ConnectToLogViewer();
    ConnectToQGC();
  }
  connecting_ = false;
  connected_ = true;
}

void MavLinkApi::ResetState() {
  // reset state
  is_hil_mode_set_ = false;
  vehicle_type_ = mavlinkcom::MAV_TYPE::MAV_TYPE_GENERIC;
  hil_state_freq_ = -1;
  actuators_message_supported_ = false;
  state_version_ = 0;
  current_state_ = mavlinkcom::VehicleState();
  target_height_ = 0;
  sim_time_us_ = 0;
  last_actuator_time_ = 0;
  last_gps_time_ = 0;
  last_update_time_ = 0;
  last_hil_sensor_time_ = 0;
  is_api_control_enabled_ = false;
  thrust_controller_ = PidController(); // TODO: Review: To introduce an aditional PID loop to comunicate with PX4 doesn´t seem correct.
  std::fill(std::begin(control_outputs_), std::end(control_outputs_), 0.0f);
  was_reset_ = false;
  received_actuator_controls_ = false;
  lock_step_active_ = false;
  lock_step_enabled_ = connection_info_.lock_step;
  has_gps_lock_ = false;
  send_params_ = false;
  // TODO: mocap_pose_ = Pose::nanPose();
  ground_variance_ = 1;
  ground_filter_.initialize(25, 0.1f);
  CancelLastTask();

  first_message_sent_ = false;
  first_gps_message_sent_ = false;
  got_first_actuator_control_ = false;
}

// put PX4 in normal mode (i.e. non-simulation mode)
void MavLinkApi::SetNormalMode() {
  if (is_hil_mode_set_ && connection_ != nullptr && mav_vehicle_ != nullptr) {
    GetLogger().LogTrace(
        GetControllerName(),
        "SetNormalMode: putting PX4 in non-simulation mode via cmd");
    // remove MAV_MODE_FLAG_HIL_ENABLED flag from current mode
    std::lock_guard<std::mutex> guard(set_mode_mutex_);
    int mode = mav_vehicle_->getVehicleState().mode;
    mode &=
        ~static_cast<int>(mavlinkcom::MAV_MODE_FLAG::MAV_MODE_FLAG_HIL_ENABLED);

    mavlinkcom::MavCmdDoSetMode cmd;
    cmd.command =
        static_cast<uint16_t>(mavlinkcom::MAV_CMD::MAV_CMD_DO_SET_MODE);
    cmd.Mode = static_cast<float>(mode);
    mav_vehicle_->sendCommand(cmd);

    is_hil_mode_set_ = false;
  }
}

// put PX4 in simulation mode
void MavLinkApi::SetHILMode() {
  GetLogger().LogTrace(GetControllerName(),
                       "SetHILMode: putting PX4 in simulation mode via cmd");
  if (!is_simulation_mode_)
    throw std::logic_error(
        "Attempt to set device in HIL mode while not in simulation mode");

  CheckValidVehicle();

  // add MAV_MODE_FLAG_HIL_ENABLED flag to current mode
  std::lock_guard<std::mutex> guard(set_mode_mutex_);
  int mode = mav_vehicle_->getVehicleState().mode;
  mode |=
      static_cast<int>(mavlinkcom::MAV_MODE_FLAG::MAV_MODE_FLAG_HIL_ENABLED);

  mavlinkcom::MavCmdDoSetMode cmd;
  cmd.command = static_cast<uint16_t>(mavlinkcom::MAV_CMD::MAV_CMD_DO_SET_MODE);
  cmd.Mode = static_cast<float>(mode);
  mav_vehicle_->sendCommand(cmd);

  is_hil_mode_set_ = true;
}

void MavLinkApi::AddStatusMessage(const std::string& message) {
  if (message.size() != 0) {
    GetLogger().LogTrace(GetControllerName(), message.c_str());
    std::lock_guard<std::mutex> guard_status(status_text_mutex_);
    // if queue became too large, clear it first
    // TODO: if (status_messages_.size() > status_messages_MaxSize)
    //  Utils::clear(status_messages_,
    //               status_messages_MaxSize - status_messages_.size());
    status_messages_.push(message);
  }
}

std::string MavLinkApi::FindPX4() {
  auto result = mavlinkcom::MavLinkConnection::findSerialPorts(0, 0);
  for (auto iter = result.begin(); iter != result.end(); iter++) {
    mavlinkcom::SerialPortInfo info = *iter;
    if (((info.vid == pixhawkVendorId) &&
         (info.pid == pixhawkFMUV5ProductId ||
          info.pid == pixhawkFMUV4ProductId ||
          info.pid == pixhawkFMUV2ProductId ||
          info.pid == pixhawkFMUV2OldBootloaderProductId)) ||
        ((info.displayName.find(L"PX4_") != std::string::npos))) {
      // printf("Auto Selecting COM port: %S\n", info.displayName.c_str());

      std::string portName_str;

      for (wchar_t ch : info.portName) {
        portName_str.push_back(static_cast<char>(ch));
      }
      return portName_str;
    }
  }
  return "";
}

void MavLinkApi::CreateMavConnection(
    const MavLinkConnectionInfo& connection_info) {
  if (connection_info.use_serial) {
    CreateMavSerialConnection(connection_info.serial_port,
                              connection_info.baud_rate);
  } else {
    CreateMavEthernetConnection(connection_info);
  }
}

void MavLinkApi::CreateMavEthernetConnection(
    const MavLinkConnectionInfo& connection_info) {
  GetLogger().LogTrace(GetControllerName(),
                       "Attempting to create Ethernet connection");
  Disconnect();

  got_first_heartbeat_ = false;
  is_hil_mode_set_ = false;
  is_armed_ = false;
  has_home_ = false;
  vehicle_type_ = mavlinkcom::MAV_TYPE::MAV_TYPE_GENERIC;
  std::string remoteIpAddr;
  std::fill(std::begin(control_outputs_), std::end(control_outputs_), 0.0f);

  if (connection_info.use_tcp) {
    if (connection_info.tcp_port == 0) {
      throw std::invalid_argument("TcpPort setting has an invalid value.");
    }

    auto msg = GetLogger().FormatMessage(
        "Waiting for TCP connection on port %d, local IP %s",
        connection_info.tcp_port, connection_info_.local_host_ip.c_str());
    AddStatusMessage(msg);
    try {
      connection_ = std::make_shared<mavlinkcom::MavLinkConnection>();

      // Attach handler for simulator channel
      GetLogger().LogTrace(
          GetControllerName(),
          "createMavEthernetConnection: Subscribing processMavMessages for "
          "messages on hil_node_ connection");
      connection_->subscribe(
          [=](std::shared_ptr<mavlinkcom::MavLinkConnection> connection,
              const mavlinkcom::MavLinkMessage& msg) {
            // unused(connection);
            ProcessMavMessages(msg);
          });

      // Start listening for connections from PX4 on the simulator channel
      remoteIpAddr = connection_->acceptTcp(
          "hil", connection_info_.local_host_ip, connection_info.tcp_port);
    } catch (std::exception& e) {
      AddStatusMessage(
          "Accepting TCP socket failed, is another instance running?");
      AddStatusMessage(e.what());
      return;
    } catch (...) {
      AddStatusMessage(
          "Accepting TCP socket failed, is another instance running?");
      return;
    }
  } else if (connection_info.udp_address.size() > 0) {
    if (connection_info.udp_port == 0) {
      throw std::invalid_argument("UdpPort setting has an invalid value.");
    }

    connection_ = mavlinkcom::MavLinkConnection::connectRemoteUdp(
        "hil", connection_info_.local_host_ip, connection_info.udp_address,
        connection_info.udp_port);
  } else {
    throw std::invalid_argument(
        "Please provide valid connection info for your drone.");
  }

  hil_node_ = std::make_shared<mavlinkcom::MavLinkNode>(
      connection_info_.sim_sysid, connection_info_.sim_compid);
  hil_node_->connect(connection_);

  if (connection_info.use_tcp) {
    AddStatusMessage(std::string("Connected to SITL over TCP."));
  } else {
    AddStatusMessage(std::string("Connected to SITL over UDP."));
  }

  mav_vehicle_ = std::make_shared<mavlinkcom::MavLinkVehicle>(
      connection_info_.vehicle_sysid, connection_info_.vehicle_compid);

  if (connection_info_.control_ip_address != "") {
    if (connection_info_.control_port_local == 0) {
      throw std::invalid_argument("ControlPort setting has an invalid value.");
    }
    if (!connection_info.use_tcp ||
        connection_info_.control_ip_address != "remote") {
      remoteIpAddr = connection_info_.control_ip_address;
    }
    if (remoteIpAddr == "local" || remoteIpAddr == "localhost") {
      remoteIpAddr = "127.0.0.1";
    }

    // The PX4 SITL mode app cannot receive commands to control the drone over
    // the same HIL mavlink connection. The HIL mavlink connection can only
    // handle HIL_SENSOR messages.  This separate channel is needed for
    // everything else.
    AddStatusMessage(GetLogger().FormatMessage(
        "Connecting to PX4 Control UDP port %d, local IP %s, remote IP...",
        connection_info_.control_port_local,
        connection_info_.local_host_ip.c_str(),
        connection_info_.control_ip_address.c_str()));

    // if we try and connect the UDP port too quickly it doesn't work, bug in
    // PX4 ?
    for (int retries = 60; retries >= 0 && connecting_; retries--) {
      try {
        std::shared_ptr<mavlinkcom::MavLinkConnection> gcsConnection;
        if (remoteIpAddr == "127.0.0.1") {
          gcsConnection = mavlinkcom::MavLinkConnection::connectLocalUdp(
              "gcs", connection_info_.local_host_ip,
              connection_info_.control_port_local);
        } else {
          gcsConnection = mavlinkcom::MavLinkConnection::connectRemoteUdp(
              "gcs", connection_info_.local_host_ip, remoteIpAddr,
              connection_info_.control_port_remote);
        }
        mav_vehicle_->connect(gcsConnection);
        // need to try and send something to make sure the connection is good.
        mav_vehicle_->setMessageInterval(
            mavlinkcom::MavLinkHomePosition::kMessageId, 1);
        break;
      } catch (std::exception& e) {
        AddStatusMessage("Error connecting UDP port: ");
        AddStatusMessage(e.what());
        AddStatusMessage("Waiting 1 sec to try again.");
        std::this_thread::sleep_for(std::chrono::seconds(1));
      } catch (...) {
        AddStatusMessage("Unknown exception while connecting UDP port.");
        AddStatusMessage("Waiting 1 sec to try again.");
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }

    if (mav_vehicle_ == nullptr) {
      // play was stopped!
      return;
    }

    if (mav_vehicle_->getConnection() != nullptr) {
      AddStatusMessage(std::string("Ground control connected over UDP."));
    } else {
      AddStatusMessage(
          std::string("Timeout trying to connect ground control over UDP."));
      return;
    }
  }

  try {
    ConnectVehicle();
  } catch (std::exception& e) {
    AddStatusMessage("Error connecting vehicle:");
    AddStatusMessage(e.what());
  } catch (...) {
    AddStatusMessage("Unknown exception while connecting vehicle.");
  }
  SetupGimbalConnection(remoteIpAddr);
}

void MavLinkApi::SetupGimbalConnection(const std::string& remoteIpAddr) {
  if (!enable_gimbal_) {
    return;
  }

  gimbal_node_ = std::make_shared<mavlinkcom::MavLinkNode>(
      1, static_cast<int>(mavlinkcom::MAV_COMPONENT::MAV_COMP_ID_GIMBAL));
  std::shared_ptr<mavlinkcom::MavLinkConnection> gimbalConnection;
  AddStatusMessage("Creating connection for gimbal");
  for (int retries = 60; retries >= 0 && connecting_; retries--) {
    try {
      if (remoteIpAddr == "127.0.0.1") {
        AddStatusMessage("Setting up local udp for local gimbal");
        gimbalConnection = mavlinkcom::MavLinkConnection::connectLocalUdp(
            "gimbal", connection_info_.local_host_ip,
            connection_info_.gimbal_port_local);
      } else {
        AddStatusMessage("Setting up remote udp for gimbal");
        gimbalConnection = mavlinkcom::MavLinkConnection::connectRemoteUdp(
            "gimbal", connection_info_.local_host_ip, remoteIpAddr,
            connection_info_.gimbal_port_remote);
      }
      gimbal_node_->connect(gimbalConnection);

      mavlinkcom::MavLinkHeartbeat heartbeat;
      heartbeat.autopilot = static_cast<uint8_t>(
          mavlinkcom::MAV_AUTOPILOT::MAV_AUTOPILOT_INVALID);
      heartbeat.type =
          static_cast<uint8_t>(mavlinkcom::MAV_TYPE::MAV_TYPE_GIMBAL);
      heartbeat.mavlink_version = 3;
      heartbeat.base_mode = 0;
      heartbeat.custom_mode = 0;
      heartbeat.system_status =
          static_cast<uint8_t>(mavlinkcom::MAV_STATE::MAV_STATE_UNINIT);
      gimbal_node_->startHeartbeat(heartbeat);

      AddStatusMessage("Got Connection");
      // need to try and send something to make sure the connection is good.
      AddStatusMessage("Sent random test message Gimbal");
      break;
    } catch (std::exception& e) {
      AddStatusMessage("Error connecting UDP port: ");
      AddStatusMessage(e.what());
      AddStatusMessage("Waiting 1 sec to try again.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (...) {
      AddStatusMessage("Unknown exception while connecting UDP port.");
      AddStatusMessage("Waiting 1 sec to try again.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  gimbalConnection->subscribe(
      [=](std::shared_ptr<mavlinkcom::MavLinkConnection> connection_val,
          const mavlinkcom::MavLinkMessage& msg) {
        // unused(connection_val);
        HandleGimbalMessages(msg);
      });
}

void MavLinkApi::HandleGimbalMessages(const mavlinkcom::MavLinkMessage& msg) {
  if (msg.msgid == CommandLongMessage.msgid) {
    if (CommandLongMessage.command ==
        static_cast<int>(mavlinkcom::MAV_CMD::MAV_CMD_REQUEST_MESSAGE)) {
      int msg_id = static_cast<int>(CommandLongMessage.param1 + 0.5);
      if (msg_id ==
          static_cast<int>(mavlinkcom::MavLinkMessageIds::
                               MAVLINK_MSG_ID_GIMBAL_DEVICE_INFORMATION)) {
        // Send Gimbal_Device_information for each gimbal in our configuration
        // Place holder for now that sends the default struct
        AddStatusMessage("Received Gimbal discovery message, responding now.");
        auto ack = mavlinkcom::MavLinkCommandAck();
        ack.command = CommandLongMessage.command;
        ack.result =
            static_cast<uint8_t>(mavlinkcom::MAV_RESULT::MAV_RESULT_ACCEPTED);
        gimbal_node_->sendMessage(ack);

        SendGimbalDeviceInformation();
        setup_gimbal_ = true;
      }
    }
  } else if (msg.msgid ==
             mavlinkcom::MavLinkGimbalDeviceSetAttitude::kMessageId) {
    GimbalDeviceSetAttitude.decode(msg);
    auto comp_id = GimbalDeviceSetAttitude.target_component;
    auto gimbal_state_itr = gimbal_component_id_to_state_.find(comp_id);
    auto current_gimbal_state = &gimbal_state_itr->second;
    auto quat =
        Quaternion(GimbalDeviceSetAttitude.q[0], GimbalDeviceSetAttitude.q[1],
                   GimbalDeviceSetAttitude.q[2], GimbalDeviceSetAttitude.q[3]);
    auto rpy = TransformUtils::ToRPY(quat);
    {
      std::lock_guard<std::mutex> guard(gimbal_mutex_);
      current_gimbal_state->roll = rpy[0];
      current_gimbal_state->pitch = rpy[1];
      current_gimbal_state->yaw = rpy[2];

      current_gimbal_state->roll_vel =
          GimbalDeviceSetAttitude.angular_velocity_x;
      current_gimbal_state->pitch_vel =
          GimbalDeviceSetAttitude.angular_velocity_y;
      current_gimbal_state->yaw_vel =
          GimbalDeviceSetAttitude.angular_velocity_z;

      current_gimbal_state->roll_lock =
          (GimbalDeviceSetAttitude.flags &
           static_cast<uint16_t>(
               mavlinkcom::GIMBAL_DEVICE_FLAGS::GIMBAL_DEVICE_FLAGS_ROLL_LOCK));
      current_gimbal_state->pitch_lock =
          (GimbalDeviceSetAttitude.flags &
           static_cast<uint16_t>(mavlinkcom::GIMBAL_DEVICE_FLAGS::
                                     GIMBAL_DEVICE_FLAGS_PITCH_LOCK));
      current_gimbal_state->yaw_lock =
          (GimbalDeviceSetAttitude.flags &
           static_cast<uint16_t>(
               mavlinkcom::GIMBAL_DEVICE_FLAGS::GIMBAL_DEVICE_FLAGS_YAW_LOCK));
    }
  } else if (msg.msgid ==
             mavlinkcom::MavLinkAutopilotStateForGimbalDevice::kMessageId) {
    // Not sure what to do with Autopilot Message since it provides a few extra
    // niceties we don't currently implement
  }
}

void MavLinkApi::ConnectVehicle() {
  // listen to this UDP mavlink connection also
  auto mavcon = mav_vehicle_->getConnection();
  if (mavcon != connection_) {
    GetLogger().LogTrace(
        GetControllerName(),
        "connectVehicle: Subscribing processMavMessages for messages on "
        "mav_vehicle connection");
    mavcon->subscribe(
        [=](std::shared_ptr<mavlinkcom::MavLinkConnection> connection,
            const mavlinkcom::MavLinkMessage& msg) {
          // unused(connection);
          ProcessMavMessages(msg);
        });
  } else {
    mav_vehicle_->connect(connection_);
  }

  connected_ = true;

  // Also request home position messages
  mav_vehicle_->setMessageInterval(mavlinkcom::MavLinkHomePosition::kMessageId,
                                   1);

  // now we can start our heartbeats.
  GetLogger().LogTrace(GetControllerName(), "Starting heartbeat");
  mavlinkcom::MavLinkHeartbeat heartbeat;
  heartbeat.autopilot =
      static_cast<uint8_t>(mavlinkcom::MAV_AUTOPILOT::MAV_AUTOPILOT_GENERIC);
  heartbeat.type = static_cast<uint8_t>(mavlinkcom::MAV_TYPE::MAV_TYPE_GCS);
  heartbeat.mavlink_version = 3;
  heartbeat.base_mode = 0;      // ignored by PX4
  heartbeat.custom_mode = 0;    // ignored by PX4
  heartbeat.system_status = 0;  // ignored by PX4
  mav_vehicle_->startHeartbeat(heartbeat);

  // wait for px4 to connect so we can safely send any configured parameters
  while (!send_params_ && connected_) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (connected_) {
    if (send_params_) {
      // GetLogger().LogTrace(GetControllerName(), "Sending params");
      send_params_ = false;
      SendParams();
    }

    connected_vehicle_ = true;
  }
}

void MavLinkApi::CreateMavSerialConnection(const std::string& port_name,
                                           int baud_rate) {
  GetLogger().LogTrace(GetControllerName(),
                       "Attempting to create serial connection");

  Disconnect();

  bool reported = false;
  std::string port_name_auto = port_name;
  while (port_name_auto == "" || port_name_auto == "*") {
    port_name_auto = FindPX4();
    if (port_name_auto == "") {
      if (!reported) {
        reported = true;
        AddStatusMessage(
            "Could not detect a connected PX4 flight controller on any USB "
            "ports.");
        AddStatusMessage("You can specify USB port in settings.json.");
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  if (port_name_auto == "") {
    AddStatusMessage(
        "USB port for PX4 flight controller is empty. Please set it in "
        "settings.json.");
    return;
  }

  if (baud_rate == 0) {
    AddStatusMessage(
        "Baud rate specified in settings.json is 0 which is invalid");
    return;
  }

  AddStatusMessage(GetLogger().FormatMessage(
      "Connecting to PX4 over serial port: %s, baud rate %d ....",
      port_name_auto.c_str(), baud_rate));
  reported = false;

  while (connecting_) {
    try {
      connection_ = mavlinkcom::MavLinkConnection::connectSerial(
          "hil", port_name_auto, baud_rate);
      connection_->ignoreMessage(
          mavlinkcom::MavLinkAttPosMocap::
              kMessageId);  // TODO: find better way to communicate debug pose
                            // instead of using fake Mo-cap messages
      hil_node_ = std::make_shared<mavlinkcom::MavLinkNode>(
          connection_info_.sim_sysid, connection_info_.sim_compid);
      hil_node_->connect(connection_);
      AddStatusMessage(GetLogger().FormatMessage(
          "Connected to PX4 over serial port: %s", port_name_auto.c_str()));

      mav_vehicle_ = std::make_shared<mavlinkcom::MavLinkVehicle>(
          connection_info_.vehicle_sysid, connection_info_.vehicle_compid);
      mav_vehicle_->connect(
          connection_);  // in this case we can use the same connection.
      GetLogger().LogTrace(GetControllerName(),
                           "Start sending heartbeat to PX4 every 1 sec");
      mavlinkcom::MavLinkHeartbeat heartbeat;
      heartbeat.autopilot = static_cast<uint8_t>(
          mavlinkcom::MAV_AUTOPILOT::MAV_AUTOPILOT_GENERIC);
      heartbeat.type = static_cast<uint8_t>(mavlinkcom::MAV_TYPE::MAV_TYPE_GCS);
      heartbeat.mavlink_version = 3;
      heartbeat.base_mode = 0;      // ignored by PX4
      heartbeat.custom_mode = 0;    // ignored by PX4
      heartbeat.system_status = 0;  // ignored by PX4
      mav_vehicle_->startHeartbeat(heartbeat);
      // start listening to the HITL connection.
      connection_->subscribe(
          [=](std::shared_ptr<mavlinkcom::MavLinkConnection> connection,
              const mavlinkcom::MavLinkMessage& msg) {
            // unused(connection);
            ProcessMavMessages(msg);
          });

      mav_vehicle_->setMessageInterval(
          mavlinkcom::MavLinkHomePosition::kMessageId, 1);

      return;
    } catch (std::exception& e) {
      if (!reported) {
        reported = true;
        AddStatusMessage("Error connecting to mavlink vehicle.");
        AddStatusMessage(e.what());
        AddStatusMessage("Please check your USB port in settings.json.");
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (...) {
      if (!reported) {
        reported = true;
        AddStatusMessage(
            "Unknown exception while connecting to mavlink vehicle.");
        AddStatusMessage("Please check your USB port in settings.json.");
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

bool MavLinkApi::ConnectToQGC() {
  if (connection_info_.qgc_ip_address.size() > 0) {
    std::shared_ptr<mavlinkcom::MavLinkConnection> connection;
    CreateProxy("QGC", connection_info_.qgc_ip_address,
                connection_info_.qgc_ip_port, connection_info_.local_host_ip,
                qgc_proxy_, connection);
    if (!SendTestMessage(qgc_proxy_)) {
      // error talking to QGC, so don't keep trying, and close the connection
      // also.
      qgc_proxy_->getConnection()->close();
      qgc_proxy_ = nullptr;
    } else {
      connection->subscribe(
          [=](std::shared_ptr<mavlinkcom::MavLinkConnection> connection_val,
              const mavlinkcom::MavLinkMessage& msg) {
            // unused(connection_val);
            ProcessQgcMessages(msg);
          });
    }
  }
  return qgc_proxy_ != nullptr;
}

// bool MavLinkApi::ConnectToLogViewer() {
//   // set up logviewer proxy
//   if (connection_info_.logviewer_ip_address.size() > 0) {
//     std::shared_ptr<mavlinkcom::MavLinkConnection> connection;
//     CreateProxy("LogViewer", connection_info_.logviewer_ip_address,
//                 connection_info_.logviewer_ip_port,
//                 connection_info_.local_host_ip, logviewer_proxy_,
//                 connection);
//     if (!SendTestMessage(logviewer_proxy_)) {
//       // error talking to log viewer, so don't keep trying, and close the
//       // connection also.
//       logviewer_proxy_->getConnection()->close();
//       logviewer_proxy_ = nullptr;
//     }

//     std::shared_ptr<mavlinkcom::MavLinkConnection> out_connection;
//     CreateProxy("LogViewerOut", connection_info_.logviewer_ip_address,
//                 connection_info_.logviewer_ip_sport,
//                 connection_info_.local_host_ip, logviewer_out_proxy_,
//                 out_connection);
//     if (!SendTestMessage(logviewer_out_proxy_)) {
//       // error talking to log viewer, so don't keep trying, and close the
//       // connection also.
//       logviewer_out_proxy_->getConnection()->close();
//       logviewer_out_proxy_ = nullptr;
//     } else if (mav_vehicle_ != nullptr) {
//       // TODO:
//       // mav_vehicle_->getConnection()->startLoggingSendMessage(
//       //    std::make_shared<MavLinkLogViewerLog>(logviewer_out_proxy_));
//     }
//   }
//   return logviewer_proxy_ != nullptr;
// }

void MavLinkApi::CreateProxy(
    std::string name, std::string ip, int port, std::string local_host_ip,
    std::shared_ptr<mavlinkcom::MavLinkNode>& node,
    std::shared_ptr<mavlinkcom::MavLinkConnection>& connection) {
  if (connection_ == nullptr)
    throw std::domain_error(
        "MavLinkMultirotorApi requires connection object to be set before "
        "createProxy call");

  connection = mavlinkcom::MavLinkConnection::connectRemoteUdp(
      "Proxy to: " + name + " at " + ip + ":" + std::to_string(port),
      local_host_ip, ip, port);

  // it is ok to reuse the simulator sysid and compid here because this node is
  // only used to send a few messages directly to this endpoint and all other
  // messages are funneled through from PX4 via the Join method below.
  node = std::make_shared<mavlinkcom::MavLinkNode>(connection_info_.sim_sysid,
                                                   connection_info_.sim_compid);
  node->connect(connection);

  // now join the main connection to this one, this causes all PX4 messages to
  // be sent to the proxy and all messages from the proxy will be send directly
  // to the PX4 (using whatever sysid/compid comes from that remote node).
  connection_->join(connection);

  auto mavcon = mav_vehicle_->getConnection();
  if (mavcon != connection_) {
    mavcon->join(connection);
  }
}

// simulation time handling
void MavLinkApi::AdvanceSimTime(void) {
  sim_time_us_ = SimClock::Get()->NowSimMicros();
}

TimeMicro MavLinkApi::GetSimTimeMicros(void) {
  // This ensures HIL_SENSOR and HIL_GPS have matching clocks.
  if (lock_step_active_) {
    if (sim_time_us_ == 0) AdvanceSimTime();

    return sim_time_us_;
  } else {
    return SimClock::Get()->NowSimMicros();
  }
}

void MavLinkApi::HandleLockStep(void) {
  {
    std::lock_guard<std::mutex> lg(mutex_received_actuator_controls_);

    received_actuator_controls_ = true;
    cv_received_actuator_controls_.notify_one();
  }

  // If lockstep isn't active but is enabled and we've finished the vehicle
  // setup, see if we can enter lockstep mode
  if (!lock_step_active_ && lock_step_enabled_ && connected_vehicle_) {
    // && (HilActuatorControlsMessage.flags & 0x1))    // todo: enable this
    // check when this flag is widely available...
    // if the timestamps match then it means we are in lockstep mode.
    if (last_hil_sensor_time_ == HilActuatorControlsMessage.time_usec) {
      AddStatusMessage("Enabling lockstep mode");
      lock_step_active_ = true;
    }
  }
}

// message handling

void MavLinkApi::ProcessMavMessages(const mavlinkcom::MavLinkMessage& msg) {
  if (msg.msgid == HeartbeatMessage.msgid) {
    std::lock_guard<std::mutex> guard_heartbeat(heartbeat_mutex_);

    heartbeat_messages_received_count_ = heartbeat_messages_received_count_ + 1;
    if (heartbeat_messages_received_count_ < 4) {
      GetLogger().LogTrace(
          GetControllerName(),
          "ProcessMavMessages: received heartbeat message. count: %d",
          heartbeat_messages_received_count_);
    }

    HeartbeatMessage.decode(msg);

    bool armed =
        (HeartbeatMessage.base_mode &
         static_cast<uint8_t>(
             mavlinkcom::MAV_MODE_FLAG::MAV_MODE_FLAG_SAFETY_ARMED)) > 0;
    SetArmed(armed);
    // if (!got_first_heartbeat_) {
    if (heartbeat_messages_received_count_ == 1) {
      got_first_heartbeat_ = true;

      vehicle_type_ = static_cast<mavlinkcom::MAV_TYPE>(HeartbeatMessage.type);
      if (vehicle_type_ != mavlinkcom::MAV_TYPE::MAV_TYPE_GENERIC &&
          vehicle_type_ != mavlinkcom::MAV_TYPE::MAV_TYPE_QUADROTOR &&
          vehicle_type_ != mavlinkcom::MAV_TYPE::MAV_TYPE_HEXAROTOR &&
          vehicle_type_ !=
              mavlinkcom::MAV_TYPE::MAV_TYPE_VTOL_TAILSITTER_QUADROTOR) {
        GetLogger().LogWarning(
            GetControllerName(),
            "ProcessMavMessages: Unsupported PX4 vehicle type.");
      }

      GetLogger().LogTrace(GetControllerName(),
                           "ProcessMavMessages: received first heartbeat... "
                           "setting send_params to true");
      send_params_ = true;
    } else if (is_simulation_mode_ && !is_hil_mode_set_) {
      GetLogger().LogTrace(GetControllerName(),
                           "ProcessMavMessages: setting HIL mode");
      SetHILMode();
    }
  } else if (msg.msgid == StatusTextMessage.msgid) {
    StatusTextMessage.decode(msg);
    // lock is established by below method
    std::string msg_text = GetLogger().FormatMessage(
        "ProcessMavMessages: message from PX4: %s", StatusTextMessage.text);
    // AddStatusMessage(std::string(StatusTextMessage.text));
    AddStatusMessage(msg_text);
  } else if (msg.msgid == CommandLongMessage.msgid) {
    // GetLogger().LogTrace(GetControllerName(),
    //                     "ProcessMavMessages: received CommandLong message");
    CommandLongMessage.decode(msg);
    if (CommandLongMessage.command ==
        static_cast<int>(mavlinkcom::MAV_CMD::MAV_CMD_SET_MESSAGE_INTERVAL)) {
      int msg_id = static_cast<int>(CommandLongMessage.param1 + 0.5);
      if (msg_id == 115) {  // HIL_STATE_QUATERNION
        hil_state_freq_ = static_cast<int>(CommandLongMessage.param2 + 0.5);
      }
    }
  } else if (msg.msgid == HilControlsMessage.msgid) {
    GetLogger().LogTrace(
        GetControllerName(),
        "ProcessMavMessages: received HilControlsMessage message");
    if (!actuators_message_supported_) {
      std::lock_guard<std::mutex> guard_controls(hil_controls_mutex_);

      HilControlsMessage.decode(msg);
      control_outputs_[0] = HilControlsMessage.roll_ailerons;
      control_outputs_[1] = HilControlsMessage.pitch_elevator;
      control_outputs_[2] = HilControlsMessage.yaw_rudder;
      control_outputs_[3] = HilControlsMessage.throttle;
      control_outputs_[4] = HilControlsMessage.aux1;
      control_outputs_[5] = HilControlsMessage.aux2;
      control_outputs_[6] = HilControlsMessage.aux3;
      control_outputs_[7] = HilControlsMessage.aux4;

      NormalizeRotorControls();

      last_actuator_time_ = static_cast<uint64_t>(GetSimTimeMicros());
      HandleLockStep();
    }
  } else if (msg.msgid == HilActuatorControlsMessage.msgid) {
    // GetLogger().LogTrace(
    //     GetControllerName(),
    //     "ProcessMavMessages: received HilActuatorControlsMessage message");
    actuators_message_supported_ = true;

    std::lock_guard<std::mutex> guard_actuator(
        hil_controls_mutex_);  // use same mutex as HIL_CONTROl

    HilActuatorControlsMessage.decode(msg);

    // std::cout << "ProcessMavMessages: received HilActuatorControlsMessage "
    //              "message for timestamp: "
    //           << HilActuatorControlsMessage.time_usec << std::endl;

    bool isarmed = (HilActuatorControlsMessage.mode & 128) != 0;

    if (got_first_actuator_control_ == false) {
      GetLogger().LogTrace(
          GetControllerName(),
          "processMavMessages: first HilActuatorControlsMessage message: mode: "
          "'%d' armed: '%d'",
          HilActuatorControlsMessage.mode, isarmed);
      got_first_actuator_control_ = true;
    }

    for (auto i = 0; i < kControlOutputsCount; ++i) {
      if (isarmed) {
        control_outputs_[i] = HilActuatorControlsMessage.controls[i];
      } else {
        control_outputs_[i] = 0;
      }
    }
    if (isarmed) {
      NormalizeRotorControls();
    }

    last_actuator_time_ = static_cast<uint64_t>(GetSimTimeMicros());
    HandleLockStep();
  } else if (msg.msgid == MavLinkGpsRawInt.msgid) {
    // GetLogger().LogTrace(
    //    GetControllerName(),
    //    "ProcessMavMessages: received MavLinkGpsRawInt message");
    MavLinkGpsRawInt.decode(msg);
    auto fix_type =
        static_cast<mavlinkcom::GPS_FIX_TYPE>(MavLinkGpsRawInt.fix_type);
    auto locked = (fix_type != mavlinkcom::GPS_FIX_TYPE::GPS_FIX_TYPE_NO_GPS &&
                   fix_type != mavlinkcom::GPS_FIX_TYPE::GPS_FIX_TYPE_NO_FIX);
    if (locked && !has_gps_lock_) {
      AddStatusMessage("ProcessMavMessages: Got GPS lock");
      has_gps_lock_ = true;

      for (int i = 0; i < 5; ++i) {
        // Also request home position messages
        mav_vehicle_->setMessageInterval(
            mavlinkcom::MavLinkHomePosition::kMessageId, 1);
      }
    }
    if (!has_home_ && current_state_.home.is_set) {
      AddStatusMessage("ProcessMavMessages: Got GPS Home Location");
      has_home_ = true;
    }
    /*
    else {
      int version = mav_vehicle_->getVehicleStateVersion();
      mavlinkcom::VehicleState current_state = mav_vehicle_->getVehicleState();

      GetLogger().LogTrace(GetControllerName(),
                           "ProcessMavMessages: received MavLinkGpsRawInt
    message " "current_state: version: %d home_is_set: %d ", version,
          current_state.home.is_set);
    }
    */

    send_params_ = true;
  } else if (msg.msgid == mavlinkcom::MavLinkLocalPositionNed::kMessageId) {
    // GetLogger().LogTrace(
    //    GetControllerName(),
    //    "ProcessMavMessages: received MavLinkLocalPositionNed message");
    // we are getting position information... so we can use this to check the
    // stability of the z coordinate before takeoff.
    if (current_state_.controls.landed) {
      MonitorGroundAltitude();
    }
  } else if (msg.msgid == mavlinkcom::MavLinkExtendedSysState::kMessageId) {
    // GetLogger().LogTrace(
    //    GetControllerName(),
    //    "ProcessMavMessages: received MavLinkExtendedSysState message");
    // check landed state.
    GetLandedState();
    send_params_ = true;
  } else if (msg.msgid == mavlinkcom::MavLinkHomePosition::kMessageId) {
    // GetLogger().LogTrace(
    //     GetControllerName(),
    //     "ProcessMavMessages: received MavLinkHomePosition message");
    mavlinkcom::MavLinkHomePosition home;
    home.decode(msg);
    // GetLogger().LogTrace(
    //     GetControllerName(),
    //     "ProcessMavMessages: received MavLinkHomePosition message: lat: %f
    //     long: %f", home.latitude, home.longitude);

    // this is a good time to send the params
    send_params_ = true;
  } else if (msg.msgid == mavlinkcom::MavLinkSysStatus::kMessageId) {
    // this is a good time to send the params
    send_params_ = true;
  } else if (msg.msgid == mavlinkcom::MavLinkAutopilotVersion::kMessageId) {
    // this is a good time to send the params
    send_params_ = true;
  }
  // else ignore message
}

void MavLinkApi::ProcessQgcMessages(const mavlinkcom::MavLinkMessage& msg) {
  /*
  if (msg.msgid == MocapPoseStampedMessage.msgid) {
    std::lock_guard<std::mutex> guard(mocap_pose_mutex_);
    MocapPoseStampedMessage.decode(msg);
    getMocapPose(mocap_pose_.position, mocap_pose_.orientation);
  }
  // else ignore message
  */
}

void MavLinkApi::SendGimbalState() {
  auto& actuators = sim_robot_.GetActuators();
  int gimbal_idx = 0;
  for (auto& actuator_wrapper : actuators) {
    auto& actuator = actuator_wrapper.get();
    if (actuator.GetType() == ActuatorType::kGimbal) {
      auto gimbal = static_cast<Gimbal&>(actuator);
      auto state = gimbal.GetGimbalState();
      auto gimbal_id = actuator.GetId();
      mavlinkcom::MavLinkGimbalDeviceAttitudeStatus msg;
      auto quat =
          TransformUtils::ToQuaternion(state.roll, state.pitch, state.yaw);
      msg.q[0] = quat.w();
      msg.q[1] = quat.x();
      msg.q[2] = quat.y();
      msg.q[3] = quat.z();
      msg.angular_velocity_x = state.roll_vel;
      msg.angular_velocity_y = state.pitch_vel;
      msg.angular_velocity_z = state.yaw_vel;
      msg.time_boot_ms = GetSimTimeMicros() / 1000;
      msg.protocol_version = 2;
      msg.target_system = 0;
      msg.target_component = 0;
      gimbal_node_->sendMessage(msg);
      gimbal_idx++;
    }
  }
  last_gimbal_time_ = SimClock::Get()->NowSimMicros();
}

void MavLinkApi::SendSensorData() {
  // Clear received_actuator_controls_ before sending either HIL_GPS or
  // HIL_SENSOR messages just in case PX4 may respond with a
  // HIL_ACTUATOR_CONTROL message before expected.
  received_actuator_controls_ = false;
  // TODO Consider storing the last received HIL_ACTUATOR_CONTROL message
  // timestamp as the criteria to check if it's at the latest sim time to move
  // to the next step instead of synchronizing a bool flag.

  // ----------------------- HIL_GPS sensor message --------------------------

  if (gps_sensor_ != nullptr) {
    // Send HIL_GPS before HIL_SENSOR so that the data is already up to date
    // when sending the HIL_SENSOR message triggers the next lock step loop
    const GpsMessage& gps_output = gps_sensor_->getOutput();
    auto gps_data = gps_output.getData();
    auto gps_time = static_cast<uint64_t>(gps_data["time_utc_millis"]);
    // TODO: if (gps_output is valid)
    if (gps_time > last_gps_time_) {
      last_gps_time_ = gps_time;

      auto vel = gps_data["velocity"];
      Vector3 gps_velocity(vel["x"], vel["y"],
                           vel["z"]);  // gps_output.gnss.velocity;
      Vector3 gps_velocity_xy = gps_velocity;
      gps_velocity_xy[2] = 0;
      float gps_cog;
      if (MathUtils::IsApproximatelyZero(gps_velocity.y(), 1E-2f) &&
          MathUtils::IsApproximatelyZero(gps_velocity.x(), 1E-2f))
        gps_cog = 0;
      else
        gps_cog = TransformUtils::ToDegrees(
            atan2(gps_velocity.y(), gps_velocity.x()));
      if (gps_cog < 0) gps_cog += 360;

      // SendHILGps(gps_output.gnss.geo_point, gps_velocity,
      // gps_velocity_xy.norm(),
      //            gps_cog, gps_output.gnss.eph, gps_output.gnss.epv,
      //            gps_output.gnss.fix_type, 10);
      GeoPoint geo_point(gps_data["latitude"], gps_data["longitude"],
                         gps_data["altitude"]);
      SendHILGps(geo_point, gps_velocity, gps_velocity_xy.norm(), gps_cog,
                 gps_data["eph"], gps_data["epv"], gps_data["fix_type"], 15);
    }
  }

  // ----------------------- HIL_SENSOR sensor message -----------------------

  float altitude;
  Vector3 angular_velocity;
  float diff_pressure;
  Vector3 linear_acceleration;
  Vector3 magnetic_field;
  float pressure;

  float* paltitude = nullptr;
  Vector3* pangular_velocity = nullptr;
  float* pdiff_pressure = nullptr;
  Vector3* plinear_acceleration = nullptr;
  Vector3* pmagnetic_field = nullptr;
  float* ppressure = nullptr;

  if (airspeed_sensor_ != nullptr) {
    const AirspeedMessage& airspeed_output = airspeed_sensor_->getOutput();

    diff_pressure =
        static_cast<float>(airspeed_output.getData()["diff_pressure"]) *
        0.01f;  // Convert from Pascal to hectoPascal
    pdiff_pressure = &diff_pressure;
  }

  if (barometer_sensor_ != nullptr) {
    const BarometerMessage& baro_output = barometer_sensor_->getOutput();

    altitude = static_cast<float>(baro_output.getData()["altitude"]);
    paltitude = &altitude;
    pressure = static_cast<float>(baro_output.getData()["pressure"]) *
               0.01f;  // Convert from Pascal to hectoPascal
    ppressure = &pressure;
  }

  if (imu_sensor_ != nullptr) {
    const ImuMessage& imu_output = imu_sensor_->getOutput();

    angular_velocity = imu_output.GetAngularVelocity();
    pangular_velocity = &angular_velocity;
    linear_acceleration = imu_output.GetLinearAcceleration();
    plinear_acceleration = &linear_acceleration;
  }

  if (magnetometer_sensor_ != nullptr) {
    const MagnetometerMessage& mag_output = magnetometer_sensor_->getOutput();
    auto magnetic_field_body = mag_output.getData()["magnetic_field_body"];

    magnetic_field = Vector3(magnetic_field_body["x"], magnetic_field_body["y"],
                             magnetic_field_body["z"]);
    pmagnetic_field = &magnetic_field;
  }

  SendHILSensor(plinear_acceleration, pangular_velocity, pmagnetic_field,
                ppressure, pdiff_pressure, paltitude);

  // ------------------- DISTANCE_SENSOR sensor message -----------------------
  if (distance_sensor_ != nullptr) {
    const auto& distance_output = distance_sensor_->getOutput();

    SendDistanceSensor(
        distance_sensor_->GetDistanceSensorSettings().min_distance /
            100,  // m -> cm
        distance_sensor_->GetDistanceSensorSettings().max_distance /
            100,  // m -> cm
        distance_output.GetCurrentDistance(),
        0,   // sensor type: //TODO: allow changing in settings?
        77,  // sensor id, //TODO: should this be something real?
        distance_output.GetPose()
            .orientation);  // TODO: convert from radians to degrees?
  }
}

void MavLinkApi::SendHILSensor(const Vector3* acceleration, const Vector3* gyro,
                               const Vector3* mag, float* abs_pressure,
                               float* diff_pressure, float* pressure_alt) {
  if (!is_simulation_mode_)
    throw std::logic_error(
        "Attempt to send simulated sensor messages while not in simulation "
        "mode");

  mavlinkcom::MavLinkHilSensor hil_sensor;
  last_hil_sensor_time_ = GetSimTimeMicros();
  hil_sensor.time_usec = last_hil_sensor_time_;
  hil_sensor.fields_updated = 0;

  if (acceleration != nullptr) {
    hil_sensor.xacc = acceleration->x();
    hil_sensor.yacc = acceleration->y();
    hil_sensor.zacc = acceleration->z();
    hil_sensor.fields_updated = 0b111;  // Set accel bit fields
  }

  if (gyro != nullptr) {
    hil_sensor.xgyro = gyro->x();
    hil_sensor.ygyro = gyro->y();
    hil_sensor.zgyro = gyro->z();

    hil_sensor.fields_updated |= 0b111000;  // Set gyro bit fields
  }

  if (mag != nullptr) {
    hil_sensor.xmag = mag->x();
    hil_sensor.ymag = mag->y();
    hil_sensor.zmag = mag->z();

    hil_sensor.fields_updated |= 0b111000000;  // Set mag bit fields
  }

  if (abs_pressure != nullptr) {
    hil_sensor.abs_pressure = *abs_pressure;
    hil_sensor.fields_updated |=
        0b0001000000000;  // Set baro pressure bit field
  }

  if (diff_pressure != nullptr) {
    hil_sensor.diff_pressure = *diff_pressure;
    hil_sensor.fields_updated |= 0b0010000000000;  // Set air speed bit field
  }

  if (pressure_alt != nullptr) {
    hil_sensor.pressure_alt = *pressure_alt;
    hil_sensor.fields_updated |=
        0b0100000000000;  // Set baro altitude bit field
  }

  // TODO: Add actual temperature value from barometer sensor
  hil_sensor.temperature = 25.0f;                // stub to 25 degC for now
  hil_sensor.fields_updated |= 0b1000000000000;  // Set temperature field

  if (was_reset_) {
    hil_sensor.fields_updated = static_cast<uint32_t>(1 << 31);
  }

  if (hil_node_ != nullptr) {
    if (first_message_sent_ == false) {
      if (pressure_alt != nullptr) {
        GetLogger().LogTrace(
            GetControllerName(),
            "sendHILSensor: sending first sensor message: pressure_alt: %f",
            *pressure_alt);
      } else {
        GetLogger().LogTrace(
            GetControllerName(),
            "sendHILSensor: sending first sensor message (no pressure_alt)");
      }

      first_message_sent_ = true;
    }

    hil_node_->sendMessage(hil_sensor);

    // std::cout << "SendHILSensor: simtime = " << hil_sensor.time_usec
    //           << std::endl;
  }

  std::lock_guard<std::mutex> guard(last_message_mutex_);
  last_sensor_message_ = hil_sensor;
}

void MavLinkApi::SendDistanceSensor(float min_distance, float max_distance,
                                    float current_distance, float sensor_type,
                                    float sensor_id, Quaternion orientation) {
  if (!is_simulation_mode_)
    throw std::logic_error(
        "Attempt to send simulated distance sensor messages while not in "
        "simulation mode");

  mavlinkcom::MavLinkDistanceSensor distance_sensor;
  distance_sensor.time_boot_ms =
      static_cast<uint32_t>(GetSimTimeMicros() / 1000);

  distance_sensor.min_distance = static_cast<uint16_t>(min_distance);
  distance_sensor.max_distance = static_cast<uint16_t>(max_distance);
  distance_sensor.current_distance = static_cast<uint16_t>(current_distance);
  distance_sensor.type = static_cast<uint8_t>(sensor_type);
  distance_sensor.id = static_cast<uint8_t>(sensor_id);

  // Use custom orientation
  distance_sensor.orientation = 100;  // MAV_SENSOR_ROTATION_CUSTOM
  distance_sensor.quaternion[0] = orientation.w();
  distance_sensor.quaternion[1] = orientation.x();
  distance_sensor.quaternion[2] = orientation.y();
  distance_sensor.quaternion[3] = orientation.z();

  // TODO: use covariance parameter?

  if (hil_node_ != nullptr) {
    hil_node_->sendMessage(distance_sensor);
  }

  std::lock_guard<std::mutex> guard(last_message_mutex_);
  last_distance_message_ = distance_sensor;
}

void MavLinkApi::SendHILGps(const GeoPoint& geo_point, const Vector3& velocity,
                            float velocity_xy, float cog, float eph, float epv,
                            int fix_type, unsigned int satellites_visible) {
  if (!is_simulation_mode_)
    throw std::logic_error(
        "Attempt to send simulated GPS messages while not in simulation mode");

  mavlinkcom::MavLinkHilGps hil_gps;
  hil_gps.time_usec = GetSimTimeMicros();
  hil_gps.lat = static_cast<int32_t>(geo_point.latitude * 1E7);
  hil_gps.lon = static_cast<int32_t>(geo_point.longitude * 1E7);
  hil_gps.alt = static_cast<int32_t>(geo_point.altitude * 1000);
  hil_gps.vn = static_cast<int16_t>(velocity.x() * 100);
  hil_gps.ve = static_cast<int16_t>(velocity.y() * 100);
  hil_gps.vd = static_cast<int16_t>(velocity.z() * 100);
  hil_gps.eph = static_cast<uint16_t>(eph * 100);
  hil_gps.epv = static_cast<uint16_t>(epv * 100);
  hil_gps.fix_type = static_cast<uint8_t>(fix_type);
  hil_gps.vel = static_cast<uint16_t>(velocity_xy * 100);
  hil_gps.cog = static_cast<uint16_t>(cog * 100);
  hil_gps.satellites_visible = static_cast<uint8_t>(15);

  if (hil_node_ != nullptr) {
    if (first_gps_message_sent_ == false) {
      first_gps_message_sent_ = true;
      GetLogger().LogTrace(
          GetControllerName(),
          "sendHILGps: sending first gps message: latitude: %f",
          geo_point.latitude);
    }

    hil_node_->sendMessage(hil_gps);

    // std::cout << "SendHILGps: simtime = " << hil_gps.time_usec << std::endl;
  }

  if (hil_gps.lat < 0.1f && hil_gps.lat > -0.1f) {
    GetLogger().LogError(GetControllerName(), "hil_gps.lat was too close to 0");
  }

  std::lock_guard<std::mutex> guard(last_message_mutex_);
  last_gps_message_ = hil_gps;
}

inline mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS operator|(
    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS a,
    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS b) {
  return static_cast<mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS>(static_cast<int>(a) |
                                                          static_cast<int>(b));
}

void MavLinkApi::SendGimbalDeviceInformation() {
  // This for loop doesn't work as it is because one gimbal_node/connection can
  // only handle one gi peripheral (had to find out through testing - no
  // documentation). However, if we open more mavlink channels on PX4 - should
  // be easy make changes to make it work. There are currently limits on how mav
  // channels are allowed Supposed to be a max of 3 according to
  // https://docs.px4.io/main/en/peripherals/mavlink_peripherals.html, which we
  // are already at.
  for (auto& it : gimbal_id_to_component_id_) {
    mavlinkcom ::MavLinkGimbalDeviceInformation msg;
    auto capflags = mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_RETRACT |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_AXIS |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_LOCK |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_AXIS |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_LOCK |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_AXIS |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_FOLLOW |
                    mavlinkcom::GIMBAL_DEVICE_CAP_FLAGS::
                        GIMBAL_DEVICE_CAP_FLAGS_SUPPORTS_INFINITE_YAW;

    msg.cap_flags = static_cast<uint16_t>(capflags);
    msg.hardware_version = 1;
    msg.time_boot_ms = GetSimTimeMicros();
    msg.firmware_version = 1;

    msg.roll_min = -180;
    msg.roll_max = 180;
    msg.pitch_min = -180;
    msg.pitch_max = 180;
    msg.yaw_min = -180;
    msg.yaw_max = 180;
    msg.protocol_version = 2;
    msg.sysid = 1;
    msg.compid = it.second;
    gimbal_node_->sendMessage(msg);
  }
}

// Helpers

void MavLinkApi::CheckValidVehicle() {
  if (!IsValidConnection() || !connected_) {
    auto error_msg =
        "Cannot perform operation when no vehicle is connected or vehicle is "
        "not responding";
    GetLogger().LogError(GetControllerName(), error_msg);
    throw std::logic_error(error_msg);
  }
}

bool MavLinkApi::IsValidConnection() const {
  return ((mav_vehicle_ != nullptr) && (connection_ != nullptr) &&
          connection_->isOpen());
}

// status update methods should call this first!
void MavLinkApi::UpdateState() const {
  StatusLock lock(this);
  if (mav_vehicle_ != nullptr) {
    int version = mav_vehicle_->getVehicleStateVersion();
    if (version != state_version_) {
      current_state_ = mav_vehicle_->getVehicleState();
      state_version_ = version;
    }
  }
}

ReadyState MavLinkApi::GetReadyState() const {
  ReadyState state;
  if (!is_ready_ && is_ready_message_.size() > 0) {
    state.ReadyMessage = is_ready_message_;
  }
  state.ReadyVal = is_ready_;
  return state;
}

bool MavLinkApi::CanArm() const { return is_ready_ && has_gps_lock_; }

void MavLinkApi::WaitForHomeLocation(float timeout_sec,
                                     int64_t command_start_time_nanos) {
  if (!current_state_.home.is_set) {
    AddStatusMessage("Waiting for valid GPS home location...");
    if (!RunFlightCommand([&]() { return current_state_.home.is_set; },
                          timeout_sec, command_start_time_nanos)
             .IsComplete()) {
      auto error_msg = "Vehicle does not have a valid GPS home location";
      GetLogger().LogError(GetControllerName(), error_msg);
      throw std::logic_error(error_msg);
    }
  }
}

void MavLinkApi::WaitForStableGroundPosition(float timeout_sec,
                                             int64_t command_start_time_nanos) {
  // wait for ground stabilization
  if (ground_variance_ > GroundTolerance) {
    AddStatusMessage("Waiting for z-position to stabilize...");
    if (!RunFlightCommand([&]() { return ground_variance_ <= GroundTolerance; },
                          timeout_sec, command_start_time_nanos)
             .IsComplete()) {
      auto error_msg = GetLogger().FormatMessage(
          "Ground is not stable, variance is %f", ground_variance_);
      GetLogger().LogError(GetControllerName(), error_msg.c_str());
      throw std::logic_error(error_msg);
    }
  }
}

bool MavLinkApi::StartOffboardMode() {
  CheckValidVehicle();
  try {
    mav_vehicle_->requestControl();
  } catch (std::exception& ex) {
    EnsureSafeMode();
    AddStatusMessage(std::string("Request control failed: ") + ex.what());
    return false;
  } catch (...) {
    EnsureSafeMode();
    AddStatusMessage("Request control failed with unknown exception.");
    return false;
  }
  return true;
}

void MavLinkApi::EndOffboardMode() {
  // bugbug: I removed this releaseControl because it makes back-to-back move
  // operations less smooth. The side effect of this is that with some drones
  // (e.g. PX4 based) the drone itself will timeout when you stop sending move
  // commands and the behavior on timeout is then determined by the drone
  // itself. mav_vehicle_->releaseControl();
  EnsureSafeMode();
}

void MavLinkApi::EnsureSafeMode() {
  if (mav_vehicle_ != nullptr) {
    const mavlinkcom::VehicleState& state = mav_vehicle_->getVehicleState();
    if (state.controls.landed || !state.controls.armed) {
      return;
    }
  }
}

void MavLinkApi::NormalizeRotorControls() {
  size_t num_rotors_to_normalize = 0;
  if (vehicle_type_ == mavlinkcom::MAV_TYPE::MAV_TYPE_GENERIC) {
    // If vehicle type is generic, we shouldn't assume anything about which
    // outputs are rotors for normalizing the scaling, so leave them alone.
    // Note: As of v1.12.3, PX4 doesn't properly set the vehicle type in the
    // heartbeat message sent to simulators, so all vehicles are treated as
    // generic.
    num_rotors_to_normalize = 0;
  } else if (vehicle_type_ == mavlinkcom::MAV_TYPE::MAV_TYPE_QUADROTOR ||
             vehicle_type_ ==
                 mavlinkcom::MAV_TYPE::MAV_TYPE_VTOL_TAILSITTER_QUADROTOR) {
    num_rotors_to_normalize = 4;
  } else if (vehicle_type_ == mavlinkcom::MAV_TYPE::MAV_TYPE_HEXAROTOR) {
    num_rotors_to_normalize = 6;
  } else {
    GetLogger().LogWarning(
        GetControllerName(),
        "NormalizeRotorControls: Unsupported PX4 vehicle type.");
  }

  for (size_t i = 0; i < num_rotors_to_normalize; ++i) {
    if (vehicle_type_ ==
        mavlinkcom::MAV_TYPE::MAV_TYPE_VTOL_TAILSITTER_QUADROTOR) {
      // For VTOL rotors, leave PX4's 0~1.0 rotor output at full scale to allow
      // the VTOL rotors to fully stop during fixed wing flight.
      control_outputs_[i] = MathUtils::Clip(control_outputs_[i], 0.0f, 1.0f);
    } else {
      // Rescale PX4's 0~1.0 rotor output to 0.2~1.0 for idling rotors at 0.2
      // when armed.
      control_outputs_[i] =
          MathUtils::Clip(0.8f * control_outputs_[i] + 0.20f, 0.0f, 1.0f);
    }
  }

  // Additional non-rotor controls like wing control surfaces should not be
  // normalized in order to allow full -1.0~1.0 range of actuation
}

bool MavLinkApi::SendTestMessage(
    std::shared_ptr<mavlinkcom::MavLinkNode> node) {
  try {
    // try and send a test message.
    mavlinkcom::MavLinkHeartbeat test;
    test.autopilot =
        static_cast<int>(mavlinkcom::MAV_AUTOPILOT::MAV_AUTOPILOT_PX4);
    test.type = static_cast<uint8_t>(mavlinkcom::MAV_TYPE::MAV_TYPE_GCS);
    test.base_mode = 0;
    test.custom_mode = 0;
    test.mavlink_version = 3;
    node->sendMessage(test);
    test.system_status = 0;
    return true;
  } catch (std::exception&) {
    return false;
  } catch (...) {
    return false;
  }
}

void MavLinkApi::SendParams() {
  // send any mavlink parameters from drone config through to the connected
  // vehicle.
  if ((mav_vehicle_ != nullptr) && (connection_info_.params.size() > 0)) {
    GetLogger().LogTrace(GetControllerName(), "Sending params");
    try {
      for (auto iter : connection_info_.params) {
        auto key = iter.first;
        auto value = iter.second;
        mavlinkcom::MavLinkParameter p;
        p.name = key;
        p.value = value;
        bool result = false;
        mav_vehicle_->setParameter(p).wait(1000, &result);
        if (!result) {
          GetLogger().LogError(GetControllerName(),
                               "Failed to set mavlink parameter '%s'",
                               key.c_str());
        }
      }
    } catch (std::exception& ex) {
      AddStatusMessage("Exception sending parameters to vehicle");
      AddStatusMessage(ex.what());
    } catch (...) {
      AddStatusMessage(
          "Unknown exception while sending parameters to vehicle.");
    }
  }
}

void MavLinkApi::SetArmed(bool armed) {
  is_armed_ = armed;
  if (!armed) {
    // reset motor controls
    for (size_t i = 0; i < kControlOutputsCount; ++i) {
      control_outputs_[i] = 0;
    }
  }
}

void MavLinkApi::MonitorGroundAltitude() {
  // used to ensure stable altitude before takeoff.
  auto position = GetPosition();
  auto result = ground_filter_.filter(position.z());
  auto variance = std::get<1>(result);
  if (variance >= 0) {  // filter returns -1 if we don't have enough data yet.
    ground_variance_ = variance;
  }
}

int MavLinkApi::TimeoutToMilliseconds(float timeout_sec) {
  if (timeout_sec <= 0.0f)
    return (0);  // Timeout immediately (don't wait)
  else if (timeout_sec >= (std::numeric_limits<int>::max() / 1000))
    return (-1);  // Infinite timeout (wait forever)

  return static_cast<int>(timeout_sec) * 1000;
}

}  // namespace projectairsim
}  // namespace microsoft