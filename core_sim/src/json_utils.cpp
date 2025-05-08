// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/json_utils.hpp"

#include <array>
#include <iostream>

#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/geodetic_converter.hpp"
#include "core_sim/math_utils.hpp"
#include "string_utils.hpp"

namespace microsoft {
namespace projectairsim {

const std::regex identifier_regex("^[a-zA-Z][a-zA-Z0-9_-]*$");

const std::regex vector3_regex("^([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)$");

const std::regex vector4_regex(
    "^([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)$");

static Vector3 ParseVector3(const std::string& string);
static Vector4 ParseVector4(const std::string& string);

const std::string JsonUtils::GetIdentifier(const json& data_json,
                                           const std::string& key,
                                           const std::string& default_value) {
  auto value = StringUtils::TrimCopy(data_json.value(key, default_value));

  if (!std::regex_match(value, identifier_regex)) {
    throw Error("Invalid value:" + value + " for " + key);
  }

  return value;
}

bool JsonUtils::GetBoolean(const json& data_json, const std::string& key,
                           const bool default_value) {
  return !!data_json.value(key, (int)default_value);
}

const std::string JsonUtils::GetString(const json& data_json,
                                       const std::string& key,
                                       const std::string& default_value) {
  return StringUtils::TrimCopy(data_json.value(key, default_value));
}

int JsonUtils::GetInteger(const json& data_json, const std::string& key,
                          const int default_value) {
  return data_json.value(key, default_value);
}

const json JsonUtils::GetJsonObject(const json& data_json,
                                    const std::string& key,
                                    const json& default_value) {
  auto value = data_json.value(key, default_value);

  if (!value.is_null() && !value.is_object()) {
    throw Error("Type error");
  }

  return value;
}

const json JsonUtils::GetArray(const json& data_json, const std::string& key,
                               const json& default_value) {
  auto value = data_json.value(key, default_value);

  if (!value.is_array()) {
    throw Error("Type error");
  }

  return value;
}

Vector3 JsonUtils::GetVector3(const json& data_json, const std::string& key,
                              const Vector3& default_val) {
  if (HasKey(data_json, key)) {
    return ParseVector3(
        StringUtils::TrimCopy(GetString(data_json, key, "0 0 0")));
  } else {
    return default_val;
  }
}

Vector4 JsonUtils::GetVector4(const json& data_json, const std::string& key,
                              const Vector4 default_val) {
  if (HasKey(data_json, key)) {
    return ParseVector4(
        StringUtils::TrimCopy(GetString(data_json, key, "0 0 0 0")));
  } else {
    return default_val;
  }
}

Transform JsonUtils::GetTransform(const json& data_json, const std::string& key,
                                  const GeoPoint home_geo_point) {
  auto value = GetJsonObject(data_json, key);

  Vector3 translation;
  if (HasKey(value, Constant::Config::geo_point)) {
    // Convert lat/lon to NED
    auto geo_point = GetVector3(value, Constant::Config::geo_point);
    auto geoConv = projectairsim::GeodeticConverter(home_geo_point.latitude,
                                                    home_geo_point.longitude,
                                                    home_geo_point.altitude);
    double n, e, d;
    geoConv.geodetic2Ned(geo_point.x(), geo_point.y(), geo_point.z(), &n, &e,
                         &d);
    translation = Vector3(n, e, d);
  } else {
    translation = GetVector3(value, Constant::Config::translation);
  }

  Quaternion rotation;
  if (HasKey(value, Constant::Config::rotation_rad)) {
    rotation = TransformUtils::ToQuaternion(
        GetVector3(value, Constant::Config::rotation_rad));
  } else {
    rotation = TransformUtils::ToQuaternion(TransformUtils::ToRadians(
        GetVector3(value, Constant::Config::rotation_deg)));
  }
  return {translation, rotation};
}

Quaternion JsonUtils::GetQuaternion(const json& data_json,
                                    const std::string& key,
                                    const Quaternion& default_val) {
  if (HasKey(data_json, key)) {
    return TransformUtils::ToQuaternion(GetVector3(data_json, key));
  } else {
    return default_val;
  }
}

bool JsonUtils::IsEmpty(const json& data_json) {
  return data_json == "{ }"_json;
}

bool JsonUtils::IsEmptyArray(const json& data_json) {
  return data_json == "[ ]"_json;
}

bool JsonUtils::HasKey(const json& data_json, const std::string& key) {
  return data_json.count(key) == 1;
}

Vector3 ParseVector3(const std::string& string) {
  std::match_results<std::string::const_iterator> match_result;
  if (!std::regex_search(string, match_result, vector3_regex)) {
    throw Error("Invalid vector3");
  }

  auto x = std::stof(match_result[1]);
  auto y = std::stof(match_result[2]);
  auto z = std::stof(match_result[3]);

  return {x, y, z};
}

Vector4 ParseVector4(const std::string& string) {
  std::match_results<std::string::const_iterator> match_result;
  if (!std::regex_search(string, match_result, vector4_regex)) {
    throw Error("Invalid vector4");
  }

  auto r = std::stof(match_result[1]);
  auto g = std::stof(match_result[2]);
  auto b = std::stof(match_result[3]);
  auto a = std::stof(match_result[4]);

  return {r, g, b, a};
}

std::string GetTypeNameAsStr(json::value_t type_name) {
  switch (type_name) {
    case json::value_t::boolean:
      return "boolean";
    case json::value_t::number_integer:
      return "integer";
    case json::value_t::number_unsigned:
      return "unsigned integer";
    case json::value_t::number_float:
      return "float";
    case json::value_t::array:
      return "array";
    case json::value_t::string:
      return "string";
    case json::value_t::object:
      return "object";
    default:
      return "null";
  }
}

}  // namespace projectairsim
}  // namespace microsoft
