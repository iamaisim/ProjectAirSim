// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "core_sim/actor/robot.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Scene {  // : public ::testing::Test {
               // protected:
 public:
  static Robot MakeRobot(const std::string& id) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    return Robot("TestRobotID", origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger));
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }

  static json get_actuator_config() {
    json config = R"({
            "links": [ { "name": "Frame" } ],
            "actuators": [ {
                    "name": "ID123",
                    "type": "rotor",
                    "enabled": true,
                    "parent-link": "ParentLink",
                    "child-link": "ChildLink",
                    "origin": {
                        "xyz": "0 0 0",
                        "rpy-deg": "0 0 0"
                    }
                }
            ]
        })"_json;
    return config;
  }
};

class FakeController : public IController {
 public:
  void BeginUpdate() override {}
  void EndUpdate() override {}
  void Reset() override {}
  void SetKinematics(const Kinematics* kinematics) override {}
  void Update() override {}

  int GetControlSignalIndex(const std::string& actuator_id) override {
    return GetControlSignalIndex(actuator_id, 0);
  }

  int GetControlSignalIndex(const std::string& actuator_id,
                            size_t signal_offset) override {
    ++index_lookup_count;
    last_actuator_id = actuator_id;
    return 7 + static_cast<int>(signal_offset);
  }

  void GetControlSignalSnapshot(std::vector<float>& control_signals) override {
    control_signals = {0.0f};
  }

  std::vector<float> GetControlSignals(int signal_index) override {
    return {static_cast<float>(signal_index)};
  }

  std::vector<float> GetControlSignals(
      const std::string& actuator_id) override {
    return GetControlSignals(GetControlSignalIndex(actuator_id));
  }

  const GimbalState& GetGimbalSignal(const std::string& gimbal_id) override {
    return gimbal_state_;
  }

  int index_lookup_count = 0;
  std::string last_actuator_id;

 private:
  GimbalState gimbal_state_;
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(Actuator, HandlesNoActuators) {
  // General description:
  // Verifies handles no actuators for Actuator.
  // Arrange: prepare context for `auto config_json = R"({`.
  auto config_json = R"({
      "links": [ { "name": "Frame" } ]
    })"_json;
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& actuators = robot.GetActuators();`.
  auto& actuators = robot.GetActuators();
  // Assert: check result from `EXPECT_EQ(actuators.size(), 0);`.
  EXPECT_EQ(actuators.size(), 0);
}

TEST(Actuator, HandlesEmptyActuators) {
  // General description:
  // Verifies handles empty actuators for Actuator.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::get_actuator_config();`.
  auto config_json = projectairsim::Scene::get_actuator_config();
  //! Explicitly empty actuators for testing
  config_json["actuators"] = "[]"_json;
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& actuators = robot.GetActuators();`.
  auto& actuators = robot.GetActuators();
  // Assert: check result from `EXPECT_EQ(actuators.size(), 0);`.
  EXPECT_EQ(actuators.size(), 0);
}

TEST(Actuator, LoadsOneActuator) {
  // General description:
  // Verifies loads one actuator for Actuator.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::get_actuator_config();`.
  auto config_json = projectairsim::Scene::get_actuator_config();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& actuators = robot.GetActuators();`.
  auto& actuators = robot.GetActuators();
  // Assert: check result from `EXPECT_EQ(actuators.size(), 1);`.
  EXPECT_EQ(actuators.size(), 1);
}

TEST(Actuator, CachesControllerSignalIndex) {
  auto config_json = projectairsim::Scene::get_actuator_config();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  auto controller = std::make_unique<projectairsim::FakeController>();
  auto* controller_ptr = controller.get();

  robot.SetController(std::move(controller));

  auto& actuator = robot.GetActuators().at(0).get();
  EXPECT_EQ(actuator.GetSignalIndex(), 7);
  EXPECT_EQ(controller_ptr->index_lookup_count, 1);
  EXPECT_EQ(controller_ptr->last_actuator_id, actuator.GetId());
  EXPECT_EQ(controller_ptr->GetControlSignals(actuator.GetSignalIndex()).at(0),
            7.0f);
  EXPECT_EQ(controller_ptr->index_lookup_count, 1);
}

TEST(Actuator, CachesAllWheelSignalIndices) {
  auto config_json = projectairsim::Scene::get_actuator_config();
  config_json["actuators"][0]["type"] = "wheel";
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  auto controller = std::make_unique<projectairsim::FakeController>();
  auto* controller_ptr = controller.get();

  robot.SetController(std::move(controller));

  auto& actuator = robot.GetActuators().at(0).get();
  EXPECT_EQ(actuator.GetSignalCount(), 3);
  EXPECT_EQ(actuator.GetSignalIndex(0), 7);
  EXPECT_EQ(actuator.GetSignalIndex(1), 8);
  EXPECT_EQ(actuator.GetSignalIndex(2), 9);
  EXPECT_EQ(controller_ptr->index_lookup_count, 3);
}

TEST(Actuator, LoadsTwoActuatorsSameID) {
  // General description:
  // Verifies loads two actuators same id for Actuator.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::get_actuator_config();`.
  auto config_json = projectairsim::Scene::get_actuator_config();
  //! Clone existing actuator
  config_json["actuators"].push_back(config_json["actuators"].at(0));
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& actuators = robot.GetActuators();`.
  auto& actuators = robot.GetActuators();
  // Assert: check result from `EXPECT_EQ(actuators.size(),`.
  EXPECT_EQ(actuators.size(),
            2);  // Multiple actuators can have the same ID, and each will refer
                 // to the same control output ID signal in the controller's
                 // "actuator-order" list (such as doubling a quadrotor to be a
                 // stacked octorotor). However, it is not possible to have
                 // multiple controller output signals tied to the same
                 // actuator, so the controller's "actuator-order" list can not
                 // have duplicate actuator IDs in it.
}

TEST(Actuator, LoadsTwoActuatorsDifferentID) {
  // General description:
  // Verifies loads two actuators different id for Actuator.
  // Arrange: prepare context for `json config_json = R"({`.
  json config_json = R"({
        "links": [ { "name": "Frame" } ],
        "actuators": [
          {
            "name": "ID123",
            "type": "rotor",
            "enabled": true,
            "parent-link": "ParentLink",
            "child-link": "ChildLink",
            "origin": {
              "xyz": "0 0 0",
              "rpy-deg": "0 0 0"
            }
          },
          {
            "name": "ID1234",
            "type": "rotor",
            "enabled": true,
            "parent-link": "ParentLink",
            "child-link": "ChildLink",
            "origin": {
              "xyz": "0 0 0",
              "rpy-deg": "0 0 0"
            }
          }
        ]
    })"_json;

  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& actuators = robot.GetActuators();`.
  auto& actuators = robot.GetActuators();
  // Assert: check result from `EXPECT_EQ(actuators.size(), 2);`.
  EXPECT_EQ(actuators.size(), 2);
}
