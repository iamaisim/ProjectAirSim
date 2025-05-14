// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/service.hpp"

#include <string>

namespace microsoft {
namespace projectairsim {

class Service::Impl {
 public:
  Impl(const std::string& name);

  const std::string& GetName();

 private:
  std::string name_;
};

Service::Service() : pimpl_(std::shared_ptr<Service::Impl>(nullptr)) {}

Service::Service(const std::string& name)
    : pimpl_(std::shared_ptr<Service::Impl>(new Service::Impl(name))) {}

const std::string& Service::GetName() const { return pimpl_->GetName(); }

Service::Impl::Impl(const std::string& name) : name_(name) {}

const std::string& Service::Impl::GetName() { return name_; }

}  // namespace projectairsim
}  // namespace microsoft
