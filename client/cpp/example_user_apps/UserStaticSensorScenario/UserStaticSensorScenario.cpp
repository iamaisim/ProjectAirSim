// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
  std::cerr << "[ERROR] " << where << ": " << StatusToString(status)
            << std::endl;
  return false;
}

bool RunStaticSensorScenario(std::shared_ptr<pasc::Client>& client,
                             std::shared_ptr<pasc::World>& world,
                             const std::string& actor_name,
                             const std::string& camera_name) {
  pasc::StaticSensorActor actor;
  if (!Ensure(actor.Initialize(client, world, actor_name),
              "StaticSensorActor Initialize"))
    return false;

  json images;
  if (!Ensure(actor.GetImages(camera_name, {0}, &images), "GetImages"))
    return false;

  std::cout << "[OK] Retrieved images from camera '" << camera_name << "'"
            << std::endl;
  std::cout << "[OK] Image payload keys: " << images.size() << std::endl;

  return true;
}

}  // namespace

int main(int argc, const char* argv[]) {
  std::string sim_host = "127.0.0.1";
  std::string sim_config = "client/python/example_user_scripts/sim_config";
  std::string scene_file = "scene_computer_vision.jsonc";
  std::string actor_name = "CV";
  std::string camera_name = "Camera";

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
    } else if (arg == "--vehicle" || arg == "--actor") {
      if (++i >= argc) {
        std::cerr << "Missing value for --vehicle/--actor" << std::endl;
        return 1;
      }
      actor_name = argv[i];
    } else if (arg == "--camera") {
      if (++i >= argc) {
        std::cerr << "Missing value for --camera" << std::endl;
        return 1;
      }
      camera_name = argv[i];
    } else if (arg == "-h" || arg == "--help") {
      std::cout
          << "user_static_sensor_scenario [--simhost 127.0.0.1] [--simconfig "
             "path] [--scene scene_computer_vision.jsonc] [--vehicle CV] "
             "[--camera Camera]"
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

  const bool ok =
      RunStaticSensorScenario(client, world, actor_name, camera_name);
  client->Disconnect();

  if (!ok) {
    std::cerr << "[FAIL] StaticSensor scenario failed" << std::endl;
    return 1;
  }

  std::cout << "[PASS] StaticSensor scenario completed" << std::endl;
  return 0;
}
