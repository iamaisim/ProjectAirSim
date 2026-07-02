// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <ProjectAirSimMessage/response_message.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace pasc = microsoft::projectairsim::client;
namespace mp = microsoft::projectairsim;
using pasc::Status;
using nlohmann::json;

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

bool RequestJson(std::shared_ptr<pasc::Client>& client, const std::string& method,
                 const json& params, json* result_out = nullptr) {
  pasc::Message message_response;
  Status status = client->Request(method, params, &message_response);
  if (!Ensure(status, method)) return false;

  mp::ResponseMessage response;
  response.Deserialize(message_response);

  if (response.GetErrorCode() != 0) {
    std::cerr << "[ERROR] " << method << " rejected by server: "
              << response.GetResult().dump() << std::endl;
    return false;
  }

  if (result_out != nullptr) *result_out = response.GetResult();
  return true;
}

bool EnableArmTakeoff(std::shared_ptr<pasc::Drone>& drone, const std::string& name) {
  bool state = false;
  if (!Ensure(drone->EnableAPIControl(&state), name + " EnableAPIControl")) return false;
  if (!Ensure(drone->Arm(&state), name + " Arm")) return false;
  auto ar = drone->TakeoffAsync();
  if (!Ensure(ar.Wait(), name + " TakeoffAsync")) return false;
  return true;
}

bool LandDisarm(std::shared_ptr<pasc::Drone>& drone, const std::string& name) {
  bool state = false;
  auto ar = drone->LandAsync();
  if (!Ensure(ar.Wait(), name + " LandAsync")) return false;
  if (!Ensure(drone->Disarm(&state), name + " Disarm")) return false;
  if (!Ensure(drone->DisableAPIControl(&state), name + " DisableAPIControl")) return false;
  return true;
}

bool InitDrone(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world,
               const std::string& name, std::shared_ptr<pasc::Drone>* drone_out) {
  auto drone = std::make_shared<pasc::Drone>();
  if (!Ensure(drone->Initialize(client, world, name), "Initialize " + name)) return false;
  *drone_out = drone;
  return true;
}

bool RunBasic(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world) {
  std::shared_ptr<pasc::Drone> drone;
  if (!InitDrone(client, world, "Drone1", &drone)) return false;
  if (!EnableArmTakeoff(drone, "Drone1")) return false;

  auto move = drone->MoveByVelocityAsync(0.0f, 0.0f, -1.0f, 3.0f);
  if (!Ensure(move.Wait(), "Drone1 MoveByVelocityAsync")) return false;

  return LandDisarm(drone, "Drone1");
}

bool TrySensorCall(std::shared_ptr<pasc::Drone>& drone,
                   Status (pasc::Drone::*sensor_call)(const std::string&, json*) const,
                   const std::vector<std::string>& sensor_names,
                   const std::string& label) {
  for (const auto& sensor_name : sensor_names) {
    json data;
    Status status = ((*drone).*sensor_call)(sensor_name, &data);
    if (status == Status::OK) {
      std::cout << "[OK] " << label << " with sensor '" << sensor_name << "': "
                << data.dump().substr(0, 220) << std::endl;
      return true;
    }
  }

  std::cerr << "[WARN] No sensor worked for " << label << std::endl;
  return false;
}

bool RunSensors(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world) {
  std::shared_ptr<pasc::Drone> drone;
  if (!InitDrone(client, world, "Drone1", &drone)) return false;
  if (!EnableArmTakeoff(drone, "Drone1")) return false;

  // Use standard sensor names from scene config
  // (GetSensorCount/GetSensorName not yet implemented on server side, so use hardcoded list)
  std::vector<std::string> imu_sensors = {"IMU1"};
  std::vector<std::string> gps_sensors = {"GPS"};
  std::vector<std::string> baro_sensors = {"Barometer"};
  std::vector<std::string> mag_sensors = {"Magnetometer"};
  std::vector<std::string> airspeed_sensors = {"Airspeed"};
  std::vector<std::string> camera_sensors = {"DownCamera", "Chase"};

  std::cout << "[INFO] Testing standard sensor names from scene config" << std::endl;

  bool ok = true;
  ok &= TrySensorCall(drone, &pasc::Drone::GetIMUData, imu_sensors, "GetIMUData");
  ok &= TrySensorCall(drone, &pasc::Drone::GetGPSData, gps_sensors, "GetGPSData");
  ok &= TrySensorCall(drone, &pasc::Drone::GetBarometerData, baro_sensors,
                      "GetBarometerData");
  ok &= TrySensorCall(drone, &pasc::Drone::GetMagnetometerData, mag_sensors,
                      "GetMagnetometerData");
  ok &= TrySensorCall(drone, &pasc::Drone::GetAirspeedData, airspeed_sensors,
                      "GetAirspeedData");

  json image_data;
  bool image_ok = false;
  for (const auto& camera_name : camera_sensors) {
    Status status = drone->GetImages(camera_name, {0}, &image_data);
    if (status == Status::OK) {
      std::cout << "[OK] GetImages with camera '" << camera_name << "'" << std::endl;
      image_ok = true;
      break;
    }
  }

  if (!image_ok) {
    std::cerr << "[WARN] No camera sensor accepted GetImages" << std::endl;
  }

  json battery_state;
  if (Ensure(drone->GetBatteryState(&battery_state), "GetBatteryState")) {
    std::cout << "[OK] Battery state: " << battery_state.dump().substr(0, 220)
              << std::endl;
  } else {
    ok = false;
  }

  if (!LandDisarm(drone, "Drone1")) return false;
  return ok;
}

bool RunTwoDrones(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world) {
  std::shared_ptr<pasc::Drone> drone1;
  std::shared_ptr<pasc::Drone> drone2;

  if (!InitDrone(client, world, "Drone1", &drone1)) return false;
  if (!InitDrone(client, world, "Drone2", &drone2)) return false;

  if (!EnableArmTakeoff(drone1, "Drone1")) return false;
  if (!EnableArmTakeoff(drone2, "Drone2")) return false;

  // Move drones sequentially to avoid state conflicts
  if (!Ensure(drone1->MoveByVelocityAsync(1.0f, 0.0f, 0.0f, 2.5f).Wait(), 
              "Drone1 MoveByVelocityAsync")) return false;
  if (!Ensure(drone2->MoveByVelocityAsync(0.0f, 1.0f, 0.0f, 2.5f).Wait(), 
              "Drone2 MoveByVelocityAsync")) return false;

  json kinematics1;
  json kinematics2;
  Ensure(drone1->GetGroundTruthKinematics(&kinematics1), "Drone1 kinematics");
  Ensure(drone2->GetGroundTruthKinematics(&kinematics2), "Drone2 kinematics");

  std::cout << "[OK] Drone1 kinematics sample: "
            << kinematics1.dump().substr(0, 220) << std::endl;
  std::cout << "[OK] Drone2 kinematics sample: "
            << kinematics2.dump().substr(0, 220) << std::endl;

  if (!LandDisarm(drone1, "Drone1")) return false;
  if (!LandDisarm(drone2, "Drone2")) return false;

  return true;
}

bool RunWindAndWorld(std::shared_ptr<pasc::Client>& client,
                     std::shared_ptr<pasc::World>& world) {
  const std::string parent = world->GetParentTopic();

  bool ok = true;
  json response;

  ok &= RequestJson(client, parent + "/SetSunLightIntensity", {{"intensity", 12000.0f}});
  ok &= RequestJson(client, parent + "/GetSunLightIntensity", {}, &response);
  std::cout << "[OK] Sun intensity: " << response.dump() << std::endl;

  ok &= RequestJson(client, parent + "/SetCloudShadowStrength", {{"strength", 0.35f}});
  ok &= RequestJson(client, parent + "/GetCloudShadowStrength", {}, &response);
  std::cout << "[OK] Cloud shadow strength: " << response.dump() << std::endl;

  ok &= RequestJson(client, parent + "/SetWindVelocity",
                    {{"v_x", 2.0f}, {"v_y", 1.0f}, {"v_z", 0.0f}});
  ok &= RequestJson(client, parent + "/GetWindVelocity", {}, &response);
  std::cout << "[OK] Wind velocity: " << response.dump() << std::endl;

  ok &= RequestJson(client, parent + "/Pause", {{"do_pause", true}});
  ok &= RequestJson(client, parent + "/GetSimTime", {}, &response);
  std::cout << "[OK] Sim time (paused): " << response.dump() << std::endl;
  ok &= RequestJson(client, parent + "/Pause", {{"do_pause", false}});

  return ok;
}

bool RunBattery(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world) {
  std::shared_ptr<pasc::Drone> drone;
  if (!InitDrone(client, world, "Drone1", &drone)) return false;

  json battery;
  if (!Ensure(drone->GetBatteryState(&battery), "GetBatteryState")) return false;
  std::cout << "[OK] Initial battery: " << battery.dump().substr(0, 220) << std::endl;

  bool set_ok = false;
  if (!Ensure(drone->SetBatteryDrainRate(0.05f, &set_ok), "SetBatteryDrainRate"))
    return false;

  float drain_rate = 0.0f;
  if (!Ensure(drone->GetBatteryDrainRate(&drain_rate), "GetBatteryDrainRate"))
    return false;

  std::cout << "[OK] Drain rate now: " << drain_rate << std::endl;

  if (!EnableArmTakeoff(drone, "Drone1")) return false;
  auto hover = drone->HoverAsync();
  if (!Ensure(hover.Wait(), "HoverAsync")) return false;

  return LandDisarm(drone, "Drone1");
}

bool RunRover(std::shared_ptr<pasc::Client>& client, std::shared_ptr<pasc::World>& world) {
  std::vector<std::string> topics = client->GetTopicInfo();
  if (topics.empty()) {
    std::cerr << "[ERROR] No topics were returned by the server." << std::endl;
    return false;
  }

  size_t rover_topic_count = 0;
  for (const auto& topic : topics) {
    if (topic.find("/Rover1/") != std::string::npos) {
      ++rover_topic_count;
    }
  }

  if (rover_topic_count == 0) {
    std::cerr << "[ERROR] Rover topics were not discovered for Rover1." << std::endl;
    return false;
  }

  std::cout << "[OK] Rover topics discovered for Rover1: " << rover_topic_count
            << std::endl;

  // Exercise world-level services on the rover scene even though Rover API is not
  // implemented yet in the C++ client library.
  return RunWindAndWorld(client, world);
}

int main(int argc, const char* argv[]) {
  std::string scenario = "basic";
  std::string sim_host = "127.0.0.1";
  std::string sim_config = "client/python/example_user_scripts/sim_config";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--scenario") {
      if (++i >= argc) {
        std::cerr << "Missing value for --scenario" << std::endl;
        return 1;
      }
      scenario = argv[i];
    } else if (arg == "--simhost") {
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
    } else if (arg == "-h" || arg == "--help") {
      std::cout << "cpp_client_scenarios --scenario <basic|sensors|two_drones|wind|battery|rover> "
                   "[--simhost 127.0.0.1] [--simconfig path]"
                << std::endl;
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return 1;
    }
  }

  std::string scene_file;
  if (scenario == "basic") {
    scene_file = "scene_basic_drone.jsonc";
  } else if (scenario == "sensors") {
    scene_file = "scene_drone_sensors.jsonc";
  } else if (scenario == "two_drones") {
    scene_file = "scene_two_drones.jsonc";
  } else if (scenario == "wind") {
    scene_file = "scene_drone_wind.jsonc";
  } else if (scenario == "battery") {
    scene_file = "scene_battery_simple.jsonc";
  } else if (scenario == "rover") {
    scene_file = "scene_basic_rover.jsonc";
  } else {
    std::cerr << "Unknown scenario: " << scenario << std::endl;
    return 1;
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
    return 1;
  }

  bool ok = false;
  if (scenario == "basic") {
    ok = RunBasic(client, world);
  } else if (scenario == "sensors") {
    ok = RunSensors(client, world);
  } else if (scenario == "two_drones") {
    ok = RunTwoDrones(client, world);
  } else if (scenario == "wind") {
    ok = RunWindAndWorld(client, world);
  } else if (scenario == "battery") {
    ok = RunBattery(client, world);
  } else if (scenario == "rover") {
    ok = RunRover(client, world);
  }

  client->Disconnect();

  if (!ok) {
    std::cerr << "[FAIL] Scenario '" << scenario << "' failed" << std::endl;
    return 1;
  }

  std::cout << "[PASS] Scenario '" << scenario << "' completed" << std::endl;
  return 0;
}
