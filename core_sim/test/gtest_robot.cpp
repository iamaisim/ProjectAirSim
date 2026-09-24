// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// Tests for Robot class

#include <limits>
#include <memory>
#include <string>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "core_sim/service_method.hpp"
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
  static ServiceMethod MakeRobotTypeMethod(const std::string& path) {
    return ServiceMethod(path + "/GetRobotType", {});
  }

  static Robot MakeRobot(const std::string& id) {
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);
    return Robot(id, origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger), "");
  }

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

class TestSimpleDriveController : public projectairsim::IController {
 public:
  void BeginUpdate() override {}
  void EndUpdate() override {}
  void Reset() override {}
  void SetKinematics(const projectairsim::Kinematics*) override {}
  void Update() override { output_ = {0.7f, -0.25f, 0.1f}; }
  std::vector<float> GetControlSignals(const std::string&) override {
    return output_;
  }
  const GimbalState& GetGimbalSignal(const std::string&) override {
    return gimbal_state_;
  }

 private:
  std::vector<float> output_;
  GimbalState gimbal_state_{};
};

std::string GetProjectAirSimPluginPath() {
  return std::string(PROJECTAIRSIM_SOURCE_DIR) +
         "/unreal/Blocks/Plugins/ProjectAirSim";
}

}  // namespace

TEST(Robot, Constructor) {
  // General description:
  // Verifies constructor for Robot.
  // Arrange: prepare context for `EXPECT_FALSE(projectairsim::Scene::MakeRobot("abc").IsLoaded());`.
  // Act: run `EXPECT_FALSE(projectairsim::Scene::MakeRobot("abc").IsLoaded());`.
  // Assert: check result from `EXPECT_FALSE(projectairsim::Scene::MakeRobot("abc").IsLoaded());`.
  EXPECT_FALSE(projectairsim::Scene::MakeRobot("abc").IsLoaded());
}

TEST(Robot, LoadRobot) {
  // General description:
  // Verifies load robot for Robot.
  // Arrange: prepare context for `json json = R"({`.
  json json = R"({
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");
  // Act: run `projectairsim::Scene::LoadRobot(robot, json);`.
  projectairsim::Scene::LoadRobot(robot, json);
  // Assert: check result from `}`.
}

TEST(Robot, IsLoaded) {
  // General description:
  // Verifies is loaded for Robot.
  // Arrange: prepare context for `json json = R"({`.
  json json = R"({
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");
  // Act: run `projectairsim::Scene::LoadRobot(robot, json);`.
  projectairsim::Scene::LoadRobot(robot, json);
  // Assert: check result from `EXPECT_TRUE(robot.IsLoaded());`.
  EXPECT_TRUE(robot.IsLoaded());
}

TEST(Robot, GetID) {
  // General description:
  // Verifies get id for Robot.
  // Arrange: prepare context for `json json = R"({`.
  json json = R"({
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");
  // Act: run `projectairsim::Scene::LoadRobot(robot, json);`.
  projectairsim::Scene::LoadRobot(robot, json);
  // Assert: check result from `EXPECT_EQ(robot.GetID(), "a");`.
  EXPECT_EQ(robot.GetID(), "a");
}

TEST(Robot, GetType) {
  // General description:
  // Verifies get type for Robot.
  // Arrange: prepare context for `json json = R"({`.
  json json = R"({
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");
  // Act: run `projectairsim::Scene::LoadRobot(robot, json);`.
  projectairsim::Scene::LoadRobot(robot, json);
  // Assert: check result from `EXPECT_EQ(robot.GetType(), projectairsim::ActorType::kRobot);`.
  EXPECT_EQ(robot.GetType(), projectairsim::ActorType::kRobot);
}

