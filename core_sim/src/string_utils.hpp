// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_SRC_STRING_UTILS_HPP_
#define CORE_SIM_SRC_STRING_UTILS_HPP_

#include <string>

namespace microsoft {
namespace projectairsim {

class StringUtils {
 public:
  static void LTrim(std::string& string);

  static void RTrim(std::string& string);

  static void Trim(std::string& string);

  static std::string LTrimCopy(std::string string);

  static std::string RTrimCopy(std::string string);

  static std::string TrimCopy(std::string string);
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_STRING_UTILS_HPP_
