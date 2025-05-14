// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "arducopter_api.hpp"

#include <core_sim/actuators/gimbal.hpp>
#include <json.hpp>

#include "Utils.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/message/flight_control_setpoint_message.hpp"
#include "core_sim/transforms/transform_utils.hpp"
// This module is based on the implementation of PX4 for project AirSim and
// ArduCopter for AirSim OSS. There are some comments for further developing
// lidar, lock step and control API.
namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class ArduCopterApi

ArduCopterApi::ArduCopterApi(const Robot& robot, TransformTree* ptransformtree)
    : VTOLFWApiBase(robot, ptransformtree) {
  LoadSettings(robot);
}

ArduCopterApi::~ArduCopterApi() {
  // TODO: revisit for reload-scene / RL training
  CloseAllConnections();
  if (this->connect_thread_.joinable()) {
    this->connect_thread_.join();
  }
}

void ArduCopterApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  // GetJsonObject
  const json& ardupilot_api_settings_json =
      controller_settings_json.value("ardupilot-settings", "{ }"_json);

  // GetArray
  const json& actuator_order_json =
      ardupilot_api_settings_json.value("actuator-order", "[ ]"_json);

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

  // get ardupilot-ip from json
  connection_info_.ardupilot_ip = ardupilot_api_settings_json.value(
      "ardupilot-ip", connection_info_.ardupilot_ip);
  // get ardupilot-udp-port from json
  connection_info_.ardupilot_udp_port = ardupilot_api_settings_json.value(
      "ardupilot-udp-port", connection_info_.ardupilot_udp_port);
  // get local-host-ip from json
  connection_info_.local_host_ip = ardupilot_api_settings_json.value(
      "local-host-ip", connection_info_.local_host_ip);
  // get local-udp-port from json
  connection_info_.local_host_udp_port = ardupilot_api_settings_json.value(
      "local-host-udp-port", connection_info_.local_host_udp_port);
  // get use-lidar from json
  // use_lidar_ = ardupilot_api_settings_json.value("use-lidar", use_lidar_);
  // get use-sensor-distance from json
  use_distance_sensor_ = ardupilot_api_settings_json.value(
      "use-sensor-distance", use_distance_sensor_);
  // Lock step timeout thresholds
  // connection_info_.timeout_lock_step_update_ms =
  //     ardupilot_api_settings_json.value(
  //         "timeout-lock-step-update-ms",
  //         connection_info_.timeout_lock_step_update_ms);
}

void ArduCopterApi::GetSensors(const Robot& robot) {
  const std::vector<std::reference_wrapper<Sensor>>& sensors =
      robot.GetSensors();

  imu_sensor_ = nullptr;
  gps_sensor_ = nullptr;
  distance_sensor_ = nullptr;
  // lidar_sensor_ = nullptr;

  std::for_each(
      sensors.begin(), sensors.end(),
      [this](const std::reference_wrapper<Sensor> sensor_wrapper) {
        Sensor& sensor = sensor_wrapper.get();
        if (sensor.IsEnabled()) {
          if (imu_sensor_ == nullptr && sensor.GetType() == SensorType::kImu) {
            imu_sensor_ = static_cast<Imu*>(&((Imu&)sensor));
            auto id = imu_sensor_->GetId();
          } else if (gps_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kGps) {
            gps_sensor_ = static_cast<Gps*>(&((Gps&)sensor));
          } else if (distance_sensor_ == nullptr &&
                     sensor.GetType() == SensorType::kDistanceSensor) {
            distance_sensor_ =
                static_cast<DistanceSensor*>(&((DistanceSensor&)sensor));
          }  // else if (lidar_sensor_ == nullptr &&
          //            sensor.GetType() == SensorType::kLidar) {
          //   lidar_sensor_ = static_cast<Lidar*>(&((Lidar&)sensor));
          // }
        }
      });

  if (imu_sensor_ == nullptr)
    throw Error(
        "ArduCopterApi: IMU sensor is required but not specified in sensor "
        "configuration");
}

//---------------------------------------------------------------------------
// IController overrides

void ArduCopterApi::BeginUpdate() {
  MultirotorApiBase::BeginUpdate();

  is_simulation_mode_ = true;  // currently only sim mode is supported

  // TODO: revisit for reload-scene / RL training
  Reset();
  GetSensors(sim_robot_);
  InitializeConnections();
}

void ArduCopterApi::EndUpdate() {
  MultirotorApiBase::EndUpdate();

  CloseAllConnections();
  if (this->connect_thread_.joinable()) {
    this->connect_thread_.join();
  }
}

