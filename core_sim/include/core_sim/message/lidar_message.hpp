// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_LIDAR_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_LIDAR_MESSAGE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class LidarMessage : public Message {
 public:
  LidarMessage(TimeNano time_stamp_val, std::vector<float> point_cloud_val,
               std::vector<float> azimuth_elevation_range_cloud_val,
               std::vector<int> segmentation_cloud_val,
               std::vector<float> intensity_cloud_val,
               std::vector<int> laser_index_cloud_val, Pose pose_val);

  LidarMessage();

  ~LidarMessage() override;

  const std::vector<float> GetPointCloud() const;

  const std::vector<float> GetAzimuthElevationRangeCloud() const;

  const std::vector<int> GetSegmentationCloud() const;

  const std::vector<float> GetIntensityCloud() const;

  const std::vector<int> GetLaserIndexCloud() const;

  const Pose GetPose() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_LIDAR_MESSAGE_HPP_
