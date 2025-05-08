// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MODEL_OUTPUT_MESSAGE_HPP_
#define MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MODEL_OUTPUT_MESSAGE_HPP_

#include <sstream>
#include <string>

#include "common_message_utils.hpp"
#include "msgpack.hpp"
#include "control_message.hpp"

namespace microsoft {
namespace projectairsim {

class ControlModelOutputMessage : ControlMessage {
 public:
  ControlModelOutputMessage()
      : ControlMessage(ControlMessageType::kControlModelOutput) {}

  ControlModelOutputMessage(
      uint64_t time_stamp_val, std::vector<float> control_values_val)
      : ControlMessage(ControlMessageType::kControlModelOutput),
        time_stamp(time_stamp_val),
        control_values(control_values_val) {}

  ~ControlModelOutputMessage() override {}

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

  MSGPACK_DEFINE_MAP(time_stamp, control_values);

  // TODO Encapsulate with get/set methods?
  uint64_t time_stamp;
  std::vector<float> control_values;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MESSAGE_CONTROL_MODEL_OUTPUT_MESSAGE_HPP_