void ArduCopterApi::Reset() {
  MultirotorApiBase::Reset();

  ResetState();
  // SetNormalMode();
}

void ArduCopterApi::SetKinematics(const Kinematics* kinematics) {}

void ArduCopterApi::Update() {
  try {
    MultirotorApiBase::Update();

    if (imu_sensor_ == nullptr || !connected_ || udp_socket_.get() == nullptr) {
      GetLogger().LogTrace(GetControllerName(),
                           "Update: Not ready to send sensor data.");
      return;
    }

    auto now = SimClock::Get()->NowSimMicros();
    /* if (lock_step_active_) {
      if ((last_update_time_ != 0) &&
          ((now - last_update_time_) >=
           (connection_info_.timeout_lock_step_update_ms * 1000))) {
        // if we haven't received an actuator control message within the
        // timeout then something is terribly wrong, reset lockstep mode
        lock_step_active_ = false;
        AddStatusMessage(
            "ArduCopterApi::Update(): too long between calls--resetting lock
    step " "mode");
      }
    } */

    // last_update_time_ = now;

    AdvanceSimTime();

    // send sensor updates
    SendSensorData();

    ReciveRotorsControls();

  } catch (std::exception& e) {
    AddStatusMessage("Exception sending messages to vehicle");
    AddStatusMessage(e.what());
    Disconnect();
    Connect();  // re-start a new connection so ArduPilot can be restarted and
                // AirSim will happily continue on.
  } catch (...) {
    AddStatusMessage("Unknown exception sending messages to vehicle.");
    Disconnect();
    Connect();  // re-start a new connection so ArduPilot can be restarted and
                // AirSim will happily continue on.
  }
}

void ArduCopterApi::ReciveRotorsControls() {
  // Receive motor data
  RotorControlMessage pkt;
  int recv_ret = udp_socket_->recv(&pkt, sizeof(pkt), 100);
  while (recv_ret != sizeof(pkt)) {
    if (recv_ret <= 0) {
      GetLogger().LogError(
          GetControllerName(),
          "Error while receiving rotor control data - ErrorNo: %d", recv_ret);
    } else {
      AddStatusMessage(GetLogger().FormatMessage(
          "Received %d bytes instead of %zu bytes", recv_ret, sizeof(pkt)));
    }

    recv_ret = udp_socket_->recv(&pkt, sizeof(pkt), 100);
  }

  for (auto i = 0; i < k_ardu_copter_rotor_control_count_; ++i) {
    control_outputs_[i] = pkt.pwm[i];
  }
  NormalizeRotorControls();
  // HandleLockStep();
}

std::vector<float> ArduCopterApi::GetControlSignals(const std::string& actuator_id) {
  if (!is_simulation_mode_) {
    throw std::logic_error(
        "Attempt to read motor controls while not in simulation mode");
  }

  auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);
  if (actuator_map_itr == actuator_id_to_output_idx_map_.end()) {
    GetLogger().LogWarning(
        GetControllerName(),
        "ArduCopterApi::GetControlSignal() called for invalid actuator: %s",
        actuator_id.c_str());
    return std::vector<float>(1,0.f);
  }

  // std::lock_guard<std::mutex> guard(hil_controls_mutex_);
  return std::vector<float>(control_outputs_[actuator_map_itr->second]);
}

//---------------------------------------------------------------------------
// IMultirotorApi overrides

bool ArduCopterApi::EnableApiControl() {
  AddStatusMessage("Not Implemented: EnableApiControl");
  return false;
}

bool ArduCopterApi::DisableApiControl() {
  AddStatusMessage("Not Implemented: DisableApiControl");
  return false;
}

bool ArduCopterApi::IsApiControlEnabled() {
  AddStatusMessage("Not Implemented: IsApiControlEnabled");
  return false;
}

bool ArduCopterApi::Arm(int64_t command_start_time_nanos) {
  AddStatusMessage("Not Implemented: Arm");
  return false;
}

bool ArduCopterApi::Disarm() {
  AddStatusMessage("Not Implemented: Disarm");
  return false;
}

bool ArduCopterApi::MoveToPosition(float x, float y, float z, float velocity,
                                   float timeout_sec,
                                   DrivetrainType /*drivetrain*/,
                                   bool yaw_is_rate, float yaw,
                                   float /*lookahead*/,
                                   float /*adaptive_lookahead*/,
                                   int64_t command_start_time_nanos) {
  unused(x);
  unused(y);
  unused(z);
  unused(yaw);
  AddStatusMessage("Not Implemented: CommandPosition");
  return false;
}

