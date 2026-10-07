// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP
// MIT License. All rights reserved.

#include <memory>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "simple_drive/simple_drive_api.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

// Robot construction/loading is private and grants friendship to Scene.
class Scene {
 public:
  static Robot MakeSimpleDriveRobot() {
    Logger logger([](const std::string&, LogLevel, const std::string&) {});
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    Robot robot("SnapshotRover", origin, logger, TopicManager(logger), "",
                ServiceManager(logger), StateManager(logger), "");
    const nlohmann::json config = {
        {"physics-type", "unreal-physics"},
        {"wheeled-vehicle-class", "/Game/Vehicles/Test.Test_C"},
        {"controller", {{"id", "SimpleDrive"},
                         {"type", "simple-drive-api"},
                         {"vehicle-setup", "ackermann"}}}};
    robot.Load(config);
    return robot;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

namespace {

class SimpleDriveSnapshotTest : public ::testing::Test {
 protected:
  void SetUp() override {
    robot_ = projectairsim::Scene::MakeSimpleDriveRobot();
    api_ = std::make_unique<projectairsim::simple_drive::SimpleDriveApi>(
        robot_, nullptr);
    api_->BeginUpdate();
    ASSERT_TRUE(api_->EnableApiControl());
  }

  void TearDown() override { api_->EndUpdate(); }

  void Drive() {
    ASSERT_TRUE(api_->Arm(0));
    ASSERT_TRUE(api_->SetRoverControls(0.7f, -0.25f, 0.1f));
    api_->Update();
  }

  projectairsim::Robot robot_;
  std::unique_ptr<projectairsim::simple_drive::SimpleDriveApi> api_;
};

TEST_F(SimpleDriveSnapshotTest, ArmedSnapshotContainsAllThreeChannelsInOrder) {
  Drive();
  std::vector<float> snapshot(8, 99.0f);
  api_->GetControlSignalSnapshot(snapshot);
  ASSERT_EQ(snapshot.size(), 3u);
  EXPECT_FLOAT_EQ(snapshot[0], 0.7f);
  EXPECT_FLOAT_EQ(snapshot[1], -0.25f);
  EXPECT_FLOAT_EQ(snapshot[2], 0.1f);

  for (const auto* wheel : {"front-left", "rear-right"}) {
    for (size_t offset = 0; offset < snapshot.size(); ++offset) {
      const int index = api_->GetControlSignalIndex(wheel, offset);
      EXPECT_EQ(index, static_cast<int>(offset));
      EXPECT_EQ(api_->GetControlSignals(index),
                (std::vector<float>{snapshot[offset]}));
    }
    EXPECT_EQ(api_->GetControlSignalIndex(wheel, 3), -1);
  }
}

TEST_F(SimpleDriveSnapshotTest, InactiveSnapshotOverwritesStaleValuesWithZeros) {
  std::vector<float> snapshot(8, 99.0f);
  api_->GetControlSignalSnapshot(snapshot);
  EXPECT_EQ(snapshot, (std::vector<float>{0.0f, 0.0f, 0.0f}));
}

TEST_F(SimpleDriveSnapshotTest, DisarmingClearsPreviouslyCapturedCommands) {
  Drive();
  std::vector<float> snapshot;
  api_->GetControlSignalSnapshot(snapshot);
  ASSERT_EQ(snapshot.size(), 3u);
  ASSERT_GT(snapshot[0], 0.0f);
  ASSERT_TRUE(api_->Disarm());
  api_->GetControlSignalSnapshot(snapshot);
  EXPECT_EQ(snapshot, (std::vector<float>{0.0f, 0.0f, 0.0f}));
}

TEST_F(SimpleDriveSnapshotTest, MissingUnderlyingControllerProducesThreeZeros) {
  Drive();
  api_->EndUpdate();
  std::vector<float> snapshot(8, 99.0f);
  api_->GetControlSignalSnapshot(snapshot);
  EXPECT_EQ(snapshot, (std::vector<float>{0.0f, 0.0f, 0.0f}));
}

}  // namespace
