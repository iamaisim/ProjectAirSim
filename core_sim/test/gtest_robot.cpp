// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// Tests for Robot class

#include <atomic>
#include <future>
#include <limits>
#include <thread>
#include <vector>
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
  void Update() override {
    if (sequence_output) {
      const float value = static_cast<float>(++update_count) / 4096.0f;
      output = {value, -value, value / 2.0f};
    }
  }
  int GetControlSignalIndex(const std::string& id) override {
    return GetControlSignalIndex(id, 0);
  }
  int GetControlSignalIndex(const std::string& id, size_t offset) override {
    if (offset >= 3) return -1;
    // Exercise channel remapping and negative/equal-to-size/oversized indices.
    const int mapped[] = {2, 0, 1};
    const int invalid[] = {-1, 3, 99};
    return id == "invalid" ? invalid[offset] : mapped[offset];
  }
  void GetControlSignalSnapshot(std::vector<float>& signals) override {
    ++snapshot_calls;
    signals = output;
  }
  std::vector<float> GetControlSignals(int index) override {
    ++scalar_calls;
    return {index >= 0 && index < static_cast<int>(output.size())
                ? output[index] : 0.0f};
  }
  std::vector<float> GetControlSignals(const std::string&) override {
    // Match SimpleDrive's incoming scalar accessor: it is not a snapshot API.
    return GetControlSignals(0);
  }
  std::vector<float> output{0.7f, -0.25f, 0.1f};
  int snapshot_calls = 0;
  int scalar_calls = 0;
  int update_count = 0;
  bool sequence_output = false;
  const GimbalState& GetGimbalSignal(const std::string&) override {
    return gimbal_state_;
  }

 private:
  GimbalState gimbal_state_{};
};

// These tests specify the capture-once contract for the shared robot snapshot.
projectairsim::Robot MakeSnapshotRobot(bool with_wheels = false) {
  auto robot = projectairsim::Scene::MakeRobot("SnapshotRobot");
  json config = {
      {"physics-type", "unreal-physics"},
      {"wheeled-vehicle-class", "/Game/Vehicles/Test.Test_C"},
      {"controller", {{"id", "SimpleDrive"}, {"type", "simple-drive-api"}}}};
  if (with_wheels) {
    config["links"] = {{{"name", "body"}}};
    config["actuators"] = json::array();
    for (const auto* id : {"mapped", "invalid"}) {
      config["actuators"].push_back({
          {"name", id}, {"type", "wheel"}, {"enabled", true},
          {"parent-link", "body"}, {"child-link", id},
          {"wheel-settings", {{"normal-vector", "0 0 -1"},
                              {"wheel-type", 1.0},
                              {"coeff-of-friction", 1.0},
                              {"smoothing-tc", 0.0}}}});
    }
  }
  projectairsim::Scene::LoadRobot(robot, config);
  return robot;
}

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

TEST(Robot, SnapshotCapturedOncePerControlTickAndSharedWithActuators) {
  auto robot = MakeSnapshotRobot(true);
  auto controller = std::make_unique<TestSimpleDriveController>();
  auto* source = controller.get();
  robot.SetController(std::move(controller));

  robot.UpdateControlInput();
  EXPECT_EQ(source->snapshot_calls, 1);
  EXPECT_EQ(robot.GetControllerOutput(), source->output);
  robot.UpdateActuators(10'000'000, 10'000'000);
  EXPECT_EQ(source->snapshot_calls, 1);
  EXPECT_EQ(source->scalar_calls, 0);

  source->output = {0.2f, -0.1f, 0.3f};
  robot.UpdateControlInput();
  robot.UpdateActuators(20'000'000, 10'000'000);
  EXPECT_EQ(source->snapshot_calls, 2);
  EXPECT_EQ(robot.GetControllerOutput(), source->output);
  EXPECT_EQ(source->scalar_calls, 0);
}

