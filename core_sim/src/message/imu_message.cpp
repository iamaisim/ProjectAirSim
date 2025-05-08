// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/imu_message.hpp"

#include <array>
#include <memory>
#include <sstream>
#include <string>

#include "core_sim/math_utils.hpp"
#include "json.hpp"
#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;
class ImuMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val, Quaternion orientation_val,
       Vector3 angular_velocity_val, Vector3 linear_acceleration_val);

  ~Impl() override {}

  const Quaternion GetOrientation() const;
  const Vector3 GetAngularVelocity() const;
  const Vector3 GetLinearAcceleration() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, orientation, angular_velocity,
                     linear_acceleration);
  json getData() const;

 private:
  TimeNano time_stamp;
  QuaternionMsgpack orientation;
  Vector3Msgpack angular_velocity;
  Vector3Msgpack linear_acceleration;
};

ImuMessage::ImuMessage() : Message(std::make_shared<ImuMessage::Impl>()) {}

ImuMessage::ImuMessage(TimeNano time_stamp_val, Quaternion orientation_val,
                       Vector3 angular_velocity_val,
                       Vector3 linear_acceleration_val)
    : Message(std::make_shared<ImuMessage::Impl>(
          time_stamp_val, orientation_val, angular_velocity_val,
          linear_acceleration_val)) {}

ImuMessage::~ImuMessage() {}

const Quaternion ImuMessage::GetOrientation() const {
  return static_cast<ImuMessage::Impl*>(pimpl_.get())->GetOrientation();
}

const Vector3 ImuMessage::GetAngularVelocity() const {
  return static_cast<ImuMessage::Impl*>(pimpl_.get())->GetAngularVelocity();
}

const Vector3 ImuMessage::GetLinearAcceleration() const {
  return static_cast<ImuMessage::Impl*>(pimpl_.get())->GetLinearAcceleration();
}

std::string ImuMessage::Serialize() const {
  return static_cast<ImuMessage::Impl*>(pimpl_.get())->Serialize();
}

void ImuMessage::Deserialize(const std::string& buffer) {
  static_cast<ImuMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json ImuMessage::getData() const {
  return static_cast<ImuMessage::Impl*>(pimpl_.get())->getData();
}

ImuMessage::Impl::Impl() : MessageImpl(MessageType::kImu) {}

ImuMessage::Impl::Impl(TimeNano time_stamp_val, Quaternion orientation_val,
                       Vector3 angular_velocity_val,
                       Vector3 linear_acceleration_val)
    : MessageImpl(MessageType::kImu),
      time_stamp(time_stamp_val),
      orientation(orientation_val),
      angular_velocity(angular_velocity_val),
      linear_acceleration(linear_acceleration_val) {}

const Quaternion ImuMessage::Impl::GetOrientation() const {
  return orientation.ToQuaternion();
}

const Vector3 ImuMessage::Impl::GetAngularVelocity() const {
  return angular_velocity.ToVector3();
}

const Vector3 ImuMessage::Impl::GetLinearAcceleration() const {
  return linear_acceleration.ToVector3();
}

std::string ImuMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void ImuMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json ImuMessage::Impl::getData() const {
  auto orientation_quat = GetOrientation();
  auto angular_vel_vec = GetAngularVelocity();
  auto linear_acc_vec = GetLinearAcceleration();
  json orientation_json = {{"w", orientation_quat.w()},
                           {"x", orientation_quat.x()},
                           {"y", orientation_quat.y()},
                           {"z", orientation_quat.z()}};
  json angular_velocity_json = {{"x", angular_vel_vec.x()},
                                {"y", angular_vel_vec.y()},
                                {"z", angular_vel_vec.z()}};
  json linear_acceleration_json = {{"x", linear_acc_vec.x()},
                                   {"y", linear_acc_vec.y()},
                                   {"z", linear_acc_vec.z()}};
  return json({{"time_stamp", time_stamp},
               {"orientation", orientation_json},
               {"angular_velocity", angular_velocity_json},
               {"linear_acceleration", linear_acceleration_json}});
}

}  // namespace projectairsim
}  // namespace microsoft
