// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SIMULATOR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SIMULATOR_HPP_

#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "core_sim/client_authorization.hpp"
#include "core_sim/log_level.hpp"
#include "core_sim/scene.hpp"

namespace microsoft {
namespace projectairsim {

class Simulator {
 public:
  using LoggerCallback =
      std::function<void(const std::string&, LogLevel, const std::string&)>;

  Simulator();

  Simulator(LoggerCallback logger_callback, LogLevel level = LogLevel::kError);

  // Simulator controls

  void LoadSimulator(const std::string& sim_json,
                     std::string client_auth_public_key = std::string());

  void UnloadSimulator();

  bool IsLoaded();

  void StartSimulator();

  void StopSimulator();

  const std::string& GetID();

  void RegisterServiceMethod(const ServiceMethod& method,
                             MethodHandler method_handler);

  // Scene controls

  void LoadSceneWithJSON(const std::string& json_data = "");

  void UnloadScene();

  void StartScene();

  void StopScene();

  Logger& GetLogger();

  Scene& GetScene();

  const std::string& GetSceneConfigJSON() const;

 private:
  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SIMULATOR_HPP_
