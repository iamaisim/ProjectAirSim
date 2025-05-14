// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// Tests for IMU sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/airspeed.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static AirspeedSensor MakeAirspeed(const std::string& id, bool is_enabled,
                                     const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/airspeed_test_topic";
    return AirspeedSensor(id, is_enabled, parent_link, logger,
                          TopicManager(logger), parent_topic_path,
                          ServiceManager(logger), StateManager(logger));
  }

  static void LoadAirspeed(AirspeedSensor& airspeedsensor,
                           ConfigJson config_json) {
    airspeedsensor.Load(config_json);
  }

  static json GetBasicAirspeedConfig() {
    json config = R"({
      "id": "Airspeed1",
      "type": "airspeed",
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

TEST(Airspeed, SetsAirspeedID) {
  const auto airspeed_json = projectairsim::Robot::GetBasicAirspeedConfig();
  const auto& id = airspeed_json["id"];
  const auto& is_enabled = airspeed_json["enabled"];
  const auto& parent_link = airspeed_json["parent-link"];
  auto airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.GetId(), std::string("Airspeed1"));
}

TEST(Airspeed, SetsSensorType) {
  const auto airspeed_json = projectairsim::Robot::GetBasicAirspeedConfig();
  const auto& id = airspeed_json["id"];
  const auto& is_enabled = airspeed_json["enabled"];
  const auto& parent_link = airspeed_json["parent-link"];
  auto airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.GetType(), projectairsim::SensorType::kAirspeed);
}

TEST(Airspeed, SetsIsEnabled) {
  const auto airspeed_json = projectairsim::Robot::GetBasicAirspeedConfig();
  const auto& id = airspeed_json["id"];
  auto is_enabled = airspeed_json["enabled"];
  const auto& parent_link = airspeed_json["parent-link"];
  auto airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.IsEnabled(), true);
  is_enabled = false;
  airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.IsEnabled(), false);
}

TEST(Airspeed, SetsParentLink) {
  const auto airspeed_json = projectairsim::Robot::GetBasicAirspeedConfig();
  const auto& id = airspeed_json["id"];
  const auto& is_enabled = airspeed_json["enabled"];
  auto parent_link = airspeed_json["parent-link"];
  auto airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.GetParentLink(), std::string("ParentLink"));
  parent_link = std::string("NEW-PARENT-LINK-123");
  airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.GetParentLink(), parent_link.get<std::string>());
}

TEST(Airspeed, LoadsAirspeed) {
  const auto airspeed_json = projectairsim::Robot::GetBasicAirspeedConfig();
  const auto& id = airspeed_json["id"];
  const auto& is_enabled = airspeed_json["enabled"];
  const auto& parent_link = airspeed_json["parent-link"];
  auto airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  projectairsim::Robot::LoadAirspeed(airspeed, airspeed_json);
  EXPECT_EQ(airspeed.IsLoaded(), true);
}

TEST(Airspeed, SetsIsLoaded) {
  const auto airspeed_json = projectairsim::Robot::GetBasicAirspeedConfig();
  const auto& id = airspeed_json["id"];
  const auto& is_enabled = airspeed_json["enabled"];
  const auto& parent_link = airspeed_json["parent-link"];
  auto airspeed = projectairsim::Robot::MakeAirspeed(id, is_enabled, parent_link);
  EXPECT_EQ(airspeed.IsLoaded(), false);
  projectairsim::Robot::LoadAirspeed(airspeed, airspeed_json);
  EXPECT_EQ(airspeed.IsLoaded(), true);
}
