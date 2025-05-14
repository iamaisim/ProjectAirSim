// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/flight_control_rc_input_message.hpp"

#include <initializer_list>
#include <memory>
#include <sstream>
#include <vector>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class FlightControlRCInputMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(std::initializer_list<float> list_channels);

  ~Impl() override {}

  std::vector<float> GetChannelsVec(void);

  std::string Serialize(void) override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(channels);

 private:
  std::vector<float> channels;
};

// -----------------------------------------------------------------------------
// class FlightControlRCInputMessage

FlightControlRCInputMessage::FlightControlRCInputMessage(void)
    : Message(std::make_shared<FlightControlRCInputMessage::Impl>()) {}

FlightControlRCInputMessage::FlightControlRCInputMessage(
    std::initializer_list<float> list_channels)
    : Message(
          std::make_shared<FlightControlRCInputMessage::Impl>(list_channels)) {}

FlightControlRCInputMessage::~FlightControlRCInputMessage(void) {}

std::vector<float> FlightControlRCInputMessage::GetChannelsVec(void) const {
  return static_cast<FlightControlRCInputMessage::Impl*>(pimpl_.get())
      ->GetChannelsVec();
}

std::string FlightControlRCInputMessage::Serialize(void) const {
  return static_cast<FlightControlRCInputMessage::Impl*>(pimpl_.get())
      ->Serialize();
}

void FlightControlRCInputMessage::Deserialize(const std::string& buffer) {
  static_cast<FlightControlRCInputMessage::Impl*>(pimpl_.get())
      ->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class FlightControlRCInputMessage::Impl

FlightControlRCInputMessage::Impl::Impl(void)
    : MessageImpl(MessageType::kFlightControlRCInput), channels() {}

FlightControlRCInputMessage::Impl::Impl(
    std::initializer_list<float> list_channels)
    : MessageImpl(MessageType::kFlightControlRCInput), channels() {
  channels.assign(list_channels.begin(), list_channels.end());
}

std::vector<float> FlightControlRCInputMessage::Impl::GetChannelsVec() {
  return channels;
}

std::string FlightControlRCInputMessage::Impl::Serialize(void) {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void FlightControlRCInputMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