bool ArduCopterApi::Takeoff(float timeout_sec,
                            int64_t command_start_time_nanos) {
  AddStatusMessage("Not Implemented: Takeoff");
  return false;
}

bool ArduCopterApi::Land(float timeout_sec, int64_t command_start_time_nanos) {
  AddStatusMessage("Not Implemented: Land");
  return false;
}

bool ArduCopterApi::GoHome(float timeout_sec, float velocity,
                           int64_t command_start_time_nanos) {
  AddStatusMessage("Not Implemented: GoHome");
  return false;
}

bool ArduCopterApi::Hover(int64_t /*command_start_time_nanos*/) {
  AddStatusMessage("Not Implemented: Hover");
  return false;
}

bool ArduCopterApi::RequestControl(int64_t /*command_start_time_nanos*/) {
  AddStatusMessage("Not Implemented: RequestControl");
  return false;
}

bool ArduCopterApi::SetMissionMode(int64_t /*command_start_time_nanos*/) {
  AddStatusMessage("Not Implemented: SetMissionMode");
  return false;
}

bool ArduCopterApi::SetVTOLMode(VTOLMode vtolmode) {
  AddStatusMessage("Not Implemented: SetMissionMode");
  return false;
}

//---------------------------------------------------------------------------
// Implementation for MultirotorApiBase

void ArduCopterApi::CommandMotorPWMs(float front_right_pwm, float rear_left_pwm,
                                     float front_left_pwm,
                                     float rear_right_pwm) {
  unused(front_right_pwm);
  unused(front_left_pwm);
  unused(rear_right_pwm);
  unused(rear_left_pwm);
  AddStatusMessage("Not Implemented: commandMotorPWMs");
}

void ArduCopterApi::CommandRollPitchYawZ(float roll, float pitch, float yaw,
                                         float z) {
  unused(roll);
  unused(pitch);
  unused(yaw);
  unused(z);
  AddStatusMessage("Not Implemented: commandRollPitchYawZ");
}

void ArduCopterApi::CommandRollPitchYawThrottle(float roll, float pitch,
                                                float yaw, float throttle) {
  unused(roll);
  unused(pitch);
  unused(yaw);
  unused(throttle);
  AddStatusMessage("Not Implemented: commandRollPitchYawThrottle");
}

void ArduCopterApi::CommandRollPitchYawrateThrottle(float roll, float pitch,
                                                    float yaw_rate,
                                                    float throttle) {
  unused(roll);
  unused(pitch);
  unused(yaw_rate);
  unused(throttle);
  AddStatusMessage("Not Implemented: commandRollPitchYawrateThrottle");
}

void ArduCopterApi::CommandRollPitchYawrateZ(float roll, float pitch,
                                             float yaw_rate, float z) {
  unused(roll);
  unused(pitch);
  unused(yaw_rate);
  unused(z);
  AddStatusMessage("Not Implemented: commandRollPitchYawrateZ");
}

void ArduCopterApi::CommandAngleRatesZ(float roll_rate, float pitch_rate,
                                       float yaw_rate, float z) {
  unused(roll_rate);
  unused(pitch_rate);
  unused(yaw_rate);
  unused(z);
  AddStatusMessage("Not Implemented: commandAngleRatesZ");
}

void ArduCopterApi::CommandAngleRatesThrottle(float roll_rate, float pitch_rate,
                                              float yaw_rate, float throttle) {
  unused(roll_rate);
  unused(pitch_rate);
  unused(yaw_rate);
  unused(throttle);
  AddStatusMessage("Not Implemented: commandAngleRatesThrottle");
}

void ArduCopterApi::CommandHeading(float heading, float speed, float vz) {
  unused(heading);
  unused(speed);
  unused(vz);
  AddStatusMessage("Not Implemented: CommandHeading");
}

void ArduCopterApi::CommandVelocity(float vx, float vy, float vz,
                                    bool yaw_is_rate, float yaw) {
  unused(vx);
  unused(vy);
  unused(vz);
  unused(yaw);
  AddStatusMessage("Not Implemented: CommandVelocity");
}

void ArduCopterApi::CommandVelocityZ(float vx, float vy, float z,
                                     bool yaw_is_rate, float yaw) {
  unused(vx);
  unused(vy);
  unused(z);
  unused(yaw_is_rate);
  unused(yaw);
  AddStatusMessage("Not Implemented: CommandVelocityZ");
}

