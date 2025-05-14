// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// Tests for IMU sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/gps.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static Gps MakeGps(const std::string& id, bool is_enabled,
                     const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/gps_test_topic";
    return Gps(id, is_enabled, parent_link, logger, TopicManager(logger),
               parent_topic_path, ServiceManager(logger), StateManager(logger));
  }

  static void LoadGps(Gps& gps, ConfigJson config_json) {
    gps.Load(config_json);
  }

  static json GetBasicGpsConfig() {
    json config = R"({
      "id": "Gps1",
      "type": "gps",
      "enabled": true,
      "parent-link": "ParentLink"
    })"_json;
    return config;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(Gps, SetsGpsID) {
  const auto gps_json = projectairsim::Robot::GetBasicGpsConfig();
  const auto& id = gps_json["id"];
  const auto& is_enabled = gps_json["enabled"];
  const auto& parent_link = gps_json["parent-link"];
  auto gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.GetId(), std::string("Gps1"));
}

TEST(Gps, SetsSensorType) {
  const auto gps_json = projectairsim::Robot::GetBasicGpsConfig();
  const auto& id = gps_json["id"];
  const auto& is_enabled = gps_json["enabled"];
  const auto& parent_link = gps_json["parent-link"];
  auto gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.GetType(), projectairsim::SensorType::kGps);
}

TEST(Gps, SetsIsEnabled) {
  const auto gps_json = projectairsim::Robot::GetBasicGpsConfig();
  const auto& id = gps_json["id"];
  auto is_enabled = gps_json["enabled"];
  const auto& parent_link = gps_json["parent-link"];
  auto gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.IsEnabled(), true);
  is_enabled = false;
  gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.IsEnabled(), false);
}

TEST(Gps, SetsParentLink) {
  const auto gps_json = projectairsim::Robot::GetBasicGpsConfig();
  const auto& id = gps_json["id"];
  const auto& is_enabled = gps_json["enabled"];
  auto parent_link = gps_json["parent-link"];
  auto gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.GetParentLink(), std::string("ParentLink"));
  parent_link = std::string("NEW-PARENT-LINK-123");
  gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.GetParentLink(), parent_link.get<std::string>());
}

TEST(Gps, LoadsGps) {
  const auto gps_json = projectairsim::Robot::GetBasicGpsConfig();
  const auto& id = gps_json["id"];
  const auto& is_enabled = gps_json["enabled"];
  const auto& parent_link = gps_json["parent-link"];
  auto gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  projectairsim::Robot::LoadGps(gps, gps_json);
  EXPECT_EQ(gps.IsLoaded(), true);
}

TEST(Gps, SetsIsLoaded) {
  const auto gps_json = projectairsim::Robot::GetBasicGpsConfig();
  const auto& id = gps_json["id"];
  const auto& is_enabled = gps_json["enabled"];
  const auto& parent_link = gps_json["parent-link"];
  auto gps = projectairsim::Robot::MakeGps(id, is_enabled, parent_link);
  EXPECT_EQ(gps.IsLoaded(), false);
  projectairsim::Robot::LoadGps(gps, gps_json);
  EXPECT_EQ(gps.IsLoaded(), true);
}
