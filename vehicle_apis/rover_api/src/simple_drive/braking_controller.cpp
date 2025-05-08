// Copyright (C) Microsoft Corporation. All rights reserved.

#include <simple_drive/braking_controller.hpp>

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

void BrakingController::Update(void) {
  auto goal = (*pgoals_)[GoalTargetIndex::kBrake];

  if (goal.mode == Goal::Mode::kPassthrough) output_ = goal.value;
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
