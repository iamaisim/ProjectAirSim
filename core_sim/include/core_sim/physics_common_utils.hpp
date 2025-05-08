// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_PHYSICS_COMMON_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_PHYSICS_COMMON_UTILS_HPP_

#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class PhysicsUtils {
 public:
  static Vector3 TransformVectorToBodyFrame(const Vector3& v_world,
                                            const Quaternion& q_world,
                                            bool assume_unit_quat = true) {
    if (!assume_unit_quat) {
      return q_world.inverse()._transformVector(v_world);
    } else {
      return q_world.conjugate()._transformVector(v_world);
    }
  }

  static Vector3 TransformVectorToWorldFrame(const Vector3& v_body,
                                             const Quaternion& q_world) {
    // More performant method is at
    // http://gamedev.stackexchange.com/a/50545/20758 quaternionT vq(0, v.x(),
    // v.y(), v.z()); quaternionT qi = assume_unit_quat ? q.conjugate() :
    // q.inverse(); return (q * vq * qi).vec();
    return q_world._transformVector(v_body);
  }

  static bool HasNan(const Vector3& v) {
    return std::isnan(v.x()) || std::isnan(v.y()) || std::isnan(v.z());
  }

  static bool HasNan(const Quaternion& q) {
    return std::isnan(q.x()) || std::isnan(q.y()) || std::isnan(q.z()) ||
           std::isnan(q.w());
  }

  static bool HasNan(const Pose& p) {
    return HasNan(p.position) || HasNan(p.orientation);
  }

  template <typename TReal>
  static TReal GetRoll(const Quaternion& q) {
    return std::atan2(2.0f * (q.z() * q.y() + q.w() * q.x()),
                      1.0f - 2.0f * (q.x() * q.x() + q.y() * q.y()));
  }

  template <typename TReal>
  static TReal GetPitch(const Quaternion& q) {
    return std::asin(2.0f * (q.y() * q.w() - q.z() * q.x()));
  }

  template <typename TReal>
  static TReal GetYaw(const Quaternion& q) {
    return std::atan2(2.0f * (q.z() * q.w() + q.x() * q.y()),
                      -1.0f + 2.0f * (q.w() * q.w() + q.x() * q.x()));
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_PHYSICS_COMMON_UTILS_HPP_
