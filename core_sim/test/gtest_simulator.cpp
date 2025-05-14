// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include <memory>
#include <sstream>

#include "core_sim/error.hpp"
#include "core_sim/simulator.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;

TEST(Simulator, Constructor) {
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  EXPECT_NE(std::make_unique<projectairsim::Simulator>(), nullptr);
  EXPECT_NE(std::make_unique<projectairsim::Simulator>(callback), nullptr);
}

TEST(Simulator, LoadSimulator) {
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

  simulator.LoadSimulator(sim_config);

  // Check error for loading config with missing end brace
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"abc\" ";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);

  // Check error for loading config with invalid id (can't start with number)
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"1abc\" }";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);

  // Check error for loading config with empty default-scene
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"sim1\", \"default-scene\": \"\" }";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);

  // Check error for loading config with missing default-scene
  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"sim1\" }";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);
}

TEST(Simulator, IsLoaded) {
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  auto simulator = projectairsim::Simulator(callback);
  EXPECT_FALSE(simulator.IsLoaded());

  simulator = projectairsim::Simulator(callback);
  std::string sim_config = R"(
    {
      "id": "sim1",
      "default-scene": "{\"id\": \"DefaultScene\"}"
    }
  )";
  simulator.LoadSimulator(sim_config);
  EXPECT_TRUE(simulator.IsLoaded());

  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"abc\" ";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);
  EXPECT_FALSE(simulator.IsLoaded());
}

TEST(Simulator, GetID) {
  auto callback = [](const std::string& component, projectairsim::LogLevel level,
                     const std::string& message) {};

  auto simulator = projectairsim::Simulator(callback);
  EXPECT_TRUE(simulator.GetID().empty());

  simulator = projectairsim::Simulator(callback);
  std::string sim_config = R"(
    {
      "id": "sim1",
      "default-scene": "{\"id\": \"DefaultScene\"}"
    }
  )";
  simulator.LoadSimulator(sim_config);
  EXPECT_EQ(simulator.GetID(), "sim1");

  simulator = projectairsim::Simulator(callback);
  sim_config = "{ \"id\": \"abc\" ";
  EXPECT_THROW(simulator.LoadSimulator(sim_config), std::exception);
  EXPECT_TRUE(simulator.GetID().empty());
}
