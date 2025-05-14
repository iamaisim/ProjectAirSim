// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// Tests for Lidar sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/lidar.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static Lidar MakeLidar(const std::string& id, bool is_enabled,
                         const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/lidar_test_topic";
    return Lidar(id, is_enabled, parent_link, logger, TopicManager(logger),
                 parent_topic_path, ServiceManager(logger),
                 StateManager(logger));
  }

  static void LoadLidar(Lidar& lidar, ConfigJson config_json) {
    lidar.Load(config_json);
  }

  static json GetBasiclidarConfig() {
    json config = R"({
      "id": "ID123",
      "type": "lidar",
      "enabled": true,
      "parent-link": "ParentLink",
      "number-of-channels": 16,
      "range": 100,
      "points-per-second": 100000,
      "horizontal-rotation-frequency": 10,
      "horizontal-fov-start-deg": 0.0,
      "horizontal-fov-end-deg": 360.0,
      "vertical-fov-upper-deg": 0.0,
      "vertical-fov-lower-deg": -90.0,
      "draw-debug-points": false,
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

TEST(Lidar, SetslidarID) {
  auto lidar_json = projectairsim::Robot::GetBasiclidarConfig();
  auto id = lidar_json["id"];
  const auto& is_enabled = lidar_json["enabled"];
  const auto& parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.GetId(), std::string("ID123"));
  id = "lidar123";
  lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.GetId(), std::string("lidar123"));
}

TEST(Lidar, SetsSensorType) {
  auto lidar_json = projectairsim::Robot::GetBasiclidarConfig();
  auto id = lidar_json["id"];
  const auto& is_enabled = lidar_json["enabled"];
  const auto& parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.GetType(), projectairsim::SensorType::kLidar);
}

TEST(Lidar, SetsIsEnabled) {
  auto lidar_json = projectairsim::Robot::GetBasiclidarConfig();
  auto id = lidar_json["id"];
  auto is_enabled = false;
  const auto& parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.IsEnabled(), false);
  is_enabled = true;
  lidar = projectairsim::Robot::MakeLidar("TestLidar", is_enabled, parent_link);
  EXPECT_EQ(lidar.IsEnabled(), true);
}

TEST(Lidar, SetsParentLink) {
  auto lidar_json = projectairsim::Robot::GetBasiclidarConfig();
  auto id = lidar_json["id"];
  const auto& is_enabled = false;
  auto parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.GetParentLink(), "ParentLink");
  parent_link = "ParentLink123";
  lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.GetParentLink(), "ParentLink123");
}

TEST(Lidar, LoadsLidar) {
  auto lidar_json = projectairsim::Robot::GetBasiclidarConfig();
  auto id = lidar_json["id"];
  const auto& is_enabled = lidar_json["enabled"];
  const auto& parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  projectairsim::Robot::LoadLidar(lidar, lidar_json);
  EXPECT_EQ(lidar.IsLoaded(), true);
}

TEST(Lidar, SetsIsLoaded) {
  auto lidar_json = projectairsim::Robot::GetBasiclidarConfig();
  auto id = lidar_json["id"];
  const auto& is_enabled = lidar_json["enabled"];
  const auto& parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.IsLoaded(), false);
  projectairsim::Robot::LoadLidar(lidar, lidar_json);
  EXPECT_EQ(lidar.IsLoaded(), true);
}

TEST(Lidar, LidarSetting) {
  json lidar_json = R"({
      "id": "ID123",
      "type": "lidar",
      "enabled": true,
      "parent-link": "ParentLink",
      "number-of-channels": 16,
      "range": 100,
      "points-per-second": 100000,
      "horizontal-rotation-frequency": 10,
      "horizontal-fov-start-deg": 0.0,
      "horizontal-fov-end-deg": 360.0,
      "vertical-fov-upper-deg": 0.0,
      "vertical-fov-lower-deg": -90.0,
      "draw-debug-points": false,
      "origin": {}
    })"_json;
  auto id = lidar_json["id"];
  const auto& is_enabled = lidar_json["enabled"];
  const auto& parent_link = lidar_json["parent-link"];
  auto lidar = projectairsim::Robot::MakeLidar(id, is_enabled, parent_link);
  EXPECT_EQ(lidar.IsLoaded(), false);
  projectairsim::Robot::LoadLidar(lidar, lidar_json);
  const auto& lidar_settings = lidar.GetLidarSettings();
  EXPECT_EQ(lidar_settings.number_of_channels, 16);
  EXPECT_FLOAT_EQ(lidar_settings.range, 100);
  EXPECT_EQ(lidar_settings.points_per_second, 100000);
  EXPECT_EQ(lidar_settings.horizontal_rotation_frequency, 10);
  EXPECT_FLOAT_EQ(lidar_settings.horizontal_fov_start_deg, 0);
  EXPECT_FLOAT_EQ(lidar_settings.horizontal_fov_end_deg, 360);
  EXPECT_FLOAT_EQ(lidar_settings.vertical_fov_upper_deg, 0);
  EXPECT_FLOAT_EQ(lidar_settings.vertical_fov_lower_deg, -90);
  EXPECT_EQ(lidar_settings.draw_debug_points, false);
}
