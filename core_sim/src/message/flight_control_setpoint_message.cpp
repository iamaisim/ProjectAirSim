// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/flight_control_setpoint_message.hpp"

#include <memory>
#include <sstream>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class FlightControlSetpointMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(float axes_0_val, float axes_1_val, float axes_2_val, float axes_3_val);

  ~Impl() override {}

  std::vector<float> GetAxesVec();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(axes_0, axes_1, axes_2, axes_3);

 private:
  float axes_0;
  float axes_1;
  float axes_2;
  float axes_3;
};

// -----------------------------------------------------------------------------
// class FlightControlSetpointMessage

FlightControlSetpointMessage::FlightControlSetpointMessage()
    : Message(std::make_shared<FlightControlSetpointMessage::Impl>()) {}

FlightControlSetpointMessage::FlightControlSetpointMessage(float axes_0_val,
                                                           float axes_1_val,
                                                           float axes_2_val,
                                                           float axes_3_val)
    : Message(std::make_shared<FlightControlSetpointMessage::Impl>(
          axes_0_val, axes_1_val, axes_2_val, axes_3_val)) {}

FlightControlSetpointMessage::~FlightControlSetpointMessage() {}

std::vector<float> FlightControlSetpointMessage::GetAxesVec() const {
  return static_cast<FlightControlSetpointMessage::Impl*>(pimpl_.get())
      ->GetAxesVec();
}

std::string FlightControlSetpointMessage::Serialize() const {
  return static_cast<FlightControlSetpointMessage::Impl*>(pimpl_.get())
      ->Serialize();
}

void FlightControlSetpointMessage::Deserialize(const std::string& buffer) {
  static_cast<FlightControlSetpointMessage::Impl*>(pimpl_.get())
      ->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class FlightControlSetpointMessage::Impl

FlightControlSetpointMessage::Impl::Impl()
    : MessageImpl(MessageType::kFlightControlSetpoint) {}

FlightControlSetpointMessage::Impl::Impl(float axes_0_val, float axes_1_val,
                                         float axes_2_val, float axes_3_val)
    : MessageImpl(MessageType::kFlightControlSetpoint),
      axes_0(axes_0_val),
      axes_1(axes_1_val),
      axes_2(axes_2_val),
      axes_3(axes_3_val) {}

std::vector<float> FlightControlSetpointMessage::Impl::GetAxesVec() {
  return std::vector<float>({axes_0, axes_1, axes_2, axes_3});
}

std::string FlightControlSetpointMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void FlightControlSetpointMessage::Impl::Deserialize(
    const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
