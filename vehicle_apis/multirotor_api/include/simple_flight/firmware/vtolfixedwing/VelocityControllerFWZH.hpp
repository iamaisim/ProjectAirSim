// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VELOCITYCONTROLLERFWZH_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VELOCITYCONTROLLERFWZH_HPP_

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
#include "../multirotor/AngleLevelController.hpp"

namespace simple_flight {

// Like PositionController except it is specific to fixed-wing flight and
// controls the Y-axis torque to attain the specified Z-axis linear velocity
class VelocityControllerFWZH : public IFWAngleController,
                               public IGoal  // for internal child controller
{
 public:
  VelocityControllerFWZH(Params* params, const IBoardClock* clock = nullptr)
      : params_(params), clock_(clock) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    axis_ = axis;
    goal_ = goal;
    state_estimator_ = state_estimator;

    PidConfig<float> pid_config(params_->velocity_pid.p[axis],
                                params_->velocity_pid.i[axis],
                                params_->velocity_pid.d[axis]);
    pid_.reset(new PidController<float>(clock_, pid_config));

    // we will be setting goal for child controller so we need these two things
    child_mode_ = GoalMode::GetUnknown();
    child_mode_[IAxisController::kYPitch] =
        GoalModeType::kAngleLevel;  // vx = - pitch

    // initialize child controller
    child_controller_.reset(new AngleLevelController(params_, clock_));
    child_controller_->Initialize(IAxisController::kYPitch, this,
                                  state_estimator_);
  }

  void Reset() override {
    IAxisController::Reset();

    pid_->Reset();
    child_controller_->Reset();
    child_goal_ = Axis4r();
    output_ = TReal();
  }

  void Update() override {
    IAxisController::Update();

    // First get PID output
    const Axis3r& goal_velocity_world =
        Axis4r::Axis4ToXyz(goal_->GetGoalValue(), true);
    pid_->SetGoal(goal_velocity_world[2]);

    const Axis3r& measured_velocity_world =
        state_estimator_->GetLinearVelocity();
    pid_->SetMeasured(measured_velocity_world[2]);
    pid_->Update();

    // use this to drive child controller
    angle_output_ = -pid_->GetOutput() * kPI / 4.0 - kPI_2;
    child_goal_[IAxisController::kYPitch] = angle_output_;
    child_controller_->Update();
    output_ = child_controller_->GetOutput();

#ifdef LVMON_REPORTING
    LVMon::Set("VCFWZ/state/vel/z", measured_velocity_world[2]);
    LVMon::Set("VCFWZ/goal/vel/z", goal_velocity_world[2]);
    LVMon::Set("VCFWZ/goal/pitch",
               child_goal_[IAxisController::kYPitch] * 180.0f / kPI);
    LVMon::Set("VCFWZ/pid/output", pid_->GetOutput());
    LVMon::Set("VCFWZ/child/output", output_);
#endif  // LVMON_REPORTING
  }

  TReal GetOutput() override { return output_; }
  TReal GetOutputAngle() override { return angle_output_; }

  /********************  IGoal ********************/
  const Axis4r& GetGoalValue() const override { return child_goal_; }

  const GoalMode& GetGoalMode() const override { return child_mode_; }

 private:
  unsigned int axis_;
  const IGoal* goal_;
  const IStateEstimator* state_estimator_;

  GoalMode child_mode_;
  Axis4r child_goal_;

  TReal angle_output_;
  TReal output_;

  Params* params_;
  const IBoardClock* clock_;
  std::unique_ptr<PidController<float>> pid_;
  std::unique_ptr<IAxisController> child_controller_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VELOCITYCONTROLLERFWZH_HPP_