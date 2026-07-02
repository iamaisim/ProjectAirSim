// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pasc = microsoft::projectairsim::client;
using pasc::Status;

namespace {

void LogSink(pasc::Log::Severity /*severity*/, const char* message) noexcept {
  std::cout << message << std::endl;
}

std::string StatusToString(Status status) {
  char buffer[256] = {0};
  pasc::GetStatusString(status, buffer);
  return std::string(buffer);
}

bool Ensure(Status status, const std::string& where) {
  if (status == Status::OK) return true;
  std::cerr << "[ERROR] " << where << ": " << StatusToString(status)
            << std::endl;
  return false;
}

void GetLoopTheLoopCoords(float radius, float center_y, float center_z,
                          int num_points, std::vector<float>* y_coords,
                          std::vector<float>* z_coords) {
  constexpr float kPi = 3.14159265358979323846f;
  const float angle_step = 2.0f * kPi / static_cast<float>(num_points);

  std::vector<float> angles;
  angles.reserve(num_points + num_points / 2 + 1);
  for (int idx = 0; idx <= num_points; ++idx) {
    angles.emplace_back(-kPi + angle_step * static_cast<float>(idx));
  }

  const float angle_start = angles.back();
  for (int idx = 1; idx <= num_points / 2; ++idx) {
    angles.emplace_back(angle_start + angle_step * static_cast<float>(idx));
  }

  y_coords->clear();
  z_coords->clear();
  y_coords->reserve(angles.size());
  z_coords->reserve(angles.size());
  for (float angle : angles) {
    y_coords->emplace_back(center_y + radius * std::cos(angle));
    z_coords->emplace_back(center_z + radius * std::sin(angle));
  }
}

bool RunEnvActorScenario(std::shared_ptr<pasc::Client>& client,
                         std::shared_ptr<pasc::World>& world) {
  pasc::EnvActor actor_with_config_traj;
  pasc::EnvActor tiltrotor_with_config_traj_offset;
  pasc::EnvActor actor_with_api_traj;
  pasc::EnvActor actor_with_api_traj_and_offset;

  if (!Ensure(actor_with_config_traj.Initialize(client, world,
                                                "ActorWithConfigTraj"),
              "Initialize ActorWithConfigTraj"))
    return false;
  if (!Ensure(tiltrotor_with_config_traj_offset.Initialize(
                  client, world, "TiltrotorWithConfigTrajOffset"),
              "Initialize TiltrotorWithConfigTrajOffset"))
    return false;
  if (!Ensure(actor_with_api_traj.Initialize(client, world, "ActorWithApiTraj"),
              "Initialize ActorWithApiTraj"))
    return false;
  if (!Ensure(actor_with_api_traj_and_offset.Initialize(
                  client, world, "ActorWithApiTrajAndOffset"),
              "Initialize ActorWithApiTrajAndOffset"))
    return false;

  std::cout << "[..] Setting TiltrotorWithConfigTrajOffset trajectory to "
               "right_and_descend_config"
            << std::endl;
  const std::map<std::string, float> shroud_angle_map = {
      {"Shroud_FL", 90.0f},
      {"Shroud_RL", 90.0f},
      {"Shroud_FR", 90.0f},
      {"Shroud_RR", 90.0f},
  };
  if (!Ensure(tiltrotor_with_config_traj_offset.SetLinkRotationAngles(
                  shroud_angle_map),
              "SetLinkRotationAngles"))
    return false;

  if (!Ensure(tiltrotor_with_config_traj_offset.SetTrajectory(
                  "right_and_descend_config", true, 3.0f, 2.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.57f),
              "SetTrajectory right_and_descend_config"))
    return false;

  std::vector<float> time_sec = {4.0f};
  std::vector<float> pose_x = {1.0f};
  std::vector<float> pose_y = {0.0f};
  std::vector<float> pose_z = {-6.0f};

  std::vector<float> y_looptheloop;
  std::vector<float> z_looptheloop;
  GetLoopTheLoopCoords(3.0f, 10.0f, -6.0f, 20, &y_looptheloop,
                       &z_looptheloop);

  const float step_size = 0.2f;
  const float looptheloop_start = time_sec.back() + 2.0f;
  for (size_t idx = 0; idx < y_looptheloop.size(); ++idx) {
    time_sec.emplace_back(looptheloop_start +
                          step_size * static_cast<float>(idx + 1));
    pose_x.emplace_back(pose_x.back());
    pose_y.emplace_back(y_looptheloop[idx]);
    pose_z.emplace_back(z_looptheloop[idx]);
  }

  time_sec.emplace_back(time_sec.back() + 2.0f);
  pose_x.emplace_back(pose_x.back());
  pose_y.emplace_back(pose_y.back() + 3.0f);
  pose_z.emplace_back(pose_z.back());

  std::cout << "[..] Importing looptheloop NED trajectory" << std::endl;
  if (!Ensure(world->ImportNEDTrajectory("looptheloop", time_sec, pose_x,
                                         pose_y, pose_z),
              "ImportNEDTrajectory looptheloop"))
    return false;

  std::cout << "[..] Setting looptheloop trajectory to API actors"
            << std::endl;
  if (!Ensure(actor_with_api_traj.SetTrajectory("looptheloop", true),
              "SetTrajectory ActorWithApiTraj"))
    return false;
  if (!Ensure(actor_with_api_traj_and_offset.SetTrajectory("looptheloop", true,
                                                           2.0f),
              "SetTrajectory ActorWithApiTrajAndOffset"))
    return false;

  std::cout << "[OK] EnvActor trajectories configured" << std::endl;
  return true;
}

}  // namespace

int main(int argc, const char* argv[]) {
  std::string sim_host = "127.0.0.1";
  std::string sim_config = "client/python/example_user_scripts/sim_config";
  std::string scene_file = "scene_env_actor.jsonc";

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--simhost") {
      if (++i >= argc) {
        std::cerr << "Missing value for --simhost" << std::endl;
        return 1;
      }
      sim_host = argv[i];
    } else if (arg == "--simconfig") {
      if (++i >= argc) {
        std::cerr << "Missing value for --simconfig" << std::endl;
        return 1;
      }
      sim_config = argv[i];
    } else if (arg == "--scene") {
      if (++i >= argc) {
        std::cerr << "Missing value for --scene" << std::endl;
        return 1;
      }
      scene_file = argv[i];
    } else if (arg == "-h" || arg == "--help") {
      std::cout
          << "user_env_actor_scenario [--simhost 127.0.0.1] [--simconfig path] "
             "[--scene scene_env_actor.jsonc]"
          << std::endl;
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return 1;
    }
  }

  pasc::log.SetLogSink(LogSink);

  std::filesystem::path sim_path;
  std::filesystem::path scene_path;
  try {
    sim_path = std::filesystem::canonical(sim_config);
    scene_path = sim_path / scene_file;
  } catch (const std::filesystem::filesystem_error& ex) {
    std::cerr << "simconfig path error: " << ex.what() << std::endl;
    return 1;
  }

  auto client = std::make_shared<pasc::Client>();
  if (!Ensure(client->Connect(sim_host), "Client Connect")) return 1;

  auto world = std::make_shared<pasc::World>();
  if (!Ensure(world->Initialize(client, scene_path.string(), sim_path.string(),
                                2.0f),
              "World Initialize")) {
    client->Disconnect();
    return 1;
  }

  const bool ok = RunEnvActorScenario(client, world);
  client->Disconnect();

  if (!ok) {
    std::cerr << "[FAIL] EnvActor scenario failed" << std::endl;
    return 1;
  }

  std::cout << "[PASS] EnvActor scenario completed" << std::endl;
  return 0;
}
