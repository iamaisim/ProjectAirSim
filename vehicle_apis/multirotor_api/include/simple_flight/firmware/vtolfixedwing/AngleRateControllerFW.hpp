// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_ANGLERATECONTROLLERFW_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_ANGLERATECONTROLLERFW_HPP_

#include <exception>
#include <memory>
#include <string>

#include "../Params.hpp"
#include "../PidController.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IBoardClock.hpp"

namespace simple_flight {

// Same as AngleRateController except all PID parameters are used
class AngleRateControllerFW : public IAxisController {
 public:
  AngleRateControllerFW(Params* params, const IBoardClock* clock)
      : params_(params), clock_(clock) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    if (axis > 2)
      throw std::invalid_argument(
          "AngleRateControllerFW only supports axis 0-2 but it was " +
          std::to_string(axis));

    axis_ = axis;
    goal_ = goal;
    state_estimator_ = state_estimator;

    pid_.reset(new PidController<float>(
        clock_, PidConfig<float>(params_->angle_rate_pid.p[axis],
                                 params_->angle_rate_pid.i[axis],
                                 params_->angle_rate_pid.d[axis])));
  }

  void Reset() override {
    IAxisController::Reset();

    pid_->Reset();
    output_ = TReal();
  }

  void Update() override {
    IAxisController::Update();

    pid_->SetGoal(goal_->GetGoalValue()[axis_]);
    pid_->SetMeasured(state_estimator_->GetAngularVelocity()[axis_]);
    pid_->Update();

    output_ = pid_->GetOutput();
  }

  TReal GetOutput() override { return output_; }

 private:
  unsigned int axis_;
  const IGoal* goal_;
  const IStateEstimator* state_estimator_;

  TReal output_;

  Params* params_;
  const IBoardClock* clock_;
  std::unique_ptr<PidController<float>> pid_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_ANGLERATECONTROLLERFW_HPP_