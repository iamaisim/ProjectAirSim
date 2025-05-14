// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SERVICE_MANAGER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SERVICE_MANAGER_HPP_

#include <memory>

#include "core_sim/client_authorization.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service.hpp"
#include "core_sim/service_method.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class ServiceManager {
 public:
  explicit ServiceManager(const Logger& logger,
                          const ClientAuthorization& client_authorization);
  explicit ServiceManager(const Logger& logger);

  void Load(const json& config_json);

  void RegisterMethod(const ServiceMethod& method,
                      MethodHandler method_handler);

  void UnregisterMethod(const ServiceMethod& method);

  void UnregisterAllMethods();

  void Start();

  void Stop();

 private:
  class Impl;
  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SERVICE_MANAGER_HPP_
