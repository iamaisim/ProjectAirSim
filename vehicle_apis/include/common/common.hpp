// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef VEHICLE_APIS_INCLUDE_COMMON_COMMON_HPP_
#define VEHICLE_APIS_INCLUDE_COMMON_COMMON_HPP_

#include <cmath>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "enable_lvmon.hpp"

namespace microsoft {
namespace projectairsim {
namespace vehicle_apis {

typedef float TReal;

static constexpr double kPI =
    static_cast<double>(3.1415926535897932384626433832795028841972);
static constexpr double k2PI = 2.0 * kPI;
static constexpr double kPI_2 = kPI / 2.0;

template <typename T>
class Axis3 {
 public:
  Axis3(const T& x_val = T(), const T& y_val = T(), const T& z_val = T())
      : vals_{x_val, y_val, z_val} {}

  // access by index
  virtual T& operator[](unsigned int index) { return vals_[index]; }
  virtual const T& operator[](unsigned int index) const { return vals_[index]; }

  // op overload for setting custom gains
  T& operator=(const std::vector<T>& vals) {
    vals_[0] = vals[0];
    vals_[1] = vals[1];
    vals_[2] = vals[2];
  }

  virtual std::string ToString() const {
    return std::to_string(static_cast<float>(vals_[0]))
        .append(", ")
        .append(std::to_string(static_cast<float>(vals_[1])))
        .append(", ")
        .append(std::to_string(static_cast<float>(vals_[2])));
  }

  bool Equals3(const Axis3<T>& other) const {
    return vals_[0] == other.vals_[0] && vals_[1] == other.vals_[1] &&
           vals_[2] == other.vals_[2];
  }

  Axis3<T> ColWiseMultiply3(const Axis3<T>& other) const {
    return Axis3<T>(vals_[0] * other.vals_[0], vals_[1] * other.vals_[1],
                    vals_[2] * other.vals_[2]);
  }

  // access as axis
  const T& X() const { return vals_[0]; }
  const T& Y() const { return vals_[1]; }
  const T& Z() const { return vals_[2]; }
  T& X() { return vals_[0]; }
  T& Y() { return vals_[1]; }
  T& Z() { return vals_[2]; }

  // access as angles
  const T& Roll() const { return vals_[0]; }
  const T& Pitch() const { return vals_[1]; }
  const T& Yaw() const { return vals_[2]; }
  T& Roll() { return vals_[0]; }
  T& Pitch() { return vals_[1]; }
  T& Yaw() { return vals_[2]; }

  static const Axis3<T>& Zero() {
    static const Axis3<T> zero_val = Axis3<T>();
    return zero_val;
  }

  static constexpr unsigned int AxisCount() { return 3; }

 private:
  T vals_[3];
};
typedef Axis3<TReal> Axis3r;

template <typename T>
class Axis4 : public Axis3<T> {
 public:
  Axis4(const T& x_val = T(), const T& y_val = T(), const T& z_val = T(),
        const T& val4_val = T())
      : Axis3<T>(x_val, y_val, z_val), val4_(val4_val) {}

  Axis4(const Axis3<T>& axis3_val, const T& val4_val = T())
      : Axis3<T>(axis3_val), val4_(val4_val) {}

  void SetValues(const std::vector<T>& vals) {
    (*this)[0] = vals[0];
    (*this)[1] = vals[1];
    (*this)[2] = vals[2];
    val4_ = vals[3];
  }

  void SetAxis3(const Axis3<T>& axis3) {
    for (unsigned int axis = 0; axis < Axis3<T>::AxisCount(); ++axis)
      (*this)[axis] = axis3[axis];
  }

  T& Val4() { return val4_; }
  const T& Val4() const { return val4_; }

  // access by index
  virtual T& operator[](unsigned int index) override {
    if (index <= 2)
      return Axis3<T>::operator[](index);
    else if (index == 3)
      return val4_;
    else
      throw std::out_of_range("index must be <= 3 but it was " +
                              std::to_string(index));
  }
  virtual const T& operator[](unsigned int index) const override {
    if (index <= 2)
      return Axis3<T>::operator[](index);
    else if (index == 3)
      return val4_;
    else
      throw std::out_of_range("index must be <= 3 but it was " +
                              std::to_string(index));
  }

  virtual std::string ToString() const override {
    return Axis3<T>::ToString().append(", ").append(
        std::to_string(static_cast<float>(val4_)));
  }

  bool Equals4(const Axis4<T>& other) const {
    return (*this)[0] == other[0] && (*this)[1] == other[1] &&
           (*this)[2] == other[2] && (*this)[3] == other[3];
  }

