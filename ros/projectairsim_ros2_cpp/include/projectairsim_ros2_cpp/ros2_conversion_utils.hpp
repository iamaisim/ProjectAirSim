#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
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
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
             0;
}

inline bool IsImageTopic(const std::string& topic) {
  return EndsWith(topic, "_camera") || EndsWith(topic, "/scene_camera") ||
         EndsWith(topic, "/depth_camera") ||
         EndsWith(topic, "/depth_planar_camera") ||
         EndsWith(topic, "/depth_vis_camera") ||
         EndsWith(topic, "/disparity_normalized_camera") ||
         EndsWith(topic, "/segmentation_camera") ||
         EndsWith(topic, "/surface_normals_camera");
}

inline bool StartsWithPathPrefix(const std::string& value,
                                 const std::string& prefix) {
  return value == prefix || (value.size() > prefix.size() &&
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
    parts.push_back(
        path.substr(start, end == std::string::npos ? end : end - start));
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
  if (parts[root_parts.size()].empty() ||
      parts[root_parts.size()].front() == '$') {
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
  if (!object.is_object() || !object.contains(key) ||
      !object[key].is_number()) {
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
                        (16.0 * kPi * kPi * std::pow(range, 4)) * 0.7 * 15.0));
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
    radar_return.elevation =
        static_cast<float>(NumberOr(detection, "elevation"));
    radar_return.doppler_velocity =
        static_cast<float>(NumberOr(detection, "velocity"));
    radar_return.amplitude =
        RadarAmplitudeFromRangeAndRcs(range, NumberOr(detection, "rcs_sqm"));
    scan->returns.push_back(radar_return);
  }
}

inline void AppendRadarTracksFromJson(
    const json& tracks,
    projectairsim_ros2_cpp::msg::RadarTracks* radar_tracks) {
  if (!tracks.is_array()) return;
  radar_tracks->tracks.reserve(radar_tracks->tracks.size() + tracks.size());
  for (const auto& track : tracks) {
    projectairsim_ros2_cpp::msg::RadarTrack ros_track;
    ros_track.id = static_cast<int32_t>(NumberOr(track, "id"));
    ros_track.position =
        ToRosVector3(track.value("position_est", json::array()));
    ros_track.velocity =
        ToRosVector3(track.value("velocity_est", json::array()));
    ros_track.acceleration =
        ToRosVector3(track.value("accel_est", json::array()));
    ros_track.azimuth = static_cast<float>(NumberOr(track, "azimuth_est"));
    ros_track.elevation = static_cast<float>(NumberOr(track, "elevation_est"));
    ros_track.range = static_cast<float>(NumberOr(track, "range_est"));
    radar_tracks->tracks.push_back(ros_track);
  }
}

inline void PopulateCameraInfoFromJson(
    const json& msg, sensor_msgs::msg::CameraInfo* camera_info) {
  camera_info->width = static_cast<uint32_t>(NumberOr(msg, "width"));
  camera_info->height = static_cast<uint32_t>(NumberOr(msg, "height"));
  camera_info->distortion_model = msg.value("distortion_model", std::string());
  camera_info->d = ArrayNumbers(msg.value("distortion_params", json::array()));
  FillFixedArray(msg.value("intrinsic_camera_matrix", json::array()),
                 &camera_info->k);
  FillFixedArray(msg.value("rectification_matrix", json::array()),
                 &camera_info->r);
  FillFixedArray(msg.value("projection_matrix", json::array()),
                 &camera_info->p);
}

