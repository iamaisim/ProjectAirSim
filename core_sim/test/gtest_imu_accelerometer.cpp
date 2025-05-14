// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// Tests for Accelerometer in IMU sensors

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

TEST(ImuAccelerometer, SetsVelocityRandomWalkValueByDefault) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_FLOAT_EQ(actual_imu_settings.accelerometer.velocity_random_walk,
                  0.002353596);
}
TEST(ImuAccelerometer, EnablesVelocityRandomWalkConfigParam) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  imu_json["accelerometer"] = R"({
    "velocity-random-walk": 0.0123
  })"_json;
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  projectairsim::Robot::LoadImu(imu, imu_json);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_FLOAT_EQ(actual_imu_settings.accelerometer.velocity_random_walk,
                  0.0123);
}

TEST(ImuAccelerometer, SetsTauByDefault) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_FLOAT_EQ(actual_imu_settings.accelerometer.tau, 800);
}

TEST(ImuAccelerometer, EnablesTauConfigParam) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  imu_json["accelerometer"] = R"({
    "tau": 123
  })"_json;
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  projectairsim::Robot::LoadImu(imu, imu_json);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_FLOAT_EQ(actual_imu_settings.accelerometer.tau, 123);
}

TEST(ImuAccelerometer, SetsBiasStabilityByDefault) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_FLOAT_EQ(actual_imu_settings.accelerometer.bias_stability,
                  0.00035303942);
}

TEST(ImuAccelerometer, EnablesBiasStabilityConfigParam) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  imu_json["accelerometer"] = R"({
    "bias-stability": 0.0123
  })"_json;
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  projectairsim::Robot::LoadImu(imu, imu_json);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_FLOAT_EQ(actual_imu_settings.accelerometer.bias_stability, 0.0123);
}

TEST(ImuAccelerometer, SetsTurnOnBiasByDefault) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_EQ(actual_imu_settings.accelerometer.turn_on_bias,
            projectairsim::Vector3(0, 0, 0));
}

TEST(ImuAccelerometer, EnablesTurnOnBiasConfigParam) {
  auto imu_json = projectairsim::Robot::GetBasicImuConfig();
  const auto& id = imu_json["id"];
  const auto& is_enabled = imu_json["enabled"];
  const auto& parent_link = imu_json["parent-link"];
  imu_json["accelerometer"] = R"({
    "turn-on-bias": "1 2 3"
  })"_json;
  auto imu = projectairsim::Robot::MakeImu(id, is_enabled, parent_link);
  projectairsim::Robot::LoadImu(imu, imu_json);
  auto actual_imu_settings = imu.GetImuSettings();
  EXPECT_EQ(actual_imu_settings.accelerometer.turn_on_bias,
            projectairsim::Vector3(1, 2, 3));
}
