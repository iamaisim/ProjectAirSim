// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_ANGLELEVELCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_ANGLELEVELCONTROLLER_HPP_

#include <exception>
#include <string>

#include "../Params.hpp"
#include "../PidController.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IBoardClock.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IUpdatable.hpp"
#include "AngleRateController.hpp"

namespace simple_flight {

class AngleLevelController : public IAxisController,
                             public IGoal  // for internal rate controller
{
 public:
  AngleLevelController(Params* params, const IBoardClock* clock = nullptr)
      : params_(params), clock_(clock) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    if (axis > 2)
      throw std::invalid_argument(
          "AngleLevelController only supports axis 0-2 but it was " +
          std::to_string(axis));

    axis_ = axis;
    goal_ = goal;
    state_estimator_ = state_estimator;

    // initialize level PID
    pid_.reset(new PidController<float>(
        clock_, PidConfig<float>(params_->angle_level_pid.p[axis],
                                 params_->angle_level_pid.i[axis],
                                 params_->angle_level_pid.d[axis])));

    // initialize rate controller
    rate_controller_.reset(new AngleRateController(params_, clock_));
    rate_controller_->Initialize(axis, this, state_estimator_);

    // we will be setting goal for rate controller so we need these two things
    rate_mode_ = GoalMode::GetUnknown();
    rate_mode_[axis] = GoalModeType::kAngleRate;
  }

  void Reset() override {
    IAxisController::Reset();

    pid_->Reset();
    rate_controller_->Reset();
    rate_goal_ = Axis4r();
    output_ = TReal();
  }

  void Update() override {
    IAxisController::Update();

    // get response of level PID
    const auto& level_goal = goal_->GetGoalValue();

    TReal goal_angle = level_goal[axis_];
    TReal measured_angle = state_estimator_->GetAngles()[axis_];

    AdjustToMinDistanceAngles(measured_angle, goal_angle);

    pid_->SetGoal(goal_angle);
    pid_->SetMeasured(measured_angle);
    pid_->Update();

    // use this to drive rate controller
    rate_goal_[axis_] =
        pid_->GetOutput() * params_->angle_rate_pid.max_limit[axis_];
    rate_controller_->Update();

    // rate controller's output is final output
    output_ = rate_controller_->GetOutput();
  }

  TReal GetOutput() override { return output_; }

  /********************  IGoal ********************/
  const Axis4r& GetGoalValue() const override { return rate_goal_; }

  const GoalMode& GetGoalMode() const override { return rate_mode_; }

 private:
  static void AdjustToMinDistanceAngles(TReal& angle1, TReal& angle2) {
    // first make sure both angles are restricted from -360 to +360
    angle1 = static_cast<TReal>(std::fmod(angle1, k2PI));
    angle2 = static_cast<TReal>(std::fmod(angle2, k2PI));

    // now make sure both angles are restricted from 0 to 360
    if (angle1 < 0) angle1 = k2PI + angle1;
    if (angle2 < 0) angle2 = k2PI + angle2;

    // measure distance between two angles
    auto dist = angle1 - angle2;

    // if its > 180 then invert first angle
    if (dist > kPI) angle1 = angle1 - k2PI;
    // if two much on other side then invert second angle
    else if (dist < -kPI)
      angle2 = angle2 - k2PI;
  }

 private:
  unsigned int axis_;
  const IGoal* goal_;
  const IStateEstimator* state_estimator_;

  GoalMode rate_mode_;
  Axis4r rate_goal_;

  TReal output_;

  Params* params_;
  const IBoardClock* clock_;
  std::unique_ptr<PidController<float>> pid_;
  std::unique_ptr<AngleRateController> rate_controller_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_ANGLELEVELCONTROLLER_HPP_