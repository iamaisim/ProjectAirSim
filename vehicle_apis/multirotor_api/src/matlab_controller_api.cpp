// Copyright (C) Microsoft Corporation. All rights reserved.

#include "matlab_controller_api.hpp"

#include "json.hpp"
#include "message/control_model_input_message.hpp"
#include "message/control_model_output_message.hpp"
#include "nng/protocol/reqrep0/req.h"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class MatlabControllerApi

MatlabControllerApi::MatlabControllerApi(const Robot& robot)
    : sim_robot_(robot) {
  LoadSettings(robot);
}

//---------------------------------------------------------------------------
// IController overrides

void MatlabControllerApi::BeginUpdate() {
  GetSensors(sim_robot_);
  running_ = true;
}

void MatlabControllerApi::EndUpdate() {
  running_ = false;
  // Close the socket
  int rv = nng_close(nng_socket_);

  if (rv != 0) {
    GetLogger().LogTrace("MatlabControllerApi", "nng_close() return value: %d",
                         rv);
  }

  connected_ = false;
}

void MatlabControllerApi::Reset() {
  std::fill(motor_output_.begin(), motor_output_.end(), 0.0f);
  // TODO: can/should we reset the matlab model from here?
}

void MatlabControllerApi::SetKinematics(const Kinematics* kinematics) {
  kinematics_ = kinematics;
}

void MatlabControllerApi::Update() {
  auto time_stamp = static_cast<uint64_t>(SimClock::Get()->NowSimNanos());

  // ------------------------------------------------------------

  if (!connected_ && running_.load()) {
    ConnectToMatlab();
  }

  // ------------------------------------------------------------

  // Send control message to the matlab control model
  KinematicsFlatMsgpack kinematics;
  kinematics.accels_angular_x = kinematics_->accels.angular.x();
  kinematics.accels_angular_y = kinematics_->accels.angular.y();
  kinematics.accels_angular_z = kinematics_->accels.angular.z();
  kinematics.accels_linear_x = kinematics_->accels.linear.x();
  kinematics.accels_linear_y = kinematics_->accels.linear.y();
  kinematics.accels_linear_z = kinematics_->accels.linear.z();
  kinematics.pose_orientation_w = kinematics_->pose.orientation.w();
  kinematics.pose_orientation_x = kinematics_->pose.orientation.x();
  kinematics.pose_orientation_y = kinematics_->pose.orientation.y();
  kinematics.pose_orientation_z = kinematics_->pose.orientation.z();
  kinematics.pose_position_x = kinematics_->pose.position.x();
  kinematics.pose_position_y = kinematics_->pose.position.y();
  kinematics.pose_position_z = kinematics_->pose.position.z();
  kinematics.twist_linear_x = kinematics_->twist.linear.x();
  kinematics.twist_linear_y = kinematics_->twist.linear.y();
  kinematics.twist_linear_z = kinematics_->twist.linear.z();
  kinematics.twist_angular_x = kinematics_->twist.angular.x();
  kinematics.twist_angular_y = kinematics_->twist.angular.y();
  kinematics.twist_angular_z = kinematics_->twist.angular.z();

  float airspeed = 0.0;
  if (airspeed_sensor_ != nullptr) {
    const AirspeedMessage& airspeed_output = airspeed_sensor_->getOutput();
    airspeed = static_cast<float>(airspeed_output.getData()["diff_pressure"]);
  }

  BarometerFlatMsgpack barometer;
  if (barometer_sensor_ != nullptr) {
    const BarometerMessage& barometer_output = barometer_sensor_->getOutput();
    barometer.altitude =
        static_cast<float>(barometer_output.getData()["altitude"]);
    barometer.pressure =
        static_cast<float>(barometer_output.getData()["pressure"]);
    barometer.qnh = static_cast<float>(barometer_output.getData()["qnh"]);
  }

  ImuFlatMsgpack imu;
  if (imu_sensor_ != nullptr) {
    const ImuMessage& imu_output = imu_sensor_->getOutput();
    imu.angular_velocity_x = imu_output.GetAngularVelocity().x();
    imu.angular_velocity_y = imu_output.GetAngularVelocity().y();
    imu.angular_velocity_z = imu_output.GetAngularVelocity().z();
    imu.linear_acceleration_x = imu_output.GetLinearAcceleration().x();
    imu.linear_acceleration_y = imu_output.GetLinearAcceleration().y();
    imu.linear_acceleration_z = imu_output.GetLinearAcceleration().z();
    imu.orientation_x = imu_output.GetOrientation().x();
    imu.orientation_y = imu_output.GetOrientation().y();
    imu.orientation_z = imu_output.GetOrientation().z();
    imu.orientation_w = imu_output.GetOrientation().w();
  }

  Vector3FlatMsgpack magnetometer;
  if (magnetometer_sensor_ != nullptr) {
    const MagnetometerMessage& magnetometer_output =
        magnetometer_sensor_->getOutput();
    magnetometer.x = static_cast<float>(
        magnetometer_output.getData()["magnetic_field_body"]["x"]);
    magnetometer.y = static_cast<float>(
        magnetometer_output.getData()["magnetic_field_body"]["y"]);
    magnetometer.z = static_cast<float>(
        magnetometer_output.getData()["magnetic_field_body"]["z"]);
  }

  float distance = 0.0;
  if (distance_sensor_ != nullptr) {
    const DistanceSensorMessage& distance_output =
        distance_sensor_->getOutput();
    distance = static_cast<float>(distance_output.GetCurrentDistance());
  }

  GpsFlatMsgpack gps;
  if (gps_sensor_ != nullptr) {
    const GpsMessage& gps_output = gps_sensor_->getOutput();
    gps.longitude = gps_output.getData()["longitude"];
    gps.latitude = gps_output.getData()["latitude"];
    gps.altitude = gps_output.getData()["altitude"];
    gps.velocity_x = gps_output.getData()["velocity"]["x"];
    gps.velocity_y = gps_output.getData()["velocity"]["y"];
    gps.velocity_z = gps_output.getData()["velocity"]["z"];
  }

  ControlModelInputMessage request_msg(time_stamp, kinematics, airspeed,
                                       barometer, imu, magnetometer, distance,
                                       gps);

  std::string request_str = request_msg.Serialize();

  int rv = nng_send(nng_socket_, request_str.data(), request_str.length(), 0);

  if (rv != 0) {
    GetLogger().LogTrace("MatlabControllerApi",
                         "nng_send() return value: %d length = %d", rv,
                         request_str.length());
  }

  if (rv != 0 || running_.load() == false) {
    connected_ = false;
    return;
  }

  // ------------------------------------------------------------

  // Wait for a response from Matlab with new control signals
  char* msg_data_buf = nullptr;
  size_t msg_data_len;

  rv = NNG_EAGAIN;
  while (rv == NNG_EAGAIN && running_.load() == true) {
    rv = nng_recv(nng_socket_, &msg_data_buf, &msg_data_len,
                  NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);
  }

  if (rv != 0) {
    GetLogger().LogTrace("MatlabControllerApi",
                         "nng_recv() return value: %d length = %d", rv,
                         msg_data_len);
  }

  if (rv != 0 || running_.load() == false) {
    connected_ = false;
    return;
  }

  // Copy the data from the buffer to a string to deserialize (must set the
  // length manually to avoid stopping at '\0' chars that it may contain)
  std::string response_str;
  response_str.resize(msg_data_len);
  response_str.assign(msg_data_buf, msg_data_len);

  // Free the NNG-allocated reply buffer memory
  nng_free(msg_data_buf, msg_data_len);

  ControlModelOutputMessage response_msg;
  response_msg.Deserialize(response_str);

  std::lock_guard<std::mutex> lock(update_lock_);

  for (auto i = 0;
       i < motor_output_.size() && i < response_msg.control_values.size();
       i++) {
    motor_output_.at(i) = response_msg.control_values.at(i);
  }
}

