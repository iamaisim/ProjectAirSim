// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <memory>
#include <string>

#include "actuators/actuator_impl.hpp"
#include "core_sim/actuators/gimbal.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {
namespace {

Logger MakeLogger() {
  auto logger_callback = [](const std::string&, LogLevel, const std::string&) {};
  return Logger(logger_callback);
}

std::unique_ptr<Actuator> LoadActuatorFromJson(const json& config_json) {
  auto logger = MakeLogger();
  return ActuatorImpl::LoadActuator(config_json, logger, "GimbalTestRobot",
                                    "robot_0", TopicManager(logger), "/tests",
                                    ServiceManager(logger), StateManager(logger));
}

void ExpectRotationMatches(const Matrix3x3& actual, const Matrix3x3& expected) {
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      EXPECT_NEAR(actual(row, col), expected(row, col), 1.0e-6f);
    }
  }
}

}  // namespace
}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(Gimbal, LoadAppliesOriginRotationToState) {
  // General description:
  // Verifies that gimbal load applies origin rotation to internal state.
  // Arrange: prepare context for `const json config_json = R"({`.
  const json config_json = R"({
    "name": "gimbal_0",
    "type": "gimbal",
    "enabled": true,
    "parent-link": "body",
    "child-link": "camera_mount",
    "origin": {
      "xyz": "0 0 0",
      "rpy-deg": "10 -20 30"
    }
  })"_json;

  // Act: run `auto actuator = projectairsim::LoadActuatorFromJson(config_json);`.
  auto actuator = projectairsim::LoadActuatorFromJson(config_json);
  ASSERT_NE(actuator, nullptr);
  ASSERT_EQ(actuator->GetType(), projectairsim::ActuatorType::kGimbal);

  auto* gimbal = dynamic_cast<projectairsim::Gimbal*>(actuator.get());
  ASSERT_NE(gimbal, nullptr);
  EXPECT_TRUE(gimbal->IsLoaded());

  // Assert: check result from `const auto& state = gimbal->GetGimbalState();`.
  const auto& state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.roll, projectairsim::TransformUtils::ToRadians(10.0f), 1.0e-6f);
  EXPECT_NEAR(state.pitch, projectairsim::TransformUtils::ToRadians(-20.0f), 1.0e-6f);
  EXPECT_NEAR(state.yaw, projectairsim::TransformUtils::ToRadians(30.0f), 1.0e-6f);
}

TEST(Gimbal, UpdateGimbalUsesRateLimitedSetpointsAndUpdatesTransform) {
  // General description:
  // Verifies setpoint update with velocity limits and final transform update.
  // Arrange: prepare context for `const json config_json = R"({`.
  const json config_json = R"({
    "name": "gimbal_1",
    "type": "gimbal",
    "enabled": true,
    "parent-link": "body",
    "child-link": "camera_mount",
    "origin": {
      "xyz": "0 0 0",
      "rpy-deg": "0 0 0"
    }
  })"_json;

  auto actuator = projectairsim::LoadActuatorFromJson(config_json);
  auto* gimbal = dynamic_cast<projectairsim::Gimbal*>(actuator.get());
  ASSERT_NE(gimbal, nullptr);

  projectairsim::GimbalCommand command;
  command.roll = 1.0f;
  command.pitch = -1.0f;
  command.yaw = 0.25f;
  command.roll_rate = 0.5f;
  command.pitch_rate = -0.25f;
  command.yaw_rate = NAN;
  command.roll_lock = true;
  command.pitch_lock = false;
  command.yaw_lock = true;

  // Act: apply the protocol-independent command and advance one second.
  gimbal->SetCommand(command);
  gimbal->UpdateActuatorOutput({}, 1'000'000'000LL);

  // Assert: check result from `const auto& state = gimbal->GetGimbalState();`.
  const auto& state = gimbal->GetGimbalState();
  EXPECT_FLOAT_EQ(state.roll, 0.5f);
  EXPECT_FLOAT_EQ(state.pitch, -0.25f);
  EXPECT_FLOAT_EQ(state.yaw, 0.25f);
  EXPECT_FLOAT_EQ(state.roll_rate, 0.5f);
  EXPECT_FLOAT_EQ(state.pitch_rate, -0.25f);
  EXPECT_FLOAT_EQ(state.yaw_rate, 0.25f);
  EXPECT_TRUE(state.roll_lock);
  EXPECT_FALSE(state.pitch_lock);
  EXPECT_TRUE(state.yaw_lock);

  // Assert: check result from `const auto& transforms = gimbal->GetActuatedTransforms();`.
  const auto& transforms = gimbal->GetActuatedTransforms();
  const auto transform_it = transforms.find("camera_mount");
  ASSERT_NE(transform_it, transforms.end());

  const auto expected_quaternion =
      projectairsim::TransformUtils::ToQuaternion(0.5f, -0.25f, 0.25f);
  projectairsim::ExpectRotationMatches(transform_it->second.affine3.linear(),
                                       expected_quaternion.toRotationMatrix());
}

