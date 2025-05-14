// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_COMMON_FUNCTION_CALLER_HPP_
#define MULTIROTOR_API_INCLUDE_COMMON_FUNCTION_CALLER_HPP_

#include "canceltoken.hpp"
#include "core_sim/clock.hpp"

namespace microsoft {
namespace projectairsim {
namespace vehicle_apis {

class FunctionCaller {
 public:
  // TODO Consider if it would be better not to mix nanos and sec time units.
  FunctionCaller(TimeSec function_call_interval_sec,
                 TimeSec command_timeout_sec, CancelToken& cancel_token,
                 TimeNano command_start_time_nanos)
      : function_call_interval_sec_(function_call_interval_sec),
        command_timeout_sec_(command_timeout_sec),
        cancel_token_(cancel_token),
        command_start_(command_start_time_nanos),
        last_interval_end_(command_start_time_nanos),
        is_complete_(false) {}

  bool WaitForInterval() {
    // Sleeps for the time needed to get current running time up to the
    // requested sleep_duration_. So this can be used to "throttle" any loop to
    // check something every sleep_duration_ seconds.

    if (IsComplete()) {
      throw std::domain_error(
          "Process was already complete. This instance of Waiter shouldn't be "
          "reused!");
    }

    // measure time spent since last iteration
    // TODO: Should we expose ElapsedSince equivalent on the Clock?
    TimeNano cur_wait_start = Clock()->NowSimNanos();

    // Calculate time elapsed during the flight controller function call
    TimeSec function_call_run_time =
        (cur_wait_start - last_interval_end_) / 1.0E9;
    TimeSec remaining_sec =
        function_call_interval_sec_ - function_call_run_time;
    if (remaining_sec > 0) {
      while (cancel_token_.IsCancelled() == false && IsTimeout() == false &&
             ((Clock()->NowSimNanos() - cur_wait_start) / 1.0E9) <
                 remaining_sec) {
        std::this_thread::yield();
      }
    }

    last_interval_end_ = Clock()->NowSimNanos();
    return (cancel_token_.IsCancelled() == false && IsTimeout() == false);
  }

  // call this mark process as complete
  void Complete() { is_complete_ = true; }

  bool IsComplete() const { return is_complete_; }

  bool IsTimeout() const {
    if (IsComplete()) {
      return false;
    } else {
      TimeNano now_nanos = Clock()->NowSimNanos();
      auto elapsed_sec = ((now_nanos - command_start_) / 1.0E9);
      return (elapsed_sec >= command_timeout_sec_);
    }
  }

 private:
  TimeSec function_call_interval_sec_, command_timeout_sec_;
  CancelToken& cancel_token_;

  TimeNano command_start_;
  TimeNano last_interval_end_;

  bool is_complete_;  // each waiter should maintain its own complete status

  static ClockBase* Clock() { return SimClock::Get(); }
};

}  // namespace vehicle_apis
}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_COMMON_FUNCTION_CALLER_HPP_
