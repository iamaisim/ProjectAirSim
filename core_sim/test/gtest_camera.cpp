// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// Tests for Camera sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {  // : public ::testing::Test {
               // protected:
 public:
  static Camera MakeCamera(const std::string& id, bool is_enabled,
                           const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/camera_test_topic";
    return Camera(id, is_enabled, parent_link, logger, TopicManager(logger),
                  parent_topic_path, ServiceManager(logger),
                  StateManager(logger));
  }

  static void LoadCamera(Camera& camera, ConfigJson config_json) {
    camera.Load(config_json);
  }

  static json GetBasicCameraConfig() {
    json config = R"({
      "id": "ID123",
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
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

// Test suit name should follow proper naming convention (no '_' strictly)

TEST(Camera, SetsCameraID) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetId(), std::string("ID123"));
  id = "Camera123";
  camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetId(), std::string("Camera123"));
}

TEST(Camera, SetsSensorType) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetType(), projectairsim::SensorType::kCamera);
}

TEST(Camera, SetsIsEnabled) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  auto is_enabled = false;
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.IsEnabled(), false);
  is_enabled = true;
  camera =
      projectairsim::Robot::MakeCamera("TestCamera", is_enabled, parent_link);
  EXPECT_EQ(camera.IsEnabled(), true);
}

TEST(Camera, SetsParentLink) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = false;
  auto parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetParentLink(), "ParentLink");
  parent_link = "ParentLink123";
  camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetParentLink(), "ParentLink123");
}

TEST(Camera, LoadsCamera) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  projectairsim::Robot::LoadCamera(camera, camera_json);
  EXPECT_EQ(camera.IsLoaded(), true);
}

TEST(Camera, SetsIsLoaded) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.IsLoaded(), false);
  projectairsim::Robot::LoadCamera(camera, camera_json);
  EXPECT_EQ(camera.IsLoaded(), true);
}

TEST(Camera, SetsDesiredCaptureInterval) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  projectairsim::Robot::LoadCamera(camera, camera_json);
  const auto& camera_settings = camera.GetCameraSettings();
  EXPECT_FLOAT_EQ(camera_settings.capture_interval, 1.23);
}
