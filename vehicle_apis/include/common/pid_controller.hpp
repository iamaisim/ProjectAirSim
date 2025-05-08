// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_COMMON_PID_CONTROLLER_HPP_
#define MULTIROTOR_API_INCLUDE_COMMON_PID_CONTROLLER_HPP_

#include <chrono>
#include <cmath>

namespace microsoft {
namespace projectairsim {

// This class implements a pid controller.
// First call setPoint to set the desired point and the PID control variables
// then call control(x) with new observed values and it will return the updated
// control output needed to achieve the setPoint goal. Integration is done using
// dt measured using system clock.
class PidController {
 private:
  float set_point_ = 0.0f;
  float kProportional_ = 0.0f;
  float kIntegral_ = 0.0f;
  float kDerivative_ = 0.0f;
  float previous_error_ = 0.0f;
  bool previous_set = 0.0f;
  float sum_ = 0.0f;
  std::chrono::time_point<std::chrono::system_clock> prev_time_ =
      std::chrono::system_clock::now();

 public:
  float getPoint(void) const { return set_point_; }

  // set desired point.
  void setPoint(float target, float kProportional, float kIntegral,
                float kDerivative) {
    set_point_ = target;
    kProportional_ = kProportional;
    kIntegral_ = kIntegral;
    kDerivative_ = kDerivative;
    prev_time_ = std::chrono::system_clock::now();
    previous_error_ = 0;
    sum_ = 0;
    previous_set = false;
  }
  float control(float processVariable) {
    auto t = std::chrono::system_clock::now();
    auto diff = std::chrono::system_clock::now() - prev_time_;

    float error = set_point_ - processVariable;
    if (!previous_set) {
      previous_set = true;
      previous_error_ = error;
      return 0;
    }

    float dt = static_cast<float>(
                   std::chrono::duration_cast<std::chrono::microseconds>(diff)
                       .count()) *
               0.000001f;
    dt = fmax(dt, 0.01f);
    float proportionalGain = 0;
    float derivativeGain = 0;
    float integralGain = 0;

    if (kProportional_ != 0) {
      proportionalGain = error * kProportional_;
    }
    if (kDerivative_ != 0) {
      float derivative = (error - previous_error_) / dt;
      derivativeGain = derivative * kDerivative_;
    }
    if (kIntegral_ != 0) {
      sum_ += error * dt;
      integralGain = sum_ * kIntegral_;
    }

    previous_error_ = error;
    prev_time_ = t;

    return proportionalGain + derivativeGain + integralGain;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_COMMON_PID_CONTROLLER_HPP_
