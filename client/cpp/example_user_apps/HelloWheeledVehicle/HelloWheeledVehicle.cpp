// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace pasc = microsoft::projectairsim::client;
using pasc::Status;

namespace {

void LogSink(pasc::Log::Severity /*severity*/, const char* message) noexcept {
  std::cout << message << std::endl;
}

bool Ensure(Status status, const std::string& operation) {
  if (status == Status::OK) return true;
  char message[256] = {0};
  pasc::GetStatusString(status, message);
  std::cerr << "[ERROR] " << operation << ": " << message << std::endl;
  return false;
}

void SleepSeconds(float seconds) {
  std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
}

bool SetControls(pasc::WheeledVehicle& vehicle, float throttle, float steering,
                 float brake) {
  bool throttle_applied = false;
  bool steering_applied = false;
  bool brakes_applied = false;
  // Set braking and steering inputs before engaging throttle.
  return Ensure(vehicle.SetBrakes(brake, &brakes_applied), "SetBrakes") &&
         brakes_applied &&
         Ensure(vehicle.SetSteering(steering, &steering_applied), "SetSteering") &&
         steering_applied &&
         Ensure(vehicle.SetThrottle(throttle, &throttle_applied), "SetThrottle") &&
         throttle_applied;
}

bool LogPosition(pasc::WheeledVehicle& vehicle, const std::string& label,
                 std::array<double, 3>* position_out) {
  nlohmann::json kinematics;
  if (!Ensure(vehicle.GetKinematics(&kinematics), "GetKinematics")) return false;

  try {
    const auto& position = kinematics.at("pose").at("position");
    *position_out = {position.at("x").get<double>(),
                     position.at("y").get<double>(),
                     position.at("z").get<double>()};
    std::cout << "[..] " << label << " position: ("
              << position.at("x").get<double>() << ", "
              << position.at("y").get<double>() << ", "
              << position.at("z").get<double>() << ")" << std::endl;
    return true;
  } catch (const std::exception& exception) {
    std::cerr << "[WARN] Could not parse vehicle position: " << exception.what()
              << std::endl;
    return false;
  }
}

}  // namespace

int main(int argc, const char* argv[]) {
  std::string sim_host = "127.0.0.1";
  std::string sim_config = "client/python/example_user_scripts/sim_config";
  std::string scene_file = "scene_wheeled_vehicle.jsonc";
  std::string vehicle_name = "WheeledVehicle";

  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--simhost" && i + 1 < argc) {
      sim_host = argv[++i];
    } else if (argument == "--simconfig" && i + 1 < argc) {
      sim_config = argv[++i];
    } else if (argument == "--scene" && i + 1 < argc) {
      scene_file = argv[++i];
    } else if (argument == "--vehicle" && i + 1 < argc) {
      vehicle_name = argv[++i];
    } else if (argument == "-h" || argument == "--help") {
      std::cout << "hello_wheeled_vehicle [--simhost HOST] "
                   "[--simconfig PATH] [--scene FILE] [--vehicle NAME]"
                << std::endl;
      return 0;
    } else {
      std::cerr << "Unknown or incomplete argument: " << argument << std::endl;
      return 1;
    }
  }

  pasc::log.SetLogSink(LogSink);

  std::filesystem::path config_directory;
  try {
    config_directory = std::filesystem::canonical(sim_config);
  } catch (const std::filesystem::filesystem_error& exception) {
    std::cerr << "[ERROR] simconfig path: " << exception.what() << std::endl;
    return 1;
  }

  auto client = std::make_shared<pasc::Client>();
  if (!Ensure(client->Connect(sim_host), "Client::Connect")) return 1;

  auto world = std::make_shared<pasc::World>();
  const auto scene_path = config_directory / scene_file;
  if (!Ensure(world->Initialize(client, scene_path.string(),
                                config_directory.string(), 2.0f, -1),
              "World::Initialize")) {
    client->Disconnect();
    return 1;
  }

  pasc::WheeledVehicle vehicle;
  const bool initialized =
      Ensure(vehicle.Initialize(client, world, vehicle_name),
             "WheeledVehicle::Initialize");
  bool ok = initialized;
  if (initialized) {
    std::array<double, 3> initial_position{}, final_position{};
    SleepSeconds(2.0f);
    const bool initial_pose_received =
        LogPosition(vehicle, "Initial", &initial_position);
    std::cout << "[..] Driving with throttle and steering" << std::endl;
    ok = SetControls(vehicle, 0.7f, 0.45f, 0.0f);
    SleepSeconds(5.0f);
    const bool final_pose_received =
        LogPosition(vehicle, "After drive", &final_position);
    const double dx = final_position[0] - initial_position[0];
    const double dy = final_position[1] - initial_position[1];
    // Do not count settling vertically or accepted RPCs as successful driving.
    if (!initial_pose_received || !final_pose_received ||
        dx * dx + dy * dy < 1.0) {
      std::cerr << "[ERROR] Could not confirm at least 1 m of horizontal motion"
                << std::endl;
      ok = false;
    }

    (void)SetControls(vehicle, 0.0f, 0.0f, 1.0f);
    SleepSeconds(1.0f);
    (void)SetControls(vehicle, 0.0f, 0.0f, 0.0f);
  }

  client->Disconnect();

  std::cout << (ok ? "[PASS] Wheeled vehicle scenario completed"
                   : "[FAIL] Wheeled vehicle scenario failed")
            << std::endl;
  return ok ? 0 : 1;
}
