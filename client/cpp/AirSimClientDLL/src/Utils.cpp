// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "Utils.h"

#include <Windows.h>

#include <codecvt>
#include <vector>

#include "AirSimClient.h"
#include "core_sim/math_utils.hpp"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

std::string v_str_version;  // Client library version

std::vector<float> ParseStringWithSpacesToFloatArray(const std::string& str) {
  std::vector<float> vec_float;
  VecStr vec_str = internal::SplitString(str);

  vec_float.reserve(vec_str.size());
  for (auto& str_value : vec_str)
    vec_float.push_back((float)atof(str_value.c_str()));

  return (vec_float);
}

std::vector<float> GetObjectRPYQuatFromConfig(json json_config) {
  std::vector<float> vec_float_ret;

  if (json_config.contains("origin")) {
    bool fis_degrees = false;
    json& json_origin = json_config["origin"];
    std::vector<float> vec_float;
    float pitch = 0;
    float roll = 0;
    float yaw = 0;

    // Convert string into array of floats
    {
      const char* sz_field = nullptr;

      if (json_origin.contains("rpy"))
        sz_field = "rpy";
      else if (json_origin.contains("rpy-deg")) {
        sz_field = "rpy-deg";
        fis_degrees = true;
      }

      if (sz_field != nullptr)
        vec_float = ParseStringWithSpacesToFloatArray(json_origin[sz_field]);
    }

    // Extract RPY from the vector
    {
      size_t cfloat = vec_float.size();

      if (cfloat > 0) roll = vec_float[0];
      if (cfloat > 1) pitch = vec_float[1];
      if (cfloat > 2) yaw = vec_float[2];
    }

    // Convert from degrees to radians if necessary
    if (fis_degrees) {
      roll = (float)MathUtils::deg2Rad(roll);
      pitch = (float)MathUtils::deg2Rad(pitch);
      yaw = (float)MathUtils::deg2Rad(yaw);
    }

    // Convert from RPY to quaternion
    vec_float_ret = RPYToQuaternionCoordinates(roll, pitch, yaw);
  }

  return (vec_float_ret);
}

void LoadJSONValue(const json& json_in, const char* sz_key, float* pr_out) {
  auto& json_value = json_in[sz_key];

  if (json_value.is_number()) *pr_out = json_value;
}

/* Helper method for converting quaternion to/from roll/pitch/yaw
 *
 *  https:#en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
 *  The Euler angles given should be based on the order of body-fixed frame ZYX
 * (yaw, pitch, then roll), which is equivalent to world-fixed frame XYZ (roll,
 * pitch, then yaw). Pitch must be within -89.9 to +89.9 degrees due to gimbal
 * lock restrictions.
 *
 *  Args:
 *      roll, pitch, yaw        RPY angles (radians)
 *      float (rgfloat_out)[4]  Buffer to receive the quaternion coordinates (w,
 * x, y, z)
 *
 *  Returns:
 *      float (rgfloat_out)[4]  Updated with the quaternion coordinates
 */
std::vector<float> RPYToQuaternionCoordinates(float roll, float pitch,
                                              float yaw) {
  // pitch angle must be within -90 ~ +90 deg, apply clipping
  static const double kPitchLimit = MathUtils::deg2Rad(89.9);

  double t0, t1, t2, t3, t4, t5;
  double w, x, y, z;

  if (abs(pitch) > kPitchLimit) {
    log.Warning("Pitch exceeded +/-89.9 deg, value will be clipped");
    pitch = (float)fmax(-kPitchLimit, fmin(pitch, kPitchLimit));
  }

  t0 = cos(yaw * 0.5);
  t1 = sin(yaw * 0.5);
  t2 = cos(roll * 0.5);
  t3 = sin(roll * 0.5);
  t4 = cos(pitch * 0.5);
  t5 = sin(pitch * 0.5);

  w = t0 * t2 * t4 + t1 * t3 * t5;  // w
  x = t0 * t3 * t4 - t1 * t2 * t5;  // x
  y = t0 * t2 * t5 + t1 * t3 * t4;  // y
  z = t1 * t2 * t4 - t0 * t3 * t5;  // z

  return (std::vector<float>{(float)w, (float)x, (float)y, (float)z});
}