TEST(Robot, SnapshotMapsWheelChannelsAndZerosInvalidIndices) {
  auto robot = MakeSnapshotRobot(true);
  auto reference = MakeSnapshotRobot(true);
  auto controller = std::make_unique<TestSimpleDriveController>();
  auto* source = controller.get();
  robot.SetController(std::move(controller));

  // Give the invalid-index wheel velocity so an unintended brake signal is
  // observable, rather than testing all three invalid channels only at rest.
  auto& invalid = static_cast<projectairsim::Wheel&>(
      robot.GetActuators()[1].get());
  auto& invalid_reference = static_cast<projectairsim::Wheel&>(
      reference.GetActuators()[1].get());
  invalid.UpdateActuatorOutput({0.4f, 0.2f, 0.0f}, 10'000'000);
  invalid_reference.UpdateActuatorOutput({0.4f, 0.2f, 0.0f}, 10'000'000);

  // Compare with wheels driven directly by the intended channel ordering.
  // The second step exercises braking against nonzero wheel velocity.
  for (const auto& signals : {std::vector<float>{0.6f, 0.2f, 0.4f},
                              std::vector<float>{0.0f, 0.3f, 0.0f}}) {
    source->output = signals;
    robot.UpdateControlInput();
    robot.UpdateActuators(10'000'000, 10'000'000);
    auto& expected = static_cast<projectairsim::Wheel&>(
        reference.GetActuators()[0].get());
    expected.UpdateActuatorOutput({signals[2], signals[0], signals[1]},
                                  10'000'000);
    const auto& actual = static_cast<const projectairsim::Wheel&>(
        robot.GetActuators()[0].get());
    EXPECT_FLOAT_EQ(actual.GetSteering(), expected.GetSteering());
    EXPECT_FLOAT_EQ(actual.GetTorque(), expected.GetTorque());
    EXPECT_FLOAT_EQ(actual.GetRotatingSpeed(), expected.GetRotatingSpeed());
    invalid_reference.UpdateActuatorOutput({0.0f, 0.0f, 0.0f}, 10'000'000);
    EXPECT_FLOAT_EQ(invalid.GetSteering(), 0.0f);
    EXPECT_FLOAT_EQ(invalid.GetTorque(), invalid_reference.GetTorque());
    EXPECT_FLOAT_EQ(invalid.GetRotatingSpeed(),
                    invalid_reference.GetRotatingSpeed());
  }
}

TEST(Robot, ActuatorOnlyUpdatesReuseLastCapturedSnapshot) {
  auto robot = MakeSnapshotRobot(true);
  auto controller = std::make_unique<TestSimpleDriveController>();
  auto* source = controller.get();
  source->output = {0.6f, 0.2f, 0.4f};
  robot.SetController(std::move(controller));
  robot.UpdateControlInput();
  robot.UpdateActuators(10'000'000, 10'000'000);
  const auto captured = robot.GetControllerOutput();
  const auto& wheel = static_cast<const projectairsim::Wheel&>(
      robot.GetActuators()[0].get());
  const float steering = wheel.GetSteering();

  // Model an external controller message arriving between simulation phases.
  source->output = {-0.8f, 0.0f, 0.0f};
  robot.UpdateActuators(20'000'000, 10'000'000);
  EXPECT_EQ(source->snapshot_calls, 1);
  EXPECT_EQ(robot.GetControllerOutput(), captured);
  EXPECT_FLOAT_EQ(wheel.GetSteering(), steering);

  robot.UpdateControlInput();
  robot.UpdateActuators(30'000'000, 10'000'000);
  EXPECT_EQ(source->snapshot_calls, 2);
  EXPECT_EQ(robot.GetControllerOutput(), source->output);
  EXPECT_LT(wheel.GetSteering(), 0.0f);
}

TEST(Robot, ControllerOutputReturnsIndependentSnapshotCopy) {
  auto robot = MakeSnapshotRobot();
  robot.SetController(std::make_unique<TestSimpleDriveController>());
  robot.UpdateControlInput();
  auto output = robot.GetControllerOutput();
  ASSERT_EQ(output.size(), 3u);
  output[0] = -1.0f;
  EXPECT_EQ(robot.GetControllerOutput(),
            (std::vector<float>{0.7f, -0.25f, 0.1f}));
}

TEST(Robot, ConcurrentControllerOutputReadsReturnWholeSnapshots) {
  auto robot = MakeSnapshotRobot();
  auto controller = std::make_unique<TestSimpleDriveController>();
  controller->sequence_output = true;
  robot.SetController(std::move(controller));
  robot.UpdateControlInput();

  std::promise<void> start;
  auto ready = start.get_future().share();
  std::atomic<bool> coherent{true};
  // Fixed iteration counts avoid timing assertions and sleep-based coordination.
  // Also run this regression under ThreadSanitizer to detect data races.
  std::thread writer([&] {
    ready.wait();
    for (int i = 0; i < 2000; ++i) robot.UpdateControlInput();
  });
  std::thread reader([&] {
    ready.wait();
    for (int i = 0; i < 2000; ++i) {
      const auto output = robot.GetControllerOutput();
      if (output.size() != 3 || output[1] != -output[0] ||
          output[2] != output[0] / 2.0f) {
        coherent = false;
      }
    }
  });
  start.set_value();
  writer.join();
  reader.join();
  EXPECT_TRUE(coherent.load());
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
