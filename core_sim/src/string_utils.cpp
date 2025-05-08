// Copyright (C) Microsoft Corporation. All rights reserved.

#include "string_utils.hpp"

#include <algorithm>
#include <locale>
#include <string>

namespace microsoft {
namespace projectairsim {

void StringUtils::LTrim(std::string& string) {
  string.erase(string.begin(),
               std::find_if(string.begin(), string.end(), [](int character) {
#ifdef WIN32
                 return !std::isspace(character, std::locale());
#else
                 return !std::isspace(character);
#endif
               }));
}

void StringUtils::RTrim(std::string& string) {
  string.erase(std::find_if(string.rbegin(), string.rend(),
                            [](int character) {
#ifdef WIN32
                              return !std::isspace(character, std::locale());
#else
                              return !std::isspace(character);
#endif
                            })
                   .base(),
               string.end());
}

void StringUtils::Trim(std::string& string) {
  LTrim(string);
  RTrim(string);
}

std::string StringUtils::LTrimCopy(std::string string) {
  LTrim(string);
  return string;
}

std::string StringUtils::RTrimCopy(std::string string) {
  RTrim(string);
  return string;
}

std::string StringUtils::TrimCopy(std::string string) {
  Trim(string);
  return string;
}

}  // namespace projectairsim
}  // namespace microsoft
