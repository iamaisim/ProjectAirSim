// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <memory>
#include <string>
#include <vector>

#include "actuators/actuator_impl.hpp"
#include "core_sim/actuators/tilt.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
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
  return ActuatorImpl::LoadActuator(config_json, logger, "TiltTestRobot",
                                    "robot_0", TopicManager(logger), "/tests",
                                    ServiceManager(logger), StateManager(logger));
}

}  // namespace
}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

TEST(Tilt, LoadPopulatesSettingsAndNormalizesAxis) {
  // General description:
  // Verifies tilt configuration load and axis normalization.
  // Arrange: prepare context for `const json config_json = R"({`.
  const json config_json = R"({
    "name": "tilt_0",
    "type": "tilt",
    "enabled": true,
    "parent-link": "body",
    "child-link": "tilt_link",
    "tilt-settings": {
      "angle-min": -0.5,
      "angle-max": 1.0,
      "smoothing-tc": 0.0,
      "axis": "0 0 2",
      "target": "rotor_0"
    }
  })"_json;

  // Act: run `auto actuator = projectairsim::LoadActuatorFromJson(config_json);`.
  auto actuator = projectairsim::LoadActuatorFromJson(config_json);
  ASSERT_NE(actuator, nullptr);
  ASSERT_EQ(actuator->GetType(), projectairsim::ActuatorType::kTilt);

  auto* tilt = dynamic_cast<projectairsim::Tilt*>(actuator.get());
  ASSERT_NE(tilt, nullptr);
  EXPECT_TRUE(tilt->IsLoaded());
  EXPECT_EQ(tilt->GetTargetID(), "rotor_0");

  // Assert: check result from `const auto& settings = tilt->GetSettings();`.
  const auto& settings = tilt->GetSettings();
  EXPECT_FLOAT_EQ(settings.radians_min, -0.5f);
  EXPECT_FLOAT_EQ(settings.dradians, 1.5f);
  EXPECT_FLOAT_EQ(settings.smoothing_tc, 0.0f);
  EXPECT_FLOAT_EQ(settings.vec3_axis.x(), 0.0f);
  EXPECT_FLOAT_EQ(settings.vec3_axis.y(), 0.0f);
  EXPECT_FLOAT_EQ(settings.vec3_axis.z(), 1.0f);
}

TEST(Tilt, UpdateActuatorOutputUpdatesRotationAndTransform) {
  // General description:
  // Verifies that UpdateActuatorOutput updates rotation and actuated transform.
  // Arrange: prepare context for `const json config_json = R"({`.
  const json config_json = R"({
    "name": "tilt_1",
    "type": "tilt",
    "enabled": true,
    "parent-link": "body",
    "child-link": "tilt_link",
    "tilt-settings": {
      "angle-min": -0.5,
      "angle-max": 0.5,
      "smoothing-tc": 0.0,
      "axis": "0 1 0",
      "target": "rotor_1"
    }
  })"_json;

  auto actuator = projectairsim::LoadActuatorFromJson(config_json);
  auto* tilt = dynamic_cast<projectairsim::Tilt*>(actuator.get());
  ASSERT_NE(tilt, nullptr);

  // Act: update the tilt with a scalar control signal.
  projectairsim::Actuator::ControlSignals control_signals = {0.25f};
  tilt->UpdateActuatorOutput(control_signals, 200'000'000LL);

  // Assert: check result from `const auto& control_rotation = tilt->GetControlRotation();`.
  const auto& control_rotation = tilt->GetControlRotation();
  const projectairsim::Quaternion expected(
      projectairsim::AngleAxis(-0.25f, projectairsim::Vector3(0.0f, 1.0f, 0.0f)));
  EXPECT_NEAR(control_rotation.w(), expected.w(), 1.0e-6f);
  EXPECT_NEAR(control_rotation.x(), expected.x(), 1.0e-6f);
  EXPECT_NEAR(control_rotation.y(), expected.y(), 1.0e-6f);
  EXPECT_NEAR(control_rotation.z(), expected.z(), 1.0e-6f);

  // Assert: check result from `const auto& transforms = tilt->GetActuatedTransforms();`.
  const auto& transforms = tilt->GetActuatedTransforms();
  const auto transform_it = transforms.find("tilt_link");
  ASSERT_NE(transform_it, transforms.end());

  const auto expected_rotation = expected.toRotationMatrix();
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      EXPECT_NEAR(transform_it->second.affine3.linear()(row, col),
                  expected_rotation(row, col), 1.0e-6f);
    }
  }
}

TEST(Tilt, EmptyTargetIsRejected) {
  // General description:
  // Verifies validation for invalid configuration with empty target.
  // Arrange: prepare context for `const json config_json = R"({`.
  const json config_json = R"({
    "name": "tilt_invalid",
    "type": "tilt",
    "enabled": true,
    "parent-link": "body",
    "child-link": "tilt_link",
    "tilt-settings": {
      "target": ""
    }
  })"_json;

  // Act: run `EXPECT_THROW(static_cast<void>(projectairsim::LoadActuatorFromJson(config_jso...`.
  // Assert: check result from `EXPECT_THROW(static_cast<void>(projectairsim::LoadActuatorFromJson(config_jso...`.
  EXPECT_THROW(static_cast<void>(projectairsim::LoadActuatorFromJson(config_json)),
               projectairsim::Error);
}
