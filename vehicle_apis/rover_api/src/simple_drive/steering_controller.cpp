// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include <math.h>

#include <simple_drive/steering_controller.hpp>

namespace v = microsoft::projectairsim::vehicle_apis;

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

SteeringController::SteeringController(std::shared_ptr<const Params>& pparams,
                                       ClockBase* clock, Logger& logger)
    : ChannelControllerBase(pparams, clock, logger),
      goals_last_(),
      pid_controller_steering_(),
      pid_controller_yaw_rate_() {}

void SteeringController::Update(void) {
  auto goals_new = *pgoals_;
  bool fgoalmodes_changed = goals_last_.CheckChangedModes(goals_new);

  auto goal = (*pgoals_)[GoalTargetIndex::kZ];

  if (goal.mode == Goal::Mode::kPassthrough)
    output_ = goal.value;
  else if (goal.mode == Goal::Mode::kNone) {
    auto goalY = (*pgoals_)[GoalTargetIndex::kY];

    goal = (*pgoals_)[GoalTargetIndex::kX];
    if ((goal.mode == Goal::Mode::kPositionWorld) &&
        (goalY.mode == Goal::Mode::kPositionWorld)) {
      // Driving towards the position (x, y)

      if (fgoalmodes_changed) {
        // Setup the steering control PID
        pid_controller_steering_.setPoint(0.0f, pparams_->angle_level_pid.p.Z(),
                                          pparams_->angle_level_pid.i.Z(),
                                          pparams_->angle_level_pid.d.Z());
        pid_controller_yaw_rate_.setPoint(1.0f, pparams_->angle_rate_pid.p.Z(),
                                          pparams_->angle_rate_pid.i.Z(),
                                          pparams_->angle_rate_pid.d.Z());
      }

      // Calculate difference in angle to target
      {
        auto axis3rAngles = pistate_estimator_->GetAngles();
        auto axis3rPosition = pistate_estimator_->GetPosition();
        float radian_target = atan2(goalY.value - axis3rPosition.Y(),
                                    goal.value - axis3rPosition.X());
        float dradianError = axis3rAngles.Yaw() - radian_target;

        if (dradianError > v::kPI)
          dradianError -= 2 * v::kPI;
        else if (dradianError < -v::kPI)
          dradianError += 2 * v::kPI;

        // Calculate steering control value where in the range (-1, +1)
        output_ = pid_controller_steering_.control(dradianError);
        if (output_ < -1.0f)
          output_ = -1.0f;
        else if (output_ > 1.0f)
          output_ = 1.0f;
      }

      // If yaw rate exceeds maximum, reduce the output
      {
        auto yaw_rate_cur = pistate_estimator_->GetAngularVelocity().Z();
        auto yaw_rate_max = pparams_->angle_rate_pid.max_limit.Z();
        auto rfactor_yaw_rate =
            pid_controller_yaw_rate_.control(yaw_rate_cur / yaw_rate_max);

        if (rfactor_yaw_rate < 0) {
          if (rfactor_yaw_rate < -100.0f) rfactor_yaw_rate = -100.0f;

          output_ += rfactor_yaw_rate * output_;
        }
      }
    }
  }
  else if (goal.mode == Goal::Mode::kAngleLevel)
  {
    if (fgoalmodes_changed) {
      // Setup the steering control PID
      pid_controller_steering_.setPoint(0.0f, pparams_->angle_level_pid.p.Z(),
                                        pparams_->angle_level_pid.i.Z(),
                                        pparams_->angle_level_pid.d.Z());
      pid_controller_yaw_rate_.setPoint(1.0f, pparams_->angle_rate_pid.p.Z(),
                                        pparams_->angle_rate_pid.i.Z(),
                                        pparams_->angle_rate_pid.d.Z());
    }

    // Calculate difference in angle to target
    {
      auto axis3rAngles = pistate_estimator_->GetAngles();
      float radian_target = goal.value;
      float dradianError = axis3rAngles.Yaw() - radian_target;

      if (dradianError > v::kPI)
        dradianError -= 2 * v::kPI;
      else if (dradianError < -v::kPI)
        dradianError += 2 * v::kPI;

      // Calculate steering control value where in the range (-1, +1)
      output_ = pid_controller_steering_.control(dradianError);
      if (output_ < -1.0f)
        output_ = -1.0f;
      else if (output_ > 1.0f)
        output_ = 1.0f;
    }

    // If yaw rate exceeds maximum, reduce the output
    {
      auto yaw_rate_cur = pistate_estimator_->GetAngularVelocity().Z();
      auto yaw_rate_max = pparams_->angle_rate_pid.max_limit.Z();
      auto rfactor_yaw_rate =
          pid_controller_yaw_rate_.control(yaw_rate_cur / yaw_rate_max);

      if (rfactor_yaw_rate < 0) {
        if (rfactor_yaw_rate < -100.0f) rfactor_yaw_rate = -100.0f;

        output_ += rfactor_yaw_rate * output_;
      }
    }
  }

  // Save new goals if different
  if (fgoalmodes_changed) goals_last_ = goals_new;
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
