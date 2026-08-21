// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "jsbsim_api.hpp"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static Robot MakeJSBSimRobot(const std::string& id) {
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};

    // Keep test logging quiet.
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);

    return Robot(id, origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger),
                 PROJECTAIRSIM_TEST_PLUGIN_DIR);
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }
};

class JSBSimApiTestPeer {
 public:
  static void SetPropertyAndHoldLock(
      JSBSimApi& api, const std::string& property, float value,
      std::atomic<bool>& property_written,
      std::atomic<bool>& competing_call_attempting,
      std::chrono::steady_clock::duration hold_duration) {
    // Take the property mutex.
    std::lock_guard<std::mutex> lock(api.jsbsim_property_mutex_);

    // Write while the lock is held.
    api.SetJSBSimPropertyUnlocked(property, value);

    // Signal that the value is ready.
    property_written.store(true, std::memory_order_release);

    // Wait until the other thread is trying to call the API.
    while (!competing_call_attempting.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    // Keep the lock held for the test interval.
    std::this_thread::sleep_for(hold_duration);
  }

  static float GetPropertyAndHoldLock(
      JSBSimApi& api, const std::string& property,
      std::atomic<bool>& property_read,
      std::atomic<bool>& competing_call_attempting,
      std::chrono::steady_clock::duration hold_duration) {
    // Take the property mutex.
    std::lock_guard<std::mutex> lock(api.jsbsim_property_mutex_);

    // Read while the lock is held.
    const float value = api.GetJSBSimPropertyUnlocked(property);

    // Signal that the value was read.
    property_read.store(true, std::memory_order_release);

    // Wait until the other thread is trying to call the API.
    while (!competing_call_attempting.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    // Keep the lock held for the test interval.
    std::this_thread::sleep_for(hold_duration);

    // Return the value read under the lock.
    return value;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

namespace {

// Use enough work to create overlap without making the tests slow.
constexpr int kThreadCount = 8;
constexpr int kIterations = 500;

// A valid C310 normalized throttle property.
constexpr char kThreadSafeProperty[] = "fcs/throttle-cmd-norm";

projectairsim::Robot MakeLoadedJSBSimRobot() {
  // Load a real JSBSim model.
  json config = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "links": [ { "name": "Frame" } ],
      "controller": {
        "type": "jsbsim-api",
        "airframe-setup": "fixed-wing"
      }
    })"_json;

  auto robot = projectairsim::Scene::MakeJSBSimRobot("jsbsim_api_test");
  projectairsim::Scene::LoadRobot(robot, config);
  return robot;
}

projectairsim::Robot MakeLoadedJSBSimRobotWithGroundMode(
    const std::string& ground_mode) {
  json config = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "links": [ { "name": "Frame" } ],
      "controller": {
        "type": "jsbsim-api",
        "airframe-setup": "fixed-wing"
      }
    })"_json;
  config["jsbsim-ground-mode"] = ground_mode;

  auto robot = projectairsim::Scene::MakeJSBSimRobot("jsbsim_ground_test");
  projectairsim::Scene::LoadRobot(robot, config);
  return robot;
}

void WaitForStart(std::atomic<int>& ready, std::atomic<bool>& start) {
  // Mark this worker as ready.
  ready.fetch_add(1, std::memory_order_acq_rel);

  // Wait for the shared start signal.
  while (!start.load(std::memory_order_acquire)) {
    std::this_thread::yield();
  }
}

void StartThreadsTogether(std::atomic<int>& ready, std::atomic<bool>& start,
                          int thread_count) {
  // Wait for all workers to reach the barrier.
  while (ready.load(std::memory_order_acquire) < thread_count) {
    std::this_thread::yield();
  }

  // Release the workers.
  start.store(true, std::memory_order_release);
}

}  // namespace

TEST(JSBSimApi, GroundModeDefaultsToConstant) {
  auto robot = MakeLoadedJSBSimRobot();

  EXPECT_EQ(robot.GetJSBSimGroundSettings().mode,
            projectairsim::JSBSimGroundMode::kConstant);
}

TEST(JSBSimApi, GroundModeLoadsConstant) {
  auto robot = MakeLoadedJSBSimRobotWithGroundMode("constant");

  EXPECT_EQ(robot.GetJSBSimGroundSettings().mode,
            projectairsim::JSBSimGroundMode::kConstant);
}

TEST(JSBSimApi, GroundModeLoadsTerrain) {
  auto robot = MakeLoadedJSBSimRobotWithGroundMode("terrain");

  EXPECT_EQ(robot.GetJSBSimGroundSettings().mode,
            projectairsim::JSBSimGroundMode::kTerrain);
}