TEST(Robot, GetLinks) {
  // General description:
  // Verifies get links for Robot.
  // Arrange: prepare context for `json json = R"({`.
  json json = R"({
        "links": [
          {
            "name": "link1",
            "visual": {
              "geometry": {
                  "type": "unreal_mesh",
                  "name": "Link1"
              }
            }
          },
          {
            "name": "link2",
            "visual": {
                "geometry": {
                  "type": "unreal_mesh",
                  "name": "Link2"
              }
            }
          }
        ]
      })"_json;

  auto robot = projectairsim::Scene::MakeRobot("a");
  // Act: run `projectairsim::Scene::LoadRobot(robot, json);`.
  projectairsim::Scene::LoadRobot(robot, json);
  // Assert: check result from `EXPECT_EQ(robot.GetLinks().size(), 2);`.
  EXPECT_EQ(robot.GetLinks().size(), 2);
}

TEST(Robot, LoadsWheeledVehicleClassWithoutLinks) {
  json json = R"({
      "physics-type": "unreal-physics",
      "wheeled-vehicle-class": "/Game/Vehicles/Test.Test_C"
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");

  projectairsim::Scene::LoadRobot(robot, json);

  EXPECT_EQ(robot.GetWheeledVehicleClass(),
            "/Game/Vehicles/Test.Test_C");
  EXPECT_TRUE(robot.GetUnrealVehicleClass().empty());
  EXPECT_TRUE(robot.GetLinks().empty());
}

TEST(Robot, RejectsBothUnrealVehicleClassOptions) {
  json json = R"({
      "physics-type": "unreal-physics",
      "unreal-vehicle-class": "/Game/Vehicles/Generic.Generic_C",
      "wheeled-vehicle-class": "/Game/Vehicles/Wheeled.Wheeled_C"
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");

  EXPECT_THROW(projectairsim::Scene::LoadRobot(robot, json),
               projectairsim::Error);
}

TEST(Robot, RobotTypeClassFieldsTakePriorityOverPhysics) {
  for (const auto& field : {"wheeled-vehicle-class", "unreal-vehicle-class"}) {
    auto robot = projectairsim::Scene::MakeRobot("CarA");
    const json config = {
        {"physics-type", "unreal-physics"}, {field, "/Game/Car.Car_C"}};
    projectairsim::Scene::LoadRobot(robot, config);
    const std::string expected = std::string(field) == "wheeled-vehicle-class"
                                     ? "wheeled-vehicle" : "unreal-vehicle";
    EXPECT_EQ(robot.GetRobotType(), expected);
    // The identity comes from validated class fields, not the physics enum.
    robot.SetPhysicsType(projectairsim::PhysicsType::kFastPhysics);
    EXPECT_EQ(robot.GetRobotType(), expected);
    robot.SetPhysicsType(projectairsim::PhysicsType::kJSBSimPhysics);
    EXPECT_EQ(robot.GetRobotType(), expected);
  }
}

TEST(Robot, RobotTypePhysicsRulesIgnoreControllerAndName) {
  for (const auto& controller : {"simple-flight-api", "ardupilot-api",
                                "jsbsim-api", "px4-api", "simple-drive-api"}) {
    auto robot = projectairsim::Scene::MakeRobot("UnrealVehicle_WheeledVehicle");
    const json config = {
        {"links", json::array({json{{"name", "Frame"}}})},
        {"controller", {{"id", "Controller"}, {"type", controller}}}};
    projectairsim::Scene::LoadRobot(robot, config);
    EXPECT_EQ(robot.GetRobotType(), "other");
    robot.SetPhysicsType(projectairsim::PhysicsType::kFastPhysics);
    EXPECT_EQ(robot.GetRobotType(), "drone");
    robot.SetPhysicsType(projectairsim::PhysicsType::kJSBSimPhysics);
    EXPECT_EQ(robot.GetRobotType(), "jsbsim");
    for (auto physics : {projectairsim::PhysicsType::kNonPhysics,
                         projectairsim::PhysicsType::kUnrealPhysics,
                         projectairsim::PhysicsType::kMatlabPhysics}) {
      robot.SetPhysicsType(physics);
      EXPECT_EQ(robot.GetRobotType(), "other");
    }
  }
}

