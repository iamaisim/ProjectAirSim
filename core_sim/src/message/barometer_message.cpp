// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/barometer_message.hpp"

#include <sstream>
#include <string>

#include "json.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class BarometerMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp, float altitude, float pressure, float qnh);

  ~Impl() override {}

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, altitude, pressure, qnh);

  json getData() const;

 private:
  TimeNano time_stamp;
  float altitude;
  float pressure;
  float qnh;
};

BarometerMessage::BarometerMessage()
    : Message(std::make_shared<BarometerMessage::Impl>()) {}

BarometerMessage::BarometerMessage(TimeNano time_stamp, float altitude,
                                   float pressure, float qnh)
    : Message(std::make_shared<BarometerMessage::Impl>(time_stamp, altitude,
                                                       pressure, qnh)) {}

BarometerMessage::~BarometerMessage() {}

std::string BarometerMessage::Serialize() const {
  return static_cast<BarometerMessage::Impl*>(pimpl_.get())->Serialize();
}

void BarometerMessage::Deserialize(const std::string& buffer) {
  static_cast<BarometerMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json BarometerMessage::getData() const {
  return static_cast<BarometerMessage::Impl*>(pimpl_.get())->getData();
}

BarometerMessage::Impl::Impl() : MessageImpl(MessageType::kBarometer) {}

BarometerMessage::Impl::Impl(TimeNano time_stamp, float altitude,
                             float pressure, float qnh)
    : MessageImpl(MessageType::kBarometer),
      time_stamp(time_stamp),
      altitude(altitude),
      pressure(pressure),
      qnh(qnh) {}

std::string BarometerMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void BarometerMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json BarometerMessage::Impl::getData() const {
  return json({{"time_stamp", time_stamp},
               {"altitude", altitude},
               {"pressure", pressure},
               {"qnh", qnh}});
}

}  // namespace projectairsim
}  // namespace microsoft
