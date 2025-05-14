// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// Tests for Env Actor class

#include <memory>

#include "core_sim/actor/env_actor.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static EnvActor MakeEnvActor(const std::string& id) {
    Transform origin = {{1, 3, 4}, {1, 0, 0, 0}};
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);
    return EnvActor(id, origin, logger, TopicManager(logger), "",
                    ServiceManager(logger), StateManager(logger));
  }

  static void LoadEnvActor(EnvActor& env_actor, ConfigJson config_json) {
    env_actor.Load(config_json);
  }

  static std::shared_ptr<Trajectory> GetTrajectoryPtr(EnvActor& env_actor) {
    return env_actor.GetTrajectoryPtr();
  }

  static json GetScript(std::string is_looping) {
    std::string script =
        R"({
        "script": {
            "loop":)" +
        is_looping + R"(,
            "trajectory": {
            "name": "test",
            "time_sec": [1, 3, 6, 9, 12],
            "pose_x": [3, 5, 8, 14, 20],
            "pose_y": [0, 7, 10, 15, 20],
            "pose_z": [-15, -14, -10, -7, -4],
            "pose_roll": [0, 0, 0, 0, 0],
            "pose_pitch": [0, 0, 0, 0, 0],
            "pose_yaw": [0, 0, 0, 0, 0],
            "velocity_linear_x": [1, 2, 3, 4, 5],
            "velocity_linear_y": [10, 11, 12, 13, 16],
            "velocity_linear_z": [3, 5, 6, 8, 9]
            }
            }
        })";

    json json;
    std::stringstream(script) >> json;

    return json;
  }
};

namespace projectairsim = microsoft::projectairsim;

TEST(EnvActor, Constructor) {
  EXPECT_FALSE(projectairsim::Scene::MakeEnvActor("abc").IsLoaded());
}

TEST(EnvActor, LoadEnvActor) {
  json json = "{ }"_json;
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  EXPECT_EQ(projectairsim::Scene::GetTrajectoryPtr(env_actor), nullptr);
}

TEST(EnvActor, IsLoaded) {
  json json = "{ }"_json;
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  EXPECT_TRUE(env_actor.IsLoaded());
}

TEST(EnvActor, GetID) {
  json json = "{ }"_json;
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  EXPECT_EQ(env_actor.GetID(), "a");
}

TEST(EnvActor, GetType) {
  json json = "{ }"_json;
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  EXPECT_EQ(env_actor.GetType(), projectairsim::ActorType::kEnvActor);
}

TEST(EnvActor, GetLinks) {
  json json =
      R"({
            "links":[
            {
                "name":"link1",
                "visual":{
                "geometry":{
                    "type":"unreal_mesh",
                    "name":"Link1"
                }
                }
            },
            {
                "name":"link2",
                "visual":{
                "geometry":{
                    "type":"unreal_mesh",
                    "name":"Link2"
                }
                }
            }
            ]
            })"_json;

  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  EXPECT_EQ(env_actor.GetLinks().size(), 2);
}

TEST(EnvActor, SetTrajectory) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  EXPECT_NE(projectairsim::Scene::GetTrajectoryPtr(env_actor), nullptr);
}

TEST(EnvActor, GetKinematics) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 3;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position, Vector3(5, 7, -14));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear, Vector3(2, 11, 5));
}

TEST(EnvActor, GetKinematicsBetweenWaypoints) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 7;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position,
            Vector3(10, 11.666667, -9));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear,
            Vector3(3.3333333, 12.333333, 6.6666665));
}

TEST(EnvActor, GetKinematicsBeforeStart) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 0.5;
  env_actor.UpdateKinematics(curr_time);
  auto initial_point = Vector3(1, 3, 4);
  EXPECT_EQ(env_actor.GetKinematics().pose.position, initial_point);
}

TEST(EnvActor, GetKinematicsAtFirstWayPoint) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 1;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position, Vector3(3, 0, -15));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear, Vector3(1, 10, 3));
}

TEST(EnvActor, GetKinematicsAtLastWayPoint) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 12;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position, Vector3(20, 20, -4));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear, Vector3(5, 16, 9));
}

TEST(EnvActor, GetKinematicsAfterLastWayPointWithLoop) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 13;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position, Vector3(1, 3, 4));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear, Vector3(0, 0, 0));
  curr_time = 16;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position,
            Vector3(7, 9, -11.3333333));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear,
            Vector3(2.66666667, 11.66666667, 5.66666667));
}

TEST(EnvActor, GetKinematicsAfterLastWayPointWithoutLoop) {
  auto json = projectairsim::Scene::GetScript("false");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  TimeSec curr_time = 12;
  env_actor.UpdateKinematics(curr_time);
  curr_time = 17;
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position, Vector3(20, 20, -4));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear, Vector3(5, 16, 9));
}

TEST(Trajectory, GetKinematicsAtFirstWaypointWithOffset) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  auto traj_ptr = projectairsim::Scene::GetTrajectoryPtr(env_actor);
  TimeSec currtime = 3;
  env_actor.SetTrajectory(traj_ptr, true, 2, -1);
  env_actor.UpdateKinematics(currtime);
  EXPECT_EQ(env_actor.GetKinematics().pose.position,
            projectairsim::Vector3(2, 0, -15));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear,
            projectairsim::Vector3(1, 10, 3));
}

TEST(Trajectory, GetKinematicsAfterLastWayPointWithLoopAndOffset) {
  auto json = projectairsim::Scene::GetScript("true");
  auto env_actor = projectairsim::Scene::MakeEnvActor("a");
  projectairsim::Scene::LoadEnvActor(env_actor, json);
  auto traj_ptr = projectairsim::Scene::GetTrajectoryPtr(env_actor);
  env_actor.SetTrajectory(traj_ptr, true, 1, 1, 1, 1);
  TimeSec curr_time = 16;
  env_actor.UpdateKinematics(curr_time);  // advances num_loops by 1
  env_actor.UpdateKinematics(curr_time);
  EXPECT_EQ(env_actor.GetKinematics().pose.position,
            projectairsim::Vector3(7, 9, -11.66666667));
  EXPECT_EQ(env_actor.GetKinematics().twist.linear,
            projectairsim::Vector3(2.3333333, 11.3333333, 5.3333333));
}

}  // namespace projectairsim
}  // namespace microsoft