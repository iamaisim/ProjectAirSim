// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_UTILS_HPP_

#include <string>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"

namespace microsoft {
namespace projectairsim {

class TransformUtils {
 public:
  // ZYX Euler->Quaternion formula can have a singularity for pitch
  // reaching near +/-90 deg, we limit just before (kEulerSingularityMinor) or
  // after (kEulerSingularityMajor) the singularity to avoid it.
  static constexpr float kEulerSingularityMinor = (float)(89.9 * M_PI / 180.0);

  static constexpr float kEulerSingularityMajor = (float)(90.1 * M_PI / 180.0);

  static Quaternion ToQuaternion(const Vector3& vector);

  static Quaternion ToQuaternion(float roll_rad, float pitch_rad,
                                 float yaw_rad);

  static Vector3 ToRPY(const Quaternion& quaternion);

  template <typename T>
  static constexpr T ToRadians(T degrees) {
    return (T)(M_PI * degrees / 180.0);
  }

  template <typename T>
  static constexpr T ToDegrees(T radians) {
    return (T)(radians * 180.0 / M_PI);
  }

  template <typename T>
  static T ToCentimeters(const T& data_meters) {
    return data_meters * 100;
  }

  template <typename T>
  static T ToMeters(const T& data_centimeters) {
    return data_centimeters * 0.01;
  }

  /* Convert a Vector3 in NED coord sys to a Vector3 in NEU coord sys
     Useful for converting from sim conventions to UE4 conventions */
  static Vector3 NedToNeuLinear(const Vector3& vec_ned);
  static Vector3 NedToNeuAngular(const Vector3& vec_ned);
  /* Convert a Vector3 in NEU coord sys to a Vector3 in NED coord sys */
  static Vector3 NeuToNedLinear(const Vector3& vec_neu);
  static Vector3 NeuToNedAngular(const Vector3& vec_neu);
  /* Convert quaternions in NEU to NED and vice versa */
  static Quaternion NeuToNedQuat(const Quaternion& q_neu);
  static Quaternion NedToNeuQuat(const Quaternion& q_neu);
};
}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_UTILS_HPP_
