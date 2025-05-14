// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_FIRST_ORDER_FILTER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_FIRST_ORDER_FILTER_HPP_

#include "clock.hpp"

namespace microsoft {
namespace projectairsim {

template <typename T>
class FirstOrderFilter {
  /*
  This class can be used to apply a first order filter on a signal.
  It allows different acceleration and deceleration time constants.
  Short review of discrete time implementation of first order system:
  Laplace:
  X(s)/U(s) = 1/(tau*s + 1)
  continuous time system:
  dx(t) = (-1/tau)*x(t) + (1/tau)*u(t)
  discretized system (ZoH):
  x(k+1) = exp(samplingTime*(-1/tau))*x(k) + (1 - exp(samplingTime*(-1/tau))) *
  u(k)
  */
 public:
  FirstOrderFilter()
      : time_constant_(0.0f),
        initial_input_(0),
        initial_output_(0),
        input_(0),
        output_(0) {
    // allow default constructor with later call for initialize
  }

  FirstOrderFilter(float time_constant, T initial_input, T initial_output) {
    Initialize(time_constant, initial_input, initial_output);
  }

  void Initialize(float time_constant, T initial_input, T initial_output) {
    time_constant_ = time_constant;
    initial_input_ = initial_input;
    initial_output_ = initial_output;
    Reset();
  }

  void SetInput(T input) { input_ = input; }

  T GetInput() const { return input_; }

  T GetOutput() const { return output_; }

  void Reset() {
    input_ = initial_input_;
    output_ = initial_output_;
  }

  void UpdateOutput(TimeSec dt_sec) {
    if (dt_sec <= 0.0f) return;

    if (time_constant_ == 0) {
      output_ = input_;
      return;
    }

    // lower the weight for previous value if its been long time
    // TODO: minimize use of exp
    double alpha = std::exp(-dt_sec / time_constant_);
    // x(k+1) = Ad*x(k) + Bd*u(k)
    output_ = static_cast<T>((output_ * alpha) + (input_ * (1.0 - alpha)));
  }

 private:
  float time_constant_;
  T output_;
  T input_;
  T initial_output_;
  T initial_input_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_FIRST_ORDER_FILTER_HPP_