TEST(Gimbal, RateOnlyCommandIntegratesUntilStopped) {
  const json config_json = R"({
    "name": "gimbal_2",
    "type": "gimbal",
    "enabled": true,
    "parent-link": "body",
    "child-link": "camera_mount",
    "origin": {
      "xyz": "0 0 0",
      "rpy-deg": "0 0 0"
    }
  })"_json;

  auto actuator = projectairsim::LoadActuatorFromJson(config_json);
  auto* gimbal = dynamic_cast<projectairsim::Gimbal*>(actuator.get());
  ASSERT_NE(gimbal, nullptr);

  projectairsim::GimbalCommand command;
  command.yaw_rate = 0.4f;
  gimbal->SetCommand(command);

  gimbal->UpdateActuatorOutput({}, 500'000'000LL);
  auto state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.yaw, 0.2f, 1.0e-6f);
  EXPECT_NEAR(state.yaw_rate, 0.4f, 1.0e-6f);

  gimbal->UpdateActuatorOutput({}, 500'000'000LL);
  state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.yaw, 0.4f, 1.0e-6f);
  EXPECT_NEAR(state.yaw_rate, 0.4f, 1.0e-6f);

  // A command without an angle or rate holds the current position.
  gimbal->SetCommand(projectairsim::GimbalCommand());
  gimbal->UpdateActuatorOutput({}, 500'000'000LL);
  state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.yaw, 0.4f, 1.0e-6f);
  EXPECT_FLOAT_EQ(state.yaw_rate, 0.0f);
}

TEST(Gimbal, FaultInjectionHoldsPositionAndZerosRates) {
  const json config_json = {
      {"name", "gimbal_fault"},
      {"type", "gimbal"},
      {"enabled", true},
      {"parent-link", "body"},
      {"child-link", "camera_mount"},
  };

  auto actuator = projectairsim::LoadActuatorFromJson(config_json);
  auto* gimbal = dynamic_cast<projectairsim::Gimbal*>(actuator.get());
  ASSERT_NE(gimbal, nullptr);

  projectairsim::GimbalCommand command;
  command.yaw_rate = 0.4f;
  gimbal->SetCommand(command);
  gimbal->UpdateActuatorOutput({}, 500'000'000LL);
  auto state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.yaw, 0.2f, 1.0e-6f);
  EXPECT_NEAR(state.yaw_rate, 0.4f, 1.0e-6f);

  EXPECT_TRUE(gimbal->UpdateFaultInjectionEnabledState(true));
  gimbal->UpdateActuatorOutput({}, 1'000'000'000LL);
  state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.yaw, 0.2f, 1.0e-6f);
  EXPECT_FLOAT_EQ(state.yaw_rate, 0.0f);

  EXPECT_TRUE(gimbal->UpdateFaultInjectionEnabledState(false));
  gimbal->UpdateActuatorOutput({}, 500'000'000LL);
  state = gimbal->GetGimbalState();
  EXPECT_NEAR(state.yaw, 0.4f, 1.0e-6f);
  EXPECT_NEAR(state.yaw_rate, 0.4f, 1.0e-6f);
}
