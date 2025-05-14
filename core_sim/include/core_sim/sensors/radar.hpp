// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_RADAR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_RADAR_HPP_

#include <limits>
#include <string>

#include "core_sim/sensors/sensor.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class ServiceManager;
class StateManager;
class RadarDetectionMessage;
class RadarTrackMessage;

struct RadarMask {
  float azimuth_min = -std::numeric_limits<float>::max();
  float azimuth_max = std::numeric_limits<float>::max();
  float elevation_min = -std::numeric_limits<float>::max();
  float elevation_max = std::numeric_limits<float>::max();
  float range_min = 0.0f;
  float range_max = std::numeric_limits<float>::max();
  float velocity_min = -std::numeric_limits<float>::max();
  float velocity_max = std::numeric_limits<float>::max();
  float rcs_sqm_min = 0.0f;
  float rcs_sqm_max = std::numeric_limits<float>::max();
};

struct RadarSettings {
  Transform origin_setting;
  float fov_azimuth_min = TransformUtils::ToRadians(-60.0f);
  float fov_azimuth_max = TransformUtils::ToRadians(60.0f);
  float fov_elevation_min = TransformUtils::ToRadians(-40.0f);
  float fov_elevation_max = TransformUtils::ToRadians(40.0f);
  float fov_azimuth_resolution = TransformUtils::ToRadians(1.0f);
  float fov_elevation_resolution = TransformUtils::ToRadians(3.0f);
  float range_max = 2000.0f;         // m
  float range_resolution = 3.0f;     // m
  float velocity_max = 200.0f;       // m/s (3.6 kph per m/s)
  float velocity_resolution = 1.0f;  // m/s
  float detection_interval = 0.02f;  // sec
  float track_interval = 0.1f;       // sec
  float rcs_adjust_factor = 1.0f;
  bool draw_debug_points = false;
  std::vector<RadarMask> masks = {};
};

struct RadarBeamPoint {
  float azimuth = 0.0f;
  float elevation = 0.0f;
  std::vector<RadarMask> beam_masks = {};

  RadarBeamPoint() {}
  RadarBeamPoint(float in_azimuth, float in_elevation,
                 const std::vector<RadarMask>& in_beam_masks)
      : azimuth(in_azimuth),
        elevation(in_elevation),
        beam_masks(in_beam_masks) {}
};

struct RadarDetection {
  float range = 0.0f;
  float azimuth = 0.0f;
  float elevation = 0.0f;
  float velocity = 0.0f;
  float rcs_sqm =
      0.0f;  // Radar Cross-Section in square meters (the area of a theoretical
             // perfectly reflecting sphere that would have the same detection
             // strength as the detected object). This is not the actual size of
             // the object, but is impacted by its size, material, reflection
             // angles, etc. Bird ~0.01 m2, Human ~1 m2, Airplane ~3 m2
             // https://en.wikipedia.org/wiki/Radar_cross-section#Measurement

  RadarDetection() {}
  RadarDetection(float in_range, float in_azimuth, float in_elevation,
                 float in_velocity, float in_rcs)
      : range(in_range),
        azimuth(in_azimuth),
        elevation(in_elevation),
        velocity(in_velocity),
        rcs_sqm(in_rcs) {}
};

struct RadarTrack {
  int id = -1;
  float azimuth_est = 0.0f;
  float elevation_est = 0.0f;
  float range_est = 0.0f;
  Vector3 position_est = {0, 0, 0};
  Vector3 velocity_est = {0, 0, 0};
  Vector3 accel_est = {0, 0, 0};
  float rcs_sqm = 0.0f;  // See comment about Radar Cross-Section above

  RadarTrack() {}
  explicit RadarTrack(int in_id) : id(in_id) {}
  RadarTrack(int in_id, float in_azim, float in_elev, float in_range,
             const Vector3& in_position, const Vector3& in_velocity,
             const Vector3& in_accel, float in_rcs)
      : id(in_id),
        azimuth_est(in_azim),
        elevation_est(in_elev),
        range_est(in_range),
        position_est(in_position),
        velocity_est(in_velocity),
        accel_est(in_accel),
        rcs_sqm(in_rcs) {}
};

class Radar : public Sensor {
 public:
  Radar();

  const RadarSettings& GetRadarSettings() const;

  void BeginUpdate();

  void PublishRadarDetectionMsg(
      const RadarDetectionMessage& radar_detection_msg);

  void PublishRadarTrackMsg(const RadarTrackMessage& radar_track_msg);

  void EndUpdate();

  Vector3 GetSensorVelocity() const;

 private:
  friend class Robot;
  friend class SensorImpl;

  Radar(const std::string& id, bool is_enabled, const std::string& parent_link,
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

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_RADAR_HPP_
