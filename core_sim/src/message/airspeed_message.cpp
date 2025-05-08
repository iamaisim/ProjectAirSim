// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/airspeed_message.hpp"

#include <sstream>
#include <string>

#include "json.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class AirspeedMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp, float diff_pressure);

  ~Impl() override {}

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, diff_pressure);

  json getData() const;

 private:
  TimeNano time_stamp;  // Timestamp when reading was taken (nanoseconds)
  float diff_pressure;  // Differential pressure reading (Pascals)
};

AirspeedMessage::AirspeedMessage()
    : Message(std::make_shared<AirspeedMessage::Impl>()) {}

AirspeedMessage::AirspeedMessage(TimeNano time_stamp, float pressure)
    : Message(std::make_shared<AirspeedMessage::Impl>(time_stamp, pressure)) {}

AirspeedMessage::~AirspeedMessage() {}

std::string AirspeedMessage::Serialize() const {
  return static_cast<AirspeedMessage::Impl*>(pimpl_.get())->Serialize();
}

void AirspeedMessage::Deserialize(const std::string& buffer) {
  static_cast<AirspeedMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json AirspeedMessage::getData() const {
  return static_cast<AirspeedMessage::Impl*>(pimpl_.get())->getData();
}

AirspeedMessage::Impl::Impl() : MessageImpl(MessageType::kBarometer) {}

AirspeedMessage::Impl::Impl(TimeNano time_stamp, float diff_pressure)
    : MessageImpl(MessageType::kBarometer),
      time_stamp(time_stamp),
      diff_pressure(diff_pressure) {}

std::string AirspeedMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void AirspeedMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json AirspeedMessage::Impl::getData() const {
  return json({{"time_stamp", time_stamp}, {"diff_pressure", diff_pressure}});
}

}  // namespace projectairsim
}  // namespace microsoft
