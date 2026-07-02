// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace pasc = microsoft::projectairsim::client;
using pasc::Status;
using nlohmann::json;

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

void SleepSeconds(float seconds) {
  std::this_thread::sleep_for(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::duration<float>(seconds)));
}

bool GetRoverPositionXY(pasc::Rover& rover, double* x_out, double* y_out) {
  json kinematics;
  if (!Ensure(rover.GetGroundTruthKinematics(&kinematics),
              "GetGroundTruthKinematics"))
    return false;

  try {
    const auto& position = kinematics.at("pose").at("position");
    *x_out = position.at("x").get<double>();
    *y_out = position.at("y").get<double>();
  } catch (const std::exception& ex) {
    std::cerr << "[ERROR] Could not parse rover position: " << ex.what()
              << std::endl;
    return false;
  }

  return true;
}

bool WaitUntilStopped(pasc::Rover& rover, float timeout_sec = 30.0f,
                      float speed_tolerance = 1.0e-5f) {
  double x_last = 0.0;
  double y_last = 0.0;
  if (!GetRoverPositionXY(rover, &x_last, &y_last)) return false;

  const double speed_tolerance_sq =
      static_cast<double>(speed_tolerance) * speed_tolerance;
  const auto start = std::chrono::steady_clock::now();
  auto last = start;

  while (true) {
    SleepSeconds(0.1f);

    double x = 0.0;
    double y = 0.0;
    if (!GetRoverPositionXY(rover, &x, &y)) return false;

    const auto now = std::chrono::steady_clock::now();
    const double elapsed =
        std::chrono::duration<double>(now - start).count();
    if (elapsed >= timeout_sec) {
      std::cerr << "[ERROR] Timed out waiting for rover to stop" << std::endl;
      return false;
    }

    const double dt = std::chrono::duration<double>(now - last).count();
    const double dx = x - x_last;
    const double dy = y - y_last;
    const double dist_sq = dx * dx + dy * dy;
    const double dist_tolerance_sq = speed_tolerance_sq * dt * dt;
    if (dist_sq <= dist_tolerance_sq) return true;

    x_last = x;
    y_last = y;
    last = now;
  }
}

bool SetRoverControlsAndWait(pasc::Rover& rover, float engine,
                             float steering_angle, float brake,
                             float wait_sec, const std::string& label) {
  std::cout << "[..] " << label << std::endl;
  if (!Ensure(rover.SetRoverControls(engine, steering_angle, brake),
              "SetRoverControls"))
    return false;
  SleepSeconds(wait_sec);
  return true;
}

bool BrakeRover(pasc::Rover& rover, float brake = 0.5f) {
  std::cout << "[..] Braking" << std::endl;
  if (!Ensure(rover.SetRoverControls(0.0f, 0.0f, brake), "SetRoverControls"))
    return false;
  return WaitUntilStopped(rover);
}

bool RunRoverScenario(std::shared_ptr<pasc::Client>& client,
                      std::shared_ptr<pasc::World>& world,
                      const std::string& vehicle_name) {
  // -- Initialize rover --
  pasc::Rover rover;
  if (!Ensure(rover.Initialize(client, world, vehicle_name), "Rover Initialize"))
    return false;
  std::cout << "[OK] Rover '" << vehicle_name << "' initialized" << std::endl;

  // -- Enable API control --
  bool api_enabled = false;
  if (!Ensure(rover.EnableAPIControl(&api_enabled), "EnableAPIControl"))
    return false;
  std::cout << "[OK] API control enabled: " << std::boolalpha << api_enabled
            << std::endl;

  // -- Arm --
  bool armed = false;
  if (!Ensure(rover.Arm(&armed), "Arm")) return false;
  std::cout << "[OK] Armed: " << armed << std::endl;

  if (!SetRoverControlsAndWait(rover, 0.0f, 0.0f, 0.0f, 5.0f,
                               "Setting rover controls to zero"))
    return false;

  if (!SetRoverControlsAndWait(rover, 0.5f, 1.0f, 0.0f, 5.0f,
                               "Setting rover controls forward and to the right"))
    return false;

  if (!SetRoverControlsAndWait(rover, 0.5f, -1.0f, 0.0f, 5.0f,
                               "Setting rover controls forward and to the left"))
    return false;

  if (!BrakeRover(rover)) return false;

  if (!SetRoverControlsAndWait(
          rover, -0.5f, 1.0f, 0.0f, 5.0f,
          "Setting rover controls backwards and to the right"))
    return false;

  if (!SetRoverControlsAndWait(
          rover, -0.5f, -1.0f, 0.0f, 5.0f,
          "Setting rover controls backwards and to the left"))
    return false;

  if (!BrakeRover(rover)) return false;

  if (!SetRoverControlsAndWait(rover, 0.0f, 0.0f, 0.0f, 1.0f,
                               "Setting rover controls zero"))
    return false;

  // -- Disarm --
  bool disarmed = false;
  if (!Ensure(rover.Disarm(&disarmed), "Disarm")) return false;
  std::cout << "[OK] Disarmed: " << disarmed << std::endl;

  bool api_disabled = false;
  if (!Ensure(rover.DisableAPIControl(&api_disabled), "DisableAPIControl"))
    return false;
  std::cout << "[OK] API control disabled: " << api_disabled << std::endl;

  return true;
}

}  // namespace

int main(int argc, const char* argv[]) {
  std::string sim_host = "127.0.0.1";
  std::string sim_config = "client/python/example_user_scripts/sim_config";
  std::string scene_file = "scene_basic_rover.jsonc";
  std::string vehicle_name = "Rover1";

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
      std::cout << "user_rover_scenario [--simhost 127.0.0.1] [--simconfig "
                   "path] [--scene scene_basic_rover.jsonc] [--vehicle Rover1]"
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

  const bool ok = RunRoverScenario(client, world, vehicle_name);
  client->Disconnect();

  if (!ok) {
    std::cerr << "[FAIL] Rover scenario failed" << std::endl;
    return 1;
  }

  std::cout << "[PASS] Rover scenario completed" << std::endl;
  return 0;
}
