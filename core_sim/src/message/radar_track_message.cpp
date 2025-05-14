// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/radar_track_message.hpp"

#include <memory>
#include <sstream>

#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class RadarTrackMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val, std::vector<RadarTrack> radar_tracks_val,
       Pose pose_val);

  ~Impl() override {}

  const std::vector<RadarTrack> GetRadarTracks() const;

  const Pose GetPose() const;

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(time_stamp, radar_tracks, pose);

 private:
  TimeNano time_stamp;
  std::vector<RadarTrackMsgpack> radar_tracks;
  PoseMsgpack pose;
};

// -----------------------------------------------------------------------------
// class RadarTrackMessage

RadarTrackMessage::RadarTrackMessage()
    : Message(std::make_shared<RadarTrackMessage::Impl>()) {}

RadarTrackMessage::RadarTrackMessage(TimeNano time_stamp_val,
                                     std::vector<RadarTrack> radar_tracks_val,
                                     Pose pose_val)
    : Message(std::make_shared<RadarTrackMessage::Impl>(
          time_stamp_val, radar_tracks_val, pose_val)) {}

RadarTrackMessage::~RadarTrackMessage() {}

const std::vector<RadarTrack> RadarTrackMessage::GetRadarTracks() const {
  return static_cast<RadarTrackMessage::Impl*>(pimpl_.get())->GetRadarTracks();
}

const Pose RadarTrackMessage::GetPose() const {
  return static_cast<RadarTrackMessage::Impl*>(pimpl_.get())->GetPose();
}

std::string RadarTrackMessage::Serialize() const {
  return static_cast<RadarTrackMessage::Impl*>(pimpl_.get())->Serialize();
}

void RadarTrackMessage::Deserialize(const std::string& buffer) {
  static_cast<RadarTrackMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class RadarTrackMessage::Impl

RadarTrackMessage::Impl::Impl() : MessageImpl(MessageType::kRadarTrack) {}

RadarTrackMessage::Impl::Impl(TimeNano time_stamp_val,
                              std::vector<RadarTrack> radar_tracks_val,
                              Pose pose_val)
    : MessageImpl(MessageType::kRadarTrack),
      time_stamp(time_stamp_val),
      pose(pose_val) {
  for (auto& track : radar_tracks_val) {
    radar_tracks.emplace_back(track);
  }
}

const std::vector<RadarTrack> RadarTrackMessage::Impl::GetRadarTracks() const {
  std::vector<RadarTrack> tracks;
  for (auto& track : radar_tracks) {
    tracks.emplace_back(track.ToRadarTrack());
  }
  return tracks;
}

const Pose RadarTrackMessage::Impl::GetPose() const { return pose.ToPose(); }

std::string RadarTrackMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void RadarTrackMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
