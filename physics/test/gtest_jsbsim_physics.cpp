// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// Tests for JSBSim physics.

#include "jsbsim_physics.hpp"

#include <string>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "core_sim/service_manager.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "FGFDMExec.h"
#include "models/FGAccelerations.h"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

#ifndef PROJECTAIRSIM_SOURCE_DIR
#define PROJECTAIRSIM_SOURCE_DIR ""
#endif

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static Robot MakeRobot(const std::string& id,
                         const std::string& working_simulation_path) {
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);
    return Robot(id, origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger),
                 working_simulation_path);
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

namespace {

std::string GetProjectAirSimPluginPath() {
  return std::string(PROJECTAIRSIM_SOURCE_DIR) +
         "/unreal/Blocks/Plugins/ProjectAirSim";
}

json MakeJSBSimRobotConfig(
    const std::string& jsbsim_model = "c310",
    const std::string& ground_mode = "constant") {
  auto config = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "jsbsim-ground-mode": "constant",
      "jsbsim-dt": 0.01,
      "links": [
        {
          "name": "Frame",
          "inertial": {
            "mass": 1.0,
            "inertia": {
              "type": "geometry",
              "geometry": {
                "box": {
                  "size": "0.180 0.110 0.040"
                }
              }
            }
          },
          "collision": {
            "restitution": 0.1,
            "friction": 0.5
          }
        }
      ]
    })"_json;
  config["jsbsim-model"] = jsbsim_model;
  config["jsbsim-ground-mode"] = ground_mode;
  return config;
}

projectairsim::Robot MakeJSBSimRobot(
    const std::string& id,
    const std::string& jsbsim_model = "c310") {
  auto robot =
      projectairsim::Scene::MakeRobot(id, GetProjectAirSimPluginPath());
  auto config = MakeJSBSimRobotConfig(jsbsim_model);
  projectairsim::Scene::LoadRobot(robot, config);
  return robot;
}

projectairsim::Kinematics MakeKinematics(
    const projectairsim::Vector3& position,
    const projectairsim::Quaternion& orientation,
    const projectairsim::Vector3& linear_velocity) {
  projectairsim::Kinematics kinematics;
  kinematics.pose.position = position;
  kinematics.pose.orientation = orientation;
  kinematics.twist.linear = linear_velocity;
  kinematics.twist.angular = projectairsim::Vector3::Zero();
  kinematics.accels.linear = projectairsim::Vector3::Zero();
  kinematics.accels.angular = projectairsim::Vector3::Zero();
  return kinematics;
}

constexpr TimeNano kTinyStepNanos = 1;
constexpr TimeNano kThreeMsNanos = 3000000;
constexpr TimeNano kTwentyMsNanos = 20000000;

}  // namespace

TEST(JSBSimPhysicsBody, StoresConfiguredDt) {
  json json = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "jsbsim-dt": 0.01,
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a",
                                               GetProjectAirSimPluginPath());
  projectairsim::Scene::LoadRobot(robot, json);

  projectairsim::JSBSimPhysicsBody body(robot);

  EXPECT_DOUBLE_EQ(body.GetJSBSimDtSec(), 0.01);
}

TEST(JSBSimPhysicsModel, AdvancesOnTickShorterThanConfiguredJSBSimDt) {
  auto robot = MakeJSBSimRobot("short_pas_tick");
  robot.SetStartLanded(false);
  robot.UpdateKinematics(
      MakeKinematics({0.0f, 0.0f, -10.0f},
                     projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f,
                                                                 0.0f),
                     {10.0f, 0.0f, 0.0f}));

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kThreeMsNanos, body);
  const double first_time = robot.GetJSBSimModel()->GetSimTime();
  const auto first_position = robot.GetKinematics().pose.position;
  model.StepPhysicsBody(kThreeMsNanos, body);
  const double second_time = robot.GetJSBSimModel()->GetSimTime();
  const auto second_position = robot.GetKinematics().pose.position;

  EXPECT_NEAR(first_time, 0.003, 1e-9);
  EXPECT_NEAR(second_time, 0.006, 1e-9);
  EXPECT_GT((second_position - first_position).norm(), 1e-5f);
}

TEST(JSBSimPhysicsModel, UsesConfiguredJSBSimDtAsMaximumSubstep) {
  auto robot = MakeJSBSimRobot("long_pas_tick");
  robot.SetStartLanded(false);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kTwentyMsNanos, body);

  EXPECT_NEAR(robot.GetJSBSimModel()->GetSimTime(), 0.02, 1e-9);
  EXPECT_DOUBLE_EQ(robot.GetJSBSimModel()->GetDeltaT(), 0.01);
}

