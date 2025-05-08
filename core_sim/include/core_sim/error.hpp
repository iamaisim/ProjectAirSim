// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ERROR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ERROR_HPP_

#include <exception>
#include <string>

namespace microsoft {
namespace projectairsim {

class Error : public std::exception {
 public:
  explicit Error(const std::string& message) : message_(message) {}

  const char* what() const noexcept override { return message_.c_str(); }

 private:
  std::string message_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ERROR_HPP_
