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

size_t CountMsgpackArrayField(const std::string& msg,
                              const std::string& field_name) {
  auto handle = msgpack::unpack(msg.data(), msg.size());
  msgpack::object object = handle.get();
  if (object.type != msgpack::type::MAP) return 0;

  for (uint32_t i = 0; i < object.via.map.size; ++i) {
    const auto& key = object.via.map.ptr[i].key;
    const auto& value = object.via.map.ptr[i].val;
    if (key.type != msgpack::type::STR) continue;

    const std::string key_string(key.via.str.ptr, key.via.str.size);
    if (key_string == field_name && value.type == msgpack::type::ARRAY) {
      return value.via.array.size;
    }
  }

  return 0;
}

bool RunRadarScenario(std::shared_ptr<pasc::Client>& client,
                      std::shared_ptr<pasc::World>& world,
                      const std::string& vehicle_name,
                      const std::string& radar_name) {
  // -- Initialize drone --
  pasc::Drone drone;
  if (!Ensure(drone.Initialize(client, world, vehicle_name),
              "Drone Initialize"))
    return false;
  std::cout << "[OK] Drone '" << vehicle_name << "' initialized" << std::endl;

  const char* detections_topic =
      drone.GetSensorTopic(radar_name, "radar_detections");
  const char* tracks_topic = drone.GetSensorTopic(radar_name, "radar_tracks");
  if (detections_topic == nullptr || std::string(detections_topic).empty() ||
      tracks_topic == nullptr || std::string(tracks_topic).empty()) {
    std::cerr << "[ERROR] Radar topics not found for sensor '" << radar_name
              << "'" << std::endl;
    return false;
  }

  std::cout << "[INFO] Subscribing to Radar detections topic: "
            << detections_topic << std::endl;
  std::cout << "[INFO] Subscribing to Radar tracks topic: " << tracks_topic
            << std::endl;

  std::mutex mutex;
  std::condition_variable cv;
  int detection_frames = 0;
  int track_frames = 0;
  bool parse_error = false;

  if (!Ensure(client->Subscribe(
                  detections_topic,
                  [&](const std::string& /*topic*/, const std::string& msg) {
                    try {
                      const size_t detection_count =
                          CountMsgpackArrayField(msg, "radar_detections");
                      {
                        std::lock_guard<std::mutex> lock(mutex);
                        ++detection_frames;
                      }
                      std::cout << "[OK] Radar detections frame "
                                << detection_frames << " received with "
                                << detection_count << " detections"
                                << std::endl;
                      cv.notify_one();
                    } catch (const std::exception& ex) {
                      std::lock_guard<std::mutex> lock(mutex);
                      parse_error = true;
                      std::cerr << "[ERROR] Failed to parse Radar detections: "
                                << ex.what() << std::endl;
                      cv.notify_one();
                    }
                  }),
              "Subscribe Radar detections"))
    return false;

  if (!Ensure(client->Subscribe(
                  tracks_topic,
                  [&](const std::string& /*topic*/, const std::string& msg) {
                    try {
                      const size_t track_count =
                          CountMsgpackArrayField(msg, "radar_tracks");
                      {
                        std::lock_guard<std::mutex> lock(mutex);
                        ++track_frames;
                      }
                      std::cout << "[OK] Radar tracks frame " << track_frames
                                << " received with " << track_count
                                << " tracks" << std::endl;
                      cv.notify_one();
                    } catch (const std::exception& ex) {
                      std::lock_guard<std::mutex> lock(mutex);
                      parse_error = true;
                      std::cerr << "[ERROR] Failed to parse Radar tracks: "
                                << ex.what() << std::endl;
                      cv.notify_one();
                    }
                  }),
              "Subscribe Radar tracks")) {
    client->Unsubscribe(detections_topic);
    return false;
  }

  {
    std::unique_lock<std::mutex> lock(mutex);
    const bool got_frames = cv.wait_for(lock, std::chrono::seconds(10), [&] {
      return (detection_frames >= 3 && track_frames >= 3) || parse_error;
    });
    if (!got_frames) {
      std::cerr << "[ERROR] Timed out waiting for Radar topic messages"
                << std::endl;
      client->Unsubscribe(detections_topic);
      client->Unsubscribe(tracks_topic);
      return false;
    }
    if (parse_error) {
      client->Unsubscribe(detections_topic);
      client->Unsubscribe(tracks_topic);
      return false;
    }
  }

  client->Unsubscribe(detections_topic);
  client->Unsubscribe(tracks_topic);

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
    std::string scene_file = "scene_radar_tower.jsonc";
    std::string vehicle_name = "RadarTower1";
    std::string radar_name = "radar1";

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
      } else if (arg == "--radar" && i + 1 < argc) {
        radar_name = argv[++i];
      } else if (arg == "--simconfig" && i + 1 < argc) {
        sim_config = argv[++i];
      } else if (arg == "--scene" && i + 1 < argc) {
        scene_file = argv[++i];
      } else if (arg == "-h" || arg == "--help") {
        std::cout << "user_radar_scenario [--simhost 127.0.0.1] [--simconfig path] "
                     "[--scene scene_radar_tower.jsonc] [--vehicle RadarTower1] "
                     "[--radar radar1]"
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
    std::cout << "[INFO] Running Radar scenario for vehicle: " << vehicle_name
              << ", sensor: " << radar_name << std::endl;
    if (!RunRadarScenario(client, world, vehicle_name, radar_name)) {
      client->Disconnect();
      return 1;
    }

    client->Disconnect();
    std::cout << "[OK] Radar scenario completed successfully" << std::endl;
    return 0;

  } catch (const std::exception& ex) {
    std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "[ERROR] Unknown exception" << std::endl;
    return 1;
  }
}
