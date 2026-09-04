// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "core_sim/message/clock_message.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;

TEST(ClockMessage, PreservesNanosecondTimestamp) {
  constexpr TimeNano kTimestamp = 1234567890123456LL;
  projectairsim::ClockMessage original(kTimestamp);

  EXPECT_EQ(original.GetType(), projectairsim::MessageType::kClock);

  projectairsim::ClockMessage unpacked;
  unpacked.Deserialize(original.Serialize());
  EXPECT_EQ(unpacked.GetTimeStamp(), kTimestamp);
}
