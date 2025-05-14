// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.
// Tests for Trajectory class

#include "core_sim/config_json.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/trajectory.hpp"
#include "gtest/gtest.h"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

class EnvActor {
 public:
  static Trajectory MakeTrajectory() {
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);
    return Trajectory(logger);
  }

  static void LoadTrajectoryFromConfig(Trajectory& trajectory,
                                       ConfigJson config_json) {
    trajectory.Load(config_json);
  }

  static void LoadTrajectoryFromAPI(
      Trajectory& trajectory, const std::string& traj_name,
      const std::vector<float>& time, const std::vector<float>& x,
      const std::vector<float>& y, const std::vector<float>& z,
      const std::vector<float>& roll, const std::vector<float>& pitch,
      const std::vector<float>& yaw, const std::vector<float>& vel_x,
      const std::vector<float>& vel_y, const std::vector<float>& vel_z) {
    trajectory.Load(traj_name, time, x, y, z, roll, pitch, yaw, vel_x, vel_y,
                    vel_z);
  }

  static void UpdateKinematics(Trajectory& trajectory, TimeSec currtime,
                               TrajectoryKinematics& traj_kinematics) {
    trajectory.KinematicsUpdate(currtime, traj_kinematics, 0, Kinematics());
  }

  static json GetScript() {
    json script =
        R"({
            "name": "test",
            "time_sec": [1, 3, 6, 9, 12],
            "pose_x": [3, 5, 8, 14, 20],
            "pose_y": [0, 7, 10, 15, 20],
            "pose_z": [-15, -14, -10, -7, -4],
            "pose_roll": [0, 0, 0, 0, 0],
            "pose_pitch": [0, 0, 0, 0, 0],
            "pose_yaw": [0, 0, 0, 0, 0],
            "velocity_linear_x": [1, 2, 3, 4, 5],
            "velocity_linear_y": [10, 11, 12, 13, 16],
            "velocity_linear_z": [3, 5, 6, 8, 9]
            })"_json;

    return script;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(Trajectory, Constructor) {
  EXPECT_FALSE(projectairsim::EnvActor::MakeTrajectory().IsLoaded());
}

TEST(Trajectory, LoadFromConfig) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
}

TEST(Trajectory, IsLoaded) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  EXPECT_TRUE(trajectory.IsLoaded());
}

TEST(Trajectory, LoadFromAPI) {
  std::string traj_name = "test";
  std::vector<float> time = {1, 3, 6, 9, 12};
  std::vector<float> x = {3, 5, 8, 14, 20};
  std::vector<float> y = {0, 7, 10, 15, 20};
  std::vector<float> z = {-15, -14, -10, -7, -4};
  std::vector<float> roll = {0, 0, 0, 0, 0};
  std::vector<float> pitch = {0, 0, 0, 0, 0};
  std::vector<float> yaw = {0, 0, 0, 0, 0};
  std::vector<float> vel_x = {1, 2, 3, 4, 5};
  std::vector<float> vel_y = {10, 11, 12, 13, 16};
  std::vector<float> vel_z = {3, 5, 6, 8, 9};

  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromAPI(trajectory, traj_name, time, x,
                                               y, z, roll, pitch, yaw, vel_x,
                                               vel_y, vel_z);
  EXPECT_TRUE(trajectory.IsLoaded());
}

TEST(Trajectory, GetKinematicsAtFirstWaypoint) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  TimeSec currtime = 1;
  projectairsim::TrajectoryKinematics traj_kinematics;
  traj_kinematics.traj_params.to_loop = true;
  projectairsim::EnvActor::UpdateKinematics(trajectory, currtime,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(3, 0, -15));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(1, 10, 3));
}

TEST(Trajectory, GetKinematicsBetweenWaypoints) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  TimeSec curr_time = 7;
  projectairsim::TrajectoryKinematics traj_kinematics;
  projectairsim::EnvActor::UpdateKinematics(trajectory, curr_time,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(10, 11.666667, -9));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(3.3333333, 12.333333, 6.6666665));
}

