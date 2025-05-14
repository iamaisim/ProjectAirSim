// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

#include "simserver.hpp"

namespace projectairsim = microsoft::projectairsim;

void LoggingCallback(const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {
  std::cout << "[" << component << "]"
            << " " << message << std::endl;
}

int main(int argc, char* argv[]) {
  try {
    projectairsim::SimServer simserver(LoggingCallback,
                                     projectairsim::LogLevel::kVerbose);
    int topics_port = 8989;
    if (argc > 1) topics_port = std::atoi(argv[1]);

    int services_port = 8990;
    if (argc > 2) services_port = std::atoi(argv[2]);

    std::cout << "Loading simulator with topics_port=" << topics_port
              << ", services_port=" << services_port << "..." << std::endl;
    simserver.LoadSimulator(topics_port, services_port);

    std::cout << "Loading default scene..." << std::endl;
    simserver.LoadScene();

    std::cout << "Starting simulator..." << std::endl;
    simserver.StartSimulator();

    std::cout << "Starting scene..." << std::endl;
    simserver.StartScene();

    std::cout << "Simulation is now running. Press Ctrl-C to end." << std::endl;
    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  } catch (std::exception& exception) {
    std::cout << "Exception encountered: " << exception.what() << std::endl;
    return 1;
  }

  std::cout << "Done." << std::endl;
  return 0;
}
