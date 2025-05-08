// Copyright (C) Microsoft Corporation. All rights reserved.

#include <chrono>
#include <thread>

#include "core_sim/clock.hpp"
#include "gtest/gtest.h"

namespace microsoft {
namespace projectairsim {

// Dummy scene class that has friend access to step SimClock
class Scene {
 public:
  void SimClockStep() { SimClock::Get()->Step(); }
};

// Add a test function to a dummy scheduled executor for confirming callback
class ScheduledExecutorTest : public ScheduledExecutor {
 public:
  static int64_t test_counter;
  static bool TestFuncTrue() {
    test_counter++;
    return true;
  }
};

int64_t ScheduledExecutorTest::test_counter = 0;

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

// -----------------------------------------------------------------------------
// ClockBase tests

TEST(SimClock, GetDefault) {
  auto simclock = projectairsim::SimClock::Get(nullptr);
  EXPECT_NE(simclock, nullptr);
}

TEST(SimClock, SecToNanos) {
  auto simclock = projectairsim::SimClock::Get(nullptr);
  const TimeSec time_sec = 3.0f;
  auto time_nanos = projectairsim::SimClock::Get()->SecToNanos(time_sec);
  EXPECT_EQ(time_nanos, 3'000'000'000);
}

TEST(SimClock, NanosToSec) {
  auto simclock = projectairsim::SimClock::Get(nullptr);
  const TimeNano time_nanos = 3'000'000'000;
  auto time_sec = projectairsim::SimClock::Get()->NanosToSec(time_nanos);
  EXPECT_FLOAT_EQ(time_sec, 3.0f);
}

// -----------------------------------------------------------------------------
// RealTimeClock tests

TEST(SimClock, RealTimeNowNanos) {
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::RealTimeClock>());

  TimeNano time = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_NE(time, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  auto time2 = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_NE(time2, time);
}

// -----------------------------------------------------------------------------
// SteppableClock tests

TEST(SimClock, SteppableNowSimNanos) {
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeNano time;
  time = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_EQ(time, 0);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_EQ(time, 2);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_EQ(time, 4);
}

TEST(SimClock, SteppableNowSimSec) {
  const TimeNano step = 2'000'000'000;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeSec time;
  time = projectairsim::SimClock::Get()->NowSimSec();
  EXPECT_FLOAT_EQ(time, 0.0f);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimSec();
  EXPECT_FLOAT_EQ(time, 2.0f);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimSec();
  EXPECT_FLOAT_EQ(time, 4.0f);
}

TEST(SimClock, SteppableNowSimMillis) {
  const TimeNano step = 2'000'000;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeMilli time;
  time = projectairsim::SimClock::Get()->NowSimMillis();
  EXPECT_EQ(time, 0);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMillis();
  EXPECT_EQ(time, 2);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMillis();
  EXPECT_EQ(time, 4);
}

TEST(SimClock, SteppableNowSimMicros) {
  const TimeNano step = 2'000;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeMicro time;
  time = projectairsim::SimClock::Get()->NowSimMicros();
  EXPECT_EQ(time, 0);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMicros();
  EXPECT_EQ(time, 2);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMicros();
  EXPECT_EQ(time, 4);
}

TEST(SimClock, SteppableSetPendingStepBy) {
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeNano time;
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 0);

  // Check for stepping directly to a new step
  projectairsim::SimClock::Get()->SetNextStep(1'000'000);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000'000);

  // Check for return to default step
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000'002);

  // Check for not allowing step backwards
  projectairsim::SimClock::Get()->SetNextStep(-2);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000'002);

  // Check for only applying last pending step request
  projectairsim::SimClock::Get()->SetNextStep(5'000'000);
  projectairsim::SimClock::Get()->SetNextStep(2'000'000);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 3'000'002);
}

TEST(SimClock, SteppableSetPendingStepTo) {
  const TimeNano step = 2;
  const TimeNano start = 1'000;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeNano time;
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000);

  // Check for stepping directly to a new time
  projectairsim::SimClock::Get()->SetNextSimTime(1'000'000);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000'000);

  // Check for returning to default step
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000'002);

  // Check for not allowing step backwards
  projectairsim::SimClock::Get()->SetNextSimTime(0);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1'000'002);

  // Check for only applying last pending step request
  projectairsim::SimClock::Get()->SetNextSimTime(5'000'000);
  projectairsim::SimClock::Get()->SetNextSimTime(2'000'000);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 2'000'000);
}

TEST(SimClock, SteppableSimPause) {
  const TimeNano step = 1;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1);

  // Pause, do a step, check time didn't advance
  simclock->SimPause(true);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 1);

  // Resume, do a step, check time did advance
  simclock->SimPause(false);
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 2);
}

TEST(SimClock, SteppableContinueForSimTime) {
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 2);

  // Set to continue for a delta_time, step past it and confirm time stopped
  simclock->ContinueForSimTime(10);
  for (int i = 0; i < 20; ++i) {
    scene.SimClockStep();
  }
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 12);
}

TEST(SimClock, SteppableContinueUntilSimTime) {
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 2);

  // Set to continue to a target_time, step past it and confirm time stopped
  simclock->ContinueUntilSimTime(10);
  for (int i = 0; i < 20; ++i) {
    scene.SimClockStep();
  }
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 10);
}

TEST(SimClock, SteppableContinueForNSteps) {
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 2);

  // Set to continue for N steps, step past it and confirm time stopped
  simclock->ContinueForNBaseSteps(10);
  for (int i = 0; i < 20; ++i) {
    scene.SimClockStep();
  }
  time = simclock->NowSimNanos();
  EXPECT_EQ(time, 22);
}

TEST(SimClock, SteppableIsPaused) {
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  bool is_paused;

  // Check default initial state is unpaused
  is_paused = simclock->IsPaused();
  EXPECT_FALSE(is_paused);

  // Pause and check status
  simclock->SimPause(true);
  is_paused = simclock->IsPaused();
  EXPECT_TRUE(is_paused);

  // Unpause and check status
  simclock->SimPause(false);
  is_paused = simclock->IsPaused();
  EXPECT_FALSE(is_paused);
}

// -----------------------------------------------------------------------------
// ScheduledExecutor tests

TEST(ScheduledExecutor, Construction) {
  projectairsim::ScheduledExecutor executor1;

  std::function<bool()> callback = nullptr;
  projectairsim::ScheduledExecutor executor2(callback, 1);
}

// TODO Split this combo test into individual tests
TEST(ScheduledExecutor, Combo) {
  TimeSec margin = 0.001f;  // 1 ms

  std::function<bool()> callback =
      projectairsim::ScheduledExecutorTest::TestFuncTrue;

  projectairsim::ScheduledExecutor executor(callback,
                                          10'000'000);  // 10 ms period

  EXPECT_EQ(projectairsim::ScheduledExecutorTest::test_counter, 0);

  executor.Start();
  EXPECT_TRUE(executor.IsRunning());

  std::this_thread::sleep_for(std::chrono::nanoseconds(100'000'000));  // 100 ms
  auto sleep_time = executor.GetSleepTimeAve();
  EXPECT_GT(sleep_time, 0.010f - margin);
  EXPECT_LT(sleep_time, 0.010f + margin);

  EXPECT_GT(projectairsim::ScheduledExecutorTest::test_counter, 0);

  executor.Stop();
  EXPECT_FALSE(executor.IsRunning());
}

// TODO Test other scheduled executor functionality