std::vector<float> MatlabControllerApi::GetControlSignals(const std::string& actuator_id) {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);
  if (actuator_map_itr == actuator_id_to_output_idx_map_.end()) {
    GetLogger().LogWarning("MatlabControllerApi",
                           "MatlabControllerApi::GetControlSignal() called for "
                           "invalid actuator: %s",
                           actuator_id.c_str());
    return std::vector<float>(1, 0.f);
  }
  return std::vector<float>(1, motor_output_.at(actuator_map_itr->second));
}

const IController::GimbalState& MatlabControllerApi::GetGimbalSignal(
    const std::string& gimbal_id) {
  throw std::runtime_error(
      "This flight controller does not support externally-controlled gimbal "
      "devices.");
}

//---------------------------------------------------------------------------
// MatlabControllerApi methods

void MatlabControllerApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  // GetJsonObject
  const json& matlab_controller_api_settings_json =
      controller_settings_json.value("matlab-controller-api-settings",
                                     "{ }"_json);

  // GetArray
  const json& actuator_order_json =
      matlab_controller_api_settings_json.value("actuator-order", "[ ]"_json);

  motor_output_.assign(actuator_order_json.size(), 0.0f);

  // build connection string
  const json& scene_connection_json =
      json::parse(robot.GetControlConnectionSettings());

  if (scene_connection_json.contains("ip") &&
      scene_connection_json.contains("port")) {
    connection_string_ = "tcp://";
    connection_string_ += scene_connection_json.value("ip", "127.0.0.1");
    connection_string_ += ":";
    connection_string_ +=
        std::to_string(scene_connection_json.value("port", 8998));
  } else {
    const json& robot_connection_json =
        matlab_controller_api_settings_json.value("control-connection",
                                                  "[ ]"_json);
    connection_string_ = "tcp://";
    connection_string_ += robot_connection_json.value("ip", "127.0.0.1");
    connection_string_ += ":";
    connection_string_ +=
        std::to_string(robot_connection_json.value("port", 8998));
  }

  try {
    int output_idx = 0;
    for (auto& actuator_json : actuator_order_json) {
      // Add actuator ID to map for this output index
      std::string id = actuator_json.value("id", "");
      actuator_id_to_output_idx_map_.insert({id, output_idx});

      // Set initial control value if specified
      if (actuator_json.contains("initial-value")) {
        float init_val = actuator_json.value("initial-value", 0.0f);
        motor_output_.at(output_idx) = init_val;
      }

      output_idx++;
    }
  } catch (...) {
    throw;
  }
}

void MatlabControllerApi::GetSensors(const Robot& robot) {
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
}

void MatlabControllerApi::ConnectToMatlab() {
  // Create a Request socket to dial Matlab server
  int rv = nng_req0_open(&nng_socket_);

  // Connect to server
  // TODO Give up after a timeout?
  GetLogger().LogTrace("MatlabControllerApi",
                       "Trying to connect to Matlab physics model...");
  do {
    rv = nng_dial(nng_socket_, connection_string_.c_str(), NULL, 0);
    GetLogger().LogTrace("MatlabControllerApi", "NNG dial return value: %d",
                         rv);
    if (rv != 0) std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (rv != 0);

  connected_ = true;
}

}  // namespace projectairsim
}  // namespace microsoft
