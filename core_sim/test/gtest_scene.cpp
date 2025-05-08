// Copyright (C) Microsoft Corporation. All rights reserved.

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
                       const std::string& message) { std::cerr << message << std::endl; };
    Logger logger(callback);
    logger.SetLogLevel(LogLevel::kVerbose);
    return Scene(logger, TopicManager(logger), "", ServiceManager(logger),
                 StateManager(logger));
  }

  static void LoadScene(Scene& scene, ConfigJson config_json) {
    scene.LoadWithJSON(config_json);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(Scene, Constructor) {
  EXPECT_FALSE(projectairsim::Simulator::MakeScene().IsLoaded());
}

TEST(Scene, LoadScene) {
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  json = "{ \"id\": \"1abc\" }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
}

TEST(Scene, IsLoaded) {
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);
  EXPECT_TRUE(scene.IsLoaded());

  json = "{ \"id\": \"1abc\" }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
  EXPECT_FALSE(scene.IsLoaded());
}

TEST(Scene, GetID) {
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);
  EXPECT_EQ(scene.GetID(), "a");

  json = "{ \"id\": \"1abc\" }"_json;
  scene = projectairsim::Simulator::MakeScene();
  EXPECT_THROW(projectairsim::Simulator::LoadScene(scene, json),
               projectairsim::Error);
  EXPECT_TRUE(scene.GetID().empty());
}

TEST(Scene, GetActors) {
  json json = "{ \"id\": \"a\"}"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);
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

TEST(Scene, GetEnvActorsConfig) {
  json json =
      R"({ 
            "id": "a", 
            "environment-actors": [
                { 
                  "name": "envcar1", 
                  "type": "env_car",
                  "origin": {
                    "xyz": "259.0 308.0 -50.0",
                    "rpy-deg": "0 0 0"
                  },
                  "env-actor-config": {
                    "links": [
                      {
                        "name": "Frame",
                        "collision": {
                          "enabled": false
                        },
                        "visual": {
                          "geometry": {
                            "type": "unreal_mesh",
                            "name": "/ProjectAirSim/Shelby/Shelby_chassis"
                          }
                        }
                      }
                    ]
                  }
                }
              ] 
          })"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  EXPECT_EQ(scene.GetEnvActors().size(), 1);
}

TEST(Scene, GetEnvActorsConfigArray) {
  json json =
      R"({ 
            "id": "a", 
            "environment-actors": [
                { 
                  "name": "envcar1", 
                  "type": "env_car",
                  "origin": {
                    "xyz": "259.0 308.0 -50.0",
                    "rpy-deg": "0 0 0"
                  },
                  "env-actor-config": [
                    {
                      "links": [
                        {
                          "name": "Frame",
                          "collision": {
                            "enabled": false
                          },
                          "visual": {
                            "geometry": {
                              "type": "unreal_mesh",
                              "name": "/ProjectAirSim/Shelby/Shelby_chassis"
                            }
                          }
                        }
                      ]
                    },
                    {
                      "links": [
                        {
                          "name": "Frame",
                          "visual": {
                            "geometry": {
                              "type": "unreal_mesh",
                              "name": "/ProjectAirSim/Shelby/Shelby_chassis_updated"
                            }
                          }
                        }
                      ]
                    }
                  ]
                }
              ] 
          })"_json;
  auto scene = projectairsim::Simulator::MakeScene();
  projectairsim::Simulator::LoadScene(scene, json);

  EXPECT_EQ(scene.GetEnvActors().size(), 1);
}

// TODO Add tests for scene
// TEST(Scene, SetCallbackPhysicsSetWrenches) {}
// TEST(Scene, SetCallbackPhysicsStep) {}
// TEST(Scene, GetClockSettings) {}

// TODO How can we test scene ticks during async external clock step requests?
