// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRYTORQUECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRYTORQUECONTROLLER_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include <exception>
#include <string>

#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IFWAngleController.hpp"
#include "../multirotor/AngleRateController.hpp"
#include "../vtolfixedwing/PositionControllerFW.hpp"
#include "../vtolfixedwing/PositionControllerFWZH.hpp"
#include "../vtolfixedwing/VelocityControllerFW.hpp"
#include "VelocityControllerTRZH.hpp"

namespace simple_flight {

// Y-axis torque controller for fixed-wing flight
class TRYTorqueController : public IAxisController, private IGoal {
 public:
  TRYTorqueController(Params* params, const IBoardClock* clock = nullptr)
      : axis_controllers_(),
        clock_(clock),
        fDoTorqueControllerOnly_(false),
        fwangle_controllers_(),
        goal_(nullptr),
        goal_mode_child_(),
        goal_value_child_(),
        last_goal_mode_(),
        last_goal_val_(),
        output_(Axis4r()[IAxisController::kYPitch]),
        pangle_level_controller_(),
        params_(params),
        state_estimator_(nullptr) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    goal_ = goal;
    state_estimator_ = state_estimator;

    goal_mode_child_ = GoalMode::GetUnknown();
    goal_mode_child_[IAxisController::kYPitch] = GoalModeType::kAngleLevel;
    goal_value_child_ = Axis4r();
  }