// Convert an IEEE 754 half-precision (binary16) bit pattern to float32.
inline float HalfBitsToFloat(uint16_t half_bits) {
  const uint32_t sign = static_cast<uint32_t>(half_bits & 0x8000u) << 16;
  uint32_t exponent = (half_bits >> 10) & 0x1Fu;
  uint32_t mantissa = half_bits & 0x3FFu;
  uint32_t float_bits;
  if (exponent == 0) {
    if (mantissa == 0) {
      float_bits = sign;  // +/- zero
    } else {
      // Subnormal half: normalize into a float32 exponent/mantissa.
      exponent = 127 - 15 + 1;
      while ((mantissa & 0x400u) == 0) {
        mantissa <<= 1;
        --exponent;
      }
      mantissa &= 0x3FFu;
      float_bits = sign | (exponent << 23) | (mantissa << 13);
    }
  } else if (exponent == 31) {
    float_bits = sign | 0x7F800000u | (mantissa << 13);  // inf / NaN
  } else {
    float_bits = sign | ((exponent - 15 + 127) << 23) | (mantissa << 13);
  }
  float result;
  std::memcpy(&result, &float_bits, sizeof(result));
  return result;
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
    // Legacy sims: uint16 depth. Kept for compatibility with older servers.
    image->encoding = "mono16";
    image->data = BytesFromJsonString(msg.value("data", ""));
    image->step = 2 * image->width;
    return true;
  }
  if (encoding == "16FC1") {
    // Depth as raw IEEE 754 half-precision (binary16) METERS, little-endian,
    // bit-exact with the sim's fp16 render target. Decode to the standard
    // ROS 32FC1 float-meters depth image; non-finite pixels (sky / no hit
    // arrive as +inf) become NaN per the ROS depth convention.
    const auto payload = BytesFromJsonString(msg.value("data", ""));
    const size_t pixel_count = payload.size() / 2;
    image->encoding = "32FC1";
    image->step = 4 * image->width;
    image->data.resize(pixel_count * 4);
    const uint8_t* src = payload.data();
    float* dst = reinterpret_cast<float*>(image->data.data());
    for (size_t i = 0; i < pixel_count; ++i, src += 2) {
      const uint16_t bits = static_cast<uint16_t>(src[0]) |
                            (static_cast<uint16_t>(src[1]) << 8);
      const float meters = HalfBitsToFloat(bits);
      dst[i] = std::isfinite(meters)
                   ? meters
                   : std::numeric_limits<float>::quiet_NaN();
    }
    return true;
  }
  return false;
}

struct NativeImageMetadata {
  std::string source_encoding;
  float pos_x = 0.0F;
  float pos_y = 0.0F;
  float pos_z = 0.0F;
  float rot_w = 1.0F;
  float rot_x = 0.0F;
  float rot_y = 0.0F;
  float rot_z = 0.0F;
};

namespace detail {

inline std::string_view ImageStringView(const msgpack::object& object,
                                        const char* field_name) {
  if (object.type == msgpack::type::STR) {
    return {object.via.str.ptr, object.via.str.size};
  }
  if (object.type == msgpack::type::BIN) {
    return {object.via.bin.ptr, object.via.bin.size};
  }
  throw std::runtime_error(std::string("Image field '") + field_name +
                           "' is not a string/bin value");
}

inline std::uint64_t ImageUnsigned(const msgpack::object& object,
                                   const char* field_name) {
  if (object.type == msgpack::type::POSITIVE_INTEGER) return object.via.u64;
  if (object.type == msgpack::type::NEGATIVE_INTEGER && object.via.i64 >= 0) {
    return static_cast<std::uint64_t>(object.via.i64);
  }
  throw std::runtime_error(std::string("Image field '") + field_name +
                           "' is not an unsigned integer");
}

inline std::uint32_t ImageUint32(const msgpack::object& object,
                                 const char* field_name) {
  const auto value = ImageUnsigned(object, field_name);
  if (value > std::numeric_limits<std::uint32_t>::max()) {
    throw std::runtime_error(std::string("Image field '") + field_name +
                             "' exceeds uint32 range");
  }
  return static_cast<std::uint32_t>(value);
}

inline bool ImageBool(const msgpack::object& object, const char* field_name) {
  if (object.type == msgpack::type::BOOLEAN) return object.via.boolean;
  if (object.type == msgpack::type::POSITIVE_INTEGER)
    return object.via.u64 != 0;
  throw std::runtime_error(std::string("Image field '") + field_name +
                           "' is not a bool");
}

inline float ImageFloat(const msgpack::object& object, const char* field_name) {
  switch (object.type) {
    case msgpack::type::FLOAT32:
    case msgpack::type::FLOAT64:
      return static_cast<float>(object.via.f64);
    case msgpack::type::POSITIVE_INTEGER:
      return static_cast<float>(object.via.u64);
    case msgpack::type::NEGATIVE_INTEGER:
      return static_cast<float>(object.via.i64);
    default:
      throw std::runtime_error(std::string("Image field '") + field_name +
                               "' is not numeric");
  }
}

}  // namespace detail

