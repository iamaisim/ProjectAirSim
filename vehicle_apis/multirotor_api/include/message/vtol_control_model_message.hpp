// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MESSAGE_VTOL_CONTROL_MODEL_MESSAGE_HPP_
#define MULTIROTOR_API_INCLUDE_MESSAGE_VTOL_CONTROL_MODEL_MESSAGE_HPP_

#include <cmath>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "control_model_input_message.hpp"

namespace microsoft {
namespace projectairsim {

inline const msgpack::object& RequireExternalControlField(
    const msgpack::object& map, const char* name) {
  if (map.type == msgpack::type::MAP) {
    for (uint32_t i = 0; i < map.via.map.size; ++i) {
      const auto& entry = map.via.map.ptr[i];
      if (entry.key.type == msgpack::type::STR &&
          std::string(entry.key.via.str.ptr, entry.key.via.str.size) == name) {
        return entry.val;
      }
    }
  }
  throw std::invalid_argument(
      std::string("Missing external-controller field: ") + name);
}

struct ExternalControlSetpoint {
  enum Mode {
    Idle = 0,
    Velocity = 1,
    VelocityZ = 2,
    Position = 3,
    Heading = 4
  };
  enum Frame { WorldNED = 0, BodyFRD = 1 };
  enum Request { None = 0, Takeoff = 1, Land = 2, Hover = 3 };

  uint32_t mode = Idle;
  uint32_t frame = WorldNED;
  float x = 0, y = 0, z = 0;
  bool yaw_is_rate = true;
  float yaw = 0;
  int32_t vtol_mode = -1;
  uint32_t request = None;
  bool api_enabled = false;
  bool armed = false;

  MSGPACK_DEFINE_MAP(mode, frame, x, y, z, yaw_is_rate, yaw, vtol_mode, request,
                     api_enabled, armed);
};

struct ExternalEffector {
  std::string id;
  std::string kind;
  std::string units;
  float initial_value = 0;

  MSGPACK_DEFINE_MAP(id, kind, units, initial_value);
};

class VTOLControlModelInputMessage {
 public:
  uint32_t version = 1;
  ControlModelInputMessage state;
  ExternalControlSetpoint setpoint;
  std::vector<ExternalEffector> effectors;

  VTOLControlModelInputMessage() {
    state.time_stamp = 0;
    state.airspeed = 0;
    state.distance = 0;
  }

  std::string Serialize() const {
    std::stringstream stream;
    msgpack::pack(stream, *this);
    return stream.str();
  }

  void Deserialize(const std::string& buffer) {
    // Missing version fields must not be interpreted as a current message.
    VTOLControlModelInputMessage decoded;
    decoded.version = 0;
    auto handle = msgpack::unpack(buffer.data(), buffer.size());
    const auto& object = handle.get();
    for (const auto* key : {"version", "state", "setpoint", "effectors"}) {
      RequireExternalControlField(object, key);
    }
    const auto& command = RequireExternalControlField(object, "setpoint");
    for (const auto* key :
         {"mode", "frame", "x", "y", "z", "yaw_is_rate", "yaw", "vtol_mode",
          "request", "api_enabled", "armed"}) {
      RequireExternalControlField(command, key);
    }
    const auto& state = RequireExternalControlField(object, "state");
    for (const auto* key : {"time_stamp", "kinematics", "airspeed", "barometer",
                            "imu", "magnetometer", "distance", "gps"}) {
      RequireExternalControlField(state, key);
    }
    const auto& metadata = RequireExternalControlField(object, "effectors");
    if (metadata.type != msgpack::type::ARRAY) {
      throw std::invalid_argument("External effectors must be an array");
    }
    for (uint32_t i = 0; i < metadata.via.array.size; ++i) {
      for (const auto* key : {"id", "kind", "units", "initial_value"}) {
        RequireExternalControlField(metadata.via.array.ptr[i], key);
      }
    }
    object.convert(decoded);
    if (decoded.version != 1 || decoded.effectors.empty() ||
        decoded.setpoint.mode > ExternalControlSetpoint::Heading ||
        decoded.setpoint.frame > ExternalControlSetpoint::BodyFRD ||
        decoded.setpoint.request > ExternalControlSetpoint::Hover ||
        decoded.setpoint.vtol_mode < -1 || decoded.setpoint.vtol_mode > 1 ||
        !std::isfinite(decoded.setpoint.x) ||
        !std::isfinite(decoded.setpoint.y) ||
        !std::isfinite(decoded.setpoint.z) ||
        !std::isfinite(decoded.setpoint.yaw)) {
      throw std::invalid_argument("Invalid external-controller request");
    }
    *this = decoded;
  }

  MSGPACK_DEFINE_MAP(version, state, setpoint, effectors);
};

class VTOLControlModelOutputMessage {
 public:
  uint32_t version = 1;
  uint64_t time_stamp = 0;
  std::vector<float> control_values;
  bool ready = false;
  bool landed = true;

  std::string Serialize() const {
    std::stringstream stream;
    msgpack::pack(stream, *this);
    return stream.str();
  }

  void Deserialize(const std::string& buffer) {
    VTOLControlModelOutputMessage decoded;
    decoded.version = 0;
    auto handle = msgpack::unpack(buffer.data(), buffer.size());
    for (const auto* key :
         {"version", "time_stamp", "control_values", "ready", "landed"}) {
      RequireExternalControlField(handle.get(), key);
    }
    handle.get().convert(decoded);
    if (decoded.version != 1 || decoded.control_values.empty()) {
      throw std::invalid_argument("Invalid external-controller response");
    }
    for (float value : decoded.control_values) {
      if (!std::isfinite(value)) {
        throw std::invalid_argument("Nonfinite external-controller output");
      }
    }
    *this = decoded;
  }

  MSGPACK_DEFINE_MAP(version, time_stamp, control_values, ready, landed);
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MESSAGE_VTOL_CONTROL_MODEL_MESSAGE_HPP_
