// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "matlab_vtol_controller_api.hpp"

#include <cmath>
#include <stdexcept>

#include "core_sim/sensors/airspeed.hpp"
#include "core_sim/sensors/barometer.hpp"
#include "core_sim/sensors/distance_sensor.hpp"
#include "core_sim/sensors/gps.hpp"
#include "core_sim/sensors/imu.hpp"
#include "core_sim/sensors/magnetometer.hpp"
#include "json.hpp"
#include "nng/protocol/reqrep0/req.h"

namespace microsoft {
namespace projectairsim {

MatlabVTOLControllerApi::MatlabVTOLControllerApi(const Robot& robot,
                                                 TransformTree* ptransformtree)
    : VTOLFWApiBase(robot, ptransformtree) {
  LoadSettings();
}

MatlabVTOLControllerApi::~MatlabVTOLControllerApi() { CloseConnection(); }

void MatlabVTOLControllerApi::LoadSettings() {
  const auto config = json::parse(sim_robot_.GetControllerSettings());
  const auto& settings = config.at("matlab-vtol-controller-api-settings");
  const auto& order = settings.at("actuator-order");
  if (!order.is_array() || order.empty()) {
    throw std::invalid_argument("External controller requires actuator-order");
  }
  for (const auto& item : order) {
    ExternalEffector effector;
    effector.id = item.at("id").get<std::string>();
    effector.kind = item.at("kind").get<std::string>();
    effector.units = item.at("units").get<std::string>();
    effector.initial_value = item.value("initial-value", 0.0f);
    if (effector.id.empty() || effector.kind.empty() ||
        effector.units.empty() || !std::isfinite(effector.initial_value) ||
        !actuator_indices_.emplace(effector.id, effectors_.size()).second) {
      throw std::invalid_argument("Invalid or duplicate external effector");
    }
    effectors_.push_back(effector);
  }
  request_timeout_ms_ = settings.value("request-timeout-ms", 1000);
  command_period_ = settings.value("command-period", 0.02f);
  takeoff_z_ = settings.value("takeoff-z", -3.0f);
  if (request_timeout_ms_ <= 0 || !std::isfinite(command_period_) ||
      command_period_ <= 0 || !std::isfinite(takeoff_z_) || takeoff_z_ >= 0) {
    throw std::invalid_argument(
        "Invalid external-controller timing or takeoff settings");
  }
  const auto& scene_connection = sim_robot_.GetControlConnectionSettings();
  auto connection =
      scene_connection.empty() ? json::object() : json::parse(scene_connection);
  if (!connection.contains("ip") || !connection.contains("port")) {
    connection = settings.value("control-connection", json::object());
  }
  const auto ip = connection.value("ip", std::string("127.0.0.1"));
  const int port = connection.value("port", 8998);
  if (ip.empty() || port < 1 || port > 65535) {
    throw std::invalid_argument("Invalid external-controller connection");
  }
  connection_string_ = "tcp://" + ip + ":" + std::to_string(port);
  RestoreInitialOutputs();
}

void MatlabVTOLControllerApi::BeginUpdate() {
  VTOLFWApiBase::BeginUpdate();
  std::lock_guard<std::mutex> lock(update_lock_);
  running_ = true;
}

void MatlabVTOLControllerApi::EndUpdate() {
  {
    std::lock_guard<std::mutex> lock(update_lock_);
    running_ = false;
    ready_ = false;
    setpoint_.armed = false;
    setpoint_.api_enabled = false;
    SetIdle();
    RestoreInitialOutputs();
    ++lifecycle_generation_;
  }
  VTOLFWApiBase::EndUpdate();
  CloseConnection();
}

void MatlabVTOLControllerApi::CloseConnection() {
  std::lock_guard<std::mutex> lock(socket_lock_);
  if (socket_open_) nng_close(socket_);
  socket_open_ = false;
}

void MatlabVTOLControllerApi::RestoreInitialOutputs() {
  outputs_.clear();
  for (const auto& effector : effectors_)
    outputs_.push_back(effector.initial_value);
}

void MatlabVTOLControllerApi::SetIdle() {
  setpoint_.mode = ExternalControlSetpoint::Idle;
  setpoint_.frame = ExternalControlSetpoint::WorldNED;
  setpoint_.x = setpoint_.y = setpoint_.z = setpoint_.yaw = 0;
  setpoint_.yaw_is_rate = true;
  setpoint_.request = ExternalControlSetpoint::None;
}

void MatlabVTOLControllerApi::Reset() {
  CancelLastTask();
  std::lock_guard<std::mutex> lock(update_lock_);
  setpoint_ = ExternalControlSetpoint();
  ready_ = false;
  landed_ = true;
  RestoreInitialOutputs();
  ++lifecycle_generation_;
}

void MatlabVTOLControllerApi::SetKinematics(const Kinematics* kinematics) {
  std::lock_guard<std::mutex> lock(update_lock_);
  has_kinematics_ = kinematics != nullptr;
  if (kinematics) kinematics_estimated_ = *kinematics;
}

Kinematics MatlabVTOLControllerApi::GetKinematicsEstimated() const {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (!has_kinematics_)
    throw std::runtime_error("External controller has no kinematics");
  return kinematics_estimated_;
}

ControlModelInputMessage MatlabVTOLControllerApi::GetState() const {
  auto k = GetKinematicsEstimated();
  KinematicsFlatMsgpack flat(
      k.pose.position.x(), k.pose.position.y(), k.pose.position.z(),
      k.pose.orientation.x(), k.pose.orientation.y(), k.pose.orientation.z(),
      k.pose.orientation.w(), k.twist.linear.x(), k.twist.linear.y(),
      k.twist.linear.z(), k.twist.angular.x(), k.twist.angular.y(),
      k.twist.angular.z(), k.accels.linear.x(), k.accels.linear.y(),
      k.accels.linear.z(), k.accels.angular.x(), k.accels.angular.y(),
      k.accels.angular.z());
  ControlModelInputMessage state(
      static_cast<uint64_t>(SimClock::Get()->NowSimNanos()), flat, 0,
      BarometerFlatMsgpack(), ImuFlatMsgpack(), Vector3FlatMsgpack(), 0,
      GpsFlatMsgpack());
  // Match the legacy bridge's first enabled sensor of each supported type.
  bool airspeed = false, barometer = false, imu = false, magnetometer = false,
       distance = false, gps = false;
  for (auto& ref : sim_robot_.GetSensors()) {
    auto& sensor = ref.get();
    if (!sensor.IsEnabled()) continue;
    if (sensor.GetType() == SensorType::kAirspeed && !airspeed) {
      state.airspeed = static_cast<AirspeedSensor&>(sensor)
                           .getOutput()
                           .getData()["diff_pressure"];
      airspeed = true;
    } else if (sensor.GetType() == SensorType::kBarometer && !barometer) {
      const auto& data = static_cast<Barometer&>(sensor).getOutput().getData();
      state.barometer =
          BarometerFlatMsgpack(data["altitude"], data["pressure"], data["qnh"]);
      barometer = true;
    } else if (sensor.GetType() == SensorType::kImu && !imu) {
      const auto& data = static_cast<Imu&>(sensor).getOutput();
      state.imu.orientation_x = data.GetOrientation().x();
      state.imu.orientation_y = data.GetOrientation().y();
      state.imu.orientation_z = data.GetOrientation().z();
      state.imu.orientation_w = data.GetOrientation().w();
      state.imu.angular_velocity_x = data.GetAngularVelocity().x();
      state.imu.angular_velocity_y = data.GetAngularVelocity().y();
      state.imu.angular_velocity_z = data.GetAngularVelocity().z();
      state.imu.linear_acceleration_x = data.GetLinearAcceleration().x();
      state.imu.linear_acceleration_y = data.GetLinearAcceleration().y();
      state.imu.linear_acceleration_z = data.GetLinearAcceleration().z();
      imu = true;
    } else if (sensor.GetType() == SensorType::kMagnetometer && !magnetometer) {
      const auto& data =
          static_cast<Magnetometer&>(sensor).getOutput().getData();
      state.magnetometer = Vector3FlatMsgpack(data["magnetic_field_body"]["x"],
                                              data["magnetic_field_body"]["y"],
                                              data["magnetic_field_body"]["z"]);
      magnetometer = true;
    } else if (sensor.GetType() == SensorType::kDistanceSensor && !distance) {
      state.distance =
          static_cast<DistanceSensor&>(sensor).getOutput().GetCurrentDistance();
      distance = true;
    } else if (sensor.GetType() == SensorType::kGps && !gps) {
      const auto& data = static_cast<Gps&>(sensor).getOutput().getData();
      state.gps.longitude = data["longitude"];
      state.gps.latitude = data["latitude"];
      state.gps.altitude = data["altitude"];
      state.gps.velocity_x = data["velocity"]["x"];
      state.gps.velocity_y = data["velocity"]["y"];
      state.gps.velocity_z = data["velocity"]["z"];
      gps = true;
    }
  }
  return state;
}

void MatlabVTOLControllerApi::Update() {
  // Exchanges run on the simulation thread; shutdown waits for their timeouts.
  // Robot shutdown holds the robot mutex while closing this transport.
  const auto snapshot = sim_robot_.GetKinematicsSnapshot();
  std::lock_guard<std::mutex> transport_lock(socket_lock_);
  VTOLControlModelInputMessage request;
  uint64_t generation;
  {
    std::lock_guard<std::mutex> lock(update_lock_);
    if (!running_ || !has_kinematics_) return;
    kinematics_estimated_ = snapshot;
    request.setpoint = setpoint_;
    request.effectors = effectors_;
    generation = lifecycle_generation_;
  }
  request.state = GetState();
  try {
    if (!socket_open_) {
      if (nng_req0_open(&socket_) != 0)
        throw std::runtime_error("NNG open failed");
      socket_open_ = true;
      if (nng_setopt_ms(socket_, NNG_OPT_SENDTIMEO, request_timeout_ms_) != 0 ||
          nng_setopt_ms(socket_, NNG_OPT_RECVTIMEO, request_timeout_ms_) != 0 ||
          nng_dial(socket_, connection_string_.c_str(), nullptr,
                   NNG_FLAG_NONBLOCK) != 0) {
        throw std::runtime_error("NNG connection setup failed");
      }
    }
    auto bytes = request.Serialize();
    int rv = nng_send(socket_, &bytes[0], bytes.size(), 0);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    char* data = nullptr;
    size_t size = 0;
    rv = nng_recv(socket_, &data, &size, NNG_FLAG_ALLOC);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    bytes.assign(data, size);
    nng_free(data, size);
    VTOLControlModelOutputMessage response;
    response.Deserialize(bytes);
    if (response.time_stamp != request.state.time_stamp ||
        response.control_values.size() != effectors_.size()) {
      throw std::runtime_error(
          "External-controller timestamp or effector count mismatch");
    }
    std::lock_guard<std::mutex> lock(update_lock_);
    // An in-flight reply cannot undo cancellation, reset, disarm, or shutdown.
    if (!running_ || generation != lifecycle_generation_) return;
    ready_ = response.ready;
    landed_ = response.landed;
    if (ready_ && setpoint_.api_enabled && setpoint_.armed) {
      outputs_ = response.control_values;
    } else {
      RestoreInitialOutputs();
      if (!ready_) setpoint_.armed = false;
    }
  } catch (const std::exception& error) {
    if (socket_open_) nng_close(socket_);
    socket_open_ = false;
    std::lock_guard<std::mutex> lock(update_lock_);
    ready_ = false;
    setpoint_.armed = false;
    SetIdle();
    RestoreInitialOutputs();
    ++lifecycle_generation_;
    sim_robot_.GetLogger().LogWarning(GetControllerName(),
                                      "External controller unavailable: %s",
                                      error.what());
  }
}

std::vector<float> MatlabVTOLControllerApi::GetControlSignals(
    const std::string& id) {
  std::lock_guard<std::mutex> lock(update_lock_);
  auto found = actuator_indices_.find(id);
  return {found == actuator_indices_.end() ? 0.0f : outputs_.at(found->second)};
}

bool MatlabVTOLControllerApi::EnableApiControl() {
  std::lock_guard<std::mutex> lock(update_lock_);
  setpoint_.api_enabled = true;
  ++lifecycle_generation_;
  return true;
}

bool MatlabVTOLControllerApi::DisableApiControl() {
  CancelLastTask();
  std::lock_guard<std::mutex> lock(update_lock_);
  setpoint_.api_enabled = setpoint_.armed = false;
  RestoreInitialOutputs();
  ++lifecycle_generation_;
  return true;
}

bool MatlabVTOLControllerApi::IsApiControlEnabled() {
  std::lock_guard<std::mutex> lock(update_lock_);
  return setpoint_.api_enabled;
}

bool MatlabVTOLControllerApi::Arm(int64_t) {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (!running_ || !ready_ || !has_kinematics_ || !setpoint_.api_enabled)
    return false;
  setpoint_.armed = true;
  ++lifecycle_generation_;
  return true;
}

bool MatlabVTOLControllerApi::Disarm() {
  CancelLastTask();
  std::lock_guard<std::mutex> lock(update_lock_);
  setpoint_.armed = false;
  RestoreInitialOutputs();
  ++lifecycle_generation_;
  return true;
}

bool MatlabVTOLControllerApi::CanArm() const {
  std::lock_guard<std::mutex> lock(update_lock_);
  return running_ && ready_ && has_kinematics_ && setpoint_.api_enabled;
}

ReadyState MatlabVTOLControllerApi::GetReadyState() const {
  std::lock_guard<std::mutex> lock(update_lock_);
  const bool ready = running_ && ready_ && has_kinematics_;
  return {ready, ready ? "" : "Waiting for external controller and kinematics"};
}

LandedState MatlabVTOLControllerApi::GetLandedState() const {
  std::lock_guard<std::mutex> lock(update_lock_);
  return landed_ ? LandedState::Landed : LandedState::Flying;
}

bool MatlabVTOLControllerApi::CancelLastTask() {
  std::lock_guard<std::mutex> lock(update_lock_);
  const bool result = VTOLFWApiBase::CancelLastTask();
  SetIdle();
  ++lifecycle_generation_;
  return result;
}

bool MatlabVTOLControllerApi::SetVTOLMode(VTOLMode mode) {
  if (mode != VTOLMode::Multirotor && mode != VTOLMode::FixedWing) return false;
  std::lock_guard<std::mutex> lock(update_lock_);
  if (!setpoint_.api_enabled) return false;
  setpoint_.vtol_mode = static_cast<int32_t>(mode);
  return true;
}

void MatlabVTOLControllerApi::BeforeTask() {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (!running_ || !ready_ || !setpoint_.api_enabled || !setpoint_.armed) {
    throw std::runtime_error(
        "External controller must be ready, enabled, and armed");
  }
  setpoint_.request = ExternalControlSetpoint::None;
}

void MatlabVTOLControllerApi::AfterTask() {
  std::lock_guard<std::mutex> lock(update_lock_);
  ++lifecycle_generation_;
  if (setpoint_.request != ExternalControlSetpoint::Hover ||
      GetCancelToken().IsCancelled()) {
    SetIdle();
  }
}

void MatlabVTOLControllerApi::SetRequest(uint32_t request) {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (GetCancelToken().IsCancelled() || !running_ || !ready_ ||
      !setpoint_.api_enabled || !setpoint_.armed)
    return;
  setpoint_.request = request;
}

bool MatlabVTOLControllerApi::Takeoff(float timeout, int64_t start) {
  SingleTaskCall lock(this);
  SetRequest(ExternalControlSetpoint::Takeoff);
  return VTOLFWApiBase::Takeoff(timeout, start);
}

bool MatlabVTOLControllerApi::Land(float timeout, int64_t start) {
  SingleTaskCall lock(this);
  SetRequest(ExternalControlSetpoint::Land);
  return VTOLFWApiBase::Land(timeout, start);
}

bool MatlabVTOLControllerApi::Hover(int64_t) {
  SingleTaskCall lock(this);
  SetRequest(ExternalControlSetpoint::Hover);
  const auto state = GetKinematicsEstimated();
  CommandPosition(state.pose.position.x(), state.pose.position.y(),
                  state.pose.position.z(), false,
                  PhysicsUtils::GetYaw<float>(state.pose.orientation));
  return !GetCancelToken().IsCancelled();
}

void MatlabVTOLControllerApi::SetCommand(uint32_t mode, uint32_t frame, float x,
                                         float y, float z, bool yaw_is_rate,
                                         float yaw) {
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
      !std::isfinite(yaw)) {
    throw std::invalid_argument("External-controller setpoint must be finite");
  }
  std::lock_guard<std::mutex> lock(update_lock_);
  if (GetCancelToken().IsCancelled()) return;
  if (!setpoint_.api_enabled || !setpoint_.armed) {
    throw std::runtime_error("External controller is disabled or disarmed");
  }
  setpoint_.mode = mode;
  setpoint_.frame = frame;
  setpoint_.x = x;
  setpoint_.y = y;
  setpoint_.z = z;
  setpoint_.yaw_is_rate = yaw_is_rate;
  setpoint_.yaw = yaw;
}

void MatlabVTOLControllerApi::CommandVelocity(float x, float y, float z,
                                              bool rate, float yaw) {
  SetCommand(ExternalControlSetpoint::Velocity,
             ExternalControlSetpoint::WorldNED, x, y, z, rate, yaw);
}
void MatlabVTOLControllerApi::CommandVelocityZ(float x, float y, float z,
                                               bool rate, float yaw) {
  SetCommand(ExternalControlSetpoint::VelocityZ,
             ExternalControlSetpoint::WorldNED, x, y, z, rate, yaw);
}
void MatlabVTOLControllerApi::CommandVelocityBody(float x, float y, float z,
                                                  bool rate, float yaw) {
  SetCommand(ExternalControlSetpoint::Velocity,
             ExternalControlSetpoint::BodyFRD, x, y, z, rate, yaw);
}
void MatlabVTOLControllerApi::CommandVelocityZBody(float x, float y, float z,
                                                   bool rate, float yaw) {
  SetCommand(ExternalControlSetpoint::VelocityZ,
             ExternalControlSetpoint::BodyFRD, x, y, z, rate, yaw);
}
void MatlabVTOLControllerApi::CommandPosition(float x, float y, float z,
                                              bool rate, float yaw) {
  SetCommand(ExternalControlSetpoint::Position,
             ExternalControlSetpoint::WorldNED, x, y, z, rate, yaw);
}
void MatlabVTOLControllerApi::CommandHeading(float heading, float speed,
                                             float vz) {
  SetCommand(ExternalControlSetpoint::Heading,
             ExternalControlSetpoint::WorldNED, heading, speed, vz, false,
             heading);
}

bool MatlabVTOLControllerApi::MoveByVelocityBodyFrame(
    float vx, float vy, float vz, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, int64_t start) {
  SingleTaskCall lock(this);
  if (duration <= 0) return true;
  const float vehicle_yaw =
      PhysicsUtils::GetYaw<float>(GetKinematicsEstimated().pose.orientation);
  float wx = vx * std::cos(vehicle_yaw) - vy * std::sin(vehicle_yaw);
  float wy = vx * std::sin(vehicle_yaw) + vy * std::cos(vehicle_yaw);
  if (!yaw_is_rate) yaw += vehicle_yaw;
  AdjustYaw(wx, wy, drivetrain, yaw_is_rate, yaw);
  return RunFlightCommand(
             [&]() {
               MoveByVelocityBodyInternal(vx, vy, vz, yaw_is_rate, yaw);
               return false;
             },
             duration, start)
      .IsTimeout();
}

bool MatlabVTOLControllerApi::MoveByVelocityBodyFrameZ(
    float vx, float vy, float z, float duration, DrivetrainType drivetrain,
    bool yaw_is_rate, float yaw, int64_t start) {
  SingleTaskCall lock(this);
  if (duration <= 0) return true;
  const float vehicle_yaw =
      PhysicsUtils::GetYaw<float>(GetKinematicsEstimated().pose.orientation);
  float wx = vx * std::cos(vehicle_yaw) - vy * std::sin(vehicle_yaw);
  float wy = vx * std::sin(vehicle_yaw) + vy * std::cos(vehicle_yaw);
  if (!yaw_is_rate) yaw += vehicle_yaw;
  AdjustYaw(wx, wy, drivetrain, yaw_is_rate, yaw);
  return RunFlightCommand(
             [&]() {
               MoveByVelocityZBodyInternal(vx, vy, z, yaw_is_rate, yaw);
               return false;
             },
             duration, start)
      .IsTimeout();
}

void MatlabVTOLControllerApi::CommandMotorPWMs(float, float, float, float) {
  throw std::runtime_error(
      "External controller does not support motor PWM commands");
}
void MatlabVTOLControllerApi::CommandRollPitchYawrateThrottle(float, float,
                                                              float, float) {
  throw std::runtime_error(
      "External controller does not support attitude/throttle commands");
}
void MatlabVTOLControllerApi::CommandRollPitchYawZ(float, float, float, float) {
  throw std::runtime_error(
      "External controller does not support attitude commands");
}
void MatlabVTOLControllerApi::CommandRollPitchYawThrottle(float, float, float,
                                                          float) {
  throw std::runtime_error(
      "External controller does not support attitude/throttle commands");
}
void MatlabVTOLControllerApi::CommandRollPitchYawrateZ(float, float, float,
                                                       float) {
  throw std::runtime_error(
      "External controller does not support attitude commands");
}
void MatlabVTOLControllerApi::CommandAngleRatesZ(float, float, float, float) {
  throw std::runtime_error(
      "External controller does not support angular-rate commands");
}
void MatlabVTOLControllerApi::CommandAngleRatesThrottle(float, float, float,
                                                        float) {
  throw std::runtime_error(
      "External controller does not support angular-rate/throttle commands");
}
void MatlabVTOLControllerApi::SetControllerGains(uint8_t,
                                                 const std::vector<float>&,
                                                 const std::vector<float>&,
                                                 const std::vector<float>&) {
  throw std::runtime_error(
      "External controller gains belong to the external model");
}

GeoPoint MatlabVTOLControllerApi::GetGpsLocationEstimated() const {
  return sim_robot_.GetEnvironment().env_info.geo_point;
}
const MultirotorApiBase::MultirotorApiParams&
MatlabVTOLControllerApi::GetMultirotorApiParams() const {
  return params_;
}
float MatlabVTOLControllerApi::GetCommandPeriod() const {
  return command_period_;
}
float MatlabVTOLControllerApi::GetTakeoffZ() const { return takeoff_z_; }
float MatlabVTOLControllerApi::GetDistanceAccuracy() const {
  return params_.distance_accuracy;
}

}  // namespace projectairsim
}  // namespace microsoft