TEST(Trajectory, GetKinematicsAtLastWaypoint) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  TimeSec currtime = 12;
  projectairsim::TrajectoryKinematics traj_kinematics;
  traj_kinematics.traj_params.to_loop = true;
  projectairsim::EnvActor::UpdateKinematics(trajectory, currtime,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(20, 20, -4));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(5, 16, 9));
}

TEST(Trajectory, GetKinematicsAfterLastWayPointWithLoop) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  projectairsim::TrajectoryKinematics traj_kinematics;
  traj_kinematics.traj_params.to_loop = true;
  traj_kinematics.traj_params.num_loops = 1;
  TimeSec curr_time = 16;
  projectairsim::EnvActor::UpdateKinematics(trajectory, curr_time,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(7, 9, -11.3333333));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(2.66666667, 11.66666667, 5.66666667));
}

TEST(Trajectory, GetKinematicsAfterLastWayPointWithoutLoop) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  TimeSec curr_time = 12;
  projectairsim::TrajectoryKinematics traj_kinematics;
  traj_kinematics.traj_params.to_loop = false;
  projectairsim::EnvActor::UpdateKinematics(trajectory, curr_time,
                                          traj_kinematics);
  curr_time = 17;
  projectairsim::EnvActor::UpdateKinematics(trajectory, curr_time,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(20, 20, -4));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(5, 16, 9));
}

TEST(Trajectory, GetKinematicsAtFirstWaypointWithOffset) {
  auto json = projectairsim::EnvActor::GetScript();
  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromConfig(trajectory, json);
  TimeSec currtime = 3;
  projectairsim::TrajectoryKinematics traj_kinematics;
  traj_kinematics.traj_params.to_loop = true;
  traj_kinematics.traj_params.time_offset = 2;
  traj_kinematics.traj_params.x_offset = -1;
  projectairsim::EnvActor::UpdateKinematics(trajectory, currtime,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(2, 0, -15));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(1, 10, 3));
}

TEST(Trajectory, GetKinematicsAfterLastWayPointWithLoopAndOffset) {
  std::string traj_name = "test";
  std::vector<float> time = {1, 3, 6, 9, 12};
  std::vector<float> x = {3, 5, 8, 14, 20};
  std::vector<float> y = {0, 7, 10, 15, 20};
  std::vector<float> z = {-15, -14, -10, -7, -4};
  std::vector<float> roll = {0, 0, 0, 0, 0};
  std::vector<float> pitch = {0, 0, 0, 0, 0};
  std::vector<float> yaw = {0, 0, 0, 0, 0};
  std::vector<float> vel_x = {1, 2, 3, 4, 5};
  std::vector<float> vel_y = {10, 11, 12, 13, 16};
  std::vector<float> vel_z = {3, 5, 6, 8, 9};

  auto trajectory = projectairsim::EnvActor::MakeTrajectory();
  projectairsim::EnvActor::LoadTrajectoryFromAPI(trajectory, traj_name, time, x,
                                               y, z, roll, pitch, yaw, vel_x,
                                               vel_y, vel_z);

  projectairsim::TrajectoryKinematics traj_kinematics;
  traj_kinematics.traj_params.to_loop = true;
  traj_kinematics.traj_params.num_loops = 1;
  traj_kinematics.traj_params.time_offset = 1;
  traj_kinematics.traj_params.x_offset = 1;
  traj_kinematics.traj_params.y_offset = 1;
  traj_kinematics.traj_params.z_offset = 1;
  TimeSec curr_time = 16;
  projectairsim::EnvActor::UpdateKinematics(trajectory, curr_time,
                                          traj_kinematics);
  EXPECT_EQ(traj_kinematics.kinematics.pose.position,
            projectairsim::Vector3(7, 9, -11.66666667));
  EXPECT_EQ(traj_kinematics.kinematics.twist.linear,
            projectairsim::Vector3(2.3333333, 11.3333333, 5.3333333));
}