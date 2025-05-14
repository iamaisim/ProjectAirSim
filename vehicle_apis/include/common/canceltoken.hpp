// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_COMMON_CANCELTOKEN_HPP_
#define MULTIROTOR_API_INCLUDE_COMMON_CANCELTOKEN_HPP_

#include <atomic>
#include <mutex>

#include "core_sim/clock.hpp"

namespace microsoft {
namespace projectairsim {

class CancelToken {
 public:
  CancelToken()
      : is_cancelled_(false), is_complete_(false), recursion_count_(0) {}

  void Reset() {
    is_cancelled_ = false;
    is_complete_ = false;
    recursion_count_ = 0;
  }

  bool IsCancelled() const { return is_cancelled_; }

  void Cancel() { is_cancelled_ = true; }

  void Complete(bool is_complete = true) { is_complete_ = is_complete; }

  bool IsComplete() const { return is_complete_; }

  int GetRecursionCount() { return recursion_count_; }

  bool Try_lock() {
    bool result = wait_mutex_.try_lock();
    if (result) {
      ++recursion_count_;
    }
    return result;
  }

  void Unlock() {
    wait_mutex_.unlock();
    if (recursion_count_ > 0) {
      --recursion_count_;
    }
  }

  void Lock() {
    wait_mutex_.lock();
    ++recursion_count_;
  }

 private:
  std::atomic<bool> is_cancelled_;
  std::atomic<bool> is_complete_;

  std::atomic<int> recursion_count_;
  std::recursive_mutex wait_mutex_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_COMMON_CANCELTOKEN_HPP_