VecStr SplitString(const std::string& str) {
  VecStr vec_str;

  // Split into words that were separated by white-space
  {
    bool fin_word = false;
    auto it = str.cbegin();
    auto it_end = str.cend();
    std::string::const_iterator it_word;

    for (; it != it_end; ++it) {
      if (!fin_word) {
        if (!isspace(*it)) {
          fin_word = true;
          it_word = it;
        }
      }

      if (fin_word) {
        if (isspace(*it)) {
          vec_str.emplace_back(std::string(it_word, it));
          fin_word = false;
        }
      }
    }
    if (fin_word) vec_str.emplace_back(std::string(it_word, it));
  }

  return (vec_str);
}

void UpdateClientVersion(HMODULE hmodule) {
  if (!v_str_version.empty()) return;

  size_t cbVersionInfo = 0;
  unsigned int cwch;
  std::vector<BYTE> vecVersionInfo;
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wstring_convert;
  wchar_t wzPathModule[MAX_PATH];
  wchar_t* wzProductVersion;

  // Get the version resource from the DLL module file
  if ((GetModuleFileName(hmodule, wzPathModule, _countof(wzPathModule)) == 0) ||
      ((cbVersionInfo = GetFileVersionInfoSize(wzPathModule, 0)) == 0)) {
    return;
  }

  // Get the product version string
  vecVersionInfo.resize(cbVersionInfo);
  if ((GetFileVersionInfo(wzPathModule, 0, (int)cbVersionInfo,
                          vecVersionInfo.data()) == 0) ||
      (VerQueryValue(vecVersionInfo.data(),
                     L"\\StringFileInfo\\040904B0\\ProductVersion",
                     (LPVOID*)&wzProductVersion, &cwch) == 0)) {
    return;
  }

  v_str_version = (wstring_convert.to_bytes(wzProductVersion).c_str());
}

void FromJSON(const json& json_in, Pose* ppose_out) {
  if (json_in.contains("position"))
    FromJSON(json_in["position"], &ppose_out->position);
  if (json_in.contains("orientation"))
    FromJSON(json_in["orientation"], &ppose_out->orientation);
}

void FromJSON(const json& json_in, Quaternion* pquaternion_out) {
  LoadJSONValue(json_in, "w", &pquaternion_out->w);
  LoadJSONValue(json_in, "x", &pquaternion_out->x);
  LoadJSONValue(json_in, "y", &pquaternion_out->y);
  LoadJSONValue(json_in, "z", &pquaternion_out->z);
}

void FromJSON(const json& json_in, Transform* ptransform_out) {
  ptransform_out->frame_id = json_in["frame_id"];
  if (!json_in.contains("translation"))
    FromJSON(json_in["translation"], &ptransform_out->translation);
  if (!json_in.contains("rotation"))
    FromJSON(json_in["rotation"], &ptransform_out->rotation);
}

void FromJSON(const json& json_in, Vector3* pvec3_out) {
  LoadJSONValue(json_in, "x", &pvec3_out->x);
  LoadJSONValue(json_in, "y", &pvec3_out->y);
  LoadJSONValue(json_in, "z", &pvec3_out->z);
}

void ToJSON(const Pose& pose, json* pjson_inout) {
  json json_position;
  json json_orientation;

  ToJSON(pose.position, &json_position);
  (*pjson_inout)["position"] = json_position;

  ToJSON(pose.orientation, &json_orientation);
  (*pjson_inout)["orientation"] = json_orientation;
}

void ToJSON(const Quaternion& quaternion, json* pjson_inout) {
  (*pjson_inout)["w"] = quaternion.w;
  (*pjson_inout)["x"] = quaternion.x;
  (*pjson_inout)["y"] = quaternion.y;
  (*pjson_inout)["z"] = quaternion.z;
}

void ToJSON(const Transform& transform, json* pjson_inout) {
  json json_translation;
  json json_rotation;

  (*pjson_inout)["frame_id"] = transform.frame_id;

  ToJSON(transform.translation, &json_translation);
  (*pjson_inout)["translation"] = json_translation;

  ToJSON(transform.rotation, &json_rotation);
  (*pjson_inout)["rotation"] = json_rotation;
}

void ToJSON(const Vector3& vec3, json* pjson_inout) {
  (*pjson_inout)["x"] = vec3.x;
  (*pjson_inout)["y"] = vec3.y;
  (*pjson_inout)["z"] = vec3.z;
}

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
