// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_INCLUDE_MESSAGE_COMMON_MESSAGE_UTILS_HPP_
#define MULTIROTOR_API_INCLUDE_MESSAGE_COMMON_MESSAGE_UTILS_HPP_

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
  float pose_orientation_w = 1.0f;
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

// Airspeed is just a float

struct BarometerFlatMsgpack {
  float altitude = 0.0f;
  float pressure = 0.0f;
  float qnh = 0.0f;
  MSGPACK_DEFINE_MAP(altitude, pressure, qnh);

  BarometerFlatMsgpack() {}

  BarometerFlatMsgpack(float altitude_val, float pressure_val, float qnh_val)
      : altitude(altitude_val), pressure(pressure_val), qnh(qnh_val) {}
};

struct ImuFlatMsgpack {
  float orientation_x = 0.0f;
  float orientation_y = 0.0f;
  float orientation_z = 0.0f;
  float orientation_w = 1.0f;
  float angular_velocity_x = 0.0f;
  float angular_velocity_y = 0.0f;
  float angular_velocity_z = 0.0f;
  float linear_acceleration_x = 0.0f;
  float linear_acceleration_y = 0.0f;
  float linear_acceleration_z = 0.0f;
  MSGPACK_DEFINE_MAP(orientation_x, orientation_y, orientation_z, orientation_w,
                     angular_velocity_x, angular_velocity_y, angular_velocity_z,
                     linear_acceleration_x, linear_acceleration_y,
                     linear_acceleration_z);

  ImuFlatMsgpack() {}

  ImuFlatMsgpack(float orientation_x_val, float orientation_y_val,
                 float orientation_z_val, float orientation_w_val,
                 float angular_velocity_x_val, float angular_velocity_y_val,
                 float angular_velocity_z_val, float linear_acceleration_x_val,
                 float linear_acceleration_y_val,
                 float linear_acceleration_z_val)
      : orientation_x(orientation_x_val),
        orientation_y(orientation_y_val),
        orientation_z(orientation_z_val),
        orientation_w(orientation_w_val),
        angular_velocity_x(angular_velocity_x_val),
        angular_velocity_y(angular_velocity_y_val),
        angular_velocity_z(angular_velocity_z_val),
        linear_acceleration_x(linear_acceleration_x_val),
        linear_acceleration_y(linear_acceleration_y_val),
        linear_acceleration_z(linear_acceleration_z_val) {}
};

// Magnetometer, Gyro Sensor
struct Vector3FlatMsgpack {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  MSGPACK_DEFINE_MAP(x, y, z);

  Vector3FlatMsgpack() {}

  Vector3FlatMsgpack(float x_val, float y_val, float z_val)
      : x(x_val), y(y_val), z(z_val) {}
};

// Distance sensor is just a float

struct GpsFlatMsgpack {
  double latitude = 0.0;
  double longitude = 0.0;
  float altitude = 0.0f;
  float velocity_x = 0.0f;
  float velocity_y = 0.0f;
  float velocity_z = 0.0f;
  MSGPACK_DEFINE_MAP(latitude, longitude, altitude, velocity_x, velocity_y,
                     velocity_z);

  GpsFlatMsgpack() {}

  GpsFlatMsgpack(double latitude_val, double longitude_val, float altitude_val,
                 float velocity_x_val, float velocity_y_val,
                 float velocity_z_val)
      : latitude(latitude_val),
        longitude(longitude_val),
        altitude(altitude_val),
        velocity_x(velocity_x_val),
        velocity_y(velocity_y_val),
        velocity_z(velocity_z_val) {}
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MESSAGE_COMMON_MESSAGE_UTILS_HPP_
