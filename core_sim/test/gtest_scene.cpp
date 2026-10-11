// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <chrono>
#include <atomic>
#include <future>
#include <iostream>
#include <memory>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/scene.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class Simulator {
 public:
  static Scene MakeScene() {
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);
    return Scene(logger, TopicManager(logger), "", ServiceManager(logger),
                 StateManager(logger), "");
  }

  static void StopScene(Scene& scene) { scene.Stop(); }

  static void LoadScene(Scene& scene, ConfigJson config_json) {
    scene.LoadWithJSON(config_json);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(Scene, Constructor) {
  // General description:
  // Verifies constructor for Scene.
  // Arrange: prepare context for
  // `EXPECT_FALSE(projectairsim::Simulator::MakeScene().IsLoaded());`. Act: run
  // `EXPECT_FALSE(projectairsim::Simulator::MakeScene().IsLoaded());`. Assert:
  // check result from
  // `EXPECT_FALSE(projectairsim::Simulator::MakeScene().IsLoaded());`.
  EXPECT_FALSE(projectairsim::Simulator::MakeScene().IsLoaded());
}

TEST(Scene, LoadScene) {
  // General description:
  // Verifies load scene for Scene.
  // Arrange: prepare context for `json json = "{ \"id\": \"a\"}"_json;`.
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  json = "{ \"id\": \"1abc\" }"_json;
  // Act: run `scene = projectairsim::Simulator::MakeScene();`.
  scene = projectairsim::Simulator::MakeScene();
  // Assert: check result from
  // `EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),`.
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
}

TEST(Scene, IsLoaded) {
  // General description:
  // Verifies is loaded for Scene.
  // Arrange: prepare context for `json json = "{ \"id\": \"a\"}"_json;`.
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  // Act: run `projectairsim::Simulator::LoadScene(scene, json);`.
  projectairsim::Simulator::LoadScene(scene, json);
  // Assert: check result from `EXPECT_TRUE(scene.IsLoaded());`.
  EXPECT_TRUE(scene.IsLoaded());

  json = "{ \"id\": \"1abc\" }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
  EXPECT_FALSE(scene.IsLoaded());
}

TEST(Scene, GetID) {
  // General description:
  // Verifies get id for Scene.
  // Arrange: prepare context for `json json = "{ \"id\": \"a\"}"_json;`.
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  // Act: run `projectairsim::Simulator::LoadScene(scene, json);`.
  projectairsim::Simulator::LoadScene(scene, json);
  // Assert: check result from `EXPECT_EQ(scene.GetID(), "a");`.
  EXPECT_EQ(scene.GetID(), "a");

  json = "{ \"id\": \"1abc\" }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
  EXPECT_TRUE(scene.GetID().empty());
}

TEST(Scene, GetActors) {
  // General description:
  // Verifies get actors for Scene.
  // Arrange: prepare context for `json json = "{ \"id\": \"a\"}"_json;`.
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  // Act: run `projectairsim::Simulator::LoadScene(scene, json);`.
  projectairsim::Simulator::LoadScene(scene, json);
  // Assert: check result from `EXPECT_EQ(scene.GetActors().size(), 0);`.
  EXPECT_EQ(scene.GetActors().size(), 0);

  json = R"({"id": "ProjectAirSim",
          "default-scene": "{\"id\": \"SceneBasicDrone\"}"})"_json;
  scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);
  EXPECT_EQ(scene.GetActors().size(), 0);

  json = "{ \"id\": \"a\", \"actors\": [ { \"name\": \"actor1\" } ] }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
  EXPECT_EQ(scene.GetActors().size(), 0);

  json =
      "{ \"id\": \"a\", \"actors\": [ "
      "{ \"name\": \"actor1\", \"type\": \"robo\" } ] }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
  EXPECT_EQ(scene.GetActors().size(), 0);
}

