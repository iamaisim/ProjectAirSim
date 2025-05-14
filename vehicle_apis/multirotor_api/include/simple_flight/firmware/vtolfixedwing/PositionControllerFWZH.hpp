// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PositionControllerFWZHH_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PositionControllerFWZHH_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include "../Params.hpp"
#include "../PidController.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IBoardClock.hpp"
#include "../interfaces/IFWAngleController.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IUpdatable.hpp"
#include "AngleLevelControllerFW.hpp"

namespace simple_flight {

// Like PositionController except it is specific to fixed-wing flight and
// controls the Y-axis torque to attain the specified Z-axis position
class PositionControllerFWZH : public IFWAngleController,
                               public IGoal  // for internal child controller
{
 public:
  PositionControllerFWZH(Params* params, const IBoardClock* clock = nullptr)
      : params_(params), clock_(clock) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    if (axis != IAxisController::kZHeight)
      throw std::invalid_argument(
          "PositionControllerFWZH only supports z-height axis (3), not axis " +
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
    angle_level_controller_.reset(new AngleLevelController(params_, clock_));
    angle_level_controller_->Initialize(IAxisController::kYPitch, this,
                                        state_estimator_);

    // we will be setting goal for child controller so we need these two things
    goal_mode_child_ = GoalMode::GetUnknown();
    goal_mode_child_[IAxisController::kYPitch] = GoalModeType::kAngleLevel;
  }

  void Reset() override {
    IAxisController::Reset();

    pid_->Reset();
    angle_level_controller_->Reset();
    goal_value_child_ = Axis4r();
    output_ = TReal();
  }

  void Update() override {
    IAxisController::Update();

    // Update PID controller against target Z position
    const Axis4r& goal_position_world = goal_->GetGoalValue();
    pid_->SetGoal(goal_position_world[IAxisController::kZHeight]);
    const Axis4r& measured_position_world =
        Axis4r::XyzToAxis4(state_estimator_->GetPosition(), true);
    pid_->SetMeasured(measured_position_world[IAxisController::kZHeight]);
    pid_->Update();

    // Drive child controller to control Z speed
    angle_output_ = -pid_->GetOutput() * kPI_2 - kPI_2;
    goal_value_child_[IAxisController::kYPitch] = angle_output_;
    angle_level_controller_->Update();

    // final output
    output_ = angle_level_controller_->GetOutput();

#ifdef LVMON_REPORTING
    LVMon::Set("PCFWZH/goal", goal_position_world[IAxisController::kZHeight]);
    LVMon::Set("PCFWZH/measured",
               measured_position_world[IAxisController::kZHeight]);
    LVMon::Set("PCFWZH/pitchgoal", angle_output_);
    LVMon::Set("PCFWZH/pid", pid_->GetOutput());
    LVMon::Set("PCFWZH/output", output_);
#endif  // LVMON_REPORTING
  }

  TReal GetOutput() override { return output_; }
  TReal GetOutputAngle() override { return angle_output_; }

  /********************  IGoal ********************/
  const Axis4r& GetGoalValue() const override { return goal_value_child_; }

  const GoalMode& GetGoalMode() const override { return goal_mode_child_; }

 private:
  unsigned int axis_;
  const IGoal* goal_;
  const IStateEstimator* state_estimator_;

  GoalMode goal_mode_child_;
  Axis4r goal_value_child_;

  TReal angle_output_;
  TReal output_;

  Params* params_;
  const IBoardClock* clock_;
  std::unique_ptr<PidController<float>> pid_;
  std::unique_ptr<AngleLevelController> angle_level_controller_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PositionControllerFWZHH_HPP_