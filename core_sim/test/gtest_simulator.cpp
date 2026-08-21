// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <memory>
#include <sstream>

#include "core_sim/error.hpp"
#include "core_sim/simulator.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;

TEST(Simulator, Constructor) {
                     // Act: run `auto callback = [](const std::string& component, projectairsim::LogLevel level,`.
  // General description:
  // Verifies constructor for Simulator.
  // Arrange: prepare context for `auto callback = [](const std::string& component, projectairsim::LogLevel level,`.
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  // Assert: check result from `EXPECT_NE(std::make_unique<projectairsim::Simulator>(), nullptr);`.
  EXPECT_NE(std::make_unique<projectairsim::Simulator>(), nullptr);
  EXPECT_NE(std::make_unique<projectairsim::Simulator>(callback), nullptr);
}

TEST(Simulator, LoadSimulator) {
  // General description:
  // Verifies simulator loading behavior for valid and invalid configurations.
  // Arrange: prepare context for `auto callback = [](const std::string& component, projectairsim::LogLevel level,`.
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  projectairsim::Simulator simulator(callback);

  // Check loading normal config
  std::string sim_config = R"(
    {
      "id": "ProjectAirSim",
      "default-scene": "{\"id\": \"DefaultScene\"}",
      "topics": {"ip": "*", "port": 8989},
      "services": {"ip": "*", "port": 8990},
      "async-services": {"ip": "*", "port": 8991}
    }
  )";

  // Act: run `simulator.LoadSimulator(sim_config);`.
  simulator.LoadSimulator(sim_config);

  // Check error for loading config with missing end brace
  // Act + Assert: invalid JSON should throw.
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"abc\" ";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);

  // Check error for loading config with invalid id (can't start with number)
  // Act + Assert: invalid ID format should throw.
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"1abc\" }";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);

  // Check error for loading config with empty default-scene
  // Act + Assert: empty default-scene should throw.
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"sim1\", \"default-scene\": \"\" }";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);

  // Check error for loading config with missing default-scene
  // Act + Assert: missing default-scene should throw.
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"sim1\" }";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);
}

TEST(Simulator, IsLoaded) {
  // General description:
  // Verifies IsLoaded() state transitions for valid and invalid loads.
  // Arrange: prepare context for `auto callback = [](const std::string& component, projectairsim::LogLevel level,`.
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  auto simulator = projectairsim::Simulator(callback);

  // Assert: check result from `EXPECT_FALSE(simulator.IsLoaded());`.
  EXPECT_FALSE(simulator.IsLoaded());

  simulator = projectairsim::Simulator(callback);
  std::string sim_config = R"(
    {
      "id": "sim1",
      "default-scene": "{\"id\": \"DefaultScene\"}"
    }
  )";
  // Act: run `simulator.LoadSimulator(sim_config);`.
  simulator.LoadSimulator(sim_config);

  // Assert: check result from `EXPECT_TRUE(simulator.IsLoaded());`.
  EXPECT_TRUE(simulator.IsLoaded());

  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"abc\" ";

  // Act + Assert: failed load throws and keeps unloaded state.
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);
  EXPECT_FALSE(simulator.IsLoaded());
}

TEST(Simulator, GetID) {
  // General description:
  // Verifies GetID() before load, after valid load, and after failed load.
  // Arrange: prepare context for `auto callback = [](const std::string& component, projectairsim::LogLevel level,`.
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  auto simulator = projectairsim::Simulator(callback);

  // Assert: check result from `EXPECT_TRUE(simulator.GetID().empty());`.
  EXPECT_TRUE(simulator.GetID().empty());

  simulator = projectairsim::Simulator(callback);
  std::string sim_config = R"(
    {
      "id": "sim1",
      "default-scene": "{\"id\": \"DefaultScene\"}"
    }
  )";
  // Act: run `simulator.LoadSimulator(sim_config);`.
  simulator.LoadSimulator(sim_config);

  // Assert: check result from `EXPECT_EQ(simulator.GetID(), "sim1");`.
  EXPECT_EQ(simulator.GetID(), "sim1");

  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"abc\" ";

  // Act + Assert: failed load throws and ID remains empty.
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);
  EXPECT_TRUE(simulator.GetID().empty());
}