void ArduCopterApi::CommandVelocityBody(float vx, float vy, float vz,
                                  bool yaw_is_rate, float yaw) {
  unused(vx);
  unused(vy);
  unused(vz);
  unused(yaw_is_rate);
  unused(yaw);
  AddStatusMessage("Not Implemented: CommandVelocityBody");
}


void ArduCopterApi::CommandVelocityZBody(float vx, float vy, float z,
                                  bool yaw_is_rate, float yaw) {
  unused(vx);
  unused(vy);
  unused(z);
  unused(yaw_is_rate);
  unused(yaw);
  AddStatusMessage("Not Implemented: CommandVelocityZBody");
}

void ArduCopterApi::CommandPosition(float x, float y, float z, bool yaw_is_rate,
                                    float yaw) {
  unused(x);
  unused(y);
  unused(z);
  unused(yaw);
  AddStatusMessage("Not Implemented: CommandPosition");
}

const MultirotorApiBase::MultirotorApiParams&
ArduCopterApi::GetMultirotorApiParams() const {
  static const MultirotorApiParams vehicle_params;
  return vehicle_params;
}

void ArduCopterApi::SetControllerGains(uint8_t controller_type,
                                       const std::vector<float>& kp,
                                       const std::vector<float>& ki,
                                       const std::vector<float>& kd) {
  unused(controller_type);
  unused(kp);
  unused(ki);
  unused(kd);
  AddStatusMessage("Not Implemented: SetControllerGains");
}

void ArduCopterApi::BeforeTask() {}
void ArduCopterApi::AfterTask() {}

Kinematics ArduCopterApi::GetKinematicsEstimated() const {
  AddStatusMessage("Not Implemented: GetKinematicsEstimated");
  Kinematics kinematics;
  return kinematics;
}

Vector3 ArduCopterApi::GetPosition() const {
  AddStatusMessage("Not Implemented: GetPosition");
  Vector3 position;
  return position;
}

Vector3 ArduCopterApi::GetVelocity() const {
  AddStatusMessage("Not Implemented: GetVelocity");
  Vector3 position;
  return position;
}

Quaternion ArduCopterApi::GetOrientation() const {
  AddStatusMessage("Not Implemented: GetOrientation");
  Quaternion orientation;
  return orientation;
}

LandedState ArduCopterApi::GetLandedState() const {
  AddStatusMessage("Not Implemented: GetLandedState");
  return LandedState::Landed;
}

GeoPoint ArduCopterApi::GetGpsLocationEstimated() const {
  AddStatusMessage("Not Implemented: GetGpsLocationEstimated");
  GeoPoint location;
  return location;
}

float ArduCopterApi::GetCommandPeriod() const {
  return 1.0f / 50;  // 50hz
}

float ArduCopterApi::GetTakeoffZ() const {
  AddStatusMessage("Not Implemented: GetTakeoffZ");
  return 0;
}

float ArduCopterApi::GetDistanceAccuracy() const {
  AddStatusMessage("Not Implemented: GetDistanceAccuracy");
  return 0;
}

//---------------------------------------------------------------------------
// Helper methods

