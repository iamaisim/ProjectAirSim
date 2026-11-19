// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

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
  // General description:
  // Verifies get default for SimClock.
  // Arrange: prepare context for `auto simclock = projectairsim::SimClock::Get(nullptr);`.
  // Act: run `auto simclock = projectairsim::SimClock::Get(nullptr);`.
  auto simclock = projectairsim::SimClock::Get(nullptr);
  // Assert: check result from `EXPECT_NE(simclock, nullptr);`.
  EXPECT_NE(simclock, nullptr);
}

TEST(SimClock, SecToNanos) {
  // General description:
  // Verifies sec to nanos for SimClock.
  // Arrange: prepare context for `auto simclock = projectairsim::SimClock::Get(nullptr);`.
  auto simclock = projectairsim::SimClock::Get(nullptr);
  const TimeSec time_sec = 3.0f;
  // Act: run `auto time_nanos = projectairsim::SimClock::Get()->SecToNanos(time_sec);`.
  auto time_nanos = projectairsim::SimClock::Get()->SecToNanos(time_sec);
  // Assert: check result from `EXPECT_EQ(time_nanos, 3'000'000'000);`.
  EXPECT_EQ(time_nanos, 3'000'000'000);
}

TEST(SimClock, NanosToSec) {
  // General description:
  // Verifies nanos to sec for SimClock.
  // Arrange: prepare context for `auto simclock = projectairsim::SimClock::Get(nullptr);`.
  auto simclock = projectairsim::SimClock::Get(nullptr);
  const TimeNano time_nanos = 3'000'000'000;
  // Act: run `auto time_sec = projectairsim::SimClock::Get()->NanosToSec(time_nanos);`.
  auto time_sec = projectairsim::SimClock::Get()->NanosToSec(time_nanos);
  // Assert: check result from `EXPECT_FLOAT_EQ(time_sec, 3.0f);`.
  EXPECT_FLOAT_EQ(time_sec, 3.0f);
}

// -----------------------------------------------------------------------------
// RealTimeClock tests

TEST(SimClock, RealTimeNowNanos) {
  // General description:
  // Verifies real time now nanos for SimClock.
  // Arrange: prepare context for `auto simclock = projectairsim::SimClock::Get(`.
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::RealTimeClock>());

  // Act: run `TimeNano time = projectairsim::SimClock::Get()->NowSimNanos();`.
  TimeNano time = projectairsim::SimClock::Get()->NowSimNanos();
  // Assert: check result from `EXPECT_NE(time, 0);`.
  EXPECT_NE(time, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  auto time2 = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_NE(time2, time);
}

// -----------------------------------------------------------------------------
// SteppableClock tests

TEST(SimClock, SteppableNowSimNanos) {
  // General description:
  // Verifies steppable now sim nanos for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeNano time;
  // Act: run `time = projectairsim::SimClock::Get()->NowSimNanos();`.
  time = projectairsim::SimClock::Get()->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 0);`.
  EXPECT_EQ(time, 0);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_EQ(time, 2);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimNanos();
  EXPECT_EQ(time, 4);
}

TEST(SimClock, SteppableNowSimSec) {
  // General description:
  // Verifies steppable now sim sec for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2'000'000'000;`.
  const TimeNano step = 2'000'000'000;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeSec time;
  // Act: run `time = projectairsim::SimClock::Get()->NowSimSec();`.
  time = projectairsim::SimClock::Get()->NowSimSec();
  // Assert: check result from `EXPECT_FLOAT_EQ(time, 0.0f);`.
  EXPECT_FLOAT_EQ(time, 0.0f);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimSec();
  EXPECT_FLOAT_EQ(time, 2.0f);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimSec();
  EXPECT_FLOAT_EQ(time, 4.0f);
}

TEST(SimClock, SteppableNowSimMillis) {
  // General description:
  // Verifies steppable now sim millis for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2'000'000;`.
  const TimeNano step = 2'000'000;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeMilli time;
  // Act: run `time = projectairsim::SimClock::Get()->NowSimMillis();`.
  time = projectairsim::SimClock::Get()->NowSimMillis();
  // Assert: check result from `EXPECT_EQ(time, 0);`.
  EXPECT_EQ(time, 0);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMillis();
  EXPECT_EQ(time, 2);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMillis();
  EXPECT_EQ(time, 4);
}

TEST(SimClock, SteppableNowSimMicros) {
  // General description:
  // Verifies steppable now sim micros for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2'000;`.
  const TimeNano step = 2'000;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeMicro time;
  // Act: run `time = projectairsim::SimClock::Get()->NowSimMicros();`.
  time = projectairsim::SimClock::Get()->NowSimMicros();
  // Assert: check result from `EXPECT_EQ(time, 0);`.
  EXPECT_EQ(time, 0);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMicros();
  EXPECT_EQ(time, 2);

  scene.SimClockStep();
  time = projectairsim::SimClock::Get()->NowSimMicros();
  EXPECT_EQ(time, 4);
}

