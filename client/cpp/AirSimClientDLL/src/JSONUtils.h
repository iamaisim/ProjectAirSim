// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include "Types.h"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {
namespace client {

inline void from_json(const nlohmann::json& j, Transform& transform) {
  nlohmann::json json_rotation = j.at("rotation");
  nlohmann::json json_translation = j.at("translation");

  // The transform struct contains a timestamp, but for some reason
  // the server's to_json() for Tranform doesn't serialize it.  More
  // Pose/Transform type confusion?
  if (j.contains("timestamp")) transform.timestamp = j["timestamp"];

  transform.frame_id = j["frame_id"];

  // When used for a pose, the quaternion and vector3 fields are serialized as
  // an object rather than as an array
  transform.rotation.w = json_rotation["w"];
  transform.rotation.x = json_rotation["x"];
  transform.rotation.y = json_rotation["y"];
  transform.rotation.z = json_rotation["z"];

  transform.translation.x = json_translation["x"];
  transform.translation.y = json_translation["y"];
  transform.translation.z = json_translation["z"];
}

inline void to_json(nlohmann::json& j, const ColorRGBA& colorrgba) {
  j = nlohmann::json::array(
      {colorrgba.red, colorrgba.green, colorrgba.blue, colorrgba.alpha});
}

inline void to_json(nlohmann::json& j, const Quaternion& quat) {
  j = nlohmann::json::array({quat.w, quat.x, quat.y, quat.z});
}

inline void to_json(nlohmann::json& j, const Vector3& vec3) {
  j = nlohmann::json::array({vec3.x, vec3.y, vec3.z});
}

inline void to_json(nlohmann::json& j, const Transform& transform) {
  nlohmann::json json_rotation;
  nlohmann::json json_translation;

  // When used for a pose, the quaternion and vector3 fields are serialized as
  // an object rather than as an array
  json_rotation["w"] = transform.rotation.w;
  json_rotation["x"] = transform.rotation.x;
  json_rotation["y"] = transform.rotation.y;
  json_rotation["z"] = transform.rotation.z;

  json_translation["x"] = transform.translation.x;
  json_translation["y"] = transform.translation.y;
  json_translation["z"] = transform.translation.z;

  j["timestamp"] = transform.timestamp;
  j["frame_id"] = transform.frame_id;
  j["translation"] = json_translation;
  j["rotation"] = json_rotation;
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
