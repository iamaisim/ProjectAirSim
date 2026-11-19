// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "core_sim/clock.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// ClockBase

ClockBase::ClockBase() {}

ClockBase::~ClockBase() {}

TimeNano ClockBase::GetRealTimeSinceEpochNanos() {
  // Use steady_clock to avoid discontinuities caused by wall-clock changes.
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

TimeNano ClockBase::SecToNanos(TimeSec dt_sec) {
  // Convert seconds to integer nanoseconds used by the simulation clocks.
  return static_cast<TimeNano>(dt_sec * 1.0E9);
}

TimeSec ClockBase::NanosToSec(TimeNano dt_nanos) {
  // Convert nanoseconds back to seconds for APIs using floating-point time.
  return static_cast<TimeSec>(dt_nanos / 1.0E9);
}

TimeSec ClockBase::NowSimSec() const {
  return static_cast<TimeSec>(NowSimNanos() / 1.0E9);
}

TimeMilli ClockBase::NowSimMillis() const {
  return static_cast<TimeMilli>(NowSimNanos() / 1000000);
}

TimeMicro ClockBase::NowSimMicros() const {
  return static_cast<TimeMicro>(NowSimNanos() / 1000);
}

void ClockBase::SetNextStep(TimeNano amount_nanos) {}
void ClockBase::SetNextSimTime(TimeNano time_nanos) {}
void ClockBase::SetExternallySteppedOnly(bool external_only) {}
bool ClockBase::IsExternallySteppedOnly() const { return false; }

void ClockBase::SimPause(bool do_pause) {}
bool ClockBase::IsPaused() const { return false; }
TimeNano ClockBase::GetSimTimeToPauseNanos() const {
  return TimeNanoLimits::max();
}

void ClockBase::ContinueForSimTime(TimeNano delta_time) {}
void ClockBase::ContinueUntilSimTime(TimeNano target_time) {}
void ClockBase::ContinueForSingleStep() {}
void ClockBase::ContinueForNBaseSteps(int n_base_steps) {}

void ClockBase::Step() {}

// -----------------------------------------------------------------------------
// RealTimeClock is a clock that follows the real time of the system clock

RealTimeClock::RealTimeClock() {}
RealTimeClock::~RealTimeClock() {}

TimeNano RealTimeClock::NowSimNanos() const {
  return static_cast<TimeNano>(GetRealTimeSinceEpochNanos());
}

// -----------------------------------------------------------------------------
// SteppableClock

SteppableClock::SteppableClock(TimeNano base_step_nanos, TimeNano start)
    : current_sim_time_(start) {
  base_step_nanos_ = std::max(base_step_nanos, kMinStepNanos);
  next_step_nanos_ = base_step_nanos_.load();
}

SteppableClock::~SteppableClock() {}

TimeNano SteppableClock::NowSimNanos() const { return current_sim_time_; }

void SteppableClock::SetNextStep(TimeNano amount_nanos) {
  next_step_nanos_ = std::max(amount_nanos, kMinStepNanos);
}

void SteppableClock::SetNextSimTime(TimeNano time_nanos) {
  // Translate an absolute target time into the delta for the next Step().
  auto delta_time = static_cast<TimeNano>(time_nanos - current_sim_time_);
  next_step_nanos_ = std::max(delta_time, kMinStepNanos);
}

void SteppableClock::SetExternallySteppedOnly(bool external_only) {
  is_externally_stepped_only_ = external_only;
  // Set next_step to be ready for the next Step() call
  next_step_nanos_ = external_only ? 0 : base_step_nanos_.load();
}

bool SteppableClock::IsExternallySteppedOnly() const {
  return is_externally_stepped_only_;
}

void SteppableClock::SimPause(bool do_pause) {
  if (do_pause) {
    sim_time_to_pause_ = current_sim_time_.load();  // pause
  } else {
    sim_time_to_pause_ = TimeNanoLimits::max();  // resume
  }
}

bool SteppableClock::IsPaused() const {
  bool is_paused = (current_sim_time_ >= sim_time_to_pause_);
  return is_paused;
}

TimeNano SteppableClock::GetSimTimeToPauseNanos() const {
  return sim_time_to_pause_;
}

void SteppableClock::ContinueForSimTime(TimeNano delta_time) {
  // If delta_time is negative, just pause immediately
  sim_time_to_pause_ =
      current_sim_time_ + std::max(delta_time, static_cast<TimeNano>(0));
}

void SteppableClock::ContinueUntilSimTime(TimeNano target_time) {
  // If target_time is in the past, just pause immediately
  sim_time_to_pause_ = std::max(target_time, current_sim_time_.load());
}

void SteppableClock::ContinueForSingleStep() {
  TimeNano next_single_step = next_step_nanos_.load();
  if (is_externally_stepped_only_ && next_single_step == 0) {
    // If being externally stepped and the next step value hasn't been
    // set (next_single_step = 0), use base_step to advance the time to pause
    next_single_step = base_step_nanos_.load();
  }
  sim_time_to_pause_ = current_sim_time_ + next_single_step;
}

void SteppableClock::ContinueForNBaseSteps(int n_base_steps) {
  sim_time_to_pause_ =
      current_sim_time_ + std::max(n_base_steps, 0) * base_step_nanos_;
}

void SteppableClock::Step() {
  // Advance only while the clock is not paused by sim_time_to_pause_.
  if (current_sim_time_ < sim_time_to_pause_) {
    // Add step (either base or pending step), then reset step amount to base
    current_sim_time_ += next_step_nanos_;
    // If being externally stepped, reset next_step = 0 until the next external
    // API call to set it. Otherwise, reset next_step to internal base_step
    next_step_nanos_ =
        is_externally_stepped_only_ ? 0 : base_step_nanos_.load();
  }
}

// -----------------------------------------------------------------------------
// EngineDrivenClock

EngineDrivenClock::EngineDrivenClock(TimeNano fixed_step_nanos,
                                         TimeNano start)
    : current_sim_time_(start), accumulated_nanos_(0), next_step_nanos_(0) {
  // Fixed-step size used to convert variable frame deltas into deterministic
  // simulation steps.
  fixed_step_nanos_ = std::max(fixed_step_nanos, kMinStepNanos);
}

EngineDrivenClock::~EngineDrivenClock() {}

TimeNano EngineDrivenClock::NowSimNanos() const {
  return current_sim_time_;
}

void EngineDrivenClock::AccumulateStep(TimeNano delta_nanos) {
  // The external host provides elapsed real time per frame; accumulate it and
  // consume later in fixed-size steps during Step().
  if (delta_nanos <= 0) {
    return;
  }
  accumulated_nanos_.fetch_add(delta_nanos);
}

void EngineDrivenClock::AccumulateStep(TimeSec delta_seconds) {
  AccumulateStep(SecToNanos(delta_seconds));
}

bool EngineDrivenClock::HasPendingStep() const {
  // A host-provided physics step takes priority; otherwise a full fixed step
  // must have accumulated from host frame time.
  return next_step_nanos_.load() > 0 ||
         accumulated_nanos_.load() >= fixed_step_nanos_.load();
}

void EngineDrivenClock::SetNextStep(TimeNano amount_nanos) {
  next_step_nanos_ =
      amount_nanos > 0 ? std::max(amount_nanos, kMinStepNanos) : 0;
}

void EngineDrivenClock::SetFixedStep(TimeNano fixed_step_nanos) {
  // Allow runtime tuning of physics/update frequency in nanoseconds.
  fixed_step_nanos_ = std::max(fixed_step_nanos, kMinStepNanos);
}

void EngineDrivenClock::Step() {
  // Consume exactly one host-provided step, if present. Physics engines can
  // vary their final substep to cover the outer frame exactly.
  const TimeNano next_step_nanos = next_step_nanos_.exchange(0);
  if (next_step_nanos > 0) {
    current_sim_time_.fetch_add(next_step_nanos);
    return;
  }

  // Otherwise consume one fixed step so callers can iterate over accumulated
  // render-frame time using the legacy engine-driven behavior.
  if (!HasPendingStep()) {
    return;
  }

  const TimeNano fixed_step_nanos = fixed_step_nanos_.load();
  accumulated_nanos_.fetch_sub(fixed_step_nanos);
  current_sim_time_.fetch_add(fixed_step_nanos);
}

// -----------------------------------------------------------------------------
// SimClock

SimClock::SimClock() {}

// Output of this function should not be stored as pointer might change
ClockBase* SimClock::Get(std::shared_ptr<ClockBase> specified_clock) {
  static std::shared_ptr<ClockBase> clock;

  // If called with a specified clock, clear any existing clock and set to the
  // specified clock
  if (specified_clock != nullptr) {
    clock.reset();
    clock = specified_clock;
  }

  // If clock hasn't been set and nothing was specified, default to setting a
  // steppable clock
  if (clock == nullptr) {
    clock = std::make_shared<SteppableClock>();
  }

  return clock.get();  // .get() returns raw ptr from shared_ptr
}

// -----------------------------------------------------------------------------
// ScheduledExecutor

ScheduledExecutor::ScheduledExecutor() {}

ScheduledExecutor::ScheduledExecutor(const std::function<bool()>& callback,
                                     TimeNano period_nanos) {
  Initialize(callback, period_nanos);
}

ScheduledExecutor::~ScheduledExecutor() { Stop(); }

void ScheduledExecutor::Initialize(const std::function<bool()>& callback,
                                   TimeNano period_nanos) {
  callback_ = callback;
  period_nanos_ = period_nanos;
  started_ = false;
}

void ScheduledExecutor::Start() {
  started_ = true;
  is_first_period_ = true;

  InitializePauseState();

  sleep_time_ave_ = 0.0f;

  if (th_.joinable()) {
    // Defensive: detach any stale thread handle before creating a new worker.
    th_.detach();
  }
  // Dedicated worker thread that periodically executes callback_.
  th_ = std::thread(&ScheduledExecutor::ExecutorLoop, this);
}

void ScheduledExecutor::Stop() {
  started_ = false;
  InitializePauseState();

  try {
    if (th_.joinable()) {
      th_.join();
    }
  } catch (const std::system_error& /* e */) {
    // TODO Handle error?
    throw;
  } catch (...) {
    // TODO Handle error?
    throw;
  }
}

void ScheduledExecutor::Pause(bool is_paused) { paused_ = is_paused; }

bool ScheduledExecutor::IsPaused() const { return paused_; }

bool ScheduledExecutor::IsRunning() const { return started_ && !paused_; }

void ScheduledExecutor::ContinueForTime(TimeSec seconds) {
  // Unpause temporarily and auto-pause after the requested real-time duration.
  pause_period_start_ = RealNanos();
  pause_period_ = static_cast<TimeNano>(1E9 * seconds);
  paused_ = false;
}

TimeSec ScheduledExecutor::GetSleepTimeAve() const {
  // TODO: make this function thread safe by using atomic types
  // right now this is not implemented for performance and that
  // return of this function is purely informational/debugging purposes
  return sleep_time_ave_;
}

void ScheduledExecutor::Lock() { mutex_.lock(); }

void ScheduledExecutor::Unlock() { mutex_.unlock(); }

TimeNano ScheduledExecutor::RealNanos() {
  return std::chrono::steady_clock::now().time_since_epoch().count();
}

void ScheduledExecutor::SleepFor(TimeNano delay_nanos) {
  // This is spin loop implementation which may be suitable for
  // sub-millisecond resolution.
  // TODO: investigate below alternatives
  // On Windows we can use multimedia timers however this requires including
  // entire Win32 header. On Linux we can use nanosleep however below 2ms
  // delays in real-time scheduler settings this probbaly does spin loop
  // anyway.
  auto start = RealNanos();
  const TimeNano long_sleep_thresh = 1'000'000;
  if (delay_nanos >= long_sleep_thresh) {  // put thread to sleep
    std::this_thread::sleep_for(
        std::chrono::nanoseconds(delay_nanos - long_sleep_thresh));
  }

  // Spin for last part for higher accuracy
  while ((RealNanos() - start) < delay_nanos) {
    std::this_thread::yield();
  }
}

void ScheduledExecutor::InitializePauseState() {
  paused_ = false;
  pause_period_start_ = 0;
  pause_period_ = 0;
}

void ScheduledExecutor::ExecutorLoop() {
  while (started_) {
    // Track loop start to maintain a consistent execution period.
    TimeNano period_start = RealNanos();

    if (pause_period_start_ > 0) {
      if (RealNanos() - pause_period_start_ >= pause_period_) {
        if (!IsPaused()) {
          Pause(true);
        }

        pause_period_start_ = 0;
      }
    }

    // is this first loop?
    if (!is_first_period_) {
      if (!paused_) {
        // when we are doing work, don't let other thread to cause contention
        std::lock_guard<std::mutex> locker(mutex_);
        bool result = callback_();
        if (!result) {
          started_ = false;
        }
      }
    } else {
      is_first_period_ = false;
    }

    TimeNano elapsed_period = RealNanos() - period_start;
    // prevent underflow: https://github.com/Microsoft/AirSim/issues/617
    TimeNano delay_nanos =
        period_nanos_ > elapsed_period ? period_nanos_ - elapsed_period : 0;
    // moving average of how much we are sleeping
    sleep_time_ave_ = 0.25f * sleep_time_ave_ + 0.75f * delay_nanos / 1.0E9;

    if (delay_nanos > 0 && started_) {
      // Sleep the remaining budget so callback cadence matches period_nanos_.
      SleepFor(delay_nanos);
    }
  }
}

}  // namespace projectairsim
}  // namespace microsoft
