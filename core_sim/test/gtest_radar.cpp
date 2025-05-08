// Copyright (C) Microsoft Corporation.  All rights reserved.
// Tests for Radar sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/radar.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static Radar MakeRadar(const std::string& id, bool is_enabled,
                         const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/radar_test_topic";
    return Radar(id, is_enabled, parent_link, logger, TopicManager(logger),
                 parent_topic_path, ServiceManager(logger),
                 StateManager(logger));
  }

  static void LoadRadar(Radar& radar, ConfigJson config_json) {
    radar.Load(config_json);
  }

  static json GetBasicRadarConfig() {
    json config = R"({
      "id": "ID123",
      "type": "radar",
      "enabled": true,
      "parent-link": "ParentLink",
      "fov": {
        "azimuth-max": 10,
        "azimuth-min": 10,
        "elevation-max": 10,
        "elevation-min": 10,
        "azimuth-resolution": 10,
        "elevation-resolution": 10
      },
      "range-max": 10,
      "range-resolution": 10,
      "velocity-max": 10,
      "velocity-resolution": 10,
      "detection-interval": 10,
      "track-interval": 10,
      "draw-debug-points": false,
      "origin": {},
      "masks": [
        {
          "azimuth-min": 10,
          "azimuth-max": 10,
          "elevation-min": 10,
          "elevation-max": 10,
          "range-min": 10,
          "range-max": 10,
          "velocity-min": 10.0,
          "velocity-max": 10.0,
          "rcs-sqm-min": 10.0,
          "rcs-sqm-max": 10.0
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

// Test suit name should follow proper naming convention (no '_' strictly)

TEST(Radar, SetsRadarID) {
  auto radar_json = projectairsim::Robot::GetBasicRadarConfig();
  auto id = radar_json["id"];
  const auto& is_enabled = radar_json["enabled"];
  const auto& parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.GetId(), std::string("ID123"));
  id = "radar123";
  radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.GetId(), std::string("radar123"));
}

TEST(Radar, SetsSensorType) {
  auto radar_json = projectairsim::Robot::GetBasicRadarConfig();
  auto id = radar_json["id"];
  const auto& is_enabled = radar_json["enabled"];
  const auto& parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.GetType(), projectairsim::SensorType::kRadar);
}

TEST(Radar, SetsIsEnabled) {
  auto radar_json = projectairsim::Robot::GetBasicRadarConfig();
  auto id = radar_json["id"];
  auto is_enabled = false;
  const auto& parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.IsEnabled(), false);
  is_enabled = true;
  radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.IsEnabled(), true);
}

TEST(Radar, SetsParentLink) {
  auto radar_json = projectairsim::Robot::GetBasicRadarConfig();
  auto id = radar_json["id"];
  const auto& is_enabled = false;
  auto parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.GetParentLink(), "ParentLink");
  parent_link = "ParentLink123";
  radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.GetParentLink(), "ParentLink123");
}

TEST(Radar, LoadsRadar) {
  auto radar_json = projectairsim::Robot::GetBasicRadarConfig();
  auto id = radar_json["id"];
  const auto& is_enabled = radar_json["enabled"];
  const auto& parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  projectairsim::Robot::LoadRadar(radar, radar_json);
  EXPECT_EQ(radar.IsLoaded(), true);
}

TEST(Radar, SetsIsLoaded) {
  auto radar_json = projectairsim::Robot::GetBasicRadarConfig();
  auto id = radar_json["id"];
  const auto& is_enabled = radar_json["enabled"];
  const auto& parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.IsLoaded(), false);
  projectairsim::Robot::LoadRadar(radar, radar_json);
  EXPECT_EQ(radar.IsLoaded(), true);
}