  Axis4<T> ColWiseMultiply4(const Axis4<T>& other) const {
    return Axis4<T>((*this)[0] * other[0], (*this)[1] * other[1],
                    (*this)[2] * other[2], (*this)[3] * other[3]);
  }

  T& Throttle() { return val4_; }
  const T& Throttle() const { return val4_; }

  static const Axis4<T>& Zero() {
    static const Axis4<T> zero_val = Axis4<T>();
    return zero_val;
  }

  static constexpr unsigned int AxisCount() { return 4; }

  static Axis3<T> Axis4ToXyz(const Axis4<T> axis4, bool swap_xy) {
    return Axis3<T>(axis4[swap_xy ? 1 : 0], axis4[swap_xy ? 0 : 1], axis4[3]);
  }
  static Axis4<T> XyzToAxis4(const Axis3<T> xyz, bool swap_xy) {
    // TODO: use nan instead 0?
    return Axis4<T>(xyz[swap_xy ? 1 : 0], xyz[swap_xy ? 0 : 1], 0, xyz[2]);
  }

 private:
  T val4_ = 0;
};
typedef Axis4<TReal> Axis4r;

template <typename T>
class AxisN : public Axis4<T> {
 public:
  AxisN(const unsigned int num_channels_additional = 0)
      : Axis4<T>(Axis4<T>::Zero()) {
    num_channels_ += num_channels_additional;
    vals_ = std::vector<T>(num_channels_additional, 0);
  }
  AxisN(const std::vector<T>& vals,
        const unsigned int num_channels_additional = 0) {
    // vals contains roll, pitch, yaw, throttle and additional channels
    num_channels_ += num_channels_additional;
    SetValues(vals, num_channels_additional);
  }
  AxisN(const Axis4<T>& axis4_val, const std::vector<T>& vals,
        const unsigned int num_channels_additional = 0)
      : Axis4<T>(axis4_val), vals_(vals) {
    num_channels_ += num_channels_additional;
  }

  AxisN(std::initializer_list<T> initial_values) {
    auto cvalues = initial_values.size();

    SetValues(std::vector<T>(initial_values),
              (cvalues > 4) ? (cvalues - 4) : 0);
  }
  AxisN(const Axis4<T>& axis4_val) : Axis4<T>(axis4_val) {}

  void SetValues(const std::vector<T>& vals,
                 unsigned int num_channels_additional = 0) {
    auto num_channel_base = Axis4<T>::AxisCount();

    // Set base channels
    Axis4<T>::SetValues(vals);

    // Add additional channels to auxiliary array, vals_
    vals_.clear();
    vals_.reserve(num_channels_additional);
    num_channels_ = num_channel_base + num_channels_additional;
    for (unsigned int iaxis = num_channel_base; iaxis < num_channels_; ++iaxis)
      vals_.push_back(vals[iaxis]);
  }

  // position uses 1-index (not 0)
  T& ValN(unsigned int position) {
    if (position <= 3)
      return Axis3<T>::operator[](position - 1);
    else if (position == 4)
      return Axis4<T>::Val4();
    else if (position <= num_channels_)
      return vals_[position - Axis4<T>::AxisCount() - 1];
    else
      throw std::out_of_range(
          "position must be <= " + std::to_string(num_channels_) +
          " but it was " + std::to_string(position));
  }

  // access by index
  virtual T& operator[](unsigned int index) override {
    if (index <= 2)
      return Axis3<T>::operator[](index);
    else if (index == 3)
      return Axis4<T>::Val4();
    else if (index < num_channels_)
      return vals_[index - Axis4<T>::AxisCount()];
    else
      throw std::out_of_range("index must be < " +
                              std::to_string(num_channels_) + " but it was " +
                              std::to_string(index));
  }
  virtual const T& operator[](unsigned int index) const override {
    if (index <= 2)
      return Axis3<T>::operator[](index);
    else if (index == 3)
      return Axis4<T>::Val4();
    else if (index < num_channels_)
      return vals_[index - Axis4<T>::AxisCount()];
    else
      throw std::out_of_range("index must be < " +
                              std::to_string(num_channels_) + " but it was " +
                              std::to_string(index));
  }

  // virtual std::string ToString() const override {
  //   std::string res = Axis3<T>::ToString().append(", ").append(
  //       std::to_string(Axis4<T>::Val4()));

