#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include "geometry_msgs/msg/point.hpp"
#include "geometry_msgs/msg/quaternion.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "json.hpp"
#include "msgpack.hpp"
#include "projectairsim_ros2_cpp/msg/radar_scan.hpp"
#include "projectairsim_ros2_cpp/msg/radar_tracks.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"

namespace projectairsim_ros2_cpp::bridge {

using json = nlohmann::json;

constexpr double kPi = 3.14159265358979323846;

inline bool EndsWith(const std::string& value, const std::string& suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline bool StartsWithPathPrefix(const std::string& value,
                                 const std::string& prefix) {
  return value == prefix ||
         (value.size() > prefix.size() &&
          value.compare(0, prefix.size(), prefix) == 0 &&
          value[prefix.size()] == '/');
}

inline std::string ReplaceSuffix(std::string value, const std::string& suffix,
                                 const std::string& replacement) {
  if (EndsWith(value, suffix)) {
    value.replace(value.size() - suffix.size(), suffix.size(), replacement);
  }
  return value;
}

inline std::vector<std::string> SplitPath(const std::string& path) {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start < path.size()) {
    while (start < path.size() && path[start] == '/') {
      ++start;
    }
    if (start >= path.size()) break;

    const auto end = path.find('/', start);
    parts.push_back(path.substr(start, end == std::string::npos ? end : end - start));
    if (end == std::string::npos) break;
    start = end + 1;
  }
  return parts;
}

inline std::string SanitizeFrameId(std::string frame) {
  if (!frame.empty() && frame.front() == '/') frame.erase(frame.begin());
  std::replace(frame.begin(), frame.end(), '/', '_');
  return frame.empty() ? "map" : frame;
}

inline std::string TopicToFrameId(const std::string& topic) {
  const auto normalized_topic = ReplaceSuffix(topic, "/imu_kinematics", "/imu");
  const auto parts = SplitPath(normalized_topic);

  for (size_t i = 0; i + 1 < parts.size(); ++i) {
    if (parts[i] != "robots") continue;
    return parts[i + 1];
  }

  return SanitizeFrameId(normalized_topic);
}

inline std::string CameraFrameId(const std::string& topic) {
  return SanitizeFrameId(ReplaceSuffix(topic, "_info", ""));
}

inline std::string CameraInfoRosTopic(const std::string& image_ros_topic) {
  return image_ros_topic + "/camera_info";
}

inline std::string CameraImageTopicFromInfoTopic(const std::string& topic) {
  return ReplaceSuffix(topic, "_info", "");
}

inline std::string ParentTopicFromProjectAirSimTopic(
    const std::string& topic, const std::string& topic_root) {
  if (!StartsWithPathPrefix(topic, topic_root)) return "";

  const auto parts = SplitPath(topic);
  const auto root_parts = SplitPath(topic_root);
  if (parts.size() <= root_parts.size() + 1) return "";
  if (parts[root_parts.size()].empty() || parts[root_parts.size()].front() == '$') {
    return "";
  }

  std::string parent_topic = topic_root;
  if (!parent_topic.empty() && parent_topic.back() == '/') {
    parent_topic.pop_back();
  }
  parent_topic += "/" + parts[root_parts.size()];
  return parent_topic;
}

inline bool IsSceneParentTopic(const std::string& topic,
                               const std::string& topic_root) {
  if (!StartsWithPathPrefix(topic, topic_root) || topic == topic_root) {
    return false;
  }
  const auto parts = SplitPath(topic);
  const auto root_parts = SplitPath(topic_root);
  if (parts.size() <= root_parts.size()) return false;
  return !parts[root_parts.size()].empty() &&
         parts[root_parts.size()].front() != '$';
}

inline json MsgpackToJson(const msgpack::object& object) {
  switch (object.type) {
    case msgpack::type::NIL:
      return nullptr;
    case msgpack::type::BOOLEAN:
      return object.via.boolean;
    case msgpack::type::POSITIVE_INTEGER:
      return object.via.u64;
    case msgpack::type::NEGATIVE_INTEGER:
      return object.via.i64;
    case msgpack::type::FLOAT32:
    case msgpack::type::FLOAT64:
      return object.via.f64;
    case msgpack::type::STR:
      return std::string(object.via.str.ptr, object.via.str.size);
    case msgpack::type::BIN:
      return std::string(object.via.bin.ptr, object.via.bin.size);
    case msgpack::type::ARRAY: {
      json array = json::array();
      for (uint32_t i = 0; i < object.via.array.size; ++i) {
        array.push_back(MsgpackToJson(object.via.array.ptr[i]));
      }
      return array;
    }
    case msgpack::type::MAP: {
      json map = json::object();
      for (uint32_t i = 0; i < object.via.map.size; ++i) {
        const auto& key = object.via.map.ptr[i].key;
        if (key.type != msgpack::type::STR) continue;
        map[std::string(key.via.str.ptr, key.via.str.size)] =
            MsgpackToJson(object.via.map.ptr[i].val);
      }
      return map;
    }
    default:
      return nullptr;
  }
}

inline json UnpackProjectAirSimMessage(const std::string& payload) {
  auto handle = msgpack::unpack(payload.data(), payload.size());
  return MsgpackToJson(handle.get());
}

inline double NumberOr(const json& object, const char* key,
                       double fallback = 0.0) {
  if (!object.is_object() || !object.contains(key) || !object[key].is_number()) {
    return fallback;
  }
  return object[key].get<double>();
}

inline bool JsonToInt64(const json& value, std::int64_t* out) {
  if (value.is_number_integer()) {
    *out = value.get<std::int64_t>();
    return true;
  }
  if (value.is_number_unsigned()) {
    const auto unsigned_value = value.get<std::uint64_t>();
    if (unsigned_value >
        static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
      return false;
    }
    *out = static_cast<std::int64_t>(unsigned_value);
    return true;
  }
  if (value.is_number_float()) {
    *out = static_cast<std::int64_t>(std::llround(value.get<double>()));
    return true;
  }
  return false;
}

inline bool ExtractSimTimeNanos(const json& value, std::int64_t* nanosec) {
  if (JsonToInt64(value, nanosec)) return true;
  if (!value.is_object()) return false;

  const auto sec_it = value.find("sec");
  const auto nsec_it = value.find("nanosec");
  if (sec_it != value.end() && nsec_it != value.end()) {
    std::int64_t sec = 0;
    std::int64_t nsec = 0;
    if (JsonToInt64(*sec_it, &sec) && JsonToInt64(*nsec_it, &nsec)) {
      *nanosec = sec * 1000000000LL + nsec;
      return true;
    }
  }

  for (const auto* key : {"time_nanos", "sim_time_nanos", "nanosec", "nanos"}) {
    auto it = value.find(key);
    if (it != value.end() && JsonToInt64(*it, nanosec)) return true;
  }
  return false;
}

inline std::vector<double> ArrayNumbers(const json& value) {
  std::vector<double> numbers;
  if (!value.is_array()) return numbers;
  numbers.reserve(value.size());
  for (const auto& item : value) {
    if (item.is_number()) numbers.push_back(item.get<double>());
  }
  return numbers;
}

template <size_t N>
void FillFixedArray(const json& value, std::array<double, N>* out) {
  out->fill(0.0);
  const auto numbers = ArrayNumbers(value);
  for (size_t i = 0; i < std::min<size_t>(numbers.size(), N); ++i) {
    (*out)[i] = numbers[i];
  }
}

inline geometry_msgs::msg::Vector3 ToRosVector3(const json& value) {
  geometry_msgs::msg::Vector3 out;
  const auto numbers = ArrayNumbers(value);
  if (numbers.size() >= 3) {
    out.x = numbers[0];
    out.y = -numbers[1];
    out.z = -numbers[2];
  } else if (value.is_object()) {
    out.x = NumberOr(value, "x");
    out.y = -NumberOr(value, "y");
    out.z = -NumberOr(value, "z");
  }
  return out;
}

inline geometry_msgs::msg::Point ToRosPoint(const json& value) {
  const auto vector = ToRosVector3(value);
  geometry_msgs::msg::Point out;
  out.x = vector.x;
  out.y = vector.y;
  out.z = vector.z;
  return out;
}

inline geometry_msgs::msg::Quaternion ToRosQuaternion(const json& value) {
  geometry_msgs::msg::Quaternion out;
  if (value.is_array() && value.size() >= 4) {
    out.x = value[0].get<double>();
    out.y = -value[1].get<double>();
    out.z = -value[2].get<double>();
    out.w = value[3].get<double>();
  } else if (value.is_object()) {
    out.x = NumberOr(value, "x");
    out.y = -NumberOr(value, "y");
    out.z = -NumberOr(value, "z");
    out.w = NumberOr(value, "w", 1.0);
  } else {
    out.w = 1.0;
  }
  return out;
}

inline std::vector<uint8_t> BytesFromJsonString(const json& value) {
  std::vector<uint8_t> bytes;
  if (!value.is_string()) return bytes;
  const auto data = value.get<std::string>();
  bytes.assign(data.begin(), data.end());
  return bytes;
}

inline float RadarAmplitudeFromRangeAndRcs(double range, double rcs_sqm) {
  if (range <= 0.0 || rcs_sqm <= 0.0) return 0.0F;
  return static_cast<float>(
      10.0 * std::log10(rcs_sqm * 1000.0 /
                        (16.0 * kPi * kPi * std::pow(range, 4)) * 0.7 *
                        15.0));
}

inline void AppendRadarReturnsFromJson(
    const json& detections, projectairsim_ros2_cpp::msg::RadarScan* scan) {
  if (!detections.is_array()) return;
  scan->returns.reserve(scan->returns.size() + detections.size());
  for (const auto& detection : detections) {
    projectairsim_ros2_cpp::msg::RadarReturn radar_return;
    const auto range = NumberOr(detection, "range");
    radar_return.range = static_cast<float>(range);
    radar_return.azimuth = static_cast<float>(NumberOr(detection, "azimuth"));
    radar_return.elevation = static_cast<float>(NumberOr(detection, "elevation"));
    radar_return.doppler_velocity =
        static_cast<float>(NumberOr(detection, "velocity"));
    radar_return.amplitude =
        RadarAmplitudeFromRangeAndRcs(range, NumberOr(detection, "rcs_sqm"));
    scan->returns.push_back(radar_return);
  }
}

inline void AppendRadarTracksFromJson(
    const json& tracks, projectairsim_ros2_cpp::msg::RadarTracks* radar_tracks) {
  if (!tracks.is_array()) return;
  radar_tracks->tracks.reserve(radar_tracks->tracks.size() + tracks.size());
  for (const auto& track : tracks) {
    projectairsim_ros2_cpp::msg::RadarTrack ros_track;
    ros_track.id = static_cast<int32_t>(NumberOr(track, "id"));
    ros_track.position = ToRosVector3(track.value("position_est", json::array()));
    ros_track.velocity = ToRosVector3(track.value("velocity_est", json::array()));
    ros_track.acceleration = ToRosVector3(track.value("accel_est", json::array()));
    ros_track.azimuth = static_cast<float>(NumberOr(track, "azimuth_est"));
    ros_track.elevation = static_cast<float>(NumberOr(track, "elevation_est"));
    ros_track.range = static_cast<float>(NumberOr(track, "range_est"));
    radar_tracks->tracks.push_back(ros_track);
  }
}

inline void PopulateCameraInfoFromJson(const json& msg,
                                       sensor_msgs::msg::CameraInfo* camera_info) {
  camera_info->width = static_cast<uint32_t>(NumberOr(msg, "width"));
  camera_info->height = static_cast<uint32_t>(NumberOr(msg, "height"));
  camera_info->distortion_model = msg.value("distortion_model", std::string());
  camera_info->d = ArrayNumbers(msg.value("distortion_params", json::array()));
  FillFixedArray(msg.value("intrinsic_camera_matrix", json::array()), &camera_info->k);
  FillFixedArray(msg.value("rectification_matrix", json::array()), &camera_info->r);
  FillFixedArray(msg.value("projection_matrix", json::array()), &camera_info->p);
}

inline bool PopulateImagePayloadFromJson(const json& msg,
                                         sensor_msgs::msg::Image* image) {
  image->height = static_cast<uint32_t>(NumberOr(msg, "height"));
  image->width = static_cast<uint32_t>(NumberOr(msg, "width"));
  image->is_bigendian = static_cast<uint8_t>(NumberOr(msg, "big_endian"));
  const auto encoding = msg.value("encoding", std::string());
  if (encoding == "BGR") {
    image->encoding = "bgr8";
    image->data = BytesFromJsonString(msg.value("data", ""));
    image->step = 3 * image->width;
    return true;
  }
  if (encoding == "16UC1") {
    image->encoding = "mono16";
    image->data = BytesFromJsonString(msg.value("data", ""));
    image->step = 2 * image->width;
    return true;
  }
  return false;
}

inline std::string ResolveSceneConfigPath(const std::string& scene_config,
                                          const std::string& sim_config_path) {
  if (scene_config.empty()) return scene_config;

  const std::filesystem::path scene_path(scene_config);
  if (scene_path.is_absolute() || std::filesystem::exists(scene_path)) {
    return scene_path.string();
  }

  return (std::filesystem::path(sim_config_path) / scene_path).string();
}

}  // namespace projectairsim_ros2_cpp::bridge
