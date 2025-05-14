// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_CASCADECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_CASCADECONTROLLER_HPP_

#include <exception>
#include <string>

#include "../interfaces/ICommLink.hpp"
#include "../interfaces/IController.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IStateEstimator.hpp"
#include "AngleLevelController.hpp"
#include "AngleRateController.hpp"
#include "ConstantOutputController.hpp"
#include "PassthroughController.hpp"
#include "PositionController.hpp"
#include "VelocityController.hpp"

namespace simple_flight {

class CascadeController : public IController {
 public:
  CascadeController(Params* params, const IBoardClock* clock,
                    ICommLink* comm_link)
      : params_(params), clock_(clock), comm_link_(comm_link) {}

  void Initialize(const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    goal_ = goal;
    state_estimator_ = state_estimator;
  }

  void Reset() override {
    IController::Reset();

    last_goal_mode_ = GoalMode::GetUnknown();
    output_ = AxisNr();

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (axis_controllers_[axis] != nullptr) axis_controllers_[axis]->Reset();
    }
  }

  void Update() override {
    IController::Update();

    const auto& goal_mode = goal_->GetGoalMode();
    const auto& goal_val = goal_->GetGoalValue();

    // common_utils::Utils::log(common_utils::Utils::stringf("Pos: %s",
    // state_estimator_->getPosition().toString().c_str()));

    // if (!goal_mode.equals4(last_goal_mode_)) {
    //    common_utils::Utils::log(common_utils::Utils::stringf("GoalMode: %s",
    //    goal_mode.toString().c_str()));
    //}
    if (!goal_val.Equals4(last_goal_val_)) {
      // common_utils::Utils::log(common_utils::Utils::stringf("GoalVal : %s",
      // goal_val.toString().c_str()));
      last_goal_val_ = goal_val;
    }

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      // re-create axis controllers if goal mode was changed since last time
      if (goal_mode[axis] != last_goal_mode_[axis] ||
          params_->gains_changed == true) {
        switch (goal_mode[axis]) {
          case GoalModeType::kAngleRate:
            axis_controllers_[axis].reset(
                new AngleRateController(params_, clock_));
            break;
          case GoalModeType::kAngleLevel:
            axis_controllers_[axis].reset(
                new AngleLevelController(params_, clock_));
            break;
          case GoalModeType::kVelocityWorld:
            axis_controllers_[axis].reset(
                new VelocityController(params_, clock_));
            break;
          case GoalModeType::kVelocityBody:
            axis_controllers_[axis].reset(new VelocityController(
                params_, clock_, VelocityFrameType::kBody));
            break;
          case GoalModeType::kPositionWorld:
            axis_controllers_[axis].reset(
                new PositionController(params_, clock_));
            break;
          case GoalModeType::kPassthrough:
            axis_controllers_[axis].reset(new PassthroughController());
            break;
          case GoalModeType::kUnknown:
            axis_controllers_[axis].reset(nullptr);
            break;
          case GoalModeType::kConstantOutput:
            axis_controllers_[axis].reset(new ConstantOutputController());
            break;
          default:
            throw std::invalid_argument(
                "Axis controller type is not yet implemented for axis " +
                std::to_string(axis));
        }
        last_goal_mode_[axis] = goal_mode[axis];

        // initialize axis controller
        if (axis_controllers_[axis] != nullptr) {
          axis_controllers_[axis]->Initialize(axis, goal_, state_estimator_);
          axis_controllers_[axis]->Reset();
        }
      }

      // update axis controller
      if (axis_controllers_[axis] != nullptr) {
        axis_controllers_[axis]->Update();
        output_[axis] = axis_controllers_[axis]->GetOutput();
      } else
        comm_link_->Log(std::string("Axis controller type is not set for axis ")
                            .append(std::to_string(axis)),
                        ICommLink::kLogLevelInfo);
    }
  }

  const AxisNr& GetOutput() override { return output_; }

 private:
  Params* params_;
  const IBoardClock* clock_;

  const IGoal* goal_;
  const IStateEstimator* state_estimator_;
  ICommLink* comm_link_;

  AxisNr output_;

  GoalMode last_goal_mode_;
  Axis4r last_goal_val_;

  std::unique_ptr<IAxisController> axis_controllers_[Axis4r::AxisCount()];
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_CASCADECONTROLLER_HPP_