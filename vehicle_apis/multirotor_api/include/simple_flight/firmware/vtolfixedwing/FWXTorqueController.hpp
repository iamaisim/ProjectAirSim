// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWXTORQUECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWXTORQUECONTROLLER_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include <exception>
#include <string>

#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IFWAngleController.hpp"
#include "AngleLevelControllerFW.hpp"
#include "PositionControllerFW.hpp"
#include "VelocityControllerFW.hpp"

namespace simple_flight {

// X-axis torque controller for fixed-wing flight which controls the yaw of
// the vehicle
class FWXTorqueController : public IAxisController, public IGoal {
 public:
  FWXTorqueController(Params* params, const IBoardClock* clock = nullptr)
      : axis_controllers_(),
        clock_(clock),
        fDoTorqueControllerOnly_(false),
        fwangle_controllers_(),
        goal_(nullptr),
        goal_mode_child_(),
        goal_value_child_(),
        last_goal_mode_(),
        last_goal_val_(),
        output_(Axis4r()[IAxisController::kXRoll]),
        params_(params),
        state_estimator_(nullptr) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    goal_ = goal;
    state_estimator_ = state_estimator;

    goal_mode_child_ = GoalMode::GetUnknown();
    goal_mode_child_[IAxisController::kZYaw] = GoalModeType::kAngleLevel;
    goal_value_child_ = Axis4r();
  }

  void Reset() override {
    IAxisController::Reset();

    last_goal_mode_ = GoalMode::GetUnknown();
    output_ = Axis4r()[IAxisController::kXRoll];

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (fwangle_controllers_[axis] != nullptr)
        fwangle_controllers_[axis]->Reset();
    }
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

    const auto& goal_mode = goal_->GetGoalMode();
    const auto& goal_val = goal_->GetGoalValue();

    if (!last_goal_val_.Equals4(goal_val)) {
      last_goal_val_ = goal_val;
    }

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      // re-create axis controllers if goal mode was changed since last time
      if (goal_mode[axis] != last_goal_mode_[axis] ||
          params_->gains_changed == true) {
        axis_controllers_[axis].reset(nullptr);
        fwangle_controllers_[axis].reset(nullptr);

        switch (goal_mode[axis]) {
          default:
            throw std::invalid_argument(
                "Axis controller type is not yet implemented for X axis ");

          case GoalModeType::kAngleRate:
            if (axis == IAxisController::kZYaw)
              axis_controllers_[axis].reset(
                  new AngleRateController(params_, clock_));
            break;

          case GoalModeType::kAngleLevel:
            if (axis == IAxisController::kZYaw)
              axis_controllers_[axis].reset(
                  new AngleLevelControllerFW(params_, clock_));
            break;

          case GoalModeType::kConstantOutput:
            if (axis == IAxisController::kZYaw)
              axis_controllers_[axis].reset(new ConstantOutputController());
            break;

          case GoalModeType::kPassthrough:
            if (axis == IAxisController::kZYaw)
              axis_controllers_[axis].reset(new PassthroughController());
            break;

          case GoalModeType::kPositionWorld:
            if ((axis == IAxisController::kXRoll) ||
                (axis == IAxisController::kYPitch))
              fwangle_controllers_[axis]
                  .reset(  // TODO: Verify need for angle output--probably
                           // switch to torque output
                      new PositionControllerFW(params_, clock_));
            break;

          case GoalModeType::kUnknown:
            // Don't handle this axis or nothing to handle
            break;

          case GoalModeType::kVelocityWorld:
            if ((axis == IAxisController::kXRoll) ||
                (axis == IAxisController::kYPitch))
              fwangle_controllers_[axis]
                  .reset(  // TODO: Verify need for angle output--probably
                           // switch to torque output
                      new VelocityControllerFW(params_, clock_));
            break;
        }
        last_goal_mode_[axis] = goal_mode[axis];

        // initialize axis controller
        if (axis_controllers_[axis] != nullptr) {
          axis_controllers_[axis]->Initialize(
              axis, goal_, state_estimator_);  // In fixed-wing orientation, we
                                               // (the X torque controller)
                                               // implements the Z-yaw goals
          axis_controllers_[axis]->Reset();
        }
        if (fwangle_controllers_[axis] != nullptr) {
          fwangle_controllers_[axis]->Initialize(
              axis, goal_, state_estimator_);  // In fixed-wing orientation, we
                                               // (the X torque controller)
                                               // implements the Z-yaw goals
          fwangle_controllers_[axis]->Reset();
        }
      }

