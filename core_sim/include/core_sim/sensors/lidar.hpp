// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_LIDAR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_LIDAR_HPP_

#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core_sim/message/lidar_message.hpp"
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
// Lidar settings

// Types of lidar
enum class LidarKind {
  kGenericCylindrical,  // Generic lidar with cylindrical scan pattern
  kGPUCylindrical,      // Experimental GPU-version of the cylindrical pattern
  kGenericRosette,      // Generic lidar with rosette scan pattern
};

// Lidar setting values
//
// Note that not all settings apply to all types of Lidar.
struct LidarSettings {
  LidarKind lidar_kind;  // The type of lidar being simulated
  Transform origin;      // Sensor pose with respect to robot
  Quaternion scan_orientation =
      Quaternion::Identity();  // Specifies the scan axis orientation,
                               // defaulting up along -Z axis.  For the
                               // cylindrical pattern this specifies the
                               // direction of the cylinder axis while for the
                               // rosette pattern this specifies the direction
                               // to the center of the rosette.
  int number_of_channels;      // Number of simultaneous scan channels
  float range;                 // Maximum sense range (meters)
  int points_per_second = 100000;  // Number of points returned per second
  float report_frequency =
      0.0f;  // Sensor report frequency (<= 0 means as high as possible)
  float horizontal_rotation_frequency =
      10.0f;  // Horizontal scan rate (rotations/sec)
  float horizontal_fov_start_deg =
      0.0f;  // Horizontal field-of-view starting angle (degrees inclusive)
  float horizontal_fov_end_deg =
      360.0f;  // Horizontal field-of-view ending angle (degrees inclusive)
  float vertical_rotation_frequency =
      0.0f;  // Vertical scan rate (rotations/sec) (generic_rosette)
  float vertical_fov_lower_deg =
      -45.0f;  // Vertical field-of-view ending angle (degrees inclusive)
  float vertical_fov_upper_deg =
      -15.0f;  // Vertical field-of-view starting angle (degrees inclusive)
  float radial_scaling = 1.0f;  // Horizontal pattern scaling (generic_rosette)
  bool disable_self_hits = false;  // If true, filter out hits from owner robot
  bool report_no_return_points =
      false;  // If false, points with no LIDAR return are skipped; if true,
              // no_return_point_report value is reported
  Vector3 no_return_point_value =
      Vector3(0.0f, 0.0f, 0.0f);  // When report_no_return_points is true, value
                                  // to return for points with no LIDAR return
  bool draw_debug_points = false;  // If true, add markers at each sensor hit
                                   // points in the Unreal scene
  float distance_between_lasers =
      0.0;  // distance between mounts on multiple lasers (centimeters)
  float angle_between_lasers_pitch_max =
      0.0;  // max pitch angle between multiple lasers (degrees)
  float angle_between_lasers_pitch_min =
      0.0;  // min pitch angle between multiple lasers (degrees)
  float angle_between_lasers_yaw_max =
      0.0;  // max yaw angle between multiple lasers (degrees)
  float angle_between_lasers_yaw_min =
      0.0;  // min yaw angle between multiple lasers (degrees)
  bool report_point_cloud =
      true;  // if true, lidar output messages will contain a point cloud
  bool report_azimuth_elevation_range =
      false;  // if true, lidar output messages will contain azimuth, elevation,
              // and range

  // Default values to generic cylindrical scan with values similar to Velodyne
  // Puck
  LidarSettings(LidarKind lidar_kind_in = LidarKind::kGenericCylindrical,
                Transform origin_in = Transform(Vector3(0, 0, -1),
                                                Quaternion::Identity()),
                Quaternion scan_orientation_in = Quaternion::Identity(),
                int number_of_channels_in = 16, float range_in = 100.0f,
                int points_per_second_in = 100000,
                float report_frequency_in = 0.0f,
                float horizontal_rotation_frequency_in = 10.0f,
                float horizontal_fov_start_deg_in = 0.0f,
                float horizontal_fov_end_deg_in = 360.0f,
                float vertical_rotation_frequency_in = 0.0f,
                float vertical_fov_lower_deg_in = -45.0f,
                float vertical_fov_upper_deg_in = -15.0f,
                float radial_scaling_in = 1.0f,
                bool disable_self_hits_in = false,
                bool report_no_return_points_in = false,
                Vector3 no_return_point_value_in = Vector3(0.0f, 0.0f, 0.0f),
                bool draw_debug_points_in = false,
                float distance_between_lasers_in = 0.0,
                float angle_between_lasers_pitch_max_in = 0.0,
                float angle_between_lasers_pitch_min_in = 0.0,
                float angle_between_lasers_yaw_max_in = 0.0,
                float angle_between_lasers_yaw_min_in = 0.0,
                bool report_point_cloud_in = true,
                bool report_azimuth_elevation_range_in = false)
      : lidar_kind(lidar_kind_in),
        origin(origin_in),
        scan_orientation(scan_orientation_in),
        number_of_channels(number_of_channels_in),
        range(range_in),
        points_per_second(points_per_second_in),
        report_frequency(report_frequency_in),
        horizontal_rotation_frequency(horizontal_rotation_frequency_in),
        horizontal_fov_start_deg(horizontal_fov_start_deg_in),
        horizontal_fov_end_deg(horizontal_fov_end_deg_in),
        vertical_rotation_frequency(vertical_rotation_frequency_in),
        vertical_fov_lower_deg(vertical_fov_lower_deg_in),
        vertical_fov_upper_deg(vertical_fov_upper_deg_in),
        radial_scaling(radial_scaling_in),
        disable_self_hits(disable_self_hits_in),
        report_no_return_points(report_no_return_points_in),
        no_return_point_value(no_return_point_value_in),
        draw_debug_points(draw_debug_points_in),
        distance_between_lasers(distance_between_lasers_in),
        angle_between_lasers_pitch_max(angle_between_lasers_pitch_max_in),
        angle_between_lasers_pitch_min(angle_between_lasers_pitch_max_in),
        angle_between_lasers_yaw_max(angle_between_lasers_yaw_max_in),
        angle_between_lasers_yaw_min(angle_between_lasers_yaw_min_in),
        report_point_cloud(report_point_cloud_in),
        report_azimuth_elevation_range(report_azimuth_elevation_range_in) {}
};

// Lidar type

class Lidar : public Sensor {
 public:
  Lidar();

  const LidarSettings& GetLidarSettings() const;

  void BeginUpdate();

  void PublishLidarMsg(const LidarMessage& lidar_msg);

  void EndUpdate();

 private:
  friend class Robot;
  friend class SensorImpl;

  Lidar(const std::string& id, bool is_enabled, const std::string& parent_link,
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

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_LIDAR_HPP_