// Decode the native MessagePack image schema directly into a ROS image. This
// avoids materializing the multi-megabyte pixel payload as nlohmann::json. The
// single copy below is required to populate sensor_msgs::msg::Image::data.
inline bool PopulateImagePayloadFromMsgpack(const std::string& payload,
                                            sensor_msgs::msg::Image* image,
                                            NativeImageMetadata* metadata) {
  auto handle = msgpack::unpack(payload.data(), payload.size());
  const auto& object = handle.get();
  if (object.type != msgpack::type::MAP) {
    throw std::runtime_error("Image payload is not a MessagePack map");
  }

  NativeImageMetadata parsed_metadata;
  const char* data = nullptr;
  const msgpack::object* data_array = nullptr;
  std::size_t data_size = 0;
  bool data_is_array = false;
  bool has_height = false;
  bool has_width = false;
  bool has_encoding = false;
  bool has_big_endian = false;
  bool has_step = false;
  bool has_data = false;

  for (std::uint32_t i = 0; i < object.via.map.size; ++i) {
    const auto& key = object.via.map.ptr[i].key;
    const auto& value = object.via.map.ptr[i].val;
    if (key.type != msgpack::type::STR) continue;
    const std::string_view field(key.via.str.ptr, key.via.str.size);

    if (field == "height") {
      image->height = detail::ImageUint32(value, "height");
      has_height = true;
    } else if (field == "width") {
      image->width = detail::ImageUint32(value, "width");
      has_width = true;
    } else if (field == "encoding") {
      parsed_metadata.source_encoding =
          std::string(detail::ImageStringView(value, "encoding"));
      has_encoding = true;
    } else if (field == "big_endian") {
      image->is_bigendian =
          static_cast<std::uint8_t>(detail::ImageBool(value, "big_endian"));
      has_big_endian = true;
    } else if (field == "step") {
      (void)detail::ImageUint32(value, "step");
      has_step = true;
    } else if (field == "data") {
      if (value.type == msgpack::type::BIN) {
        data = value.via.bin.ptr;
        data_size = value.via.bin.size;
      } else if (value.type == msgpack::type::STR) {
        data = value.via.str.ptr;
        data_size = value.via.str.size;
      } else if (value.type == msgpack::type::ARRAY) {
        data_is_array = true;
        data_array = &value;
        data_size = value.via.array.size;
      } else {
        throw std::runtime_error("Image field 'data' is not bin/string/array");
      }
      has_data = true;
    } else if (field == "pos_x") {
      parsed_metadata.pos_x = detail::ImageFloat(value, "pos_x");
    } else if (field == "pos_y") {
      parsed_metadata.pos_y = detail::ImageFloat(value, "pos_y");
    } else if (field == "pos_z") {
      parsed_metadata.pos_z = detail::ImageFloat(value, "pos_z");
    } else if (field == "rot_w") {
      parsed_metadata.rot_w = detail::ImageFloat(value, "rot_w");
    } else if (field == "rot_x") {
      parsed_metadata.rot_x = detail::ImageFloat(value, "rot_x");
    } else if (field == "rot_y") {
      parsed_metadata.rot_y = detail::ImageFloat(value, "rot_y");
    } else if (field == "rot_z") {
      parsed_metadata.rot_z = detail::ImageFloat(value, "rot_z");
    }
  }

  if (!has_height || !has_width || !has_encoding || !has_big_endian ||
      !has_step || !has_data) {
    throw std::runtime_error("Image payload is missing required fields");
  }

  std::size_t bytes_per_pixel = 0;
  if (parsed_metadata.source_encoding == "BGR") {
    image->encoding = "bgr8";
    bytes_per_pixel = 3;
  } else if (parsed_metadata.source_encoding == "16UC1") {
    image->encoding = "mono16";
    bytes_per_pixel = 2;
  } else {
    if (metadata != nullptr) *metadata = std::move(parsed_metadata);
    return false;
  }

  const std::size_t row_bytes =
      static_cast<std::size_t>(image->width) * bytes_per_pixel;
  if (row_bytes > std::numeric_limits<std::uint32_t>::max()) {
    throw std::runtime_error("Image row size exceeds uint32 range");
  }
  if (image->height != 0 &&
      row_bytes > std::numeric_limits<std::size_t>::max() / image->height) {
    throw std::runtime_error("Image dimensions overflow payload size");
  }
  const std::size_t expected_min_bytes = row_bytes * image->height;
  if (data_size < expected_min_bytes) {
    throw std::runtime_error("Image payload is smaller than step*height");
  }

  image->step = static_cast<std::uint32_t>(row_bytes);
  image->data.resize(data_size);
  if (data_is_array) {
    for (std::size_t i = 0; i < data_size; ++i) {
      const auto byte =
          detail::ImageUnsigned(data_array->via.array.ptr[i], "data[]");
      if (byte > std::numeric_limits<std::uint8_t>::max()) {
        throw std::runtime_error("Image data byte exceeds uint8 range");
      }
      image->data[i] = static_cast<std::uint8_t>(byte);
    }
  } else if (data_size != 0) {
    std::memcpy(image->data.data(), data, data_size);
  }

  if (metadata != nullptr) *metadata = std::move(parsed_metadata);
  return true;
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
