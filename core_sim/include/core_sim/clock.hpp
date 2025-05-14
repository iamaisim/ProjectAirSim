// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_CLOCK_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_CLOCK_HPP_

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <system_error>
#include <thread>

using TimeNano = int64_t;
using TimeMicro = int64_t;
using TimeMilli = int64_t;
using TimeSec = float;
using TimeNanoLimits = std::numeric_limits<TimeNano>;

namespace microsoft {
namespace projectairsim {

class Scene;

// -----------------------------------------------------------------------------
// ClockBase

class ClockBase {
 public:
  static constexpr TimeNano kDefaultStepNanos = 3'000'000;

  // Get current real time from system clock
  static TimeNano GetRealTimeSinceEpochNanos();

  ClockBase();
  virtual ~ClockBase();

  // Get current sim clock time with specified units
  virtual TimeNano NowSimNanos() const = 0;
  TimeSec NowSimSec() const;
  TimeMilli NowSimMillis() const;
  TimeMicro NowSimMicros() const;

  // Time unit conversion helpers
  TimeNano SecToNanos(TimeSec dt_sec);
  TimeSec NanosToSec(TimeNano dt_nanos);

  // Externally set next pending sim clock time step
  virtual void SetNextStep(TimeNano amount_nanos);
  virtual void SetNextSimTime(TimeNano time_nanos);
  virtual void SetExternallySteppedOnly(bool external_only);
  virtual bool IsExternallySteppedOnly() const;

  // Manual simtime controls for steppable clock (no-ops otherwise)
  virtual void SimPause(bool do_pause);
  virtual bool IsPaused() const;
  virtual TimeNano GetSimTimeToPauseNanos() const;
  virtual void ContinueForSimTime(TimeNano delta_time);
  virtual void ContinueUntilSimTime(TimeNano target_time);
  virtual void ContinueForSingleStep();
  virtual void ContinueForNBaseSteps(int n_base_steps);

 protected:
  friend Scene;

  // Move sim clock time by the next step (only to be called by Scene tick)
  virtual void Step();
};

// -----------------------------------------------------------------------------
// RealTimeClock is a clock that follows the real time of the system clock

class RealTimeClock : public ClockBase {
 public:
  explicit RealTimeClock();
  ~RealTimeClock() override;

  TimeNano NowSimNanos() const override;
};

// -----------------------------------------------------------------------------
// SteppableClock

class SteppableClock : public ClockBase {
 public:
  static constexpr TimeNano kMinStepNanos = 0;

  SteppableClock(TimeNano base_step_nanos = ClockBase::kDefaultStepNanos,
                 TimeNano start = 0);
  ~SteppableClock() override;

  // For steppable clock, sim clock time is stored as current_time_
  TimeNano NowSimNanos() const override;

  // External calls to overwrite next_step_nanos_ for the next Step() call
  void SetNextStep(TimeNano amount_nanos) override;
  void SetNextSimTime(TimeNano time_nanos) override;
  void SetExternallySteppedOnly(bool external_only) override;
  bool IsExternallySteppedOnly() const override;

  // Manual simtime controls
  void SimPause(bool do_pause) override;
  bool IsPaused() const override;
  TimeNano GetSimTimeToPauseNanos() const override;
  void ContinueForSimTime(TimeNano delta_time) override;
  void ContinueUntilSimTime(TimeNano target_time) override;
  void ContinueForSingleStep() override;
  void ContinueForNBaseSteps(int n_base_steps) override;

 protected:
  void Step() override;

  // Protect from concurrent external calls to set steps
  std::atomic<TimeNano> current_sim_time_;
  std::atomic<TimeNano> sim_time_to_pause_ = TimeNanoLimits::max();
  std::atomic<TimeNano> base_step_nanos_;
  std::atomic<TimeNano> next_step_nanos_;
  std::atomic<bool> is_externally_stepped_only_ = false;
};

// -----------------------------------------------------------------------------
// SimClock

class SimClock {  // TODO Avoid factory singleton pattern?
 public:
  // Output of this function should not be stored as pointer might change
  static ClockBase* Get(std::shared_ptr<ClockBase> specified_clock = nullptr);

  // Don't allow multiple instances of this class
  SimClock(SimClock const&) = delete;
  void operator=(SimClock const&) = delete;

 private:
  // Disallow instance creation
  SimClock();
};

// -----------------------------------------------------------------------------
// Clock Settings

enum class ClockType { kSteppable = 0, kRealTime = 1 };

struct ClockSettings {
  ClockType type = ClockType::kSteppable;
  TimeNano step = ClockBase::kDefaultStepNanos;
  TimeNano scene_tick_period = ClockBase::kDefaultStepNanos;
  bool pause_on_start = false;
};

// -----------------------------------------------------------------------------
// ScheduledExecutor

class ScheduledExecutor {
 public:
  ScheduledExecutor();
  ScheduledExecutor(const std::function<bool()>& callback,
                    TimeNano period_nanos);
  ~ScheduledExecutor();

  void Initialize(const std::function<bool()>& callback, TimeNano period_nanos);

  void Start();
  void Stop();

  void Pause(bool is_paused);
  bool IsPaused() const;
  bool IsRunning() const;
  void ContinueForTime(TimeSec seconds);

  TimeSec GetSleepTimeAve() const;

  void Lock();
  void Unlock();

 private:
  static TimeNano RealNanos();
  static void SleepFor(TimeNano delay_nanos);

  void InitializePauseState();
  void ExecutorLoop();

  TimeNano period_nanos_;
  std::thread th_;
  std::function<bool()> callback_;
  bool is_first_period_;
  std::atomic_bool started_;
  std::atomic_bool paused_;
  std::atomic<TimeNano> pause_period_;
  std::atomic<TimeNano> pause_period_start_;
  TimeSec sleep_time_ave_;
  std::mutex mutex_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_CLOCK_HPP_
