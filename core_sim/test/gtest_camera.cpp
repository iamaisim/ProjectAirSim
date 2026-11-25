// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
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
  // General description:
  // Verifies sets camera id for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  // Act: run `auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);`.
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  // Assert: check result from `EXPECT_EQ(camera.GetId(), std::string("ID123"));`.
  EXPECT_EQ(camera.GetId(), std::string("ID123"));
  id = "Camera123";
  camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetId(), std::string("Camera123"));
}

TEST(Camera, SetsSensorType) {
  // General description:
  // Verifies sets sensor type for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  // Act: run `auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);`.
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  // Assert: check result from `EXPECT_EQ(camera.GetType(), projectairsim::SensorType::kCamera);`.
  EXPECT_EQ(camera.GetType(), projectairsim::SensorType::kCamera);
}

TEST(Camera, SetsIsEnabled) {
  // General description:
  // Verifies sets is enabled for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  auto is_enabled = false;
  const auto& parent_link = camera_json["parent-link"];
  // Act: run `auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);`.
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  // Assert: check result from `EXPECT_EQ(camera.IsEnabled(), false);`.
  EXPECT_EQ(camera.IsEnabled(), false);
  is_enabled = true;
  camera =
      projectairsim::Robot::MakeCamera("TestCamera", is_enabled, parent_link);
  EXPECT_EQ(camera.IsEnabled(), true);
}

TEST(Camera, SetsParentLink) {
  // General description:
  // Verifies sets parent link for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = false;
  auto parent_link = camera_json["parent-link"];
  // Act: run `auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);`.
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  // Assert: check result from `EXPECT_EQ(camera.GetParentLink(), "ParentLink");`.
  EXPECT_EQ(camera.GetParentLink(), "ParentLink");
  parent_link = "ParentLink123";
  camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  EXPECT_EQ(camera.GetParentLink(), "ParentLink123");
}

TEST(Camera, LoadsCamera) {
  // General description:
  // Verifies loads camera for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  // Act: run `projectairsim::Robot::LoadCamera(camera, camera_json);`.
  projectairsim::Robot::LoadCamera(camera, camera_json);
  // Assert: check result from `EXPECT_EQ(camera.IsLoaded(), true);`.
  EXPECT_EQ(camera.IsLoaded(), true);
}

TEST(Camera, SetsIsLoaded) {
  // General description:
  // Verifies sets is loaded for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  // Act: run `auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);`.
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  // Assert: check result from `EXPECT_EQ(camera.IsLoaded(), false);`.
  EXPECT_EQ(camera.IsLoaded(), false);
  projectairsim::Robot::LoadCamera(camera, camera_json);
  EXPECT_EQ(camera.IsLoaded(), true);
}

TEST(Camera, SetsDesiredCaptureInterval) {
  // General description:
  // Verifies sets desired capture interval for Camera.
  // Arrange: prepare context for `auto camera_json = projectairsim::Robot::GetBasicCameraConfig();`.
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  auto id = camera_json["id"];
  const auto& is_enabled = camera_json["enabled"];
  const auto& parent_link = camera_json["parent-link"];
  auto camera = projectairsim::Robot::MakeCamera(id, is_enabled, parent_link);
  projectairsim::Robot::LoadCamera(camera, camera_json);
  // Act: run `const auto& camera_settings = camera.GetCameraSettings();`.
  const auto& camera_settings = camera.GetCameraSettings();
  // Assert: check result from `EXPECT_FLOAT_EQ(camera_settings.capture_interval, 1.23);`.
  EXPECT_FLOAT_EQ(camera_settings.capture_interval, 1.23);
}

TEST(Camera, OnnxBuildAvailability) {
  auto camera_json = projectairsim::Robot::GetBasicCameraConfig();
  camera_json["post-process-model-settings"] = {
      {"enabled", true}, {"model-filepath", "test-model.onnx"}};
  auto camera = projectairsim::Robot::MakeCamera("ID123", true, "ParentLink");
#if PROJECTAIRSIM_ENABLE_ONNX_RUNTIME
  EXPECT_NO_THROW(projectairsim::Robot::LoadCamera(camera, camera_json));
#else
  try {
    projectairsim::Robot::LoadCamera(camera, camera_json);
    FAIL() << "ONNX-enabled config must fail when support is compiled out";
  } catch (const std::exception& error) {
    EXPECT_NE(std::string(error.what()).find("PROJECTAIRSIM_ENABLE_ONNX_RUNTIME=TRUE"),
              std::string::npos);
  }
#endif
  camera_json["post-process-model-settings"]["enabled"] = false;
  EXPECT_NO_THROW(projectairsim::Robot::LoadCamera(camera, camera_json));
}
