// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PIDCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PIDCONTROLLER_HPP_

#include <algorithm>
#include <cstdlib>

#include "StdPidIntegrator.hpp"
#include "interfaces/CommonStructs.hpp"
#include "interfaces/IBoardClock.hpp"
#include "interfaces/IPidIntegrator.hpp"
#include "interfaces/IUpdatable.hpp"

namespace simple_flight {

template <class T>
class PidController : public IUpdatable {
 public:
  PidController(const IBoardClock* clock = nullptr,
                const PidConfig<T>& config = PidConfig<T>())
      : clock_(clock),
        config_(config),
        goal_(0),
        integrator(),
        last_error_(0),
        last_time_(0),
        measured_(0),
        min_dt_(0),
        output_(0) {
    switch (config.integrator_type) {
      case PidConfig<T>::IntegratorType::kStandard:
        integrator = std::unique_ptr<StdPidIntegrator<T>>(
            new StdPidIntegrator<T>(config_));
        break;
      default:
        throw std::invalid_argument("PID integrator type is not recognized");
    }
  }

  void SetGoal(const T& goal) { goal_ = goal; }
  const T& GetGoal() const { return goal_; }

  void SetMeasured(const T& measured) { measured_ = measured; }
  const T& GetMeasured() const { return measured_; }

  const PidConfig<T>& GetConfig() const { return config_; }

  // allow changing config at runtime
  void SetConfig(const PidConfig<T>& config) {
    bool renabled = !config_.enabled && config.enabled;
    config_ = config;

    if (renabled) {
      last_error_ = goal_ - measured_;
      integrator->Set(output_);
    }
  }

  T GetOutput() { return output_; }

  void Reset() override {
    IUpdatable::Reset();

    goal_ = T();
    measured_ = T();
    last_time_ = clock_ == nullptr ? 0 : clock_->Millis();
    integrator->Reset();
    last_error_ = goal_ - measured_;
    min_dt_ = config_.time_scale * config_.time_scale;
  }

  void Update() override {
    IUpdatable::Update();

    if (!config_.enabled) return;

    const T error = goal_ - measured_;

    float dt = clock_ == nullptr
                   ? 1
                   : (clock_->Millis() - last_time_) * config_.time_scale;

    float pterm = error * config_.kp;
    float dterm = 0;
    if (dt > min_dt_) {
      integrator->Update(dt, error);

      float error_der = dt > 0 ? (error - last_error_) / dt : 0;
      dterm = error_der * config_.kd;
      last_error_ = error;
    }

    output_ = config_.output_bias + pterm + integrator->GetOutput() + dterm;

    // limit final output
    output_ = std::clamp(output_, config_.min_output, config_.max_output);

    last_time_ = clock_->Millis();
  }

 private:
  const IBoardClock* clock_;
  const PidConfig<T> config_;
  T goal_;
  std::unique_ptr<IPidIntegrator<T>> integrator;
  float last_error_;
  uint64_t last_time_;
  T measured_;
  float min_dt_;
  T output_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PIDCONTROLLER_HPP_