// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include <simple_drive/throttle_controller.hpp>

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

ThrottleController::ThrottleController(std::shared_ptr<const Params>& pparams,
                                       ClockBase* clock, Logger& logger)
    : ChannelControllerBase(pparams, clock, logger),
      goals_last_(),
      pid_controller_throttle_() {}

void ThrottleController::Update(void) {
  auto goals_new = *pgoals_;
  bool fgoalmodes_changed = goals_last_.CheckChangedModes(goals_new);

  auto goal = (*pgoals_)[GoalTargetIndex::kThrottle];

  if (goal.mode == Goal::Mode::kPassthrough)
    output_ = goal.value;
  else if (goal.mode == Goal::Mode::kVelocityWorld) {
    auto velocity_cur = pistate_estimator_->GetLinearVelocity().X();

    if (fgoalmodes_changed ||
        (goal.value != pid_controller_throttle_.getPoint())) {
      // Setup the steering control PID
      pid_controller_throttle_.setPoint(goal.value,
                                       pparams_->angle_level_pid.p.Throttle(),
                                       pparams_->angle_level_pid.i.Throttle(),
                                       pparams_->angle_level_pid.d.Throttle());
    }

    output_ = pid_controller_throttle_.control(velocity_cur);
    if (output_ < -1.0f)
      output_ = -1.0f;
    else if (output_ > 1.0f)
      output_ = 1.0f;
  }

  // Save new goals if different
  if (fgoalmodes_changed) goals_last_ = goals_new;
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
