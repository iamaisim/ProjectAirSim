// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef PHYSICS_INCLUDE_MESSAGE_COMMON_MESSAGE_UTILS_HPP_
#define PHYSICS_INCLUDE_MESSAGE_COMMON_MESSAGE_UTILS_HPP_

#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

struct KinematicsFlatMsgpack {
  float pose_position_x = 0.0f;
  float pose_position_y = 0.0f;
  float pose_position_z = 0.0f;
  float pose_orientation_x = 0.0f;
  float pose_orientation_y = 0.0f;
  float pose_orientation_z = 0.0f;
  float pose_orientation_w = 0.0f;
  float twist_linear_x = 0.0f;
  float twist_linear_y = 0.0f;
  float twist_linear_z = 0.0f;
  float twist_angular_x = 0.0f;
  float twist_angular_y = 0.0f;
  float twist_angular_z = 0.0f;
  float accels_linear_x = 0.0f;
  float accels_linear_y = 0.0f;
  float accels_linear_z = 0.0f;
  float accels_angular_x = 0.0f;
  float accels_angular_y = 0.0f;
  float accels_angular_z = 0.0f;
  MSGPACK_DEFINE_MAP(pose_position_x, pose_position_y, pose_position_z,
                     pose_orientation_x, pose_orientation_y, pose_orientation_z,
                     pose_orientation_w, twist_linear_x, twist_linear_y,
                     twist_linear_z, twist_angular_x, twist_angular_y,
                     twist_angular_z, accels_linear_x, accels_linear_y,
                     accels_linear_z, accels_angular_x, accels_angular_y,
                     accels_angular_z);

  KinematicsFlatMsgpack() {}

  KinematicsFlatMsgpack(float pose_position_x_val, float pose_position_y_val,
                        float pose_position_z_val, float pose_orientation_x_val,
                        float pose_orientation_y_val,
                        float pose_orientation_z_val,
                        float pose_orientation_w_val, float twist_linear_x_val,
                        float twist_linear_y_val, float twist_linear_z_val,
                        float twist_angular_x_val, float twist_angular_y_val,
                        float twist_angular_z_val, float accels_linear_x_val,
                        float accels_linear_y_val, float accels_linear_z_val,
                        float accels_angular_x_val, float accels_angular_y_val,
                        float accels_angular_z_val)
      : pose_position_x(pose_position_x_val),
        pose_position_y(pose_position_y_val),
        pose_position_z(pose_position_z_val),
        pose_orientation_x(pose_orientation_x_val),
        pose_orientation_y(pose_orientation_y_val),
        pose_orientation_z(pose_orientation_z_val),
        pose_orientation_w(pose_orientation_w_val),
        twist_linear_x(twist_linear_x_val),
        twist_linear_y(twist_linear_y_val),
        twist_linear_z(twist_linear_z_val),
        twist_angular_x(twist_angular_x_val),
        twist_angular_y(twist_angular_y_val),
        twist_angular_z(twist_angular_z_val),
        accels_linear_x(accels_linear_x_val),
        accels_linear_y(accels_linear_y_val),
        accels_linear_z(accels_linear_z_val),
        accels_angular_x(accels_angular_x_val),
        accels_angular_y(accels_angular_y_val),
        accels_angular_z(accels_angular_z_val) {}
};

struct WrenchPointFlatMsgpack {
  float force_x = 0.0f;
  float force_y = 0.0f;
  float force_z = 0.0f;
  float torque_x = 0.0f;
  float torque_y = 0.0f;
  float torque_z = 0.0f;
  float position_x = 0.0f;
  float position_y = 0.0f;
  float position_z = 0.0f;
  MSGPACK_DEFINE_MAP(force_x, force_y, force_z, torque_x, torque_y, torque_z,
                     position_x, position_y, position_z);

  WrenchPointFlatMsgpack() {}

