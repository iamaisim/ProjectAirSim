// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SERVICE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SERVICE_HPP_

#include <string>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class Service {
 public:
  Service();

  const std::string& GetName() const;

 private:
  friend class ServiceManager;

  Service(const std::string& name);

  class Impl;
  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SERVICE_HPP_