TEST(JSBSimApi, GroundModeRejectsInvalidMode) {
  json config = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "jsbsim-ground-mode": "invalid",
      "links": [ { "name": "Frame" } ],
      "controller": {
        "type": "jsbsim-api",
        "airframe-setup": "fixed-wing"
      }
    })"_json;

  auto robot = projectairsim::Scene::MakeJSBSimRobot("jsbsim_ground_test");

  EXPECT_THROW(projectairsim::Scene::LoadRobot(robot, config),
               projectairsim::Error);
}

TEST(JSBSimApi, SetJSBSimPropertyIsThreadSafe) {
  auto robot = MakeLoadedJSBSimRobot();
  projectairsim::JSBSimApi api(robot, nullptr);

  // Shared state for writer threads.
  std::atomic<int> ready{0};
  std::atomic<bool> start{false};
  std::atomic<int> failed_writes{0};

  // Avoid reallocating while launching threads.
  std::vector<std::thread> threads;
  threads.reserve(kThreadCount);

  // Launch writers that update the same property.
  for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
    threads.emplace_back([&, thread_index]() {
      // Wait until all writers start together.
      WaitForStart(ready, start);

      // Write unique valid values.
      for (int iteration = 0; iteration < kIterations; ++iteration) {
        const float value =
            static_cast<float>(thread_index * kIterations + iteration) /
            static_cast<float>(kThreadCount * kIterations);

        // Count failed writes from worker threads.
        if (!api.SetJSBSimProperty(kThreadSafeProperty, value)) {
          failed_writes.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  // Start and join all writers.
  StartThreadsTogether(ready, start, kThreadCount);
  for (auto& thread : threads) {
    thread.join();
  }

  // No setter call should fail.
  EXPECT_EQ(failed_writes.load(std::memory_order_relaxed), 0);

  // Final writer is unknown, but the value must stay valid.
  const float final_value = api.GetJSBSimProperty(kThreadSafeProperty);
  EXPECT_GE(final_value, 0.0f);
  EXPECT_LT(final_value, 1.0f);
}

TEST(JSBSimApi, GetJSBSimPropertyIsThreadSafe) {
  // Use a real JSBSimApi.
  auto robot = MakeLoadedJSBSimRobot();
  projectairsim::JSBSimApi api(robot, nullptr);

  // Give readers an initial valid value.
  ASSERT_TRUE(api.SetJSBSimProperty(kThreadSafeProperty, 0.5f));

  // Shared state for readers and writer.
  std::atomic<int> ready{0};
  std::atomic<bool> start{false};
  std::atomic<int> unexpected_reads{0};

  // Reserve space for readers plus the writer.
  std::vector<std::thread> threads;
  threads.reserve(kThreadCount + 1);

  // Writer changes the property while readers run.
  threads.emplace_back([&]() {
    WaitForStart(ready, start);
    for (int iteration = 0; iteration < kIterations; ++iteration) {
      // Keep writes in the valid range.
      const float value =
          static_cast<float>(iteration) / static_cast<float>(kIterations);
      api.SetJSBSimProperty(kThreadSafeProperty, value);
    }
  });

  // Readers repeatedly read during writes.
  for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
    threads.emplace_back([&]() {
      WaitForStart(ready, start);
      for (int iteration = 0; iteration < kIterations; ++iteration) {
        const float value = api.GetJSBSimProperty(kThreadSafeProperty);

        // Record bad values outside worker assertions.
        if (value < 0.0f || value >= 1.0f || !std::isfinite(value)) {
          unexpected_reads.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  // Start and join all workers.
  StartThreadsTogether(ready, start, kThreadCount + 1);
  for (auto& thread : threads) {
    thread.join();
  }

  // No reader should see an invalid value.
  EXPECT_EQ(unexpected_reads.load(std::memory_order_relaxed), 0);
}

TEST(JSBSimApi, GetJSBSimPropertyWaitsWhileSetHoldsPropertyLock) {
  // Use a real JSBSimApi.
  auto robot = MakeLoadedJSBSimRobot();
  projectairsim::JSBSimApi api(robot, nullptr);

  // Value written by the lock holder.
  constexpr float kBlockedWriteValue = 0.25f;

  // Lock hold time.
  const auto hold_duration = std::chrono::seconds(10);

  // Thread coordination flags.
  std::atomic<bool> property_written{false};
  std::atomic<bool> getter_attempting{false};

  // Results checked after join().
  float returned_value = -1.0f;
  std::chrono::steady_clock::duration getter_elapsed =
      std::chrono::steady_clock::duration::zero();

  // Thread 1 writes and holds the lock.
  std::thread setter_thread([&]() {
    projectairsim::JSBSimApiTestPeer::SetPropertyAndHoldLock(
        api, kThreadSafeProperty, kBlockedWriteValue, property_written,
        getter_attempting, hold_duration);
  });

  // Thread 2 measures a blocked getter.
  std::thread getter_thread([&]() {
    while (!property_written.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    // Start timing before the getter tries to lock.
    const auto getter_start = std::chrono::steady_clock::now();
    getter_attempting.store(true, std::memory_order_release);

    // This should wait for thread 1.
    returned_value = api.GetJSBSimProperty(kThreadSafeProperty);

    // Save the blocked time.
    getter_elapsed = std::chrono::steady_clock::now() - getter_start;
  });

  // Join before checking shared results.
  setter_thread.join();
  getter_thread.join();

  // Getter should wait for the lock hold time.
  EXPECT_GE(getter_elapsed, hold_duration);

  // Getter should see the written value.
  EXPECT_FLOAT_EQ(returned_value, kBlockedWriteValue);
}

TEST(JSBSimApi, GetJSBSimPropertyWaitsWhileGetHoldsPropertyLock) {
  // Use a real JSBSimApi.
  auto robot = MakeLoadedJSBSimRobot();
  projectairsim::JSBSimApi api(robot, nullptr);

  // Seed a value before either getter starts.
  constexpr float kSeedValue = 0.5f;
  ASSERT_TRUE(api.SetJSBSimProperty(kThreadSafeProperty, kSeedValue));

  // Lock hold time.
  const auto hold_duration = std::chrono::seconds(10);

  // Thread coordination flags.
  std::atomic<bool> property_read{false};
  std::atomic<bool> getter_attempting{false};

  // Results checked after join().
  float lock_holder_value = -1.0f;
  float returned_value = -1.0f;
  std::chrono::steady_clock::duration getter_elapsed =
      std::chrono::steady_clock::duration::zero();

  // Thread 1 reads and holds the lock.
  std::thread first_getter_thread([&]() {
    lock_holder_value = projectairsim::JSBSimApiTestPeer::GetPropertyAndHoldLock(
        api, kThreadSafeProperty, property_read, getter_attempting,
        hold_duration);
  });

  // Thread 2 measures a blocked getter.
  std::thread second_getter_thread([&]() {
    while (!property_read.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    // Start timing before the getter tries to lock.
    const auto getter_start = std::chrono::steady_clock::now();
    getter_attempting.store(true, std::memory_order_release);

    // This should wait for thread 1.
    returned_value = api.GetJSBSimProperty(kThreadSafeProperty);

    // Save the blocked time.
    getter_elapsed = std::chrono::steady_clock::now() - getter_start;
  });

  // Join before checking shared results.
  first_getter_thread.join();
  second_getter_thread.join();

  // Second getter should wait for the lock hold time.
  EXPECT_GE(getter_elapsed, hold_duration);

  // Both getters should see the seed value.
  EXPECT_FLOAT_EQ(lock_holder_value, kSeedValue);
  EXPECT_FLOAT_EQ(returned_value, kSeedValue);
}

TEST(JSBSimApi, SetJSBSimPropertyWaitsWhileSetHoldsPropertyLock) {
  // Use a real JSBSimApi.
  auto robot = MakeLoadedJSBSimRobot();
  projectairsim::JSBSimApi api(robot, nullptr);

  // Values written by each setter.
  constexpr float kLockHolderWriteValue = 0.25f;
  constexpr float kSecondWriteValue = 0.75f;

  // Lock hold time.
  const auto hold_duration = std::chrono::seconds(10);

  // Thread coordination flags.
  std::atomic<bool> property_written{false};
  std::atomic<bool> setter_attempting{false};

  // Results checked after join().
  bool second_set_result = false;
  std::chrono::steady_clock::duration setter_elapsed =
      std::chrono::steady_clock::duration::zero();

  // Thread 1 writes and holds the lock.
  std::thread first_setter_thread([&]() {
    projectairsim::JSBSimApiTestPeer::SetPropertyAndHoldLock(
        api, kThreadSafeProperty, kLockHolderWriteValue, property_written,
        setter_attempting, hold_duration);
  });

  // Thread 2 measures a blocked setter.
  std::thread second_setter_thread([&]() {
    while (!property_written.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    // Start timing before the setter tries to lock.
    const auto setter_start = std::chrono::steady_clock::now();
    setter_attempting.store(true, std::memory_order_release);

    // This should wait for thread 1.
    second_set_result =
        api.SetJSBSimProperty(kThreadSafeProperty, kSecondWriteValue);

    // Save the blocked time.
    setter_elapsed = std::chrono::steady_clock::now() - setter_start;
  });

  // Join before checking shared results.
  first_setter_thread.join();
  second_setter_thread.join();

  // Second setter should wait for the lock hold time.
  EXPECT_GE(setter_elapsed, hold_duration);

  // Second setter should still succeed.
  EXPECT_TRUE(second_set_result);

  // Final value should come from the second setter.
  EXPECT_FLOAT_EQ(api.GetJSBSimProperty(kThreadSafeProperty),
                  kSecondWriteValue);
}
