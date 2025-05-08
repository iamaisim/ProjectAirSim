// Copyright (C) Microsoft Corporation.  All rights reserved.
// Tests for IMU sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/barometer.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static Barometer MakeBarometer(const std::string& id, bool is_enabled,
                                 const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/barometer_test_topic";
    return Barometer(id, is_enabled, parent_link, logger, TopicManager(logger),
                     parent_topic_path, ServiceManager(logger),
                     StateManager(logger));
  }

  static void LoadBarometer(Barometer& barometer, ConfigJson config_json) {
    barometer.Load(config_json);
  }

  static json GetBasicBarometerConfig() {
    json config = R"({
      "id": "Barometer1",
      "type": "barometer",
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

TEST(Barometer, SetsBarometerID) {
  const auto barometer_json = projectairsim::Robot::GetBasicBarometerConfig();
  const auto& id = barometer_json["id"];
  const auto& is_enabled = barometer_json["enabled"];
  const auto& parent_link = barometer_json["parent-link"];
  auto barometer =
      projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.GetId(), std::string("Barometer1"));
}

TEST(Barometer, SetsSensorType) {
  const auto barometer_json = projectairsim::Robot::GetBasicBarometerConfig();
  const auto& id = barometer_json["id"];
  const auto& is_enabled = barometer_json["enabled"];
  const auto& parent_link = barometer_json["parent-link"];
  auto barometer =
      projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.GetType(), projectairsim::SensorType::kBarometer);
}

TEST(Barometer, SetsIsEnabled) {
  const auto barometer_json = projectairsim::Robot::GetBasicBarometerConfig();
  const auto& id = barometer_json["id"];
  auto is_enabled = barometer_json["enabled"];
  const auto& parent_link = barometer_json["parent-link"];
  auto barometer =
      projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.IsEnabled(), true);
  is_enabled = false;
  barometer = projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.IsEnabled(), false);
}

TEST(Barometer, SetsParentLink) {
  const auto barometer_json = projectairsim::Robot::GetBasicBarometerConfig();
  const auto& id = barometer_json["id"];
  const auto& is_enabled = barometer_json["enabled"];
  auto parent_link = barometer_json["parent-link"];
  auto barometer =
      projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.GetParentLink(), std::string("ParentLink"));
  parent_link = std::string("NEW-PARENT-LINK-123");
  barometer = projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.GetParentLink(), parent_link.get<std::string>());
}

TEST(Barometer, LoadsBarometer) {
  const auto barometer_json = projectairsim::Robot::GetBasicBarometerConfig();
  const auto& id = barometer_json["id"];
  const auto& is_enabled = barometer_json["enabled"];
  const auto& parent_link = barometer_json["parent-link"];
  auto barometer =
      projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  projectairsim::Robot::LoadBarometer(barometer, barometer_json);
  EXPECT_EQ(barometer.IsLoaded(), true);
}

TEST(Barometer, SetsIsLoaded) {
  const auto barometer_json = projectairsim::Robot::GetBasicBarometerConfig();
  const auto& id = barometer_json["id"];
  const auto& is_enabled = barometer_json["enabled"];
  const auto& parent_link = barometer_json["parent-link"];
  auto barometer =
      projectairsim::Robot::MakeBarometer(id, is_enabled, parent_link);
  EXPECT_EQ(barometer.IsLoaded(), false);
  projectairsim::Robot::LoadBarometer(barometer, barometer_json);
  EXPECT_EQ(barometer.IsLoaded(), true);
}
