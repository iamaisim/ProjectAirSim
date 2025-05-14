// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MODEL_INPUT_MESSAGE_HPP_
#define MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MODEL_INPUT_MESSAGE_HPP_

#include <sstream>
#include <string>
#include <vector>

#include "common_message_utils.hpp"
#include "control_message.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

class ControlModelInputMessage : ControlMessage {
 public:
  ControlModelInputMessage()
      : ControlMessage(ControlMessageType::kControlModelInput) {}

  ControlModelInputMessage(
      uint64_t time_stamp_val, KinematicsFlatMsgpack kinematics_val,
      float airspeed_val, BarometerFlatMsgpack barometer_val,
      ImuFlatMsgpack imu_val, Vector3FlatMsgpack magnetometer_val,
      float distance_val, GpsFlatMsgpack gps_val)
      : ControlMessage(ControlMessageType::kControlModelInput),
        time_stamp(time_stamp_val),
        kinematics(kinematics_val),
        airspeed(airspeed_val),
        barometer(barometer_val),
        imu(imu_val),
        magnetometer(magnetometer_val),
        distance(distance_val),
        gps(gps_val) {}

  ~ControlModelInputMessage() override {}

  std::string Serialize() const override {
    std::stringstream stream;
    msgpack::packer<std::stringstream> packer(stream);
    this->msgpack_pack(packer);
    return stream.str();
  }

  void Deserialize(const std::string& buffer) override {
    auto handle = msgpack::unpack(buffer.data(), buffer.size());
    auto object = handle.get();
    this->msgpack_unpack(object);
  }

  MSGPACK_DEFINE_MAP(time_stamp, kinematics, airspeed, barometer, imu,
                     magnetometer, distance, gps);

  // TODO Encapsulate with get/set methods?
  uint64_t time_stamp;
  KinematicsFlatMsgpack kinematics;
  float airspeed;
  BarometerFlatMsgpack barometer;
  ImuFlatMsgpack imu;
  Vector3FlatMsgpack magnetometer;
  float distance;
  GpsFlatMsgpack gps;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MODEL_INPUT_MESSAGE_HPP_
