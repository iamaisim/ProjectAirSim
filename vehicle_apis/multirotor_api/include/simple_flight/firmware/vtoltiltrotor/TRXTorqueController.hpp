// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRXTORQUECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRXTORQUECONTROLLER_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include <exception>
#include <string>

#include "../interfaces/IAxisController.hpp"
#include "../multirotor/AngleLevelController.hpp"
#include "../multirotor/AngleRateController.hpp"
#include "../multirotor/ConstantOutputController.hpp"
#include "../multirotor/PassthroughController.hpp"

namespace simple_flight {

// X-axis torque controller for fixed-wing flight which controls the roll of
// the vehicle
class TRXTorqueController : public IAxisController {
 public:
  TRXTorqueController(Params* params, const IBoardClock* clock = nullptr)
      : axis_controllers_(),
        clock_(clock),
        fixed_goal_(),
        goal_(nullptr),
        last_goal_mode_(),
        last_goal_val_(),
        output_(Axis4r()[IAxisController::kXRoll]), // should be 0
        params_(params),
        state_estimator_(nullptr) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    goal_ = goal;
    state_estimator_ = state_estimator;

    /*goal_mode_child_ = GoalMode::GetUnknown();
    goal_mode_child_[IAxisController::kXRoll] = GoalModeType::kAngleLevel;
    goal_value_child_ = Axis4r();*/
  }

  void Reset() override {
    IAxisController::Reset();

    last_goal_mode_ = GoalMode::GetUnknown();
    output_ = Axis4r()[IAxisController::kXRoll]; // should be 0

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (axis_controllers_[axis] != nullptr) axis_controllers_[axis]->Reset();
    }
  }

  void Update() override {
    // We're the X-roll controller and in fixed-wing mode we respond to Z-yaw
    // goals so any Z-yaw controller has priority. If there's no ZYaw
    // controller, then we respond to a combined X-axis and Y-axis
    // position/velocity goals.
    IAxisController::Update();

    const auto& goal_mode = goal_->GetGoalMode(); // added reference
    const auto& goal_val = goal_->GetGoalValue(); // added reference

    if (!goal_val.Equals4(last_goal_val_)) {
      last_goal_val_ = goal_val;
    }

    // re-create axis controllers if goal mode was changed since last time
    if (goal_mode[IAxisController::kXRoll] !=
            last_goal_mode_[IAxisController::kXRoll] ||
        params_->gains_changed == true) {
      axis_controllers_[IAxisController::kXRoll].reset();

      switch (goal_mode[IAxisController::kXRoll]) {
        default:
        case GoalModeType::kUnknown:
          // Ignore
          break;

        case GoalModeType::kPassthrough:
          axis_controllers_[IAxisController::kXRoll].reset(
              new PassthroughController());
          break;

        case GoalModeType::kAngleLevel:
          axis_controllers_[IAxisController::kXRoll].reset(
              new AngleLevelController(params_, clock_));
          break;

        case GoalModeType::kAngleRate:
          axis_controllers_[IAxisController::kXRoll].reset(
              new AngleRateController(params_, clock_));
          break;

        case GoalModeType::kVelocityWorld:
        case GoalModeType::kPositionWorld:
          // Ignore
          break;

        case GoalModeType::kConstantOutput:
          axis_controllers_[IAxisController::kXRoll].reset(
              new ConstantOutputController());
          break;
      }

      last_goal_mode_[IAxisController::kXRoll] =
          goal_mode[IAxisController::kXRoll];

      // initialize axis controller
      if (axis_controllers_[IAxisController::kXRoll] == nullptr) {
        // If no suitable controller chosen, use an angle level controller to
        // maintain a roll of 0
        axis_controllers_[IAxisController::kXRoll].reset(
            new AngleLevelController(params_, clock_));
        axis_controllers_[IAxisController::kXRoll]->Initialize(
            IAxisController::kXRoll, &fixed_goal_, state_estimator_);
      } else {
        axis_controllers_[IAxisController::kXRoll]->Initialize(
            IAxisController::kXRoll, goal_, state_estimator_);
      }
      axis_controllers_[IAxisController::kXRoll]->Reset();
    }

    // update axis controller
    output_ = Axis4r()[IAxisController::kXRoll];  // should be 0
    //if (fabs(state_estimator_->GetAngles().Pitch()) <
    //    params_->vtol.pitch_min_fixed_wing) {
    {
      if (axis_controllers_[IAxisController::kXRoll] != nullptr) {
        axis_controllers_[IAxisController::kXRoll]->Update();
        output_ = axis_controllers_[IAxisController::kXRoll]->GetOutput();
      }
    }

#ifdef LVMON_REPORTING
    LVMon::Set("FWXTC/output", output_);
#endif  // LVMON_REPORTING
  }

  TReal GetOutput() override { return (output_); }

 private:
  class FixedGoal : public IGoal {
   public:
    FixedGoal(void) : goal_mode_(), goal_val_() {
      goal_mode_[IAxisController::kXRoll] = GoalModeType::kAngleLevel;
      goal_mode_[IAxisController::kYPitch] = GoalModeType::kAngleLevel;
      goal_mode_[IAxisController::kZYaw] = GoalModeType::kAngleLevel;
      goal_mode_[IAxisController::kZHeight] = GoalModeType::kUnknown;
    }

   public:
    virtual const GoalMode& GetGoalMode() const override { return goal_mode_; }
    virtual const Axis4r& GetGoalValue() const override { return goal_val_; }

   private:
    GoalMode goal_mode_;
    Axis4r goal_val_;
  };

 private:
  std::unique_ptr<IAxisController>
      axis_controllers_[Axis4r::AxisCount()];  // Controllers for each axis
  const IBoardClock* clock_;
  FixedGoal fixed_goal_;  // Goal when there's no controller otherwise
  const IGoal* goal_;                             // External goal provide
  GoalMode last_goal_mode_;   // Previous goal modes
  Axis4r last_goal_val_;      // Previous goal values
  TReal output_;              // Our current output
  Params* params_;            // External parameters
  const IStateEstimator* state_estimator_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRXTORQUECONTROLLER_HPP_