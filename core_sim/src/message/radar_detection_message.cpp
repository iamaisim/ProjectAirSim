// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/radar_detection_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class RadarDetectionMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val,
       std::vector<RadarDetection> radar_detections_val, Pose pose_val);

  ~Impl() override {}

  const std::vector<RadarDetection> GetRadarDetections() const;

  const Pose GetPose() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, radar_detections, pose);

 private:
  TimeNano time_stamp;
  std::vector<RadarDetectionMsgpack> radar_detections;
  PoseMsgpack pose;
};

// -----------------------------------------------------------------------------
// class RadarDetectionMessage

RadarDetectionMessage::RadarDetectionMessage()
    : Message(std::make_shared<RadarDetectionMessage::Impl>()) {}

RadarDetectionMessage::RadarDetectionMessage(
    TimeNano time_stamp_val, std::vector<RadarDetection> radar_detections_val,
    Pose pose_val)
    : Message(std::make_shared<RadarDetectionMessage::Impl>(
          time_stamp_val, radar_detections_val, pose_val)) {}

RadarDetectionMessage::~RadarDetectionMessage() {}

const std::vector<RadarDetection> RadarDetectionMessage::GetRadarDetections()
    const {
  return static_cast<RadarDetectionMessage::Impl*>(pimpl_.get())
      ->GetRadarDetections();
}

const Pose RadarDetectionMessage::GetPose() const {
  return static_cast<RadarDetectionMessage::Impl*>(pimpl_.get())->GetPose();
}

std::string RadarDetectionMessage::Serialize() const {
  return static_cast<RadarDetectionMessage::Impl*>(pimpl_.get())->Serialize();
}

void RadarDetectionMessage::Deserialize(const std::string& buffer) {
  static_cast<RadarDetectionMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class RadarDetectionMessage::Impl

RadarDetectionMessage::Impl::Impl()
    : MessageImpl(MessageType::kRadarDetection) {}

RadarDetectionMessage::Impl::Impl(
    TimeNano time_stamp_val, std::vector<RadarDetection> radar_detections_val,
    Pose pose_val)
    : MessageImpl(MessageType::kRadarDetection),
      time_stamp(time_stamp_val),
      pose(pose_val) {
  for (auto& detection : radar_detections_val) {
    radar_detections.emplace_back(detection);
  }
}

const std::vector<RadarDetection>
RadarDetectionMessage::Impl::GetRadarDetections() const {
  std::vector<RadarDetection> detections;
  for (auto& detection : radar_detections) {
    detections.emplace_back(detection.ToRadarDetection());
  }
  return detections;
}

const Pose RadarDetectionMessage::Impl::GetPose() const {
  return pose.ToPose();
}

std::string RadarDetectionMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void RadarDetectionMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
