// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRZTORQUECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRZTORQUECONTROLLER_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include <exception>
#include <string>

#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IFWAngleController.hpp"
#include "../vtolfixedwing/AngleLevelControllerFW.hpp"
#include "../vtolfixedwing/PositionControllerFW.hpp"
#include "../vtolfixedwing/VelocityControllerFW.hpp"

// Z-axis torque controller for fixed-wing flight which controls the yaw of
// the vehicle
namespace simple_flight {

class TRZTorqueController : public IAxisController, public IGoal {
 public:
  TRZTorqueController(Params* params, const IBoardClock* clock = nullptr)
      : axis_controllers_(),
        clock_(clock),
        fDoTorqueControllerOnly_(false),
        fwangle_controllers_(),
        goal_(nullptr),
        goal_mode_child_(),
        goal_value_child_(),
        last_goal_mode_(),
        last_goal_val_(),
        output_(Axis4r()[IAxisController::kZYaw]),
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
    output_ = Axis4r()[IAxisController::kZYaw];

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (fwangle_controllers_[axis] != nullptr)
        fwangle_controllers_[axis]->Reset();
    }
    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (axis_controllers_[axis] != nullptr) axis_controllers_[axis]->Reset();
    }
  }

  void Update() override {
    IAxisController::Update();

    const auto& goal_mode = goal_->GetGoalMode();
    const auto& goal_val = goal_->GetGoalValue();

    if (!goal_val.Equals4(last_goal_val_)) {
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
              axis, goal_, state_estimator_);
          axis_controllers_[axis]->Reset();
        }
        if (fwangle_controllers_[axis] != nullptr) {
          fwangle_controllers_[axis]->Initialize(
              axis, goal_, state_estimator_);
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
    auto goal_mode_type = goal_mode[IAxisController::kZYaw];

    fDoTorqueControllerOnly_ =
        (((goal_mode_type == GoalModeType::kAngleLevel) ||
          (goal_mode_type == GoalModeType::kAngleRate)) &&
         axis_controllers_[IAxisController::kZYaw] != nullptr);
  }

  TReal GetOutput() override { // check
    output_ = 0.0f;

    // Get output
    if (fabs(state_estimator_->GetAngles().Pitch()) >
        (kPI / 2.0f - params_->vtol.pitch_min_fixed_wing)) {
        //If the vehicle is pitched too vertically, don't attempt to yaw
        output_ = 0.0f;
    } else if (axis_controllers_[IAxisController::kZYaw] != nullptr) {
      output_ = axis_controllers_[IAxisController::kZYaw]->GetOutput();
    } else {
      // Combing axis angle controllers
      int coutput = 0;
      auto yaw = state_estimator_->GetAngles()[IAxisController::kZYaw];
      auto sum = 0.0;

      sum = 0.0f;

      // X-roll controller's output is positive to turn eastward, negative
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
      // to turn southward.  Its contribution can be merged directly to our
      // output.
      if (axis_controllers_[IAxisController::kYPitch] != nullptr) {
        ++coutput;
        sum += axis_controllers_[IAxisController::kYPitch]->GetOutput();
      }

      output_ = (coutput > 1) ? sum / coutput : sum;
    }

#ifdef LVMON_REPORTING
    LVMon::Set("FWZTC/output", output_);
#endif  // LVMON_REPORTING

    return (output_);
  }

 public:  // IGoal Method Overrides
  const Axis4r& GetGoalValue() const override { return goal_value_child_; }
  const GoalMode& GetGoalMode() const override { return goal_mode_child_; }

 private:
  std::unique_ptr<IAxisController>
      axis_controllers_[Axis4r::AxisCount()];  // Controllers for each "axis"
                                               // (X-pitch, Y-roll, Z-yaw, and
                                               // Z-height)
  const IBoardClock* clock_;                   // Board clock
  bool fDoTorqueControllerOnly_;  // If true, apply torque controllers and
                                  // ignore angle controllers
  std::unique_ptr<IFWAngleController>
      fwangle_controllers_[Axis4r::AxisCount()];  // Controllers for angle of
                                                  // each "axis" (X-pitch,
                                                  // Y-roll, Z-yaw, and
                                                  // Z-height)
  const IGoal* goal_;        // External goals
  GoalMode goal_mode_child_;  // Goal mode for angle_level_controller_
  Axis4r goal_value_child_;   // Goal values for angle_level_controller_
  GoalMode last_goal_mode_;  // Previous goal modes
  Axis4r last_goal_val_;     // Previous goal values
  TReal output_;             // Our current output
  Params* params_;           // External parameters
  const IStateEstimator* state_estimator_;  // Vehical state estimator
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_TRZTORQUECONTROLLER_HPP_