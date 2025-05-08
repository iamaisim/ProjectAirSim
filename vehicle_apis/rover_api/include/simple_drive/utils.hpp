// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef ROVER_API_SIMPLE_DRIVE_UTILS_HPP_
#define ROVER_API_SIMPLE_DRIVE_UTILS_HPP_

#include "core_sim/physics_common_types.hpp"
#include "common/common.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

class Utils {
 public:
  static vehicle_apis::Axis3r ToAxis3r(const Vector3& vec) {
    vehicle_apis::Axis3r conv;
    conv.X() = vec.x();
    conv.Y() = vec.y();
    conv.Z() = vec.z();
    return conv;
  }

  static Vector3 ToVector3(const vehicle_apis::Axis3r& vec) {
    Vector3 conv;
    conv.x() = vec.X();
    conv.y() = vec.Y();
    conv.z() = vec.Z();
    return conv;
  }

  static Kinematics ToKinematicsState3r(
      const vehicle_apis::KinematicsState& state) {
    Kinematics state3r;
    state3r.pose.position = ToVector3(state.position);
    state3r.pose.orientation = ToQuaternion(state.orientation);
    state3r.twist.linear = ToVector3(state.linear_velocity);
    state3r.twist.angular = ToVector3(state.angular_velocity);
    state3r.accels.linear = ToVector3(state.linear_acceleration);
    state3r.accels.angular = ToVector3(state.angular_acceleration);
    return state3r;
  }

  static vehicle_apis::Axis4r ToAxis4r(const Quaternion& q) {
    vehicle_apis::Axis4r conv;
    conv.X() = q.x();
    conv.Y() = q.y();
    conv.Z() = q.z();
    conv.Val4() = q.w();
    return conv;
  }

  static Quaternion ToQuaternion(const vehicle_apis::Axis4r& q) {
    Quaternion conv;
    conv.x() = q.X();
    conv.y() = q.Y();
    conv.z() = q.Z();
    conv.w() = q.Val4();
    return conv;
  }
};

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_SIMPLE_DRIVE_UTILS_HPP_
