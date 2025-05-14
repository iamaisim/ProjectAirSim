// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RADAR_DETECTION_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RADAR_DETECTION_MESSAGE_HPP_

#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/sensors/radar.hpp"

namespace microsoft {
namespace projectairsim {

class RadarDetectionMessage : public Message {
 public:
  RadarDetectionMessage(TimeNano time_stamp_val,
                        std::vector<RadarDetection> radar_detections_val,
                        Pose pose_val);

  RadarDetectionMessage();

  ~RadarDetectionMessage() override;

  const std::vector<RadarDetection> GetRadarDetections() const;

  const Pose GetPose() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RADAR_DETECTION_MESSAGE_HPP_
