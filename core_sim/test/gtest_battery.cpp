// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <string>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/battery.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static Robot MakeRobot(const std::string& id) {
    auto logger_callback = [](const std::string&, LogLevel,
                              const std::string&) {};
    Logger logger(logger_callback);
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    return Robot(id, origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger), "");
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }

  static json GetSimpleDischargeBatteryConfig() {
    return R"({
      "links": [{"name": "Frame"}],
      "sensors": [{
        "id": "BAT1",
        "type": "battery",
        "enabled": true,
        "parent-link": "Frame",
        "battery-mode": "simple-discharge-mode",
        "total-battery-capacity": 100.0,
        "battery-capacity-on-start": 100.0,
        "battery-drain-rate-on-start": 10.0
      }]
    })"_json;
  }

  static json GetEnergyConsumptionBatteryConfig() {
    return R"({
      "links": [{"name": "Frame"}],
      "sensors": [{
        "id": "BAT2",
        "type": "battery",
        "enabled": true,
        "parent-link": "Frame",
        "battery-mode": "rotor-power-discharge-mode",
        "total-battery-capacity": 100.0,
        "battery-capacity-on-start": 100.0,
        "rotor-power-coefficient": 1.0
      }]
    })"_json;
  }

  static Battery* GetBatteryFromRobot(Robot& robot) {
    auto& sensors = robot.GetSensors();
    if (sensors.empty()) {
      return nullptr;
    }

    for (auto& sensor_ref : sensors) {
      if (sensor_ref.get().GetType() == SensorType::kBattery) {
        return dynamic_cast<Battery*>(&sensor_ref.get());
      }
    }

    return nullptr;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(BatterySensor, LoadsBatterySensor) {
  // General description:
  // Verifies that a robot correctly loads a battery sensor from JSON.
  // Arrange: prepare context for `auto robot = projectairsim::Scene::MakeRobot("TestRobot");`.
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  auto config_json = projectairsim::Scene::GetSimpleDischargeBatteryConfig();

  // Act: run `projectairsim::Scene::LoadRobot(robot, config_json);`.
  projectairsim::Scene::LoadRobot(robot, config_json);

  // Assert: check result from `auto* battery = projectairsim::Scene::GetBatteryFromRobot(robot);`.
  auto* battery = projectairsim::Scene::GetBatteryFromRobot(robot);
  ASSERT_NE(battery, nullptr);
  EXPECT_EQ(battery->GetType(), projectairsim::SensorType::kBattery);
  EXPECT_TRUE(battery->IsLoaded());
}

TEST(BatterySensor, SetBatteryRemainingValidatesInputRange) {
  // General description:
  // Verifies input-range validation when updating battery remaining percent.
  // Arrange: prepare context for `auto robot = projectairsim::Scene::MakeRobot("TestRobot");`.
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  auto config_json = projectairsim::Scene::GetSimpleDischargeBatteryConfig();
  projectairsim::Scene::LoadRobot(robot, config_json);

  auto* battery = projectairsim::Scene::GetBatteryFromRobot(robot);
  // Assert: check result from `ASSERT_NE(battery, nullptr);`.
  ASSERT_NE(battery, nullptr);

  // Act: run `EXPECT_TRUE(battery->SetBatteryRemaining(42.0f));`.
  EXPECT_TRUE(battery->SetBatteryRemaining(42.0f));
  EXPECT_FALSE(battery->SetBatteryRemaining(-1.0f));
  EXPECT_FALSE(battery->SetBatteryRemaining(101.0f));
}

TEST(BatterySensor, DrainRateCanBeSetInSimpleDischargeMode) {
  // General description:
  // Verifies that drain rate can be modified in simple-discharge mode.
  // Arrange: prepare context for `auto robot = projectairsim::Scene::MakeRobot("TestRobot");`.
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  auto config_json = projectairsim::Scene::GetSimpleDischargeBatteryConfig();
  projectairsim::Scene::LoadRobot(robot, config_json);

  auto* battery = projectairsim::Scene::GetBatteryFromRobot(robot);
  // Assert: check result from `ASSERT_NE(battery, nullptr);`.
  ASSERT_NE(battery, nullptr);

  // Act: run `EXPECT_TRUE(battery->SetBatteryDrainRate(5.0f));`.
  EXPECT_TRUE(battery->SetBatteryDrainRate(5.0f));
  EXPECT_FLOAT_EQ(battery->GetBatteryDrainRate(), 5.0f);
}

TEST(BatterySensor, DrainRateCannotBeSetInEnergyConsumptionMode) {
  // General description:
  // Verifies that drain rate cannot be set in energy-consumption mode.
  // Arrange: prepare context for `auto robot = projectairsim::Scene::MakeRobot("TestRobot");`.
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  auto config_json = projectairsim::Scene::GetEnergyConsumptionBatteryConfig();
  projectairsim::Scene::LoadRobot(robot, config_json);

  auto* battery = projectairsim::Scene::GetBatteryFromRobot(robot);
  // Assert: check result from `ASSERT_NE(battery, nullptr);`.
  ASSERT_NE(battery, nullptr);

  // Act: run `EXPECT_FALSE(battery->SetBatteryDrainRate(5.0f));`.
  EXPECT_FALSE(battery->SetBatteryDrainRate(5.0f));
}

TEST(BatterySensor, UpdateInSimpleModeReducesRemainingCharge) {
  // General description:
  // Verifies that one update cycle reduces charge and updates derived state.
  // Arrange: prepare context for `auto robot = projectairsim::Scene::MakeRobot("TestRobot");`.
  auto robot = projectairsim::Scene::MakeRobot("TestRobot");
  auto config_json = projectairsim::Scene::GetSimpleDischargeBatteryConfig();
  projectairsim::Scene::LoadRobot(robot, config_json);

  auto* battery = projectairsim::Scene::GetBatteryFromRobot(robot);
  ASSERT_NE(battery, nullptr);

  // Act: run `battery->BeginUpdate();`.
  battery->BeginUpdate();
  battery->Update(1000000000LL, 1000000000LL);
  battery->EndUpdate();

  // Assert: check result from `const auto data = battery->GetState().getData();`.
  const auto data = battery->GetState().getData();
  EXPECT_FLOAT_EQ(data.at("battery_pct_remaining").get<float>(), 90.0f);
  EXPECT_EQ(data.at("estimated_time_remaining").get<uint32_t>(), 9U);
  EXPECT_EQ(data.at("battery_charge_state").get<std::string>(),
            "BATTERY_CHARGE_STATE_OK");
}
