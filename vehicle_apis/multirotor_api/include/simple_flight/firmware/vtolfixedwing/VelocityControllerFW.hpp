// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VELOCITYCONTROLLERFW_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VELOCITYCONTROLLERFW_HPP_

#include "../interfaces/IFWAngleController.hpp"

namespace simple_flight {

// Same as PositionController except we also support the IFWAngleController
// interface
class VelocityControllerFW : public IFWAngleController,
                             public IGoal  // for internal child controller
{
 public:
  VelocityControllerFW(Params* params, const IBoardClock* clock = nullptr)
      : params_(params), clock_(clock) {}

  virtual void Initialize(unsigned int axis, const IGoal* goal,
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

    // we will be setting goal for child controller so we need these two things
    child_mode_ = GoalMode::GetUnknown();
    switch (axis_) {
      case 0:
        child_controller_.reset(new AngleLevelController(params_, clock_));
        child_mode_[axis_] = GoalModeType::kAngleLevel;  // vy = roll
        break;

      case 1:
        child_controller_.reset(new AngleLevelController(params_, clock_));
        child_mode_[axis_] = GoalModeType::kAngleLevel;  // vx = - pitch
        break;

      case 2:
        // we control yaw
        throw std::invalid_argument(
            "axis must be 0, 1 or 3 but it was " + std::to_string(axis_) +
            " because yaw cannot be controlled by VelocityController");

      case 3:
        // not really required
        // output of parent controller is -1 to 1 which
        // we will transform to 0 to 1
        child_controller_.reset(new PassthroughController());
        child_mode_[axis_] = GoalModeType::kPassthrough;
        break;

      default:
        throw std::invalid_argument("axis must be 0 to 2");
    }

    // initialize child controller
    child_controller_->Initialize(axis_, this, state_estimator_);
  }

  virtual void Reset() override {
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
    const Axis4r& goal_velocity_local = Axis4r::XyzToAxis4(
        state_estimator_->TransformVectorToBodyFrame(goal_velocity_world),
        true);
    pid_->SetGoal(goal_velocity_local[axis_]);

    const Axis3r& measured_velocity_world =
        state_estimator_->GetLinearVelocity();
    const Axis4r& measured_velocity_local = Axis4r::XyzToAxis4(
        state_estimator_->TransformVectorToBodyFrame(measured_velocity_world),
        true);
    pid_->SetMeasured(measured_velocity_local[axis_]);
    pid_->Update();

    // use this to drive child controller
    switch (axis_) {
      case 0:  //+vy is +ve roll
        child_goal_[axis_] =
            pid_->GetOutput() * params_->angle_level_pid.max_limit[axis_];
        child_controller_->Update();
        output_ = child_controller_->GetOutput();
        break;

      case 1:  //+vx is -ve pitch
        child_goal_[axis_] =
            -pid_->GetOutput() * params_->angle_level_pid.max_limit[axis_];
        child_controller_->Update();
        output_ = child_controller_->GetOutput();
        break;

      case 3:  //+vz is -ve throttle (NED coordinates)
        output_ = (-pid_->GetOutput() + 1) / 2;  //-1 to 1 --> 0 to 1
        output_ = std::max(output_, params_->velocity_pid.min_throttle);
        break;

      default:
        throw std::invalid_argument(
            "axis must be 0, 1 or 3 for VelocityController");
    }
  }

  virtual TReal GetOutput() override { return output_; }

  /********************  IFWAngleController ********************/
  TReal GetOutputAngle() override { return (child_goal_[axis_]); }

  /********************  IGoal ********************/
  const Axis4r& GetGoalValue() const override { return child_goal_; }

  const GoalMode& GetGoalMode() const override { return child_mode_; }

 private:
  class VelocityControllerInternal : public VelocityController {
   public:
    VelocityControllerInternal(Params* params, const IBoardClock* clock)
        : VelocityController(params, clock) {}

    TReal GetOutputAngle(void) {
      return (((axis_ == 0) || (axis_ == 1)) ? child_goal_[axis_] : 0.0f);
    }
  };

 private:
  unsigned int axis_;
  const IGoal* goal_;
  const IStateEstimator* state_estimator_;

  GoalMode child_mode_;
  Axis4r child_goal_;

  TReal output_;

  Params* params_;
  const IBoardClock* clock_;
  std::unique_ptr<PidController<float>> pid_;
  std::unique_ptr<IAxisController> child_controller_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VELOCITYCONTROLLERFW_HPP_