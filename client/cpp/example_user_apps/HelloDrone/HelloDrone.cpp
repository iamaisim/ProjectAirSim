// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.
//
// Demonstrates flying a quadrotor drone.
//

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <cctype>
#include <iostream>
#include <string_view>
#include <thread>

namespace pasc = microsoft::projectairsim::client;
using pasc::Status;

void MyLogOutputSink(pasc::Log::Severity /*severity*/,
                     const char* sz_message) noexcept {
  // Route log output to standard output
  std::cout << sz_message << std::endl;
}

bool IsCaseInsensitiveEqual(const char* lhs, const char* rhs) {
  std::string_view left(lhs == nullptr ? "" : lhs);
  std::string_view right(rhs == nullptr ? "" : rhs);

  if (left.size() != right.size()) return false;

  for (size_t i = 0; i < left.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(left[i])) !=
        std::tolower(static_cast<unsigned char>(right[i])))
      return false;
  }

  return true;
}

int main(int argc, const char* argv[]) {
  pasc::AsyncResult asyncresult;
  bool f;
  pasc::LandedState landed_state;
  pasc::ReadyState ready_state;
  std::shared_ptr<pasc::Client> pclient;
  std::shared_ptr<pasc::Drone> pdrone;
  std::shared_ptr<pasc::World> pworld;
  Status status;
  std::string strSimulationHost = "localhost";
  std::string strSceneFile = "scene_basic_drone.jsonc";
  std::string strPathSimConfig = "sim_config/";

  // Parse arguments
  if (argc > 1) {
    if (IsCaseInsensitiveEqual(argv[1], "/?") ||
        IsCaseInsensitiveEqual(argv[1], "-h") ||
        IsCaseInsensitiveEqual(argv[1], "--help")) {
      std::cout << "Perform a simple drone takeoff, move up, and landing "
                   "in Project AirSim."
                << std::endl
                << std::endl
                << "hello_drone [flags] [--simhost host_or_ip] [--simconfig "
                   "sim_config_path]"
                << std::endl
                << std::endl
                << "  /?, -h, --help        Print this help" << std::endl
                << "  --simhost host_or_ip  Hostname or IP address of Project "
                   "AirSim simulation server (default \"localhost\")"
                << std::endl
                << "  --simconfig sim_config_path" << std::endl
                << "                        Path to directory containing "
                   "the simulation configuration files (default \"simconfig/\")"
                << std::endl;
      return (0);
    }

    for (const char **pszArg = &argv[1], **pszArgEnd = &argv[argc];
         pszArg < pszArgEnd; ++pszArg) {
      if (strcmp(*pszArg, "--simhost") == 0) {
        if (++pszArg == pszArgEnd) {
          std::cerr << "Missing argument to flag \"--simhost\"" << std::endl;
          return (1);
        }
        strSimulationHost = *pszArg;
      } else if (strcmp(*pszArg, "--simconfig") == 0) {
        if (++pszArg == pszArgEnd) {
          std::cerr << "Missing argument to flag \"--simhost\"" << std::endl;
          return (1);
        }
        try {
          std::filesystem::path pathConfig =
              std::filesystem::canonical(*pszArg);
          strPathSimConfig = pathConfig.string();
          strSceneFile = (pathConfig / strSceneFile).string();
        } catch (const std::filesystem::filesystem_error& e) {
          std::cerr << "sim_config path error: " << e.what() << std::endl;
          return (1);
        }
      } else
      {
          std::cerr << "Unknown flag \"" << *pszArg << "\"" << std::endl;
          return (1);
      }
    }
  }

  // Set custom log output handler
  pasc::log.SetLogSink(MyLogOutputSink);

  // Connect to simulation environment
  pclient.reset(new pasc::Client());
  if ((status = pclient->Connect(strSimulationHost)) != Status::OK) goto LError;

  // Create a World object to interact with the sim world and load a scene
  pworld.reset(new pasc::World());
  pasc::log.InfoF("Looking for scene file \"%s\"", strSceneFile.c_str());
  if ((status = pworld->Initialize(pclient, strSceneFile, strPathSimConfig,
                                   2.0f)) != Status::OK)
    goto LError;

  // Connect to the drone
  pdrone.reset(new pasc::Drone());
  if ((status = pdrone->Initialize(pclient, pworld, "Drone1")) != Status::OK)
    goto LError;

  /*----------------------------------------------------------------------*/

  // Make the drone ready to fly
  if (((status = pdrone->EnableAPIControl(&f)) != Status::OK) ||
      ((status = pdrone->Arm(&f)) != Status::OK)) {
    goto LError;
  }
  if ((status = pdrone->GetReadyState(&ready_state)) != Status::OK) goto LError;
  pasc::log.InfoF("Ready state: %s (%s)",
                 ready_state.is_ready ? "ready" : "not ready",
                 ready_state.message.c_str());

  /*----------------------------------------------------------------------*/

  // Launch the drone
  pasc::log.Info("TakeoffAsync: starting");
  asyncresult = pdrone->TakeoffAsync();

  // Example 1: Wait on the result of async operation
  if ((status = asyncresult.Wait()) != Status::OK) goto LError;
  pasc::log.Info("TakeoffAsync: completed");

  /*----------------------------------------------------------------------*/

  // Command the drone to move up in NED coordinate system at 1 m/s for 4
  // seconds
  asyncresult = pdrone->MoveByVelocityAsync(0.0f,   // v_north
                                            0.0f,   // v_east
                                            -1.0f,  // v_down
                                            4.0);   // sec_duration
  pasc::log.Info("Move-Up invoked");

  // Example 2: Wait for async task to complete before continuing
  while (!asyncresult.FIsDone())
    std::this_thread::sleep_for(std::chrono::duration<float>(0.005f));

  // Note: must call AsyncResult::Wait() even though AsyncResult::FIsDone() was used
  if ((status = asyncresult.Wait()) != Status::OK) goto LError;
  pasc::log.Info("Move-Up completed");

  /*----------------------------------------------------------------------*/

  // Land the drone
  pasc::log.Info("LandAsync: starting");
  asyncresult = pdrone->LandAsync();
  if ((status = asyncresult.Wait()) != Status::OK) goto LError;
  if ((status = pdrone->GetLandedState(&landed_state)) != Status::OK)
    goto LError;
  pasc::log.InfoF("Landed state: %s",
                 landed_state == pasc::LandedState::Landed ? "landed"
                                                          : "flying");
  pasc::log.Info("LandAsync: completed");

  /*----------------------------------------------------------------------*/

  // Shut down the drone
  if (((status = pdrone->Disarm(&f)) != Status::OK) ||
      ((status = pdrone->DisableAPIControl(&f)) != Status::OK)) {
    goto LError;
  }

  /*----------------------------------------------------------------------*/

  // All done!
  pasc::log.Info("Drone landed.");
  pclient->Disconnect();

  return (0);

LError : {
  char szErr[256];

  pasc::GetStatusString(status, szErr);
  pasc::log.Critical(szErr);
}

  return (1);
}
