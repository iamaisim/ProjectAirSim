// Copyright (C) 2026 IAMAI CONSULTING CORP
// MIT License. All rights reserved.

#include "core_sim/clock.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "simple_flight_api.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft { namespace projectairsim {
class Scene {
 public:
  static Robot MakePathTestRobot() {
    Logger logger([](const std::string&, LogLevel, const std::string&) {});
    Robot robot("path_test", {{0, 0, 0}, {1, 0, 0, 0}}, logger,
                TopicManager(logger), "", ServiceManager(logger),
                StateManager(logger), "");
    auto config = nlohmann::json::parse(R"({"physics-type":"non-physics",
      "links":[{"name":"body"}],"controller":{"type":"simple-flight-api",
      "airframe-setup":"quadrotor-x"}})");
    robot.Load(config);
    return robot;
  }
};
}}  // namespace microsoft::projectairsim

namespace projectairsim = microsoft::projectairsim;
namespace {
class ManualClock : public projectairsim::ClockBase {
 public:
  TimeNano NowSimNanos() const override { return now; }
  TimeNano now = 0;
};

class PathFollower : public projectairsim::SimpleFlightApi {
 public:
  PathFollower(const projectairsim::Robot& robot, ManualClock& clock)
      : SimpleFlightApi(robot, nullptr), clock_(clock) {}
  projectairsim::Kinematics GetKinematicsEstimated() const override { return state; }
  projectairsim::Vector3 GetPosition() const override { return state.pose.position; }
  projectairsim::Kinematics state{};
  projectairsim::Vector3 last_velocity = projectairsim::Vector3::Zero();
  bool stalled = false;
  int cancel_after = 0;
  int commands = 0;

 protected:
  float GetCommandPeriod() const override { return 1.0f / 128; }
  float GetDistanceAccuracy() const override { return 0.001f; }
  float GetTakeoffZ() const override { return -2.0f; }
  void CommandVelocity(float x, float y, float z, bool, float) override {
    last_velocity = {x, y, z};
    if (!stalled) state.pose.position += last_velocity * GetCommandPeriod();
    clock_.now += 7'812'500;
    if (++commands == cancel_after) CancelLastTask();
  }
 private:
  ManualClock& clock_;
};

class PathCompletion : public ::testing::Test {
 protected:
  void SetUp() override { projectairsim::SimClock::Get(clock); }
  void TearDown() override {
    projectairsim::SimClock::Get(std::make_shared<projectairsim::RealTimeClock>());
  }
  std::shared_ptr<ManualClock> clock = std::make_shared<ManualClock>();
  projectairsim::Robot robot = projectairsim::Scene::MakePathTestRobot();
  PathFollower api{robot, *clock};
};

TEST_F(PathCompletion, TakeoffReachesAltitudeBeforeReportingSuccess) {
  EXPECT_TRUE(api.Takeoff(10.0f, 0));
  EXPECT_NEAR(api.state.pose.position.z(), -2.0f, 0.001f);
  EXPECT_GT(api.commands, 1);
  EXPECT_TRUE(api.last_velocity.isZero());
}

TEST_F(PathCompletion, PathShorterThanLookaheadStillMovesToDestination) {
  EXPECT_TRUE(api.MoveToPosition(0.05f, 0, 0, 0.5f, 10.0f,
      projectairsim::DrivetrainType::MaxDegreeOfFreedom, true, 0, 0.1f, 0, 0));
  EXPECT_NEAR(api.state.pose.position.x(), 0.05f, 0.001f);
  EXPECT_GT(api.commands, 1);
  EXPECT_TRUE(api.last_velocity.isZero());
}

TEST_F(PathCompletion, StalledVehicleTimesOutWithoutSuccess) {
  api.stalled = true;
  EXPECT_FALSE(api.Takeoff(1.0f, 0));
  EXPECT_TRUE(api.state.pose.position.isZero());
  EXPECT_GE(clock->now, 1'000'000'000);
}

TEST_F(PathCompletion, CancelledTakeoffDoesNotReportSuccess) {
  api.cancel_after = 5;
  EXPECT_FALSE(api.Takeoff(10.0f, 0));
  EXPECT_EQ(api.commands, 5);
  EXPECT_LT(clock->now, 1'000'000'000);
}
}  // namespace
