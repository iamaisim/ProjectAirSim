// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_STDPIDINTEGRATOR_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_STDPIDINTEGRATOR_HPP_

#include "interfaces/CommonStructs.hpp"
#include "interfaces/IPidIntegrator.hpp"

namespace simple_flight {

template <typename T>
class StdPidIntegrator : public IPidIntegrator<T> {
 public:
  StdPidIntegrator(const PidConfig<T>& config) : iterm_(0), config_(config) {}

  virtual ~StdPidIntegrator() {}

  void Reset() override { iterm_ = T(); }

  void Set(T val) override {
    iterm_ = val;
    ClampIterm();
  }

  void Update(float dt, T error) override {
    // to support changes in ki at runtime, we accumulate iterm instead of error
    iterm_ = iterm_ * config_.iterm_discount + dt * error * config_.ki;

    // do not let iterm grow beyond limits (integral windup)
    ClampIterm();
  }

  T GetOutput() override { return iterm_; }

 private:
  void ClampIterm() {
    iterm_ = std::clamp(iterm_, config_.min_output, config_.max_output);
  }

 private:
  float iterm_;
  const PidConfig<T> config_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_STDPIDINTEGRATOR_HPP_