// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/gps_message.hpp"

#include <sstream>
#include <string>

#include "core_sim/message/message.hpp"
#include "json.hpp"
#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;
class GpsMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp, TimeMilli time_utc_millis, float latitude,
       float longitude, float altitude, float epv, float eph,
       int position_cov_type, int fix_type, Vector3 velocity);

  ~Impl() override {}

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, time_utc_millis, latitude, longitude, altitude,
                     epv, eph, position_cov_type, fix_type, velocity);
  json getData() const;

 private:
  TimeNano time_stamp;
  TimeMilli time_utc_millis;
  float latitude;
  float longitude;
  float altitude;
  float epv;
  float eph;
  int position_cov_type;  // Position covariance type
  int fix_type;           // GnssFixType Enum
  Vector3Msgpack velocity;
};

GpsMessage::GpsMessage() : Message(std::make_shared<GpsMessage::Impl>()) {}

GpsMessage::GpsMessage(TimeNano time_stamp, TimeMilli time_utc_millis,
                       float latitude, float longitude, float altitude,
                       float epv, float eph, int position_cov_type,
                       int fix_type, Vector3 velocity)
    : Message(std::make_shared<GpsMessage::Impl>(
          time_stamp, time_utc_millis, latitude, longitude, altitude, epv, eph,
          position_cov_type, fix_type, velocity)) {}

GpsMessage::~GpsMessage() {}

std::string GpsMessage::Serialize() const {
  return static_cast<GpsMessage::Impl*>(pimpl_.get())->Serialize();
}

void GpsMessage::Deserialize(const std::string& buffer) {
  static_cast<GpsMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json GpsMessage::getData() const {
  return static_cast<GpsMessage::Impl*>(pimpl_.get())->getData();
}

GpsMessage::Impl::Impl() : MessageImpl(MessageType::kGps) {}

GpsMessage::Impl::Impl(TimeNano time_stamp, TimeMilli time_utc_millis,
                       float latitude, float longitude, float altitude,
                       float epv, float eph, int position_cov_type,
                       int fix_type, Vector3 velocity)
    : MessageImpl(MessageType::kGps),
      time_stamp(time_stamp),
      time_utc_millis(time_utc_millis),
      latitude(latitude),
      longitude(longitude),
      altitude(altitude),
      epv(epv),
      eph(eph),
      position_cov_type(position_cov_type),
      fix_type(fix_type),
      velocity(velocity) {}

std::string GpsMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void GpsMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json GpsMessage::Impl::getData() const {
  json vel = {{"x", velocity.x}, {"y", velocity.y}, {"z", velocity.z}};
  return json({{"time_stamp", time_stamp},
               {"time_utc_millis", time_utc_millis},
               {"latitude", latitude},
               {"longitude", longitude},
               {"altitude", altitude},
               {"epv", epv},
               {"eph", eph},
               {"position_cov_type", position_cov_type},
               {"fix_type", fix_type},
               {"velocity", vel}});
}

}  // namespace projectairsim
}  // namespace microsoft
