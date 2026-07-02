// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

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
  std::cerr << "[ERROR] " << where << ": " << StatusToString(status) << std::endl;
  return false;
}

bool RunUserScenario(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world,
                     const std::string& vehicle_name) {
  auto drone = std::make_shared<pasc::Drone>();
  if (!Ensure(drone->Initialize(client, world, vehicle_name), "Initialize " + vehicle_name)) {
    return false;
  }

  bool state = false;
  if (!Ensure(drone->EnableAPIControl(&state), vehicle_name + " EnableAPIControl")) return false;
  if (!Ensure(drone->Arm(&state), vehicle_name + " Arm")) return false;

  auto takeoff = drone->TakeoffAsync();
  if (!Ensure(takeoff.Wait(), vehicle_name + " TakeoffAsync")) return false;

  // Template action: move up for 3 seconds. Replace this block with custom logic.
  auto move = drone->MoveByVelocityAsync(0.0f, 0.0f, -1.0f, 3.0f);
  if (!Ensure(move.Wait(), vehicle_name + " MoveByVelocityAsync")) return false;

  auto land = drone->LandAsync();
  if (!Ensure(land.Wait(), vehicle_name + " LandAsync")) return false;

  if (!Ensure(drone->Disarm(&state), vehicle_name + " Disarm")) return false;
  if (!Ensure(drone->DisableAPIControl(&state), vehicle_name + " DisableAPIControl")) {
    return false;
  }

  return true;
}

}  // namespace

int main(int argc, const char* argv[]) {
  std::string sim_host = "127.0.0.1";
  std::string sim_config = "client/python/example_user_scripts/sim_config";
  std::string scene_file = "scene_basic_drone.jsonc";
  std::string vehicle_name = "Drone1";

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
    } else if (arg == "--vehicle") {
      if (++i >= argc) {
        std::cerr << "Missing value for --vehicle" << std::endl;
        return 1;
      }
      vehicle_name = argv[i];
    } else if (arg == "-h" || arg == "--help") {
      std::cout
          << "user_scenario_template [--simhost 127.0.0.1] [--simconfig path] "
             "[--scene scene_basic_drone.jsonc] [--vehicle Drone1]"
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
  if (!Ensure(world->Initialize(client, scene_path.string(), sim_path.string(), 2.0f),
              "World Initialize")) {
    client->Disconnect();
    return 1;
  }

  const bool ok = RunUserScenario(client, world, vehicle_name);
  client->Disconnect();

  if (!ok) {
    std::cerr << "[FAIL] User scenario failed" << std::endl;
    return 1;
  }

  std::cout << "[PASS] User scenario completed" << std::endl;
  return 0;
}
