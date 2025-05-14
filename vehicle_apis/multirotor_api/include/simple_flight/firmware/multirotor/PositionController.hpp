// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_POSITIONCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_POSITIONCONTROLLER_HPP_

#include "../Params.hpp"
#include "../PidController.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IBoardClock.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IUpdatable.hpp"
#include "VelocityController.hpp"

namespace simple_flight {

class PositionController : public IAxisController,
                           public IGoal  // for internal child controller
{
 public:
  PositionController(Params* params, const IBoardClock* clock = nullptr)
      : params_(params), clock_(clock) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    if (axis == 2)
      throw std::invalid_argument(
          "PositionController does not support yaw axis i.e. " +
          std::to_string(axis));

    axis_ = axis;
    goal_ = goal;
    state_estimator_ = state_estimator;

    // initialize parent PID
    pid_.reset(new PidController<float>(
        clock_, PidConfig<float>(params_->position_pid.p[axis],
                                 params_->position_pid.i[axis],
                                 params_->position_pid.d[axis])));

    // initialize child controller
    velocity_controller_.reset(new VelocityController(params_, clock_));
    velocity_controller_->Initialize(axis, this, state_estimator_);

    // we will be setting goal for child controller so we need these two things
    velocity_mode_ = GoalMode::GetUnknown();
    velocity_mode_[axis] = GoalModeType::kVelocityWorld;
  }

  void Reset() override {
    IAxisController::Reset();

    pid_->Reset();
    velocity_controller_->Reset();
    velocity_goal_ = Axis4r();
    output_ = TReal();
  }

  void Update() override {
    IAxisController::Update();

    const Axis4r& goal_position_world = goal_->GetGoalValue();
    pid_->SetGoal(goal_position_world[axis_]);
    const Axis4r& measured_position_world =
        Axis4r::XyzToAxis4(state_estimator_->GetPosition(), true);
    pid_->SetMeasured(measured_position_world[axis_]);
    pid_->Update();

    // use this to drive child controller
    velocity_goal_[axis_] =
        pid_->GetOutput() * params_->velocity_pid.max_limit[axis_];
    velocity_controller_->Update();

    // final output
    output_ = velocity_controller_->GetOutput();
  }

  TReal GetOutput() override { return output_; }

  /********************  IGoal ********************/
  const Axis4r& GetGoalValue() const override { return velocity_goal_; }

  const GoalMode& GetGoalMode() const override { return velocity_mode_; }

 private:
  unsigned int axis_;
  const IGoal* goal_;
  const IStateEstimator* state_estimator_;

  GoalMode velocity_mode_;
  Axis4r velocity_goal_;

  TReal output_;

  Params* params_;
  const IBoardClock* clock_;
  std::unique_ptr<PidController<float>> pid_;
  std::unique_ptr<VelocityController> velocity_controller_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_POSITIONCONTROLLER_HPP_