// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_IMU_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_IMU_HPP_

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core_sim/message/imu_message.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class StateManager;
class ServiceManager;

//! IMU parameters
// See docs/sensors/imu.md for info

struct ImuParams {
  struct Accelerometer {
    float gravity =
        9.80665f;  // m/s^2  TODO: Use value def by phy engine
    float velocity_random_walk = 0.24f * gravity / 1.0E3f;
    float tau = 800;
    float bias_stability = 36.0f * 1E-6f * gravity;
    Vector3 turn_on_bias = Vector3(0, 0, 0);
  } accelerometer;

  struct Gyroscope {
    float angle_random_walk =
        0.3 / sqrt(3600.f) * M_PI / 180;  // deg/sqrt(hr) --> rad/sqrt(sec)
    float tau = 500;
    float bias_stability = 4.6f / 3600 * M_PI / 180;  // deg/hr --> rad/sec
    Vector3 turn_on_bias = Vector3(0, 0, 0);
  } gyro;

  float min_sample_time = 1 / 1000.0f;  // Internal IMU frequency
};

//! IMU sensor type

class Imu : public Sensor {
 public:
  Imu();

  const ImuParams& GetImuSettings() const;

  void BeginUpdate();
  void Update(const TimeNano, const TimeNano) override;
  ImuMessage getOutput() const;
  void EndUpdate();

 private:
  friend class Robot;
  friend class SensorImpl;

  Imu(const std::string& id, bool is_enabled, const std::string& parent_link,
      const Logger& logger, const TopicManager& topic_manager,
      const std::string& parent_topic_path,
      const ServiceManager& service_manager, const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  void Initialize(const Kinematics&, const Environment&) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_IMU_HPP_
