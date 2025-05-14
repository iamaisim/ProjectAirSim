// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/lidar_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class LidarMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val, std::vector<float> point_cloud_val,
       std::vector<float> azimuth_elevation_range_cloud_val,
       std::vector<int> segmentation_cloud_val,
       std::vector<float> intensity_cloud_val,
       std::vector<int> laser_index_cloud_val, Pose pose_val);

  ~Impl() override {}

  const std::vector<float> GetPointCloud() const;

  const std::vector<float> GetAzimuthElevationRangeCloud() const;

  const std::vector<int> GetSegmentationCloud() const;

  const std::vector<float> GetIntensityCloud() const;

  const std::vector<int> GetLaserIndexCloud() const;

  const Pose GetPose() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, point_cloud, azimuth_elevation_range_cloud, segmentation_cloud,
                     intensity_cloud, laser_index_cloud, pose);

 private:
  TimeNano time_stamp;
  std::vector<float> point_cloud;  // todo generic vectortemplatemsgpack?
  std::vector<float> azimuth_elevation_range_cloud;
  std::vector<int> segmentation_cloud;
  std::vector<float> intensity_cloud;
  std::vector<int> laser_index_cloud;
  PoseMsgpack pose;
};

// -----------------------------------------------------------------------------
// class LidarMessage

LidarMessage::LidarMessage()
    : Message(std::make_shared<LidarMessage::Impl>()) {}

LidarMessage::LidarMessage(TimeNano time_stamp_val,
                           std::vector<float> point_cloud_val,
                           std::vector<float> azimuth_elevation_range_cloud_val,
                           std::vector<int> segmentation_cloud_val,
                           std::vector<float> intensity_cloud_val,
                           std::vector<int> laser_index_cloud_val,
                           Pose pose_val)
    : Message(std::make_shared<LidarMessage::Impl>(
          time_stamp_val, point_cloud_val, azimuth_elevation_range_cloud_val, segmentation_cloud_val,
          intensity_cloud_val, laser_index_cloud_val, pose_val)) {}

LidarMessage::~LidarMessage() {}

const std::vector<float> LidarMessage::GetPointCloud() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->GetPointCloud();
}

const std::vector<float> LidarMessage::GetAzimuthElevationRangeCloud() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->GetAzimuthElevationRangeCloud();
}

const std::vector<int> LidarMessage::GetSegmentationCloud() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->GetSegmentationCloud();
}

const std::vector<float> LidarMessage::GetIntensityCloud() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->GetIntensityCloud();
}

const std::vector<int> LidarMessage::GetLaserIndexCloud() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->GetLaserIndexCloud();
}

const Pose LidarMessage::GetPose() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->GetPose();
}

std::string LidarMessage::Serialize() const {
  return static_cast<LidarMessage::Impl*>(pimpl_.get())->Serialize();
}

void LidarMessage::Deserialize(const std::string& buffer) {
  static_cast<LidarMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class LidarMessage::Impl

LidarMessage::Impl::Impl() : MessageImpl(MessageType::kLidar) {}

LidarMessage::Impl::Impl(TimeNano time_stamp_val,
                         std::vector<float> point_cloud_val,
                         std::vector<float> azimuth_elevation_range_cloud_val,
                         std::vector<int> segmentation_cloud_val,
                         std::vector<float> intensity_cloud_val,
                         std::vector<int> laser_index_cloud_val, Pose pose_val)
    : MessageImpl(MessageType::kLidar),
      time_stamp(time_stamp_val),
      point_cloud(point_cloud_val),
      azimuth_elevation_range_cloud(azimuth_elevation_range_cloud_val),
      segmentation_cloud(segmentation_cloud_val),
      intensity_cloud(intensity_cloud_val),
      laser_index_cloud(laser_index_cloud_val),
      pose(pose_val) {}

const std::vector<float> LidarMessage::Impl::GetPointCloud() const {
  return point_cloud;
}

const std::vector<float> LidarMessage::Impl::GetAzimuthElevationRangeCloud() const {
  return azimuth_elevation_range_cloud;
}

const std::vector<int> LidarMessage::Impl::GetSegmentationCloud() const {
  return segmentation_cloud;
}

const std::vector<float> LidarMessage::Impl::GetIntensityCloud() const {
  return intensity_cloud;
}

const std::vector<int> LidarMessage::Impl::GetLaserIndexCloud() const {
  return laser_index_cloud;
}

const Pose LidarMessage::Impl::GetPose() const { return pose.ToPose(); }

std::string LidarMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void LidarMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