      // update axis controller
      if (axis_controllers_[axis] != nullptr) {
        axis_controllers_[axis]->Update();
      }
      // update angle controller
      if (fwangle_controllers_[axis] != nullptr) {
        fwangle_controllers_[axis]->Update();
      }
    }

    // Ax X-roll torque controller has an angle-level or angle-rate goal which
    // sets a specific X-roll target and takes precedence over our angle
    // controllers which have indirect goals like controlling the X or Y
    // position. If we have an X-roll torque controller, we should only apply
    // that.
    {
      auto goal_mode_type = goal_mode[IAxisController::kXRoll];

      fDoTorqueControllerOnly_ =
          (((goal_mode_type == GoalModeType::kAngleLevel) ||
            (goal_mode_type == GoalModeType::kAngleRate)) &&
           axis_controllers_[IAxisController::kXRoll] != nullptr);
    }
  }

  TReal GetOutput() override {
    output_ = 0.0f;

    // Get output
    if (fabs(state_estimator_->GetAngles().Pitch()) <
        params_->vtol.pitch_min_fixed_wing)
      //If the vehicle is pitched vertically too much, don't attempt to yaw
      output_ = 0.0f;
    else if (axis_controllers_[IAxisController::kZYaw] != nullptr) {
      output_ = axis_controllers_[IAxisController::kZYaw]->GetOutput();
    } else {
      // Combing axis angle controllers
      int coutput = 0;
      auto yaw = state_estimator_->GetAngles()[IAxisController::kZYaw];
      auto sum = 0.0;

      sum = 0.0f;

      // X-roll controller's output is positive to turn eastware, negative
      // to turn westward.  It's contribution to our output depends on which
      // way we're currently facing.
      if (axis_controllers_[IAxisController::kXRoll] != nullptr) {
        auto outputEW = axis_controllers_[IAxisController::kXRoll]->GetOutput();

        coutput = 1;

        sum = ((yaw >= -kPI_2) && (yaw <= kPI_2))
                  ? outputEW    // We're facing northward, keep
                                // controller's torque sign
                  : -outputEW;  // We're faacing southward, invert
                                // controller's torque sign
      }

      // Y-pitch controller's output is positive to turn northward, negative
      // to turn southward.  It's contribution can be merged directly to our
      // output.
      if (axis_controllers_[IAxisController::kYPitch] != nullptr) {
        ++coutput;
        sum += axis_controllers_[IAxisController::kYPitch]->GetOutput();
      }

      output_ = (coutput > 1) ? sum / coutput : sum;
    }

#ifdef LVMON_REPORTING
    LVMon::Set("FWXTC/output", output_);
#endif  // LVMON_REPORTING

    return (output_);
  }

 public:  // IGoal Method Overrides
  const Axis4r& GetGoalValue() const override { return goal_value_child_; }
  const GoalMode& GetGoalMode() const override { return goal_mode_child_; }

 private:
  std::unique_ptr<IAxisController>
      axis_controllers_[Axis4r::AxisCount()];  // Controllers for each axis
  const IBoardClock* clock_;
  bool fDoTorqueControllerOnly_;  // If true, apply torque controllers and
                                  // ignore angle controllers
  std::unique_ptr<IFWAngleController>
      fwangle_controllers_[Axis4r::AxisCount()];  // Controllers for angle of
                                                  // each "axis" (X-pitch,
                                                  // Y-roll, Z-yaw, and
                                                  // Z-height)
  const IGoal* goal_;                             // External goal provide
  GoalMode goal_mode_child_;  // Goal mode for angle_level_controller_
  Axis4r goal_value_child_;   // Goal values for angle_level_controller_
  GoalMode last_goal_mode_;   // Previous goal modes
  Axis4r last_goal_val_;      // Previous goal values
  TReal output_;              // Our current output
  Params* params_;            // External parameters
  const IStateEstimator* state_estimator_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWXTORQUECONTROLLER_HPP_