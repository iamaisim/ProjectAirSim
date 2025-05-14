// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/magnetometer_message.hpp"

#include <sstream>
#include <string>

#include "json.hpp"
#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class MagnetometerMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp, Vector3 magnetic_field_body,
       std::vector<float> magnetic_field_covariance);

  ~Impl() override {}

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, magnetic_field_body,
                     magnetic_field_covariance);
  json getData() const;

 private:
  TimeNano time_stamp;
  Vector3Msgpack magnetic_field_body;  // in Gauss
  std::vector<float>
      magnetic_field_covariance;  // 9 elements of the 3x3 covariance matrix
};

MagnetometerMessage::MagnetometerMessage()
    : Message(std::make_shared<MagnetometerMessage::Impl>()) {}

MagnetometerMessage::MagnetometerMessage(
    TimeNano time_stamp, Vector3 magnetic_field_body,
    std::vector<float> magnetic_field_covariance)
    : Message(std::make_shared<MagnetometerMessage::Impl>(
          time_stamp, magnetic_field_body, magnetic_field_covariance)) {}

MagnetometerMessage::~MagnetometerMessage() {}

std::string MagnetometerMessage::Serialize() const {
  return static_cast<MagnetometerMessage::Impl*>(pimpl_.get())->Serialize();
}

void MagnetometerMessage::Deserialize(const std::string& buffer) {
  static_cast<MagnetometerMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json MagnetometerMessage::getData() const {
  return static_cast<MagnetometerMessage::Impl*>(pimpl_.get())->getData();
}

MagnetometerMessage::Impl::Impl() : MessageImpl(MessageType::kMagnetometer) {}

MagnetometerMessage::Impl::Impl(TimeNano time_stamp,
                                Vector3 magnetic_field_body,
                                std::vector<float> magnetic_field_covariance)
    : MessageImpl(MessageType::kMagnetometer),
      time_stamp(time_stamp),
      magnetic_field_body(magnetic_field_body),
      magnetic_field_covariance(magnetic_field_covariance) {}

std::string MagnetometerMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void MagnetometerMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json MagnetometerMessage::Impl::getData() const {
  json mfb = {{"x", magnetic_field_body.x},
              {"y", magnetic_field_body.y},
              {"z", magnetic_field_body.z}};
  return json({{"time_stamp", time_stamp},
               {"magnetic_field_body", mfb},
               {"magnetic_field_covariance", magnetic_field_covariance}});
}

}  // namespace projectairsim
}  // namespace microsoft
