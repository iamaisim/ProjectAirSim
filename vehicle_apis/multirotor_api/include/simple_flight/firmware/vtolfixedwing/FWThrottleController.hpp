// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWTHROTTLECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWTHROTTLECONTROLLER_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include "../Params.hpp"
#include "../PidController.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IBoardClock.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IUpdatable.hpp"
#include "../multirotor/AngleLevelController.hpp"

namespace simple_flight {

// Throttle controller for fixed-wing flight
class FWThrottleController : public IAxisController {
 public:
  FWThrottleController(Params* params, const IBoardClock* clock = nullptr)
      : params_(params), clock_(clock) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    axis_ = axis;
    goal_ = goal;
    state_estimator_ = state_estimator;

    PidConfig<float> pid_config(params_->velocity_pid.p[axis],
                                params_->velocity_pid.i[axis],
                                params_->velocity_pid.d[axis]);
    pid_config.iterm_discount = params_->velocity_pid.iterm_discount[axis];
    pid_config.output_bias = params_->velocity_pid.output_bias[axis];

    pid_.reset(new PidController<float>(clock_, pid_config));
  }

  void Reset() override {
    IAxisController::Reset();

    pid_->Reset();
    output_ = TReal();
  }

  void Update() override {
    bool fNoControllers = true;
    TReal velocityGoal = 0;
    TReal velocityMeasured = 0;

    IAxisController::Update();

    const Axis3r& goal_velocity_world =
        Axis4r::Axis4ToXyz(goal_->GetGoalValue(), true);
    const Axis3r& measured_velocity_world =
        state_estimator_->GetLinearVelocity();

    {
      auto goalmode = goal_->GetGoalMode();
      float velocitySquaredSum = 0;

      for (unsigned int axis = 0, caxis = Axis3r::AxisCount(); axis < caxis;
           ++axis) {
        if (goalmode[axis] == GoalModeType::kVelocityWorld) {
          auto velocity = goal_velocity_world[axis];
          velocitySquaredSum += velocity * velocity;
          fNoControllers = false;
        }
      }

      if (fNoControllers)
        velocityGoal = params_->fixed_wing.speed_forward_default;
      else
        velocityGoal = sqrt(velocitySquaredSum);

      pid_->SetGoal(velocityGoal);
    }

    if (fNoControllers) {
      pid_->SetMeasured(measured_velocity_world[0]);
    } else {
      float velocitySquaredSum = 0;

      for (int axis = 0, caxis = Axis3r::AxisCount(); axis < caxis; ++axis) {
        auto velocity = measured_velocity_world[axis];

        velocitySquaredSum += velocity * velocity;
      }
      velocityMeasured = sqrt(velocitySquaredSum);
      pid_->SetMeasured(velocityMeasured);

      pid_->Update();
    }

    output_ = pid_->GetOutput();
    if (output_ < 0.0f) output_ = 0.0f;

#ifdef LVMON_REPORTING
    LVMon::Set("VCFWT/goal", velocityGoal);
    LVMon::Set("VCFWT/measured", velocityMeasured);
    LVMon::Set("VCFWT/output", output_);
#endif  // LVMON_REPORTING
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

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWTHROTTLECONTROLLER_HPP_