// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_DISTANCE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_DISTANCE_HPP_

#include <exception>
#include <string>

#include "core_sim/message/distance_sensor_message.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

// --------------------------------------------------------------------------------------------
// Distance settings

// Distance setting values
struct DistanceSensorSettings {
  Transform origin;  // Sensor pose with respect to robot
  float report_frequency =
      0.0f;  // Sensor report frequency (<= 0 means as high as possible)
  float min_distance = 0.2f;  // Minimum distance measured by the sensor
                              // (meters), only used for Mavlink messages
  float max_distance =
      40.0f;  // Maximum distance measured by the sensor (meters)
  bool draw_debug_points = false;  // If true, add markers at each sensor hit
                                   // points in the Unreal scene

  DistanceSensorSettings(Transform origin_in = Transform(Vector3(0, 0, -1),
                                                   Quaternion::Identity()),
                   float report_frequency_in = 0.0f,
                   float min_distance_in = 0.2f, float max_distance_in = 40.0f,
                   bool draw_debug_points_in = false)
      : origin(origin_in),
        report_frequency(report_frequency_in),
        min_distance(min_distance_in),
        max_distance(max_distance_in),
        draw_debug_points(draw_debug_points_in) {}
};

// Distance class

class DistanceSensor : public Sensor {
 public:
  DistanceSensor();

  const DistanceSensorSettings& GetDistanceSensorSettings() const;

  void BeginUpdate();

  void PublishDistanceSensorMsg(const DistanceSensorMessage& distance_msg);

  void EndUpdate();

  DistanceSensorMessage getOutput();

 private:
  friend class Robot;
  friend class SensorImpl;

  DistanceSensor(const std::string& id, bool is_enabled, const std::string& parent_link,
        const Logger& logger, const TopicManager& topic_manager,
        const std::string& parent_topic_path,
        const ServiceManager& service_manager,
        const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  void Initialize(const Kinematics&, const Environment&) override;

  void Update(const TimeNano, const TimeNano) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_DISTANCE_HPP_