TEST(SimClock, SteppableSetPendingStepBy) {
  // General description:
  // Verifies steppable set pending step by for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeNano time;
  // Act: run `time = simclock->NowSimNanos();`.
  time = simclock->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 0);`.
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
  // General description:
  // Verifies steppable set pending step to for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 1'000;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;

  TimeNano time;
  // Act: run `time = simclock->NowSimNanos();`.
  time = simclock->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 1'000);`.
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
  // General description:
  // Verifies steppable sim pause for SimClock.
  // Arrange: prepare context for `const TimeNano step = 1;`.
  const TimeNano step = 1;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  // Act: run `time = simclock->NowSimNanos();`.
  time = simclock->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 1);`.
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
  // General description:
  // Verifies steppable continue for sim time for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  // Act: run `time = simclock->NowSimNanos();`.
  time = simclock->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 2);`.
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
  // General description:
  // Verifies steppable continue until sim time for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  // Act: run `time = simclock->NowSimNanos();`.
  time = simclock->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 2);`.
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
  // General description:
  // Verifies steppable continue for nsteps for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  TimeNano time;

  // Do a single step and check the time
  scene.SimClockStep();
  // Act: run `time = simclock->NowSimNanos();`.
  time = simclock->NowSimNanos();
  // Assert: check result from `EXPECT_EQ(time, 2);`.
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
  // General description:
  // Verifies steppable is paused for SimClock.
  // Arrange: prepare context for `const TimeNano step = 2;`.
  const TimeNano step = 2;
  const TimeNano start = 0;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::SteppableClock>(step, start));
  projectairsim::Scene scene;
  bool is_paused;

  // Check default initial state is unpaused
  // Act: run `is_paused = simclock->IsPaused();`.
  is_paused = simclock->IsPaused();
  // Assert: check result from `EXPECT_FALSE(is_paused);`.
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

TEST(SimClock, EngineDrivenAcceptsVariableHostSteps) {
  const TimeNano fixed_step = 3'000'000;
  const TimeNano start = 1'000;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::EngineDrivenClock>(fixed_step, start));
  auto* engine_clock =
      static_cast<projectairsim::EngineDrivenClock*>(simclock);
  projectairsim::Scene scene;

  engine_clock->SetNextStep(2'750'000);
  scene.SimClockStep();
  EXPECT_EQ(simclock->NowSimNanos(), start + 2'750'000);
  EXPECT_FALSE(engine_clock->HasPendingStep());

  engine_clock->SetNextStep(3'125'000);
  scene.SimClockStep();
  EXPECT_EQ(simclock->NowSimNanos(), start + 5'875'000);
}

TEST(SimClock, EngineDrivenRetainsFixedAccumulatorBehavior) {
  const TimeNano fixed_step = 3'000'000;
  auto simclock = projectairsim::SimClock::Get(
      std::make_shared<projectairsim::EngineDrivenClock>(fixed_step, 0));
  auto* engine_clock =
      static_cast<projectairsim::EngineDrivenClock*>(simclock);
  projectairsim::Scene scene;

  engine_clock->AccumulateStep(static_cast<TimeNano>(7'000'000));
  ASSERT_TRUE(engine_clock->HasPendingStep());
  scene.SimClockStep();
  scene.SimClockStep();
  EXPECT_EQ(simclock->NowSimNanos(), 6'000'000);
  EXPECT_FALSE(engine_clock->HasPendingStep());
}

// -----------------------------------------------------------------------------
// ScheduledExecutor tests

TEST(ScheduledExecutor, Construction) {
  // General description:
  // Verifies construction for ScheduledExecutor.
  // Arrange: prepare context for `projectairsim::ScheduledExecutor executor1;`.
  projectairsim::ScheduledExecutor executor1;

  std::function<bool()> callback = nullptr;
  // Act: run `projectairsim::ScheduledExecutor executor2(callback, 1);`.
  projectairsim::ScheduledExecutor executor2(callback, 1);
  // Assert: check result from `}`.
}

// TODO Split this combo test into individual tests
TEST(ScheduledExecutor, Combo) {
  // General description:
  // Verifies combo for ScheduledExecutor.
  // Arrange: prepare context for `TimeSec margin = 0.001f;  // 1 ms`.
  TimeSec margin = 0.001f;  // 1 ms

  std::function<bool()> callback =
      projectairsim::ScheduledExecutorTest::TestFuncTrue;
                                          // Act: run `projectairsim::ScheduledExecutor executor(callback,`.

  projectairsim::ScheduledExecutor executor(callback,
                                          10'000'000);  // 10 ms period

  // Assert: check result from `EXPECT_EQ(projectairsim::ScheduledExecutorTest::test_counter, 0);`.
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