TEST(JSBSimPhysicsModel, CalcNextKinematicsGrounded) {
  projectairsim::JSBSimPhysicsModel model;
  const projectairsim::Vector3 position = {1.0f, 2.0f, -3.0f};
  constexpr float kRoll = 0.3f;
  constexpr float kPitch = -0.2f;
  constexpr float kYaw = 1.1f;
  const auto orientation =
      projectairsim::TransformUtils::ToQuaternion(kRoll, kPitch, kYaw);

  const auto kin = model.CalcNextKinematicsGrounded(position, orientation);
  const auto rpy = projectairsim::TransformUtils::ToRPY(kin.pose.orientation);

  EXPECT_EQ(kin.accels.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.angular, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.twist.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.twist.angular, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.pose.position, position);
  EXPECT_NEAR(rpy.x(), 0.0f, 1e-5f);
  EXPECT_NEAR(rpy.y(), 0.0f, 1e-5f);
  EXPECT_NEAR(rpy.z(), kYaw, 1e-5f);
}

TEST(JSBSimPhysicsModel, RobotConfigUsesConstantGroundMode) {
  auto robot = MakeJSBSimRobot("constant_ground_config");

  EXPECT_EQ(robot.GetJSBSimGroundSettings().mode,
            projectairsim::JSBSimGroundMode::kConstant);
}

TEST(JSBSimPhysicsModel, ConstantGroundModeUsesInitialTerrainSample) {
  auto robot = MakeJSBSimRobot("constant_ground_initial_sample");
  constexpr double kTerrainElevationASLM = 12.5;
  robot.SetCallbackTerrainElevationUpdated(
      [=](double, double) { return kTerrainElevationASLM; });
  robot.UpdateKinematics(
      MakeKinematics({10.0f, 20.0f, -6.0f},
                     projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f,
                                                                 0.0f),
                     projectairsim::Vector3::Zero()));

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kTinyStepNanos, body);

  const double terrain_elevation_asl_m =
      robot.GetJSBSimModel()->GetPropagate()->GetTerrainElevation() *
      projectairsim::MathUtils::feets_to_meters;
  EXPECT_NEAR(terrain_elevation_asl_m, kTerrainElevationASLM, 1e-4);
}

TEST(JSBSimPhysicsModel, StepPhysicsBodyStartLandedStillAdvancesJSBSim) {
  auto robot = MakeJSBSimRobot("start_landed");
  robot.SetStartLanded(true);

  const auto initial_kin = MakeKinematics(
      {2.0f, 3.0f, -4.0f},
      projectairsim::TransformUtils::ToQuaternion(0.2f, -0.1f, 0.6f),
      projectairsim::Vector3::Zero());
  robot.UpdateKinematics(initial_kin);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kThreeMsNanos, body);

  const auto kin = robot.GetKinematics();
  EXPECT_NEAR(robot.GetJSBSimModel()->GetSimTime(), 0.003, 1e-9);
  EXPECT_TRUE(kin.pose.position.allFinite());
  EXPECT_TRUE(kin.twist.linear.allFinite());
  EXPECT_TRUE(kin.accels.linear.allFinite());
}

TEST(JSBSimPhysicsModel, LinearAccelerationUsesBodyForcePlusGravity) {
  auto robot = MakeJSBSimRobot("body_force_acceleration", "c310");
  robot.UpdateKinematics(MakeKinematics(
      {0.0f, 0.0f, -4.0f},
      projectairsim::TransformUtils::ToQuaternion(0.2f, -0.15f, 0.7f),
      {12.0f, -3.0f, 0.5f}));

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;
  model.StepPhysicsBody(kThreeMsNanos, body);

  const auto kin = robot.GetKinematics();
  const auto* accelerations = robot.GetJSBSimModel()->GetAccelerations();
  const projectairsim::Vector3 specific_force_body(
      accelerations->GetBodyAccel(1) *
          projectairsim::MathUtils::feets_to_meters,
      accelerations->GetBodyAccel(2) *
          projectairsim::MathUtils::feets_to_meters,
      accelerations->GetBodyAccel(3) *
          projectairsim::MathUtils::feets_to_meters);
  const auto expected_world =
      projectairsim::PhysicsUtils::TransformVectorToWorldFrame(
          specific_force_body, kin.pose.orientation) +
      robot.GetEnvironment().env_info.gravity;

  EXPECT_TRUE(kin.accels.linear.isApprox(expected_world, 1e-5f));
}

TEST(JSBSimPhysicsModel, StepPhysicsBodyTakeoffCommandReleasesGroundedState) {
  auto robot = MakeJSBSimRobot("takeoff_releases_grounded", "c310");
  robot.SetStartLanded(true);

  const auto initial_kin =
      MakeKinematics({0.0f, 0.0f, -4.0f},
                     projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f,
                                                                 0.0f),
                     projectairsim::Vector3::Zero());
  robot.UpdateKinematics(initial_kin);
  robot.GetJSBSimModel()->SetPropertyValue("projectairsim/takeoff", 1.0);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kTwentyMsNanos, body);

  const auto kin = robot.GetKinematics();
  EXPECT_FALSE(kin.twist.linear.isZero());
  EXPECT_FALSE(kin.accels.linear.isZero());
}

