// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MATH_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MATH_UTILS_HPP_

#include <limits>

#pragma warning(push)
#pragma warning(disable:4127) //Disable warnings about static expressions in conditionals in a module that we don't control
#include "Eigen/Dense"
#pragma warning(pop)

#ifndef M_PI
#define M_PI           \
  static_cast<double>( \
      3.14159265358979323846264338327950288419716939937510582097494459230781)
#define _MATH_DEFINES_DEFINED  // prevent WinUCRT from redefining math constants
#endif

namespace microsoft {
namespace projectairsim {

using Vector2 = Eigen::Vector2f;
using Vector3 = Eigen::Vector3f;
using Vector4 = Eigen::Vector4f;
using AngleAxis = Eigen::AngleAxisf;
using Quaternion = Eigen::Quaternion<float>;
using Matrix3x3 = Eigen::Matrix<float, 3, 3>;
using Affine3 = Eigen::Affine3f;

using Vector3f = Eigen::Vector3f;
using Vector3d = Eigen::Vector3d;
using Vector2f = Eigen::Vector2f;
using Vector2d = Eigen::Vector2d;

using Matrix3x3f = Eigen::Matrix<float, 3, 3>;
using Matrix3x3d = Eigen::Matrix<double, 3, 3>;
using Matrix4x4d = Eigen::Matrix<double, 4, 4>;

using Box2f = Eigen::AlignedBox<float, 2>;
using Box2d = Eigen::AlignedBox<double, 2>;

class MathUtils {
 public:
  template <typename TReal>
  static constexpr TReal Epsilon() {
    return std::numeric_limits<TReal>::epsilon();
  }

  template <typename TReal>
  static constexpr bool IsApproximatelyZero(
      TReal a, TReal tolerance = Epsilon<TReal>()) {
    if (std::fabs(a) <= tolerance) return true;
    return false;
  }

  template <typename T>
  static constexpr T Max() {
    return std::numeric_limits<T>::max();
  }

  // Implements relative method - do not use for comparing with zero
  template <typename TReal>
  static bool IsApproximatelyEqual(TReal a, TReal b,
                                   TReal tolerance = Epsilon<TReal>()) {
    TReal diff = std::fabs(a - b);
    if (diff <= tolerance) return true;
    if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance) return true;
    return false;
  }

  template <typename TReal>
  static bool IsDefinitelyLessThan(TReal a, TReal b,
                                   TReal tolerance = Epsilon<TReal>()) {
    TReal diff = a - b;
    if (diff < tolerance) return true;
    if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance) return true;
    return false;
  }

  template <typename TReal>
  static bool IsDefinitelyGreaterThan(TReal a, TReal b,
                                      TReal tolerance = Epsilon<TReal>()) {
    TReal diff = a - b;
    if (diff > tolerance) return true;
    if (diff > std::fmax(std::fabs(a), std::fabs(b)) * tolerance) return true;
    return false;
  }

  // Limits absolute value whole preserving sign
  template <typename TReal>
  static TReal Clip(TReal val, TReal min_value, TReal max_value) {
    return std::max(min_value, std::min(val, max_value));
  }

  static double eps(int number);

  static double rad2Deg(const double radians);

  static double deg2Rad(const double degrees);

  // The input angle is expected to be in radians
  // Normalizes the input to be between -max_angle/2 and +max_angle/2
  template <typename TReal>
  static TReal NormalizeAngle(TReal angle,
                              TReal max_angle = static_cast<TReal>(2 * M_PI)) {
    angle = static_cast<TReal>(std::fmod(angle, max_angle));
    if (angle > max_angle / 2)
      return angle - max_angle;
    else if (angle < -max_angle / 2)
      return angle + max_angle;
    else
      return angle;
  }

  // Convert strongly typed enum to underlying scaler types
  template <typename E>
  static constexpr typename std::underlying_type<E>::type ToNumeric(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
  }

  template <typename E>
  static constexpr E ToEnum(typename std::underlying_type<E>::type u) {
    return static_cast<E>(u);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MATH_UTILS_HPP_
