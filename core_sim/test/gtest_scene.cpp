// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <iostream>
#include <memory>

#include "core_sim/config_json.hpp"
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