  void Reset() override {
    IAxisController::Reset();

    last_goal_mode_ = GoalMode::GetUnknown();
    output_ = Axis4r()[IAxisController::kYPitch];

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (fwangle_controllers_[axis] != nullptr)
        fwangle_controllers_[axis]->Reset();
    }
    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (axis_controllers_[axis] != nullptr) axis_controllers_[axis]->Reset();
    }

    if (pangle_level_controller_ != nullptr)
      pangle_level_controller_.reset(nullptr);
  }

  void Update() override {
    IAxisController::Update();

    const auto& goal_mode = goal_->GetGoalMode();
    const auto& goal_val = goal_->GetGoalValue();

    if (!goal_val.Equals4(last_goal_val_)) {
      last_goal_val_ = goal_val;
    }

    if (pangle_level_controller_ == nullptr) {
      pangle_level_controller_.reset(
          new AngleLevelControllerFW(params_, clock_));
      pangle_level_controller_->Initialize(IAxisController::kYPitch, this,
                                           state_estimator_);
    }

    // re-create axis controllers if goal mode was changed since last time
    for (int axis = 0, caxis = Axis4r::AxisCount(); axis < caxis; ++axis) {
      if (goal_mode[axis] != last_goal_mode_[axis] ||
          params_->gains_changed == true) {
        axis_controllers_[axis].reset(nullptr);
        fwangle_controllers_[axis].reset(nullptr);

        switch (axis) {
          default:
          case IAxisController::kXRoll:
          case IAxisController::kZYaw:
            // Don't handle this axis
            break;

          case IAxisController::kYPitch:
            axis_controllers_[axis].reset(GetYAxisController(goal_mode));
            break;

          case IAxisController::kZHeight:
            fwangle_controllers_[axis].reset(GetZHeightController(goal_mode));
            break;
        }

        last_goal_mode_[axis] = goal_mode[axis];

        // initialize axis controller
        if (axis_controllers_[axis] != nullptr) {
          axis_controllers_[axis]->Initialize(axis, goal_, state_estimator_);
          axis_controllers_[axis]->Reset();
        }
        if (fwangle_controllers_[axis] != nullptr) {
          fwangle_controllers_[axis]->Initialize(axis, goal_, state_estimator_);
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

    // A Y-pitch torque controller has an angle-level or angle-rate goal which
    // sets a specific Y-pitch target and takes precedence over our angle
    // controllers which have indirect goals like controlling the Z position.
    // If we have a Y-pitch torque controller, we should only apply that.
    {
      auto goal_mode_type = goal_mode[IAxisController::kYPitch];

      fDoTorqueControllerOnly_ =
          (((goal_mode_type == GoalModeType::kAngleLevel) ||
            (goal_mode_type == GoalModeType::kAngleRate)) &&
           axis_controllers_[IAxisController::kYPitch] != nullptr);
    }

    // If we're applying the Y-pitch angle controllers, drive the child angle
    // controller
    if (!fDoTorqueControllerOnly_) {
      int ccontroller = 0;
      TReal sum = 0;
      TReal anglePitch;

      // Target pitch is average of angle controller outputs
      for (unsigned int axis = 0, caxis = Axis4r::AxisCount(); axis < caxis;
           ++axis) {
        if (fwangle_controllers_[axis] != nullptr) {
          ++ccontroller;
          sum += fwangle_controllers_[axis]->GetOutputAngle();
        }
      }

      anglePitch = (ccontroller > 0) ? (sum / ccontroller) : -kPI_2;

      goal_value_child_[IAxisController::kYPitch] = anglePitch;
      pangle_level_controller_->Update();

#ifdef LVMON_REPORTING
      {
        LVMon::Set("FWYTC/pitchgoal", anglePitch);

        if (axis_controllers_[IAxisController::kYPitch] != nullptr)
          LVMon::Set("FWYTC/childYPitch/output",
                     axis_controllers_[IAxisController::kYPitch]->GetOutput());

        if (fwangle_controllers_[IAxisController::kZHeight] != nullptr)
          LVMon::Set("FWYTC/childZHeight/output_angle",
                     fwangle_controllers_[IAxisController::kZHeight]
                         ->GetOutputAngle());

        auto output_alc = pangle_level_controller_->GetOutput();
        LVMon::Set("FWYTC/ALC/output", output_alc);
      }
#endif  // LVMON_REPORTING
    }
  }

  TReal GetOutput() override {
    output_ = 0.0f;

    output_ = fDoTorqueControllerOnly_
                  ? axis_controllers_[IAxisController::kYPitch]->GetOutput()
                  : pangle_level_controller_->GetOutput();

#ifdef LVMON_REPORTING
    LVMon::Set("FWYTC/output", output_);
#endif  // LVMON_REPORTING

    return (output_);
  }

 private:
  IAxisController* GetYAxisController(const GoalMode& goal_mode) {
    switch (goal_mode[IAxisController::kYPitch]) {
      default:
        return (nullptr);

      case GoalModeType::kAngleRate:
        return (new AngleRateControllerFW(params_, clock_));

      case GoalModeType::kAngleLevel:
        return (new AngleLevelControllerFW(params_, clock_));
    }
  }

  IFWAngleController* GetZHeightController(const GoalMode& goal_mode) {
    switch (goal_mode[IAxisController::kZHeight]) {
      default:
        return (nullptr);

      case GoalModeType::kPositionWorld:
        return (new PositionControllerFWZH(params_, clock_));

      case GoalModeType::kVelocityWorld:
        return (new VelocityControllerTRZH(params_, clock_));
    }
  }

 public:
  const GoalMode& GetGoalMode(void) const override {
    return (goal_mode_child_);
  }
  const Axis4r& GetGoalValue(void) const override {
    return (goal_value_child_);
  }

 private:
  std::unique_ptr<IAxisController>
      axis_controllers_[Axis4r::AxisCount()];  // Controllers for torque to
                                               // apply to each "axis"
                                               // (X-pitch, Y-roll, Z-yaw, and
                                               // Z-height)
  const IBoardClock* clock_;
  bool fDoTorqueControllerOnly_;  // If true, apply torque controllers and
                                  // ignore angle controllers
  std::unique_ptr<IFWAngleController>
      fwangle_controllers_[Axis4r::AxisCount()];  // Controllers for angle of
                                                  // each "axis" (X-pitch,
                                                  // Y-roll, Z-yaw, and
                                                  // Z-height)
  const IGoal* goal_;                             // External goals
  GoalMode goal_mode_child_;  // Goal modes for angle_level_controller_
  Axis4r goal_value_child_;   // Goal values for angle_level_controller_
  GoalMode last_goal_mode_;   // Previous goal modes
  Axis4r last_goal_val_;      // Previous goal values
  TReal output_;              // Our current output
  std::unique_ptr<AngleLevelControllerFW>
      pangle_level_controller_;  // Child angle level controller used to apply
                                 // net output of fwangle_controllers_
  Params* params_;               // External parameters
  const IStateEstimator* state_estimator_;  // Vehicle state estimator
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRYTORQUECONTROLLER_HPP_