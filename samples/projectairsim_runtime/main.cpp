// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025-2026 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include "ground_collision_host.hpp"
#include "simserver.hpp"

namespace projectairsim = microsoft::projectairsim;

namespace {

std::atomic<bool> keep_running(true);

struct Options {
  int topics_port = 8989;
  int services_port = 8990;
  float ground_height_m = 0.0f;
  float ground_clearance_m = 0.0f;
};

void LoggingCallback(const std::string& component, projectairsim::LogLevel,
                     const std::string& message) {
  std::cout << "[" << component << "] " << message << std::endl;
}

void HandleSignal(int) { keep_running.store(false); }

void PrintUsage(const char* executable) {
  std::cout
      << "Project AirSim Runtime by IAMAI\n"
      << "Lightweight, engine-independent autonomy simulation runtime\n\n"
      << "Usage: " << executable
      << " [topics_port] [services_port] [options]\n\n"
      << "Options:\n"
      << "  --topics-port PORT       Topics port (default: 8989)\n"
      << "  --services-port PORT     Services port (default: 8990)\n"
      << "  --ground-height METERS   Flat ground Z in NED (default: 0)\n"
      << "  --ground-clearance M     Robot-origin to landing-gear contact "
         "distance\n"
      << "  -h, --help               Show this help\n";
}

std::string RequireValue(int argc, char* argv[], int& index,
                         const std::string& option) {
  if (index + 1 >= argc) {
    throw std::invalid_argument("Missing value for " + option);
  }
  return argv[++index];
}

Options ParseOptions(int argc, char* argv[]) {
  Options options;
  int positional_index = 0;

  for (int index = 1; index < argc; ++index) {
    const std::string argument(argv[index]);
    if (argument == "-h" || argument == "--help") {
      PrintUsage(argv[0]);
      std::exit(0);
    } else if (argument == "--topics-port") {
      options.topics_port =
          std::stoi(RequireValue(argc, argv, index, argument));
    } else if (argument == "--services-port") {
      options.services_port =
          std::stoi(RequireValue(argc, argv, index, argument));
    } else if (argument == "--ground-height") {
      options.ground_height_m =
          std::stof(RequireValue(argc, argv, index, argument));
    } else if (argument == "--ground-clearance") {
      options.ground_clearance_m =
          std::stof(RequireValue(argc, argv, index, argument));
    } else if (!argument.empty() && argument[0] == '-') {
      throw std::invalid_argument("Unknown option: " + argument);
    } else if (positional_index == 0) {
      options.topics_port = std::stoi(argument);
      ++positional_index;
    } else if (positional_index == 1) {
      options.services_port = std::stoi(argument);
      ++positional_index;
    } else {
      throw std::invalid_argument("Unexpected positional argument: " +
                                  argument);
    }
  }

  if (options.ground_clearance_m < 0.0f) {
    throw std::invalid_argument("--ground-clearance cannot be negative");
  }
  return options;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    const Options options = ParseOptions(argc, argv);
    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    projectairsim::SimServer simserver(LoggingCallback,
                                       projectairsim::LogLevel::kVerbose);
    projectairsim::GroundCollisionHost collision_host(
        {options.ground_height_m, options.ground_clearance_m});

    // SimServer invokes these callbacks when a client reloads the scene. This
    // is the same external-host lifecycle used by the Unreal integration.
    simserver.SetCallbackLoadExternalScene(
        [&]() { collision_host.LoadScene(simserver.GetScene()); });
    simserver.SetCallbackUnloadExternalScene(
        [&]() { collision_host.UnloadScene(); });

    std::cout << "Starting Project AirSim Runtime by IAMAI with topics_port="
              << options.topics_port
              << ", services_port=" << options.services_port
              << ", ground_height=" << options.ground_height_m
              << " m, ground_clearance=" << options.ground_clearance_m << " m"
              << std::endl;
    simserver.LoadSimulator(options.topics_port, options.services_port);
    simserver.LoadScene();
    collision_host.LoadScene(simserver.GetScene());
    simserver.StartSimulator();
    simserver.StartScene();

    std::cout << "Project AirSim Runtime is running. Press Ctrl-C to end."
              << std::endl;
    while (keep_running.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    simserver.StopScene();
    collision_host.UnloadScene();
    simserver.UnloadScene();
    simserver.StopSimulator();
    simserver.UnloadSimulator();
  } catch (const std::exception& exception) {
    std::cerr << "Project AirSim Runtime error: " << exception.what()
              << std::endl;
    return 1;
  }

  return 0;
}