void ArduCopterApi::InitializeConnections() {
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

void ArduCopterApi::OpenAllConnections() {
  Disconnect();  // just in case if connections were open
  ResetState();  // reset all variables we might have changed during last
                 // session
  Connect();
}

void ArduCopterApi::CloseAllConnections() { Disconnect(); }

void ArduCopterApi::Connect() {
  if (!connecting_) {
    connecting_ = true;
    if (this->connect_thread_.joinable()) {
      this->connect_thread_.join();
    }
    this->connect_thread_ = std::thread(&ArduCopterApi::ConnectThread, this);
    GetLogger().LogTrace(GetControllerName(), "Created connect thread");
  }
}

void ArduCopterApi::Disconnect() {
  AddStatusMessage("Disconnecting arducopter vehicle");
  connected_ = false;
  // connected_vehicle_ = false;

  if (udp_socket_ != nullptr) udp_socket_->close();
}

void ArduCopterApi::ConnectThread() {
  GetLogger().LogTrace(GetControllerName(), "ConnectThread is running");
  AddStatusMessage("Waiting for arducopter vehicle...");
  CreateUdpConnection(connection_info_);
  connecting_ = false;
  connected_ = true;
}

void ArduCopterApi::ResetState() {
  // reset state
  // is_hil_mode_set_ = false;
  // current_state_ = mavlinkcom::VehicleState(); //TO-DO: should be an
  // equivalent from simpleflight or more generic structure
  sim_time_us_ = 0;
  last_gps_time_ = 0;
  // last_update_time_ = 0;
  // last_hil_sensor_time_ = 0;
  std::fill(std::begin(control_outputs_), std::end(control_outputs_), 0.0f);
  // received_actuator_controls_ = false;
  // lock_step_active_ = false;
  // lock_step_enabled_ = connection_info_.lock_step;
  CancelLastTask();
}

/* put ArduPilot in normal mode (i.e. non-simulation mode)
void ArduCopterApi::SetNormalMode() {
  if (is_hil_mode_set_ && connection_ != nullptr && mav_vehicle_ != nullptr) {
    GetLogger().LogTrace(
        GetControllerName(),
        "SetNormalMode: putting ArduPilot in non-simulation mode via cmd");
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
*/

void ArduCopterApi::AddStatusMessage(const std::string& message) const {
  if (message.size() != 0) {
    GetLogger().LogTrace(GetControllerName(), message.c_str());
  }
}

void ArduCopterApi::CreateUdpConnection(
    const ArduPilotConnectionInfo& connection_info) {
  GetLogger().LogTrace(GetControllerName(),
                       "Attempting to create Ethernet connection");
  Disconnect();

  std::string remoteIpAddr;
  std::fill(std::begin(control_outputs_), std::end(control_outputs_), 0.0f);

  if (connection_info.ardupilot_udp_port == 0) {
    throw std::invalid_argument("UdpPort setting has an invalid value.");
  }

  port_ = static_cast<uint16_t>(connection_info_.ardupilot_udp_port);
  ip_ = connection_info_.ardupilot_ip;

  if (ip_ == "") {
    throw std::invalid_argument("UdpIp setting is invalid.");
  }

  if (port_ == 0) {
    throw std::invalid_argument("UdpPort setting has an invalid value.");
  }

  AddStatusMessage(GetLogger().FormatMessage(
      "Using UDP port %d, local IP %s, remote IP %s for sending sensor data",
      port_, connection_info_.local_host_ip.c_str(), ip_.c_str()));
  AddStatusMessage(GetLogger().FormatMessage(
      "Using UDP port %d for receiving rotor power",
      connection_info_.local_host_udp_port,
      connection_info_.local_host_ip.c_str(), ip_.c_str()));

  udp_socket_ = std::make_unique<mavlinkcom::UdpSocket>();
  udp_socket_->bind(connection_info_.local_host_ip,
                    connection_info_.local_host_udp_port);
  AddStatusMessage(std::string("Connected to SITL over UDP."));
}

// simulation time handling
void ArduCopterApi::AdvanceSimTime(void) {
  sim_time_us_ = SimClock::Get()->NowSimMicros();
}

TimeMicro ArduCopterApi::GetSimTimeMicros(void) {
  // This ensures HIL_SENSOR and HIL_GPS have matching clocks.
  // if (lock_step_active_) {
  if (sim_time_us_ == 0) AdvanceSimTime();

  return sim_time_us_;
  //} else {
  //  return SimClock::Get()->NowSimMicros();
  //}
}
/*
void ArduCopterApi::HandleLockStep(void) {
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
*/
// message handling
void ArduCopterApi::SendSensorData() {
  // Clear received_actuator_controls_ before sending sensor messages
  // received_actuator_controls_ = false;
  // TODO Consider storing the last received ACTUATOR_CONTROL message
  // timestamp as the criteria to check if it's at the latest sim time to move
  // to the next step instead of synchronizing a bool flag.

  std::ostringstream buf;

  // Start of JSON element
  buf << "{";

  buf << "\"timestamp\": " << sim_time_us_ << ",";

  const ImuMessage& imu_output = imu_sensor_->getOutput();
  auto angular_velocity = imu_output.GetAngularVelocity();
  auto linear_acceleration = imu_output.GetLinearAcceleration();
  buf << "\"imu\": {" << std::fixed << std::setprecision(7)
      << "\"angular_velocity\": [" << angular_velocity[0] << ","
      << angular_velocity[1] << "," << angular_velocity[2] << "]"
      << ","
      << "\"linear_acceleration\": [" << linear_acceleration[0] << ","
      << linear_acceleration[1] << "," << linear_acceleration[2] << "]"
      << "}";
  /*    */

  auto rpy = TransformUtils::ToRPY(imu_output.GetOrientation());

  buf << ","
      << "\"pose\": {"
      << "\"roll\": " << rpy[0] << ","
      << "\"pitch\": " << rpy[1] << ","
      << "\"yaw\": " << rpy[2] << "}";

  if (gps_sensor_ != nullptr) {
    const GpsMessage& gps_output = gps_sensor_->getOutput();
    auto gps_data = gps_output.getData();
    auto gps_time = static_cast<uint64_t>(gps_data["time_utc_millis"]);
    // TODO: if (gps_output is valid)
    if (gps_time > last_gps_time_) {
      last_gps_time_ = gps_time;

      auto vel = gps_data["velocity"];
      Vector3 gps_velocity(vel["x"], vel["y"], vel["z"]);
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

      GeoPoint geo_point(gps_data["latitude"], gps_data["longitude"],
                         gps_data["altitude"]);

      buf << ","
             "\"gps\": {"
          << std::fixed << std::setprecision(7)
          << "\"lat\": " << geo_point.latitude << ","
          << "\"lon\": " << geo_point.longitude << "," << std::setprecision(3)
          << "\"alt\": " << geo_point.altitude << "},"

          << "\"velocity\": {"
          << "\"world_linear_velocity\": [" << gps_velocity[0] << ","
          << gps_velocity[1] << "," << gps_velocity[2]
          << "]"
             "}";
    }
  }

  // Send RC channels to Ardupilot if present
  /*if (is_rc_connected_ && last_rcData_.is_valid) {
      buf << ","
              "\"rc\": {"
              "\"channels\": ["
          << (last_rcData_.roll + 1) * 0.5f << ","
          << (last_rcData_.yaw + 1) * 0.5f << ","
          << (last_rcData_.throttle + 1) * 0.5f << ","
          << (-last_rcData_.pitch + 1) * 0.5f;

      // Add switches to RC channels array, 8 switches
      for (uint8_t i = 0; i < 8; ++i) {
          buf << "," << static_cast<float>(last_rcData_.getSwitch(i));
      }

      // Close JSON array & element
      buf << "]}";
  }
 */
  // Send Distance Sensors data if present
  if (distance_sensor_ != nullptr) {
    // Start JSON element
    buf << ","
           "\"rng\": {"
           "\"distances\": [";

    // More than mm level accuracy isn't needed or expected
    buf << std::fixed << std::setprecision(3);

    // Used to avoid trailing comma
    std::string sep = "";

    // Add sensor outputs in the array

    if (distance_sensor_ && use_distance_sensor_) {
      const auto& distance_output = distance_sensor_->getOutput();
      // AP uses meters so no need to convert here
      buf << sep << distance_output.GetCurrentDistance();
      sep = ",";
    }

    // Close JSON array & element
    buf << "]}";
  }

  /*TODO: Lidar
  if (lidar_sensor_ != nullptr) {
      buf << ","
              "\"lidar\": {"
              "\"point_cloud\": [";

      // More than mm level accuracy isn't needed or expected
      buf << std::fixed << std::setprecision(3);

      // Add sensor outputs in the array
      if (lidar_sensor_ && use_lidar_) {
            const auto& lidar_output_ = lidar_sensor_->GetOutput();
            // AP uses meters so no need to convert here
            buf << sep << lidar_output_.GetCurrentDistance();
            sep = ",";
        }

      // Close JSON array & element
      buf << "]}";
  }*/

  // End of JSON data, AP Parser needs newline
  buf << "}\n";

  // str copy is made since if later on something like -
  //  const char* ptr = buf.str().c_str()
  // is written, ptr is invalid since buf.str() is a temporary copy
  // Currently there's no way to get pointer to underlying buffer
  const std::string sensor_data = buf.str();
  udp_socket_->sendto(sensor_data.c_str(), sensor_data.length(), ip_, port_);
}

ReadyState ArduCopterApi::GetReadyState() const {
  ReadyState state;
  if (!is_ready_ && is_ready_message_.size() > 0) {
    state.ReadyMessage = is_ready_message_;
  }
  state.ReadyVal = is_ready_;
  return state;
}

bool ArduCopterApi::CanArm() const { return is_ready_; }

void ArduCopterApi::NormalizeRotorControls() {
  for (size_t i = 0; i < k_ardu_copter_rotor_control_count_; ++i) {
    control_outputs_[i] = (control_outputs_[i] - 1000.0f) / 1000.0f;
  }

  // Additional non-rotor controls like wing control surfaces should not be
  // normalized in order to allow full -1.0~1.0 range of actuation
}

}  // namespace projectairsim
}  // namespace microsoft