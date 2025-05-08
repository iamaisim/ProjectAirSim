// Copyright (C) Microsoft Corporation. All rights reserved.

#include <simple_drive/common.hpp>

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

bool Goals::CheckChangedModes(const Goals& goals_other) {
  auto caxis = AxisCount();

  if (caxis != goals_other.AxisCount()) return true;

  for (int iaxis = 0; iaxis < caxis; ++iaxis) {
    if ((*this)[iaxis].mode != goals_other[iaxis].mode) return true;
  }

  return false;
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
