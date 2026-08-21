// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// Tests for sensors

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

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
                 ServiceManager(logger), StateManager(logger),"");
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }

  static json GetBasicSensorConfig() {
    json config = R"({
            "links": [ { "name": "Frame" } ],
            "sensors": []
        })"_json;
    return config;
  }
  static json GetSampleSensorConfig() {
    json config = R"({
            "links": [ { "name": "Frame" } ],
            "sensors": [ {
                    "id": "ID123",
                    "type": "camera",
                    "enabled": true,
                    "parent-link": "ParentLink",
                    "capture-interval": 1.23,
                    "capture-settings": [],
                    "noise_settings": [],
                    "gimbal": {},
                    "origin": {}
                }
            ]
        })"_json;
    return config;
  }

  static json GetBasicCameraConfig() {
    json config = R"({
      "id": "CAM123",
      "type": "camera",
      "enabled": true,
      "parent-link": "ParentLink",
      "capture-interval": 1.23,
      "capture-settings": [],
      "noise_settings": [],
      "gimbal": {},
      "origin": {}
    })"_json;
    return config;
  }

  static json GetBasicImuConfig() {
    json config = R"({
      "id": "IMU123",
      "type": "imu",
      "enabled": true,
      "parent-link": "ParentLink",
      "accelerometer": {},
      "gyroscope": {},
      "origin": {}
    })"_json;
    return config;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

// GTest suit name should follow proper naming convention (no '_' strictly)

TEST(Sensor, HandlesNoSensors) {
  // General description:
  // Verifies handles no sensors for Sensor.
  // Arrange: prepare context for `auto config_json = R"({`.
  auto config_json = R"({
    "links": [ { "name": "Frame" } ]
  })"_json;
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
}

TEST(Sensor, HandlesEmptySensors) {
  // General description:
  // Verifies handles empty sensors for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetSampleSensorConfig();`.
  auto config_json = projectairsim::Scene::GetSampleSensorConfig();
  //! Explicitly empty sensors for testing
  config_json["sensors"] = "[]"_json;
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
}

TEST(Sensor, LoadsOneSensor) {
  // General description:
  // Verifies loads one sensor for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetSampleSensorConfig();`.
  auto config_json = projectairsim::Scene::GetSampleSensorConfig();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 1);`.
  EXPECT_EQ(sensors.size(), 1);
}

TEST(Sensor, LoadsTwoSensors) {
  // General description:
  // Verifies loads two sensors for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetSampleSensorConfig();`.
  auto config_json = projectairsim::Scene::GetSampleSensorConfig();
  //! Clone existing sensor; Note duplicate sensor IDs
  config_json["sensors"].push_back(config_json["sensors"].at(0));
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 2);`.
  EXPECT_EQ(sensors.size(), 2);
}

TEST(Sensor, LoadsCameraSensor) {
  // General description:
  // Verifies loads camera sensor for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetBasicSensorConfig();`.
  auto config_json = projectairsim::Scene::GetBasicSensorConfig();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
  config_json["sensors"].push_back(projectairsim::Scene::GetBasicCameraConfig());
  projectairsim::Scene::LoadRobot(robot, config_json);
  EXPECT_EQ(sensors.size(), 1);
  EXPECT_EQ(sensors.at(0).get().GetType(), projectairsim::SensorType::kCamera);
}

TEST(Sensor, LoadsTwoCameraSensors) {
  // General description:
  // Verifies loads two camera sensors for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetBasicSensorConfig();`.
  auto config_json = projectairsim::Scene::GetBasicSensorConfig();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
  auto cam1_config = projectairsim::Scene::GetBasicCameraConfig();
  auto cam2_config = projectairsim::Scene::GetBasicCameraConfig();
  cam2_config["id"] = "CAM2";
  config_json["sensors"].push_back(cam1_config);
  config_json["sensors"].push_back(cam2_config);
  projectairsim::Scene::LoadRobot(robot, config_json);
  EXPECT_EQ(sensors.size(), 2);
  EXPECT_EQ(sensors.at(0).get().GetType(), projectairsim::SensorType::kCamera);
  EXPECT_EQ(sensors.at(1).get().GetType(), projectairsim::SensorType::kCamera);
}
TEST(Sensor, LoadsImuSensor) {
  // General description:
  // Verifies loads imu sensor for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetBasicSensorConfig();`.
  auto config_json = projectairsim::Scene::GetBasicSensorConfig();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
  config_json["sensors"].push_back(projectairsim::Scene::GetBasicImuConfig());
  projectairsim::Scene::LoadRobot(robot, config_json);
  EXPECT_EQ(sensors.size(), 1);
  EXPECT_EQ(sensors.at(0).get().GetType(), projectairsim::SensorType::kImu);
}

TEST(Sensor, LoadsTwoImuSensors) {
  // General description:
  // Verifies loads two imu sensors for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetBasicSensorConfig();`.
  auto config_json = projectairsim::Scene::GetBasicSensorConfig();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
  auto imu1_config = projectairsim::Scene::GetBasicImuConfig();
  auto imu2_config = projectairsim::Scene::GetBasicImuConfig();
  imu2_config["id"] = "IMU2";
  config_json["sensors"].push_back(imu1_config);
  config_json["sensors"].push_back(imu2_config);
  projectairsim::Scene::LoadRobot(robot, config_json);
  EXPECT_EQ(sensors.size(), 2);
  EXPECT_EQ(sensors.at(0).get().GetType(), projectairsim::SensorType::kImu);
  EXPECT_EQ(sensors.at(1).get().GetType(), projectairsim::SensorType::kImu);
}

TEST(Sensor, LoadsImuAndCameraSensors) {
  // General description:
  // Verifies loads imu and camera sensors for Sensor.
  // Arrange: prepare context for `auto config_json = projectairsim::Scene::GetBasicSensorConfig();`.
  auto config_json = projectairsim::Scene::GetBasicSensorConfig();
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  projectairsim::Scene::LoadRobot(robot, config_json);
  // Act: run `auto& sensors = robot.GetSensors();`.
  auto& sensors = robot.GetSensors();
  // Assert: check result from `EXPECT_EQ(sensors.size(), 0);`.
  EXPECT_EQ(sensors.size(), 0);
  config_json["sensors"].push_back(projectairsim::Scene::GetBasicCameraConfig());
  config_json["sensors"].push_back(projectairsim::Scene::GetBasicImuConfig());
  projectairsim::Scene::LoadRobot(robot, config_json);
  EXPECT_EQ(sensors.size(), 2);
  EXPECT_EQ(sensors.at(0).get().GetType(), projectairsim::SensorType::kCamera);
  EXPECT_EQ(sensors.at(1).get().GetType(), projectairsim::SensorType::kImu);
}
