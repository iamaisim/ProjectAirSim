// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_SRC_DISPATCHER_HPP_
#define CORE_SIM_SRC_DISPATCHER_HPP_

#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#include "core_sim/logger.hpp"

namespace microsoft {
namespace projectairsim {

class Dispatcher {
 public:
  Dispatcher(const std::string& name, const Logger& logger)
      : name_(name), logger_(logger), quit_(false) {
    quit_ = false;
    thread_ = std::thread([this]() { run(); });
  }

  ~Dispatcher() {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      quit_ = true;
    }

    cv_.notify_all();

    if (thread_.joinable()) {
      thread_.join();
    }
  }

  Dispatcher(const Dispatcher&) = delete;

  Dispatcher& operator=(Dispatcher const&) = delete;

  void clear() {
    std::unique_lock<std::mutex> lock(mutex_);
    std::queue<std::function<void()>> empty_q;
    std::swap(queue_, empty_q);
  }

  void dispatch(const std::function<void()>& func) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (quit_ == true) {
        return;
      }

      queue_.push(func);
    }

    cv_.notify_all();
  }

  void dispatch(std::function<void()>&& func) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (quit_ == true) {
        return;
      }

      queue_.push(std::move(func));
    }

    cv_.notify_all();
  }

  void start() {}

  void stop() {}

 private:
  void run() {
    std::unique_lock<std::mutex> lock(mutex_);

    do {
      if (!quit_ && queue_.size() == 0) {
        // logger.LogTrace(name_, "Wait for dispactchable message");
        cv_.wait(lock, [this] { return (queue_.size() > 0 || quit_); });
      }

      // logger_.LogTrace(name_, "QueueSize '%d'.", queue_.size());

      if (!quit_ && queue_.size() > 0) {
        auto func = std::move(queue_.front());
        queue_.pop();

        lock.unlock();

        try {
          func();
        } catch (std::exception& exception) {
          logger_.LogError("Dispatcher",
                           "Exception occured in dispatcher thread '%s'.",
                           exception.what());
        } catch (...) {
          logger_.LogError("Dispatcher",
                           "Unknown exception occured in dispatcher thread.");
        }

        lock.lock();
      }
    } while (!quit_);
  }

  std::string name_;
  Logger logger_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;
  bool quit_;
  std::queue<std::function<void()>> queue_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_DISPATCHER_HPP_
