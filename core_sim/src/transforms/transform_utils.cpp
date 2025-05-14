// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/transforms/transform_utils.hpp"

#include <cmath>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

namespace microsoft {
namespace projectairsim {

Quaternion TransformUtils::ToQuaternion(const Vector3& vector) {
  return ToQuaternion(vector.x(), vector.y(), vector.z());
}

Quaternion TransformUtils::ToQuaternion(float roll_rad, float pitch_rad,
                                        float yaw_rad) {
  // z-y-x rotation convention (Tait-Bryan angles)
  // http://www.sedris.org/wg8home/Documents/WG80485.pdf

  if (std::abs(pitch_rad) > TransformUtils::kEulerSingularityMinor &&
      std::abs(pitch_rad) < TransformUtils::kEulerSingularityMajor) {
    // TODO Trigger a warning for clipped pitch angle
    if (std::abs(pitch_rad) < M_PI/2)
      pitch_rad = std::clamp(pitch_rad, -TransformUtils::kEulerSingularityMinor,
                           TransformUtils::kEulerSingularityMinor);
    else if (pitch_rad > 0)
      pitch_rad = TransformUtils::kEulerSingularityMajor;
    else
      pitch_rad = -TransformUtils::kEulerSingularityMajor;
  }

  float t0 = std::cos(yaw_rad * 0.5f);
  float t1 = std::sin(yaw_rad * 0.5f);
  float t2 = std::cos(roll_rad * 0.5f);
  float t3 = std::sin(roll_rad * 0.5f);
  float t4 = std::cos(pitch_rad * 0.5f);
  float t5 = std::sin(pitch_rad * 0.5f);

  auto w = t0 * t2 * t4 + t1 * t3 * t5;
  auto x = t0 * t3 * t4 - t1 * t2 * t5;
  auto y = t0 * t2 * t5 + t1 * t3 * t4;
  auto z = t1 * t2 * t4 - t0 * t3 * t5;

  return {w, x, y, z};
}

Vector3 TransformUtils::ToRPY(const Quaternion& quaternion) {
  // z-y-x rotation convention (Tait-Bryan angles)
  // Apply yaw, pitch and roll in order to front vector (+X)
  // http://www.sedris.org/wg8home/Documents/WG80485.pdf
  // http://www.ctralie.com/Teaching/COMPSCI290/Materials/EulerAnglesViz/

  float ysqr = quaternion.y() * quaternion.y();

  // roll (x-axis rotation in radians)
  float t0 = +2.0f * (quaternion.w() * quaternion.x() +
                      quaternion.y() * quaternion.z());
  float t1 = +1.0f - 2.0f * (quaternion.x() * quaternion.x() + ysqr);
  auto roll_rad = std::atan2(t0, t1);

  // pitch (y-axis rotation in radians)
  float t2 = +2.0f * (quaternion.w() * quaternion.y() -
                      quaternion.z() * quaternion.x());
  t2 = ((t2 > 1.0f) ? 1.0f : t2);
  t2 = ((t2 < -1.0f) ? -1.0f : t2);
  auto pitch_rad = std::asin(t2);

  // yaw (z-axis rotation in radians)
  float t3 = +2.0f * (quaternion.w() * quaternion.z() +
                      quaternion.x() * quaternion.y());
  float t4 = +1.0f - 2.0f * (ysqr + quaternion.z() * quaternion.z());
  auto yaw_rad = std::atan2(t3, t4);

  return {roll_rad, pitch_rad, yaw_rad};
}

Vector3 TransformUtils::NedToNeuLinear(const Vector3& vec_ned) {
  return Vector3(vec_ned.x(), vec_ned.y(), -vec_ned.z());
}

Vector3 TransformUtils::NedToNeuAngular(const Vector3& vec_ned) {
  return Vector3(-vec_ned.x(), -vec_ned.y(), vec_ned.z());
}

Vector3 TransformUtils::NeuToNedLinear(const Vector3& vec_neu) {
  return Vector3(vec_neu.x(), vec_neu.y(), -vec_neu.z());
}

Vector3 TransformUtils::NeuToNedAngular(const Vector3& vec_neu) {
  return Vector3(-vec_neu.x(), -vec_neu.y(), vec_neu.z());
}

Quaternion TransformUtils::NeuToNedQuat(const Quaternion& q_neu) {
  return Quaternion(q_neu.w(), -q_neu.x(), -q_neu.y(), q_neu.z());
}

Quaternion TransformUtils::NedToNeuQuat(const Quaternion& q_ned) {
  return Quaternion(q_ned.w(), -q_ned.x(), -q_ned.y(), q_ned.z());
}
}  // namespace projectairsim
}  // namespace microsoft