TEST(Scene, LoadExternalClock) {
  json json = R"({
    "id": "a",
    "clock": {
      "type": "external-clock",
      "step-ns": 3000000
    }
  })"_json;

  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  EXPECT_EQ(scene.GetClockSettings().type,
            projectairsim::ClockType::kExternalClock);
  EXPECT_EQ(scene.GetClockSettings().step, 3000000);
}

TEST(Scene, LoadSteppableClock) {
  json json = R"({
    "id": "a",
    "clock": {
      "type": "steppable",
      "step-ns": 2900000
    }
  })"_json;

  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  EXPECT_EQ(scene.GetClockSettings().type,
            projectairsim::ClockType::kSteppable);
  EXPECT_EQ(scene.GetClockSettings().step, 2900000);
}

TEST(Scene, LoadRealTimeClock) {
  json json = R"({
    "id": "a",
    "clock": {
      "type": "real-time",
      "step-ns": 2800000
    }
  })"_json;

  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  EXPECT_EQ(scene.GetClockSettings().type, projectairsim::ClockType::kRealTime);
  EXPECT_EQ(scene.GetClockSettings().step, 2800000);
}

TEST(Scene, LoadEngineDrivenClock) {
  json json = R"({
    "id": "a",
    "clock": {
      "type": "engine-driven",
      "step-ns": 2700000
    }
  })"_json;

  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  EXPECT_EQ(scene.GetClockSettings().type,
            projectairsim::ClockType::kEngineDriven);
  EXPECT_EQ(scene.GetClockSettings().step, 2700000);
}

TEST(Scene, UnrealEngineClockSettingsPreserveExistingBehaviorByDefault) {
  json config = R"({"id": "a"})"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, config);

  const auto& settings = scene.GetClockSettings();
  EXPECT_FALSE(settings.engine_substepping);
  EXPECT_FALSE(settings.engine_fixed_fps_enabled);
  EXPECT_FLOAT_EQ(settings.engine_fixed_fps, 30.0f);
}

TEST(Scene, LoadUnrealEngineClockSettings) {
  json config = R"({
    "id": "a",
    "clock": {
      "type": "engine-driven",
      "step-ns": 2500000,
      "engine-fixed-fps": 50.0,
      "engine-substepping": true
    }
  })"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, config);

  const auto& settings = scene.GetClockSettings();
  EXPECT_TRUE(settings.engine_substepping);
  EXPECT_TRUE(settings.engine_fixed_fps_enabled);
  EXPECT_FLOAT_EQ(settings.engine_fixed_fps, 50.0f);
  EXPECT_EQ(settings.step, 2500000);
}

TEST(Scene, RejectNonPositiveClockStepForEngineSubstepping) {
  json config = R"({
    "id": "a",
    "clock": {
      "type": "engine-driven",
      "step-ns": 0,
      "engine-substepping": true
    }
  })"_json;
  auto scene = projectairsim::Simulator::MakeScene();

  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, config),
               projectairsim::Error);
}

TEST(Scene, LoadEngineFixedFps) {
  json config = R"({
    "id": "a",
    "clock": {
      "type": "engine-driven",
      "engine-fixed-fps": 50.0
    }
  })"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, config);

  const auto& settings = scene.GetClockSettings();
  EXPECT_TRUE(settings.engine_fixed_fps_enabled);
  EXPECT_FLOAT_EQ(settings.engine_fixed_fps, 50.0f);
}

TEST(Scene, RejectInvalidEngineFixedFps) {
  json config = R"({
    "id": "a",
    "clock": {
      "type": "engine-driven",
      "engine-fixed-fps": 0.0
    }
  })"_json;
  auto scene = projectairsim::Simulator::MakeScene();

  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, config),
               projectairsim::Error);
}

// TODO Add tests for scene
// TEST(Scene, SetCallbackPhysicsSetWrenches) {}
// TEST(Scene, SetCallbackPhysicsStep) {}
// TEST(Scene, GetClockSettings) {}