  WrenchPointFlatMsgpack(float force_x_val, float force_y_val,
                         float force_z_val, float torque_x_val,
                         float torque_y_val, float torque_z_val,
                         float position_x_val, float position_y_val,
                         float position_z_val)
      : force_x(force_x_val),
        force_y(force_y_val),
        force_z(force_z_val),
        torque_x(torque_x_val),
        torque_y(torque_y_val),
        torque_z(torque_z_val),
        position_x(position_x_val),
        position_y(position_y_val),
        position_z(position_z_val) {}
};

struct EnvironmentInfoFlatMsgpack {
  double geopoint_latitude = 0.0;
  double geopoint_longitude = 0.0;
  float geopoint_altitude = 0.0f;
  float gravity_x = 0.0f;
  float gravity_y = 0.0f;
  float gravity_z = 0.0f;
  float temperature = 0.0f;   // Air temperature (e.g., by altitude) (Kelvin)
  float air_pressure = 0.0f;  // Atmospheric pressure (Pascal)
  float air_density = 0.0f;   // Air density (kg/m^3)
  MSGPACK_DEFINE_MAP(geopoint_latitude, geopoint_longitude, geopoint_altitude,
                     gravity_x, gravity_y, gravity_z, temperature, air_pressure,
                     air_density);

  EnvironmentInfoFlatMsgpack() {}

  EnvironmentInfoFlatMsgpack(double geopoint_latitude_val,
                             double geopoint_longitude_val,
                             float geopoint_altitude_val, float gravity_x_val,
                             float gravity_y_val, float gravity_z_val,
                             float temperature_val, float air_pressure_val,
                             float air_density_val)
      : geopoint_latitude(geopoint_latitude_val),
        geopoint_longitude(geopoint_longitude_val),
        geopoint_altitude(geopoint_altitude_val),
        gravity_x(gravity_x_val),
        gravity_y(gravity_y_val),
        gravity_z(gravity_z_val),
        temperature(temperature_val),
        air_pressure(air_pressure_val),
        air_density(air_density_val) {}
};

struct CollisionInfoFlatMsgpack {
  bool has_collided = false;
  float normal_x = 0.0f;
  float normal_y = 0.0f;
  float normal_z = 0.0f;
  float impact_point_x = 0.0f;
  float impact_point_y = 0.0f;
  float impact_point_z = 0.0f;
  float position_x = 0.0f;
  float position_y = 0.0f;
  float position_z = 0.0f;
  float penetration_depth = 0.0f;
  uint64_t time_stamp = 0;
  std::string object_name;
  int segmentation_id = -1;
  MSGPACK_DEFINE_MAP(has_collided, normal_x, normal_y, normal_z, impact_point_x,
                     impact_point_y, impact_point_z, position_x, position_y,
                     position_z, penetration_depth, time_stamp, object_name,
                     segmentation_id);

  CollisionInfoFlatMsgpack() {}

  CollisionInfoFlatMsgpack(bool has_collided_val, float normal_x_val,
                           float normal_y_val, float normal_z_val,
                           float impact_point_x_val, float impact_point_y_val,
                           float impact_point_z_val, float position_x_val,
                           float position_y_val, float position_z_val,
                           float penetration_depth_val, uint64_t time_stamp_val,
                           const std::string& object_name_val,
                           int segmentation_id_val)
      : has_collided(has_collided_val),
        normal_x(normal_x_val),
        normal_y(normal_y_val),
        normal_z(normal_z_val),
        impact_point_x(impact_point_x_val),
        impact_point_y(impact_point_y_val),
        impact_point_z(impact_point_z_val),
        position_x(position_x_val),
        position_y(position_y_val),
        position_z(position_z_val),
        penetration_depth(penetration_depth_val),
        time_stamp(time_stamp_val),
        object_name(object_name_val),
        segmentation_id(segmentation_id_val) {}
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_MESSAGE_COMMON_MESSAGE_UTILS_HPP_