  //  if (num_channels_ - Axis4<T>::AxisCount()) {  // additional channels
  //    res.append(", ");
  //    for (unsigned int axis = 0;
  //         axis < num_channels_ - Axis4<T>::AxisCount() - 1; ++axis)
  //      res.append(std::to_string(static_cast<float>(vals_[axis])))
  //          .append(", ");

  //    res.append(std::to_string(static_cast<float>(vals_.back())));
  //  }
  //  return res;
  //}

  bool EqualsN(const AxisN<T>& other) const {
    for (unsigned int axis = 0; axis < num_channels_; ++axis)
      if ((*this)[axis] != other[axis]) return false;
    return true;
  }

  AxisN<T> ColWiseMultiplyN(const AxisN<T>& other) const {
    unsigned int num_channels_additional =
        num_channels_ - Axis4<T>::AxisCount();
    std::vector<T> vals(num_channels_, 0);
    for (unsigned int axis = 0; axis < num_channels_; ++axis)
      vals[axis] = (*this)[axis] * other[axis];

    return AxisN<T>(vals, num_channels_additional);
  }

  static const AxisN<T>& Zero(const unsigned int num_channels_additional) {
    static const AxisN<T> zero_val = AxisN<T>(num_channels_additional);
    return zero_val;
  }

  unsigned int AxisCount() const { return num_channels_; }

  static Axis3<T> AxisNToXyz(const AxisN<T> axisN, bool swap_xy) {
    return Axis3<T>(axisN[swap_xy ? 1 : 0], axisN[swap_xy ? 0 : 1],
                    axisN[3]);  // isn't [3] = throttle value?
  }

  static AxisN<T> XyzToAxisN(const Axis3<T> xyz, bool swap_xy,
                             const unsigned int num_channels_additional = 0) {
    // TODO: use nan instead 0?
    std::vector<T> vals(Axis4<T>::AxisCount() + num_channels_additional, 0);
    vals[0] = xyz[swap_xy ? 1 : 0];
    vals[1] = xyz[swap_xy ? 0 : 1];
    vals[3] = xyz[2];  // why is xyz[2] = z the 4th param?
    return AxisN<T>(vals, num_channels_additional);
  }

 private:
  std::vector<T> vals_;
  unsigned int num_channels_ = 4;
};
typedef AxisN<TReal> AxisNr;

struct KinematicsState {
  Axis3r position;
  Axis4r orientation;

  Axis3r linear_velocity;
  Axis3r angular_velocity;

  Axis3r linear_acceleration;
  Axis3r angular_acceleration;
};

enum class VehicleStateType {
  kUnknown,
  kInactive,
  kBeingArmed,
  kArmed,
  kActive,
  kBeingDisarmed,
  kDisarmed
};

class VehicleState {
 public:
  VehicleStateType GetState() const { return state_; }
  void SetState(VehicleStateType state) { state_ = state; }

  static VehicleStateType FromString(const std::string& val) {
    if (val == "Armed") return VehicleStateType::kArmed;
    if (val == "Inactive") return VehicleStateType::kInactive;
    if (val == "Unknown") return VehicleStateType::kUnknown;
    if (val == "BeingArmed") return VehicleStateType::kBeingArmed;
    if (val == "Active") return VehicleStateType::kActive;
    if (val == "BeingDisarmed") return VehicleStateType::kBeingDisarmed;
    if (val == "Disarmed") return VehicleStateType::kDisarmed;

    throw std::invalid_argument(
        std::string("The value cannot be converted to VehicleStateType enum: ")
            .append(val));
  }

 private:
  VehicleStateType state_ = VehicleStateType::kUnknown;
};

// config params for PID controller
template <class T>
struct PidConfig {
  PidConfig(float kp_val = 0.01f, float ki_val = 0.0f, float kd_val = 0.0f,
            T min_output_val = -1, T max_output_val = 1,
            float time_scale_val = 1.0f / 1000, bool enabled_val = true,
            T output_bias_val = T(), float iterm_discount_val = 1)
      : kp(kp_val),
        ki(ki_val),
        kd(kd_val),
        time_scale(time_scale_val),
        min_output(min_output_val),
        max_output(max_output_val),
        enabled(enabled_val),
        output_bias(output_bias_val),
        iterm_discount(iterm_discount_val) {}

  float kp, ki, kd;
  float time_scale;
  T min_output, max_output;
  bool enabled;
  T output_bias;
  float iterm_discount;

  enum class IntegratorType { kStandard };
  IntegratorType integrator_type = IntegratorType::kStandard;
};

}  // namespace vehicle_apis
}  // namespace projectairsim
}  // namespace microsoft

#endif  // VEHICLE_APIS_INCLUDE_COMMON_COMMON_HPP_