TEST(JSBSimPhysicsModel, StepPhysicsBodyGroundCollisionAdvancesJSBSimOnce) {
  auto robot = MakeJSBSimRobot("landing_collision");
  robot.SetStartLanded(false);

  const auto initial_kin = MakeKinematics(
      {0.0f, 0.0f, -4.0f},
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, -0.4f),
      {0.0f, 0.0f, 1.0f});
  robot.UpdateKinematics(initial_kin);

  projectairsim::CollisionInfo collision(
      true, {0.0f, 0.0f, -1.0f}, {3.0f, 4.0f, -9.8f},
      {3.0f, 4.0f, -10.0f}, 0.25f, 0, "ground", 0);
  robot.UpdateCollisionInfo(collision);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kThreeMsNanos, body);

  const auto kin = robot.GetKinematics();
  EXPECT_NEAR(robot.GetJSBSimModel()->GetSimTime(), 0.003, 1e-9);
  EXPECT_TRUE(kin.pose.position.allFinite());
  EXPECT_TRUE(kin.twist.linear.allFinite());
  EXPECT_TRUE(kin.accels.linear.allFinite());
}

TEST(JSBSimPhysicsModel,
     StepPhysicsBodyPitchedGroundCollisionKeepsJSBSimAttitude) {
  auto robot = MakeJSBSimRobot("pitched_landing_collision");
  robot.SetStartLanded(false);

  const auto initial_kin = MakeKinematics(
      {0.0f, 0.0f, -4.0f},
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.25f, -0.4f),
      {0.0f, 0.0f, 1.0f});
  robot.UpdateKinematics(initial_kin);

  projectairsim::CollisionInfo collision(
      true, {0.0f, 0.0f, -1.0f}, {3.0f, 4.0f, -9.8f},
      {3.0f, 4.0f, -10.0f}, 0.25f, 0, "ground", 0);
  robot.UpdateCollisionInfo(collision);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kThreeMsNanos, body);

  const auto kin = robot.GetKinematics();
  const auto rpy = projectairsim::TransformUtils::ToRPY(kin.pose.orientation);
  EXPECT_NEAR(robot.GetJSBSimModel()->GetSimTime(), 0.003, 1e-9);
  EXPECT_NEAR(rpy.y(), 0.25f, 1e-4f);
}

TEST(JSBSimPhysicsModel, StepPhysicsBodyNonGroundCollisionUsesCollisionResponse) {
  auto robot = MakeJSBSimRobot("wall_collision");
  robot.SetStartLanded(false);

  const auto initial_kin =
      MakeKinematics({0.0f, 0.0f, -4.0f},
                     projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f,
                                                                 0.0f),
                     {1.0f, 0.0f, 0.0f});
  robot.UpdateKinematics(initial_kin);

  projectairsim::CollisionInfo collision(
      true, {-1.0f, 0.0f, 0.0f}, {10.0f, 0.0f, -4.0f},
      {10.0f, 0.0f, -4.0f}, 0.5f, 0, "wall", 0);
  robot.UpdateCollisionInfo(collision);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kTinyStepNanos, body);

  const auto kin = robot.GetKinematics();
  EXPECT_EQ(kin.accels.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.angular, projectairsim::Vector3::Zero());
  EXPECT_LT(kin.twist.linear.x(), 0.0f);
  EXPECT_NEAR(kin.twist.linear.y(), 0.0f, 1e-6f);
  EXPECT_NEAR(kin.twist.linear.z(), 0.0f, 1e-6f);
  EXPECT_NEAR(kin.pose.position.x(), 9.5f, 1e-5f);
  EXPECT_NEAR(kin.pose.position.y(), 0.0f, 1e-6f);
  EXPECT_NEAR(kin.pose.position.z(), -4.0f, 1e-6f);
}

TEST(JSBSimPhysicsModel, CollisionOverrideSyncsNextNoCollisionStep) {
  auto robot = MakeJSBSimRobot("collision_sync");
  robot.SetStartLanded(false);

  const auto initial_kin =
      MakeKinematics({0.0f, 0.0f, -4.0f},
                     projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f,
                                                                 0.0f),
                     {1.0f, 0.0f, 0.0f});
  robot.UpdateKinematics(initial_kin);

  projectairsim::CollisionInfo collision(
      true, {-1.0f, 0.0f, 0.0f}, {10.0f, 0.0f, -4.0f},
      {10.0f, 0.0f, -4.0f}, 0.5f, 0, "wall", 0);
  robot.UpdateCollisionInfo(collision);

  auto body = std::make_shared<projectairsim::JSBSimPhysicsBody>(robot);
  projectairsim::JSBSimPhysicsModel model;

  model.StepPhysicsBody(kTinyStepNanos, body);
  const auto collision_kin = robot.GetKinematics();

  robot.UpdateCollisionInfo(projectairsim::CollisionInfo());
  model.StepPhysicsBody(kTinyStepNanos, body);

  const auto no_collision_kin = robot.GetKinematics();
  EXPECT_GT(no_collision_kin.pose.position.x(),
            collision_kin.pose.position.x() - 0.05f);
  EXPECT_NEAR(no_collision_kin.pose.position.y(), collision_kin.pose.position.y(),
              0.05f);
  EXPECT_NEAR(no_collision_kin.pose.position.z(), collision_kin.pose.position.z(),
              0.05f);
}
