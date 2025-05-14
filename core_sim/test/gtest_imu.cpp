// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// Tests for IMU sensors

#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/imu.hpp"
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
  static Imu MakeImu(const std::string& id, bool is_enabled,
                     const std::string& parent_link) {
    auto logger_callback = [](const std::string& component, LogLevel level,
                              const std::string& message) {};
    Logger logger(logger_callback);
    const std::string& parent_topic_path = "/imu_test_topic";
    return Imu(id, is_enabled, parent_link, logger, TopicManager(logger),
               parent_topic_path, ServiceManager(logger), StateManager(logger));
  }

  static void LoadImu(Imu& imu, ConfigJson config_json) {
    imu.Load(config_json);
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
using json = nlohmann::json;

TEST(Imu, SetsImuID) {
  const auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  EXPECT_EQ(imu.GetId(), std::string("IMU123"));
}

TEST(Imu, SetsSensorType) {
  const auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  EXPECT_EQ(imu.GetType(), projectairsim::SensorType::kImu);
}

TEST(Imu, SetsIsEnabled) {
  const auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  auto is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  EXPECT_EQ(imu.IsEnabled(), true);
  is_enabled = false;
  imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  EXPECT_EQ(imu.IsEnabled(), false);
}

TEST(Imu, SetsParentLink) {
  const auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  auto parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled,
                                           parent_link.get<std::string>());
  EXPECT_EQ(imu.GetParentLink(), std::string("ParentLink"));
  parent_link = std::string("NEW-PARENT-LINK-123");
  imu = projectairsim::Robot::MakeImu(id, is_enabled,
                                      parent_link.get<std::string>());
  EXPECT_EQ(imu.GetParentLink(), parent_link.get<std::string>());
}

TEST(Imu, LoadsImu) {
  const auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  projectairsim::Robot::LoadImu(imu, imu_json);
  EXPECT_EQ(imu.IsLoaded(), true);
}

TEST(Imu, SetsIsLoaded) {
  const auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  EXPECT_EQ(imu.IsLoaded(), false);
  projectairsim::Robot::LoadImu(imu, imu_json);
  EXPECT_EQ(imu.IsLoaded(), true);
}
