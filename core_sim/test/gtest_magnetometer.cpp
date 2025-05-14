// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// Tests for Magnetometer sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/magnetometer.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static Magnetometer MakeMagnetometer(const std::string& id, bool is_enabled,
                                       const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/magnetometer_test_topic";
    return Magnetometer(id, is_enabled, parent_link, logger,
                        TopicManager(logger), parent_topic_path,
                        ServiceManager(logger), StateManager(logger));
  }

  static void LoadMagnetometer(Magnetometer& magnetometer,
                               ConfigJson config_json) {
    magnetometer.Load(config_json);
  }

  static nlohmann::json GetBasicMagnetometerConfig() {
    nlohmann::json config = R"({
      "id": "Magnetometer1",
      "type": "magnetometer",
      "enabled": true,
      "parent-link": "ParentLink"
    })"_json;
    return config;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(Magnetometer, SetsMagnetometerID) {
  const auto magnetometer_json =
      projectairsim::Robot::GetBasicMagnetometerConfig();
  const auto& id = magnetometer_json["id"];
  const auto& is_enabled = magnetometer_json["enabled"];
  const auto& parent_link = magnetometer_json["parent-link"];
  auto magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.GetId(), std::string("Magnetometer1"));
}

TEST(Magnetometer, SetsSensorType) {
  const auto magnetometer_json =
      projectairsim::Robot::GetBasicMagnetometerConfig();
  const auto& id = magnetometer_json["id"];
  const auto& is_enabled = magnetometer_json["enabled"];
  const auto& parent_link = magnetometer_json["parent-link"];
  auto magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.GetType(), projectairsim::SensorType::kMagnetometer);
}

TEST(Magnetometer, SetsIsEnabled) {
  const auto magnetometer_json =
      projectairsim::Robot::GetBasicMagnetometerConfig();
  const auto& id = magnetometer_json["id"];
  auto is_enabled = magnetometer_json["enabled"];
  const auto& parent_link = magnetometer_json["parent-link"];
  auto magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.IsEnabled(), true);
  is_enabled = false;
  magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.IsEnabled(), false);
}

TEST(Magnetometer, SetsParentLink) {
  const auto magnetometer_json =
      projectairsim::Robot::GetBasicMagnetometerConfig();
  const auto& id = magnetometer_json["id"];
  const auto& is_enabled = magnetometer_json["enabled"];
  auto parent_link = magnetometer_json["parent-link"];
  auto magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.GetParentLink(), std::string("ParentLink"));
  parent_link = std::string("NEW-PARENT-LINK-123");
  magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.GetParentLink(), parent_link.get<std::string>());
}

TEST(Magnetometer, LoadsMagnetometer) {
  const auto magnetometer_json =
      projectairsim::Robot::GetBasicMagnetometerConfig();
  const auto& id = magnetometer_json["id"];
  const auto& is_enabled = magnetometer_json["enabled"];
  const auto& parent_link = magnetometer_json["parent-link"];
  auto magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  projectairsim::Robot::LoadMagnetometer(magnetometer, magnetometer_json);
  EXPECT_EQ(magnetometer.IsLoaded(), true);
}

TEST(Magnetometer, SetsIsLoaded) {
  const auto magnetometer_json =
      projectairsim::Robot::GetBasicMagnetometerConfig();
  const auto& id = magnetometer_json["id"];
  const auto& is_enabled = magnetometer_json["enabled"];
  const auto& parent_link = magnetometer_json["parent-link"];
  auto magnetometer =
      projectairsim::Robot::MakeMagnetometer(id, is_enabled, parent_link);
  EXPECT_EQ(magnetometer.IsLoaded(), false);
  projectairsim::Robot::LoadMagnetometer(magnetometer, magnetometer_json);
  EXPECT_EQ(magnetometer.IsLoaded(), true);
}
