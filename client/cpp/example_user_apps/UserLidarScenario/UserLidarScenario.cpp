// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include "msgpack.hpp"

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

size_t CountLidarPoints(const std::string& msg) {
  auto handle = msgpack::unpack(msg.data(), msg.size());
  msgpack::object object = handle.get();
  if (object.type != msgpack::type::MAP) return 0;

  for (uint32_t i = 0; i < object.via.map.size; ++i) {
    const auto& key = object.via.map.ptr[i].key;
    const auto& value = object.via.map.ptr[i].val;
    if (key.type != msgpack::type::STR) continue;

    const std::string key_string(key.via.str.ptr, key.via.str.size);
    if (key_string == "point_cloud" && value.type == msgpack::type::ARRAY) {
      return value.via.array.size / 3;
    }
  }

  return 0;
}

bool RunLidarScenario(std::shared_ptr<pasc::Client>& client,
                      std::shared_ptr<pasc::World>& world,
                      const std::string& vehicle_name,
                      const std::string& lidar_name) {
  // -- Initialize drone --
  pasc::Drone drone;
  if (!Ensure(drone.Initialize(client, world, vehicle_name),
              "Drone Initialize"))
    return false;
  std::cout << "[OK] Drone '" << vehicle_name << "' initialized" << std::endl;

  // -- Enable API control --
  bool api_enabled = false;
  if (!Ensure(drone.EnableAPIControl(&api_enabled), "EnableAPIControl"))
    return false;
  std::cout << "[OK] API control enabled" << std::endl;

  // -- Arm drone --
  bool armed = false;
  if (!Ensure(drone.Arm(&armed), "Arm")) return false;
  std::cout << "[OK] Drone armed" << std::endl;

  // -- Take off --
  std::cout << "[..] Takeoff..." << std::endl;
  auto async_result = drone.TakeoffAsync(/*sec_timeout*/ 10.0f);
  if (!Ensure(async_result.Wait(), "Takeoff")) return false;
  std::cout << "[OK] Drone at altitude" << std::endl;

  // -- Retrieve Lidar data from the published topic --
  const char* lidar_topic = drone.GetSensorTopic(lidar_name, "lidar");
  if (lidar_topic == nullptr || std::string(lidar_topic).empty()) {
    std::cerr << "[ERROR] Lidar topic not found for sensor '" << lidar_name
              << "'" << std::endl;
    return false;
  }

  std::cout << "[INFO] Subscribing to Lidar topic: " << lidar_topic
            << std::endl;

  std::mutex mutex;
  std::condition_variable cv;
  int frame_count = 0;
  bool parse_error = false;

  if (!Ensure(client->Subscribe(
                  lidar_topic,
                  [&](const std::string& /*topic*/, const std::string& msg) {
                    try {
                      const size_t point_count = CountLidarPoints(msg);

                      {
                        std::lock_guard<std::mutex> lock(mutex);
                        ++frame_count;
                      }
                      std::cout << "[OK] Lidar frame " << frame_count
                                << " received with " << point_count
                                << " points" << std::endl;
                      cv.notify_one();
                    } catch (const std::exception& ex) {
                      std::lock_guard<std::mutex> lock(mutex);
                      parse_error = true;
                      std::cerr << "[ERROR] Failed to parse Lidar message: "
                                << ex.what() << std::endl;
                      cv.notify_one();
                    }
                  }),
              "Subscribe Lidar topic"))
    return false;

  {
    std::unique_lock<std::mutex> lock(mutex);
    const bool got_frames = cv.wait_for(lock, std::chrono::seconds(5), [&] {
      return frame_count >= 3 || parse_error;
    });
    if (!got_frames) {
      std::cerr << "[ERROR] Timed out waiting for Lidar topic messages"
                << std::endl;
      client->Unsubscribe(lidar_topic);
      return false;
    }
    if (parse_error) {
      client->Unsubscribe(lidar_topic);
      return false;
    }
  }
  client->Unsubscribe(lidar_topic);

  // -- Land drone --
  std::cout << "[..] Landing..." << std::endl;
  auto land_result = drone.LandAsync(/*sec_timeout*/ 10.0f);
  if (!Ensure(land_result.Wait(), "Land")) return false;
  std::cout << "[OK] Drone landed" << std::endl;

  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
  using std::filesystem::exists;

  pasc::log.SetLogSink(LogSink);

  try {
    // -- Configuration --
    std::string sim_host = "127.0.0.1";
    std::string sim_config = "client/python/example_user_scripts/sim_config";
    std::string scene_file = "scene_lidar_drone.jsonc";
    std::string vehicle_name = "Drone1";
    std::string lidar_name = "lidar1";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--simhost") {
        if (++i >= argc) {
          std::cerr << "Missing value for --simhost" << std::endl;
          return 1;
        }
        sim_host = argv[i];
      } else if (arg == "--vehicle" && i + 1 < argc) {
        vehicle_name = argv[++i];
      } else if (arg == "--lidar" && i + 1 < argc) {
        lidar_name = argv[++i];
      } else if (arg == "--simconfig" && i + 1 < argc) {
        sim_config = argv[++i];
      } else if (arg == "--scene" && i + 1 < argc) {
        scene_file = argv[++i];
      } else if (arg == "-h" || arg == "--help") {
        std::cout << "user_lidar_scenario [--simhost 127.0.0.1] [--simconfig path] "
                     "[--scene scene_lidar_drone.jsonc] [--vehicle Drone1] "
                     "[--lidar Lidar1]"
                  << std::endl;
        return 0;
      } else {
        std::cerr << "Unknown argument: " << arg << std::endl;
        return 1;
      }
    }

    std::filesystem::path sim_path;
    std::filesystem::path scene_path;
    try {
      sim_path = std::filesystem::canonical(sim_config);
      scene_path = sim_path / scene_file;
    } catch (const std::filesystem::filesystem_error& ex) {
      std::cerr << "simconfig path error: " << ex.what() << std::endl;
      return 1;
    }

    // -- Create client and world --
    auto client = std::make_shared<pasc::Client>();
    std::cout << "[INFO] Connecting to AirSim at " << sim_host << std::endl;
    if (!Ensure(client->Connect(sim_host), "Client Connect")) return 1;
    std::cout << "[OK] Client connected" << std::endl;

    auto world = std::make_shared<pasc::World>();
    if (!Ensure(world->Initialize(client, scene_path.string(), sim_path.string(),
                                  2.0f),
                "World Initialize")) {
      client->Disconnect();
      return 1;
    }
    std::cout << "[OK] World initialized" << std::endl;

    // -- Run scenario --
    std::cout << "[INFO] Running Lidar scenario for vehicle: " << vehicle_name
              << ", sensor: " << lidar_name << std::endl;
    if (!RunLidarScenario(client, world, vehicle_name, lidar_name)) {
      client->Disconnect();
      return 1;
    }

    client->Disconnect();
    std::cout << "[OK] Lidar scenario completed successfully" << std::endl;
    return 0;

  } catch (const std::exception& ex) {
    std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "[ERROR] Unknown exception" << std::endl;
    return 1;
  }
}
