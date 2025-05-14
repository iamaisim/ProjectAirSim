// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTCOMMON_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTCOMMON_HPP_

#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class AirSimSimpleFlightCommon {
 public:
  static simple_flight::Axis3r ToAxis3r(const Vector3& vec) {
    simple_flight::Axis3r conv;
    conv.X() = vec.x();
    conv.Y() = vec.y();
    conv.Z() = vec.z();
    return conv;
  }

  static Vector3 ToVector3(const simple_flight::Axis3r& vec) {
    Vector3 conv;
    conv.x() = vec.X();
    conv.y() = vec.Y();
    conv.z() = vec.Z();
    return conv;
  }

  static Kinematics ToKinematicsState3r(
      const simple_flight::KinematicsState& state) {
    Kinematics state3r;
    state3r.pose.position = ToVector3(state.position);
    state3r.pose.orientation = ToQuaternion(state.orientation);
    state3r.twist.linear = ToVector3(state.linear_velocity);
    state3r.twist.angular = ToVector3(state.angular_velocity);
    state3r.accels.linear = ToVector3(state.linear_acceleration);
    state3r.accels.angular = ToVector3(state.angular_acceleration);
    return state3r;
  }

  static simple_flight::Axis4r ToAxis4r(const Quaternion& q) {
    simple_flight::Axis4r conv;
    conv.X() = q.x();
    conv.Y() = q.y();
    conv.Z() = q.z();
    conv.Val4() = q.w();
    return conv;
  }

  static Quaternion ToQuaternion(const simple_flight::Axis4r& q) {
    Quaternion conv;
    conv.x() = q.X();
    conv.y() = q.Y();
    conv.z() = q.Z();
    conv.w() = q.Val4();
    return conv;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTCOMMON_HPP_