TEST(Radar, RadarSetting) {
  json radar_json = R"({
      "id": "ID123",
      "type": "radar",
      "enabled": true,
      "parent-link": "ParentLink",
      "fov": {
        "azimuth-max": 1.11,
        "azimuth-min": 1.12,
        "elevation-max": 2.21,
        "elevation-min": 2.22,
        "azimuth-resolution": 3.3,
        "elevation-resolution": 4.4
      },
      "range-max": 5.5,
      "range-resolution": 6.6,
      "velocity-max": 7.7,
      "velocity-resolution": 8.8,
      "detection-interval": 9.9,
      "track-interval": 10.1,
      "draw-debug-points": false,
      "origin": {},
      "masks": [
        {
          "azimuth-min": 0.1,
          "azimuth-max": 0.2,
          "elevation-min": 0.3,
          "elevation-max": 0.4,
          "range-min": 0.0,
          "range-max": 1000.0,
          "velocity-min": -15.0,
          "velocity-max": 15.0,
          "rcs-sqm-min": 0.01,
          "rcs-sqm-max": 100.0
        },
        {
          "azimuth-min": -0.1,
          "azimuth-max": -0.2,
          "elevation-min": -0.3,
          "elevation-max": -0.4,
          "range-min": 10.0,
          "range-max": 200.0,
          "velocity-min": -15.0,
          "velocity-max": 15.0,
          "rcs-sqm-min": 0.01,
          "rcs-sqm-max": 100.0
        }
      ]
    })"_json;
  auto id = radar_json["id"];
  const auto& is_enabled = radar_json["enabled"];
  const auto& parent_link = radar_json["parent-link"];
  auto radar = projectairsim::Robot::MakeRadar(id, is_enabled, parent_link);
  EXPECT_EQ(radar.IsLoaded(), false);
  projectairsim::Robot::LoadRadar(radar, radar_json);
  const auto& radar_settings = radar.GetRadarSettings();

  EXPECT_FLOAT_EQ(radar_settings.fov_azimuth_max, 1.11);
  EXPECT_FLOAT_EQ(radar_settings.fov_azimuth_min, 1.12);
  EXPECT_FLOAT_EQ(radar_settings.fov_elevation_max, 2.21);
  EXPECT_FLOAT_EQ(radar_settings.fov_elevation_min, 2.22);
  EXPECT_FLOAT_EQ(radar_settings.fov_azimuth_resolution, 3.3);
  EXPECT_FLOAT_EQ(radar_settings.fov_elevation_resolution, 4.4);

  EXPECT_FLOAT_EQ(radar_settings.range_max, 5.5);
  EXPECT_FLOAT_EQ(radar_settings.range_resolution, 6.6);
  EXPECT_FLOAT_EQ(radar_settings.velocity_max, 7.7);
  EXPECT_FLOAT_EQ(radar_settings.velocity_resolution, 8.8);
  EXPECT_FLOAT_EQ(radar_settings.detection_interval, 9.9);
  EXPECT_FLOAT_EQ(radar_settings.track_interval, 10.1);
  EXPECT_EQ(radar_settings.draw_debug_points, false);

  EXPECT_EQ(radar_settings.masks.size(), 2);

  EXPECT_FLOAT_EQ(radar_settings.masks[0].azimuth_min, 0.1);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].azimuth_max, 0.2);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].elevation_min, 0.3);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].elevation_max, 0.4);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].range_min, 0.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].range_max, 1000.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].velocity_min, -15.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].velocity_max, 15.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].rcs_sqm_min, 0.01);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].rcs_sqm_max, 100.0);

  EXPECT_FLOAT_EQ(radar_settings.masks[1].azimuth_min, -0.1);
  EXPECT_FLOAT_EQ(radar_settings.masks[1].azimuth_max, -0.2);
  EXPECT_FLOAT_EQ(radar_settings.masks[1].elevation_min, -0.3);
  EXPECT_FLOAT_EQ(radar_settings.masks[1].elevation_max, -0.4);
  EXPECT_FLOAT_EQ(radar_settings.masks[1].range_min, 10.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[1].range_max, 200.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].velocity_min, -15.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].velocity_max, 15.0);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].rcs_sqm_min, 0.01);
  EXPECT_FLOAT_EQ(radar_settings.masks[0].rcs_sqm_max, 100.0);
}
