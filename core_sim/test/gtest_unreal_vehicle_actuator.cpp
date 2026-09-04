// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "actuators/actuator_impl.hpp"
#include "core_sim/actuators/unreal_vehicle.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {
namespace {

using json = nlohmann::json;

Logger MakeUnrealVehicleActuatorTestLogger() {
  auto logger_callback = [](const std::string&, LogLevel, const std::string&) {
  };
  return Logger(logger_callback);
}

std::unique_ptr<Actuator> LoadUnrealVehicleActuator(const json& config_json) {
  auto logger = MakeUnrealVehicleActuatorTestLogger();
  return ActuatorImpl::LoadActuator(
      config_json, logger, "UnrealVehicleActuatorTestRobot", "robot_0",
      TopicManager(logger), "/tests", ServiceManager(logger),
      StateManager(logger));
}

}  // namespace
}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(UnrealVehicleActuator, LoadsMappingAndSelectsControllerSignal) {
  const json config_json = R"({
    "name": "vehicle_steering",
    "type": "unreal-vehicle",
    "enabled": true,
    "unreal-vehicle-settings": {
      "parameter-index": 7,
      "control-signal-index": 1
    }
  })"_json;

  auto actuator = projectairsim::LoadUnrealVehicleActuator(config_json);
  ASSERT_NE(actuator, nullptr);
  ASSERT_EQ(actuator->GetType(), projectairsim::ActuatorType::kUnrealVehicle);

  auto& unreal_vehicle_actuator =
      static_cast<projectairsim::UnrealVehicleActuator&>(*actuator);
  EXPECT_EQ(unreal_vehicle_actuator.GetParameterIndex(), 7);
  EXPECT_EQ(unreal_vehicle_actuator.GetControlSignalIndex(), 1);

  unreal_vehicle_actuator.UpdateActuatorOutput({0.25f, -0.75f, 1.0f}, 0);
  EXPECT_FLOAT_EQ(unreal_vehicle_actuator.GetControlSignal(), -0.75f);
}

TEST(UnrealVehicleActuator, DefaultsToFirstControllerSignal) {
  const json config_json = R"({
    "name": "motor_1",
    "type": "unreal-vehicle",
    "enabled": true,
    "unreal-vehicle-settings": {
      "parameter-index": 3
    }
  })"_json;

  auto actuator = projectairsim::LoadUnrealVehicleActuator(config_json);
  auto& unreal_vehicle_actuator =
      static_cast<projectairsim::UnrealVehicleActuator&>(*actuator);

  unreal_vehicle_actuator.UpdateActuatorOutput({0.6f}, 0);
  EXPECT_EQ(unreal_vehicle_actuator.GetControlSignalIndex(), 0);
  EXPECT_FLOAT_EQ(unreal_vehicle_actuator.GetControlSignal(), 0.6f);
}

TEST(UnrealVehicleActuator, RejectsNegativeParameterIndex) {
  const json config_json = R"({
    "name": "invalid",
    "type": "unreal-vehicle",
    "enabled": true,
    "unreal-vehicle-settings": {
      "parameter-index": -1
    }
  })"_json;

  EXPECT_THROW(projectairsim::LoadUnrealVehicleActuator(config_json),
               projectairsim::Error);
}

TEST(UnrealVehicleActuator, RejectsMissingControllerSignal) {
  const json config_json = R"({
    "name": "invalid_signal",
    "type": "unreal-vehicle",
    "enabled": true,
    "unreal-vehicle-settings": {
      "parameter-index": 0,
      "control-signal-index": 2
    }
  })"_json;

  auto actuator = projectairsim::LoadUnrealVehicleActuator(config_json);
  auto& unreal_vehicle_actuator =
      static_cast<projectairsim::UnrealVehicleActuator&>(*actuator);

  EXPECT_THROW(unreal_vehicle_actuator.UpdateActuatorOutput({0.5f}, 0),
               projectairsim::Error);
}
