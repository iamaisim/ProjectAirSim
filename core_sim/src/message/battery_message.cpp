// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/battery_message.hpp"

#include <sstream>
#include <string>

#include "json.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class BatteryStateMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp, float battery_pct_remaining,
       uint32_t estimated_time_remaining,
       const std::string& battery_charge_state);

  ~Impl() override {}

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, battery_pct_remaining, estimated_time_remaining,
                     battery_charge_state);

  json getData() const;

 private:
  TimeNano time_stamp;      // Timestamp when reading was taken (nanoseconds)
  float battery_pct_remaining;  // Battery charge percent in float 0 to 100
  uint32_t estimated_time_remaining;
  std::string battery_charge_state;
};

BatteryStateMessage::BatteryStateMessage()
    : Message(std::make_shared<BatteryStateMessage::Impl>()) {}

BatteryStateMessage::BatteryStateMessage(
    TimeNano time_stamp, float battery_pct_remaining,
    uint32_t estimated_time_remaining, const std::string& battery_charge_state)
    : Message(std::make_shared<BatteryStateMessage::Impl>(
          time_stamp, battery_pct_remaining, estimated_time_remaining,
          battery_charge_state)) {}

BatteryStateMessage::~BatteryStateMessage() {}

std::string BatteryStateMessage::Serialize() const {
  return static_cast<BatteryStateMessage::Impl*>(pimpl_.get())->Serialize();
}

void BatteryStateMessage::Deserialize(const std::string& buffer) {
  static_cast<BatteryStateMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json BatteryStateMessage::getData() const {
  return static_cast<BatteryStateMessage::Impl*>(pimpl_.get())->getData();
}

BatteryStateMessage::Impl::Impl() : MessageImpl(MessageType::kBattery) {}

BatteryStateMessage::Impl::Impl(TimeNano time_stamp, float battery_pct_remaining,
                                uint32_t estimated_time_remaining,
                                const std::string& battery_charge_state)
    : MessageImpl(MessageType::kBattery),
      time_stamp(time_stamp),
      battery_pct_remaining(battery_pct_remaining),
      estimated_time_remaining(estimated_time_remaining),
      battery_charge_state(battery_charge_state) {}

std::string BatteryStateMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void BatteryStateMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json BatteryStateMessage::Impl::getData() const {
  return json({{"time_stamp", time_stamp},
               {"battery_pct_remaining", battery_pct_remaining},
               {"estimated_time_remaining", estimated_time_remaining},
               {"battery_charge_state", battery_charge_state}});
}

}  // namespace projectairsim
}  // namespace microsoft
