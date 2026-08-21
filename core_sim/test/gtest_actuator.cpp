// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "core_sim/actor/robot.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/lift_drag_control_surface.hpp"
#include "core_sim/actuators/tilt.hpp"
#include "core_sim/actuators/wheel.hpp"
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
    return Robot(id, origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger), "");
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

TEST(Actuator, LoadsJSBSimPropertyPathActuatorID) {
  json config_json = R"({
        "links": [ { "name": "Frame" } ],
        "actuators": [
          {
            "name": "fcs/elevator-cmd-norm",
            "type": "lift-drag-control-surface",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Elevator"
          }
        ]
    })"_json;

  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  auto& actuators = robot.GetActuators();
  ASSERT_EQ(actuators.size(), 1);
  EXPECT_EQ(actuators[0].get().GetId(), "fcs/elevator-cmd-norm");
}

TEST(Actuator, LoadsTiltJSBSimSettings) {
  json config_json = R"({
        "links": [ { "name": "Frame" } ],
        "actuators": [
          {
            "name": "Prop_FL_actuator",
            "type": "rotor",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Prop_FL",
            "rotor-settings": {
              "turning-direction": "clock-wise",
              "normal-vector": "0.0 0.0 -1.0",
              "coeff-of-thrust": 0.109919,
              "coeff-of-torque": 0.040164,
              "max-rpm": 6396.667,
              "propeller-diameter": 0.2286,
              "smoothing-tc": 0.005
            }
          },
          {
            "name": "Prop_FL_tilt_actuator",
            "type": "tilt",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Shroud_FL",
            "tilt-settings": {
              "target": "Prop_FL_actuator",
              "angle-min": 0.0,
              "angle-max": 1.57,
              "axis": "0.0 -1.0 0.0",
              "smoothing-tc": 0.5,
              "jsbsim-cmd": "fcs/tilt-cmd-norm[0]",
              "jsbsim-state": "fcs/tilt-pos-norm[0]"
            }
          }
        ]
    })"_json;

  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  auto& actuators = robot.GetActuators();
  ASSERT_EQ(actuators.size(), 2);

  auto* tilt = static_cast<projectairsim::Tilt*>(&actuators[1].get());
  EXPECT_EQ(tilt->GetSettings().jsbsim_cmd, "fcs/tilt-cmd-norm[0]");
  EXPECT_EQ(tilt->GetSettings().jsbsim_state, "fcs/tilt-pos-norm[0]");
}

TEST(Actuator, LoadsLiftDragControlSurfaceJSBSimSettings) {
  json config_json = R"({
        "links": [ { "name": "Frame" } ],
        "actuators": [
          {
            "name": "Elevator_actuator",
            "type": "lift-drag-control-surface",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Elevator",
            "lift-drag-control-surface-settings": {
              "rotation-rate": 0.524,
              "smoothing-tc": 0.005,
              "jsbsim-cmd": "fcs/elevator-cmd-norm",
              "jsbsim-state": "fcs/elevator-pos-rad"
            }
          }
        ]
    })"_json;

  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  auto& actuators = robot.GetActuators();
  ASSERT_EQ(actuators.size(), 1);

      auto* surface = static_cast<projectairsim::LiftDragControlSurface*>(
        &actuators[0].get());
  EXPECT_EQ(surface->GetSettings().jsbsim_cmd, "fcs/elevator-cmd-norm");
  EXPECT_EQ(surface->GetSettings().jsbsim_state, "fcs/elevator-pos-rad");
}

TEST(Actuator, LoadsWheelJSBSimSettings) {
  json config_json = R"({
        "links": [ { "name": "Frame" } ],
        "actuators": [
          {
            "name": "Wheel_FL_actuator",
            "type": "wheel",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Front_Left",
            "wheel-settings": {
              "normal-vector": "0.0 -1.0 0.0",
              "wheel-type": 1.0,
              "coeff-of-friction": 1.0,
              "coeff-of-wheel-torque": 0.040164,
              "engine": true,
              "steering": true,
              "brake": true,
              "smoothing-tc": 0.0,
              "jsbsim-cmd-engine": "fcs/throttle-cmd-norm[0]",
              "jsbsim-cmd-steering": "fcs/steering-cmd-norm",
              "jsbsim-cmd-brake": "fcs/brake-cmd-norm"
            }
          }
        ]
    })"_json;

  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  auto& actuators = robot.GetActuators();
  ASSERT_EQ(actuators.size(), 1);

  auto* wheel = static_cast<projectairsim::Wheel*>(&actuators[0].get());
  EXPECT_FLOAT_EQ(wheel->GetWheelSettings().wheel_type, 1.0f);
  EXPECT_FLOAT_EQ(wheel->GetWheelSettings().coeff_of_torque, 0.040164f);
  EXPECT_TRUE(wheel->GetWheelSettings().engine_connected_);
  EXPECT_TRUE(wheel->GetWheelSettings().steering_connected_);
  EXPECT_TRUE(wheel->GetWheelSettings().brake_connected_);
  EXPECT_EQ(wheel->GetWheelSettings().jsbsim_cmd_engine,
            "fcs/throttle-cmd-norm[0]");
  EXPECT_EQ(wheel->GetWheelSettings().jsbsim_cmd_steering,
            "fcs/steering-cmd-norm");
  EXPECT_EQ(wheel->GetWheelSettings().jsbsim_cmd_brake,
            "fcs/brake-cmd-norm");
}
