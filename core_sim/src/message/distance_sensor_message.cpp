// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/distance_sensor_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class DistanceSensorMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val, float current_distance, Pose pose_val);

  ~Impl() override {}

  const float GetCurrentDistance() const;

  const Pose GetPose() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, current_distance, pose);

 private:
  TimeNano time_stamp;
  float current_distance;
  float max_distance;
  float min_distance;
  PoseMsgpack pose;
};

// -----------------------------------------------------------------------------
// class DistanceSensorMessage

DistanceSensorMessage::DistanceSensorMessage()
    : Message(std::make_shared<DistanceSensorMessage::Impl>()) {}

DistanceSensorMessage::DistanceSensorMessage(TimeNano time_stamp_val,
                                             float current_distance_val,
                                             Pose pose_val)
    : Message(std::make_shared<DistanceSensorMessage::Impl>(
          time_stamp_val, current_distance_val, pose_val)) {}

DistanceSensorMessage::~DistanceSensorMessage() {}

const float DistanceSensorMessage::GetCurrentDistance() const {
  return static_cast<DistanceSensorMessage::Impl*>(pimpl_.get())
      ->GetCurrentDistance();
}

const Pose DistanceSensorMessage::GetPose() const {
  return static_cast<DistanceSensorMessage::Impl*>(pimpl_.get())->GetPose();
}

std::string DistanceSensorMessage::Serialize() const {
  return static_cast<DistanceSensorMessage::Impl*>(pimpl_.get())->Serialize();
}

void DistanceSensorMessage::Deserialize(const std::string& buffer) {
  static_cast<DistanceSensorMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class DistanceSensorMessage::Impl

DistanceSensorMessage::Impl::Impl()
    : MessageImpl(MessageType::kDistanceSensor) {}

DistanceSensorMessage::Impl::Impl(TimeNano time_stamp_val,
                                  float current_distance_val, Pose pose_val)
    : MessageImpl(MessageType::kDistanceSensor),
      time_stamp(time_stamp_val),
      current_distance(current_distance_val),
      pose(pose_val) {}

const float DistanceSensorMessage::Impl::GetCurrentDistance() const {
  return current_distance;
}

const Pose DistanceSensorMessage::Impl::GetPose() const {
  return pose.ToPose();
}

std::string DistanceSensorMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void DistanceSensorMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