TEST(Robot, RobotTypeHandlerIsZeroArgumentStringAndReadOnly) {
  auto robot = projectairsim::Scene::MakeRobot("CarB");
  const json config = {
      {"physics-type", "unreal-physics"},
      {"wheeled-vehicle-class", "/Game/Car.Car_C"}};
  projectairsim::Scene::LoadRobot(robot, config);
  auto method = projectairsim::Scene::MakeRobotTypeMethod("/Sim/Scene/robots/CarB");
  EXPECT_EQ(method.GetName(), "/Sim/Scene/robots/CarB/GetRobotType");
  EXPECT_TRUE(method.GetParamsList().empty());
  std::function<std::string()> getter = [&robot]() { return robot.GetRobotType(); };
  auto handler = method.CreateMethodHandler(getter);
  const auto position = robot.GetKinematics().pose.position;
  const auto orientation = robot.GetKinematics().pose.orientation;
  const auto outputs = robot.GetControllerOutput();
  for (int i = 0; i < 3; ++i) {
    const auto result = handler({});
    EXPECT_TRUE(result.is_string());
    EXPECT_EQ(result, "wheeled-vehicle");
  }
  EXPECT_THROW(handler({1}), projectairsim::Error);
  EXPECT_TRUE(robot.GetKinematics().pose.position.isApprox(position));
  EXPECT_TRUE(robot.GetKinematics().pose.orientation.isApprox(orientation));
  EXPECT_EQ(robot.GetControllerOutput(), outputs);
  auto other = projectairsim::Scene::MakeRobotTypeMethod("/Sim/Scene/robots/CarA");
  EXPECT_NE(other.GetName(), method.GetName());
}

TEST(Robot, CapturesSimpleDriveOutputWithoutActuators) {
  json json = R"({
      "physics-type": "unreal-physics",
      "wheeled-vehicle-class": "/Game/Vehicles/Test.Test_C",
      "controller": {
        "id": "SimpleDrive",
        "type": "simple-drive-api"
      }
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a");
  projectairsim::Scene::LoadRobot(robot, json);
  robot.SetController(std::make_unique<TestSimpleDriveController>());

  robot.UpdateControlInput();

  EXPECT_EQ(robot.GetControllerOutput(),
            (std::vector<float>{0.7f, -0.25f, 0.1f}));
  EXPECT_TRUE(robot.GetActuators().empty());
}

TEST(Robot, JSBSimDtDefaultsWhenOmitted) {
  json json = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a",
                                               GetProjectAirSimPluginPath());

  projectairsim::Scene::LoadRobot(robot, json);

  EXPECT_DOUBLE_EQ(robot.GetJSBSimDtSec(),
                   projectairsim::kDefaultJSBSimDtSec);
  EXPECT_EQ(robot.GetRobotType(), "jsbsim");
}

TEST(Robot, JSBSimDtLoadsCustomValue) {
  json json = R"({
      "physics-type": "jsbsim-physics",
      "jsbsim-model": "c310",
      "jsbsim-dt": 0.01,
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("a",
                                               GetProjectAirSimPluginPath());

  projectairsim::Scene::LoadRobot(robot, json);

  EXPECT_DOUBLE_EQ(robot.GetJSBSimDtSec(), 0.01);
}

TEST(Robot, JSBSimDtRejectsInvalidValues) {
  for (const auto invalid_dt : {0.0, -0.01,
                                std::numeric_limits<double>::infinity()}) {
    json json = R"({
        "physics-type": "jsbsim-physics",
        "jsbsim-model": "c310",
        "links": [ { "name": "Frame" } ]
    })"_json;
    json["jsbsim-dt"] = invalid_dt;
    auto robot = projectairsim::Scene::MakeRobot(
        "a", GetProjectAirSimPluginPath());

    EXPECT_THROW(projectairsim::Scene::LoadRobot(robot, json),
                 projectairsim::Error);
  }
}
