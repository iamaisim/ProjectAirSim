// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include <Windows.h>

#include <string>
#include <vector>

#include "Common.h"
#include "Status.h"
#include "json.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

extern std::string v_str_version;  // Client library version

// Return the quaternion from "origin":"rpy" configuration value
std::vector<float> GetObjectRPYQuatFromConfig(json json_config);

// Load the float value from the json object, if the key exists
void LoadJSONValue(const json& json_in, const char* sz_key, float* pr_out);

// Parse a string with white space-separated floating values into an array
std::vector<float> ParseStringWithSpacesToFloatArray(const std::string& str);

// Convert RPY angles to quaternion
std::vector<float> RPYToQuaternionCoordinates(float roll, float pitch,
                                              float yaw);

// Split a string into words that were separated by white space
VecStr SplitString(const std::string& str);

// Update the client libray version from the version resource
void UpdateClientVersion(HMODULE hmodule);

// Conversion from and to JSON
void FromJSON(const json& json_in, Pose* ppose_out);
void FromJSON(const json& json_in, Quaternion* pquaternion_out);
void FromJSON(const json& json_in, Vector3* pvec3_out);
void ToJSON(const Pose& pose, json* pjson_out);
void ToJSON(const Quaternion& quaternionose, json* pjson_out);
void ToJSON(const Vector3& vec3, json* pjson_out);

}  // namespace internal

inline void from_json(const json& j, Pose& pose) {
  internal::FromJSON(j, &pose);
}

inline void to_json(json& j, const Pose& pose) { internal::ToJSON(pose, &j); }

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
