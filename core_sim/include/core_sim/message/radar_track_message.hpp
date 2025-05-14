// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RADAR_TRACK_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RADAR_TRACK_MESSAGE_HPP_

#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/sensors/radar.hpp"

namespace microsoft {
namespace projectairsim {

class RadarTrackMessage : public Message {
 public:
  RadarTrackMessage(TimeNano time_stamp_val,
                    std::vector<RadarTrack> radar_tracks_val, Pose pose_val);

  RadarTrackMessage();

  ~RadarTrackMessage() override;

  const std::vector<RadarTrack> GetRadarTracks() const;

  const Pose GetPose() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RADAR_TRACK_MESSAGE_HPP_
