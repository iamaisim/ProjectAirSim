// Copyright (C) Microsoft Corporation.  All rights reserved.
// Tests for Camera Noise Properties

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class Robot {  // : public ::testing::Test {
               // protected:
 public:
  static Camera MakeDefaultCamera() {
    const std::string& id = "TestCamID";
    const bool& is_enabled = true;
    const std::string& parent_link = "ParentLink1";
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
        "noise-settings": [],
        "gimbal": {},
        "origin": {}
    })"_json;
    return config;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(CameraNoiseSettings, EnablesNoiseSettings) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_EQ(actual_noise_settings.at(2).Enabled, false);

  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& new_actual_camera_settings = camera.GetCameraSettings();
  auto new_actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_EQ(new_actual_noise_settings.at(2).Enabled, true);
}

//! Tests for Random Noise
TEST(CameraNoiseSettings, EnablesRandContribNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "rand-contrib": 0.55
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).RandContrib, 0.55);
}

TEST(CameraNoiseSettings, EnablesRandSpeedNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "rand-speed": 1000
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).RandSpeed, 1000);
}

TEST(CameraNoiseSettings, EnablesRandSizeNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "rand-size": 100
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).RandSize, 100);
}

TEST(CameraNoiseSettings, EnablesRandDensityNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "rand-density": 1
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).RandDensity, 1);
}

//! Tests for wave/bump distortion noise parameters
TEST(CameraNoiseSettings, EnablesHorzWaveContribNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-wave-contrib": 0.12
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzWaveContrib, 0.12);
}

TEST(CameraNoiseSettings, EnablesHorzWaveStrengthNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-wave-strength": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzWaveStrength, 0.123);
}

TEST(CameraNoiseSettings, EnablesHorzWaveVertSizeNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-wave-vert-size": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzWaveVertSize, 0.123);
}

TEST(CameraNoiseSettings, EnablesHorzWaveScreenSizeNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-wave-screen-size": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzWaveScreenSize, 0.123);
}

//! Tests for line noise parameters
TEST(CameraNoiseSettings, EnablesHorzNoiseLinesContribNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-noise-lines-contrib": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzNoiseLinesContrib, 0.123);
}

TEST(CameraNoiseSettings, EnablesHorzNoiseLinesDensityYNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-noise-lines-density-y": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzNoiseLinesDensityY, 0.123);
}

TEST(CameraNoiseSettings, EnablesHorzNoiseLinesDensityXYNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-noise-lines-density-xy": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzNoiseLinesDensityXY, 0.123);
}

//! Tests for distortion line parameters
TEST(CameraNoiseSettings, EnablesHorzDistortionContribNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-distortion-contrib": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzDistortionContrib, 0.123);
}

TEST(CameraNoiseSettings, EnablesHorzDistortionStrengthNoiseParam) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
          "enabled": true,
          "image-type": 2,
          "horz-distortion-strength": 0.123
      } ]
  )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_FLOAT_EQ(actual_noise_settings.at(2).HorzDistortionStrength, 0.123);
}

TEST(CameraNoiseSettings, SupportsAllNoiseSettingsParamsConcurrently) {
  auto camera = projectairsim::Robot::MakeDefaultCamera();
  auto camera_settings = projectairsim::Robot::GetBasicCameraConfig();
  json noise_settings = R"( [ {
            "enabled": true,
            "image-type": 1,

            "rand-contrib": 0.123,
            "rand-speed": 123.0,
            "rand-size": 123.0,
            "rand-density": 1.23,

            "horz-wave-contrib": 0.123,
            "horz-wave-strength": 0.123,
            "horz-wave-vert-size": 1.23,
            "horz-wave-screen-size": 1.23,

            "horz-noise-lines-contrib": 1.23,
            "horz-noise-lines-density-y": 0.123,
            "horz-noise-lines-density-xy": 0.123,

            "horz-distortion-contrib": 1.23,
            "horz-distortion-strength": 0.123
        } ]
    )"_json;
  camera_settings["noise-settings"] = noise_settings;
  projectairsim::Robot::LoadCamera(camera, camera_settings);
  const auto& actual_camera_settings = camera.GetCameraSettings();
  auto actual_noise_settings = actual_camera_settings.noise_settings;
  EXPECT_EQ(actual_noise_settings.at(1).Enabled, true);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).RandContrib, 0.123);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).RandSpeed, 123.0);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).RandSize, 123.0);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).RandDensity, 1.23);

  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzWaveContrib, 0.123);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzWaveStrength, 0.123);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzWaveVertSize, 1.23);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzWaveScreenSize, 1.23);

  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzNoiseLinesContrib, 1.23);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzNoiseLinesDensityY, 0.123);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzNoiseLinesDensityXY, 0.123);

  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzDistortionContrib, 1.23);
  EXPECT_FLOAT_EQ(actual_noise_settings.at(1).HorzDistortionStrength, 0.123);
}