// TODO How can we test scene ticks during async external clock step requests?

namespace {
class ShutdownDependencyController : public projectairsim::IController {
 public:
  ShutdownDependencyController(std::promise<void>* notify,
                               std::shared_future<void> release)
      : notify_(notify), release_(release) {}
  void BeginUpdate() override {}
  void EndUpdate() override {
    if (release_.valid()) release_.wait();
  }
  bool CancelLastTask() {
    if (notify_ != nullptr) {
      notify_->set_value();
      notify_ = nullptr;
    }
    return true;
  }
  void Reset() override {}
  void Update() override {}
  void SetKinematics(const projectairsim::Kinematics*) override {}
  int GetControlSignalIndex(const std::string&) override { return -1; }
  void GetControlSignalSnapshot(std::vector<float>& output) override { output.clear(); }
  std::vector<float> GetControlSignals(int) override { return {}; }
  std::vector<float> GetControlSignals(const std::string&) override { return {}; }
  const GimbalState& GetGimbalSignal(const std::string&) override {
    return gimbal_;
  }

 private:
  std::promise<void>* notify_;
  std::shared_future<void> release_;
  GimbalState gimbal_;
};
}  // namespace

TEST(Scene, CancelsAllControllersBeforeClosingPeerTransports) {
  auto scene = projectairsim::Simulator::MakeScene();
  json actors = json::array();
  for (const char* name : {"first", "second"}) {
    actors.push_back({{"type", "robot"}, {"name", name},
                     {"robot-config", {{"physics-type", "non-physics"},
                                       {"links", {{{"name", "Frame"}}}}}}});
  }
  json config = {{"id", "shutdown_peers"}, {"actors", actors}};
  projectairsim::Simulator::LoadScene(scene, config);
  auto& first = static_cast<projectairsim::Robot&>(scene.GetActors()[0].get());
  auto& second = static_cast<projectairsim::Robot&>(scene.GetActors()[1].get());
  std::promise<void> peer_cancelled;
  // Closing the first transport needs the second command callback to exit.
  first.SetController(std::make_unique<ShutdownDependencyController>(
      nullptr, peer_cancelled.get_future().share()));
  second.SetController(std::make_unique<ShutdownDependencyController>(
      &peer_cancelled, std::shared_future<void>()));
  auto stop = std::async(std::launch::async, [&]() {
    projectairsim::Simulator::StopScene(scene);
  });
  const auto status = stop.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(status, std::future_status::ready);
  // Release the dependency on failure so the test can finish and report it.
  if (status != std::future_status::ready) {
    static_cast<ShutdownDependencyController*>(second.GetController())->CancelLastTask();
  }
  stop.get();
}

TEST(Scene, StopsPhysicsBeforeEndingRobotUpdates) {
  auto scene = projectairsim::Simulator::MakeScene();
  json config = {
      {"id", "shutdown_physics"},
      {"actors", {{{"type", "robot"}, {"name", "first"},
                   {"robot-config", {{"physics-type", "non-physics"},
                                     {"links", {{{"name", "Frame"}}}}}}}}}};
  projectairsim::Simulator::LoadScene(scene, config);
  auto& robot = static_cast<projectairsim::Robot&>(scene.GetActors()[0].get());
  std::promise<void> physics_stopped;
  std::atomic<bool> notified{false};
  const auto release = [&]() {
    if (!notified.exchange(true)) physics_stopped.set_value();
  };
  scene.SetCallbackPhysicsStop(release);
  robot.SetController(std::make_unique<ShutdownDependencyController>(
      nullptr, physics_stopped.get_future().share()));
  auto stop = std::async(std::launch::async, [&]() {
    projectairsim::Simulator::StopScene(scene);
  });
  const auto status = stop.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(status, std::future_status::ready);
  if (status != std::future_status::ready) release();
  stop.get();
}
