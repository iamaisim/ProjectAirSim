// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "core_sim/sensors/radar.hpp"

#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

// class Radar

class Radar::Loader {
 public:
  explicit Loader(Radar::Impl& impl);

  void Load(const nlohmann::json& json);

 private:
  void InitializeRadarSettings();
  void LoadRadarSettings(const nlohmann::json& json);
  void LoadOriginSetting(const nlohmann::json& json);

  Radar::Impl& impl;
};

class Radar::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  void Update(const TimeNano, const TimeNano);

  void CreateTopics();

  const RadarSettings& GetRadarSetting() const;

  const Transform& GetOrigin() const;

  void OnBeginUpdate() override;

  void PublishRadarDetectionMsg(
      const RadarDetectionMessage& radar_detection_msg);

  void PublishRadarTrackMsg(const RadarTrackMessage& radar_track_msg);

  void OnEndUpdate() override;

  Vector3 GetSensorVelocity() const;

 private:
  friend class Radar::Loader;

  Radar::Loader loader_;

  RadarSettings radar_settings_;

  Topic radar_detection_topic_;
  Topic radar_track_topic_;
  std::vector<Topic> topics_;

  const Kinematics* kinematics_;
};

// class Radar

Radar::Radar() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Radar::Radar(const std::string& id, bool is_enabled,
             const std::string& parent_link, const Logger& logger,
             const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new Radar::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void Radar::Load(ConfigJson config_json) {
  static_cast<Radar::Impl*>(pimpl.get())->Load(config_json);
}

void Radar::Initialize(const Kinematics& kinematics,
                       const Environment& environment) {
  static_cast<Radar::Impl*>(pimpl.get())->Initialize(kinematics, environment);
}

void Radar::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Radar::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

const RadarSettings& Radar::GetRadarSettings() const {
  return static_cast<Radar::Impl*>(pimpl.get())->GetRadarSetting();
}

void Radar::BeginUpdate() {
  static_cast<Radar::Impl*>(pimpl.get())->BeginUpdate();
}

void Radar::PublishRadarDetectionMsg(const RadarDetectionMessage& radar_msg) {
  static_cast<Radar::Impl*>(pimpl.get())->PublishRadarDetectionMsg(radar_msg);
}

void Radar::PublishRadarTrackMsg(const RadarTrackMessage& radar_msg) {
  static_cast<Radar::Impl*>(pimpl.get())->PublishRadarTrackMsg(radar_msg);
}

void Radar::EndUpdate() { static_cast<Radar::Impl*>(pimpl.get())->EndUpdate(); }

Vector3 Radar::GetSensorVelocity() const {
  return static_cast<Radar::Impl*>(pimpl.get())->GetSensorVelocity();
}

// class Radar::impl

Radar::Impl::Impl(const std::string& id, bool is_enabled,
                  const std::string& parent_link, const Logger& logger,
                  const TopicManager& topic_manager,
                  const std::string& parent_topic_path,
                  const ServiceManager& service_manager,
                  const StateManager& state_manager)
    : SensorImpl(SensorType::kRadar, id, is_enabled, parent_link,
                 Constant::Component::radar, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader_(*this) {
  SetTopicPath();
  CreateTopics();
}

void Radar::Impl::Load(ConfigJson config_json) {
  nlohmann::json json = config_json;
  loader_.Load(json);
}

void Radar::Impl::Initialize(const Kinematics& kinematics,
                             const Environment& environment) {
  // Radar sensor needs to know its own velocity to calculate relative Doppler
  // velocity to the objects it detects, so keep ptr to the full kinematics.
  kinematics_ = &kinematics;
}

void Radar::Impl::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  // Radar updates on render ticks from the Rendering engine (UE) and therefore
  // does not execute any op specifically at scene/physics ticks
}

void Radar::Impl::CreateTopics() {
  radar_detection_topic_ =
      Topic("radar_detections", topic_path_, TopicType::kPublished, 60,
            MessageType::kRadarDetection);

  radar_track_topic_ = Topic("radar_tracks", topic_path_, TopicType::kPublished,
                             60, MessageType::kRadarTrack);

  topics_.push_back(radar_detection_topic_);
  topics_.push_back(radar_track_topic_);
}

const RadarSettings& Radar::Impl::GetRadarSetting() const {
  return radar_settings_;
}

void Radar::Impl::OnBeginUpdate() {
  topic_manager_.RegisterTopic(radar_detection_topic_);
  topic_manager_.RegisterTopic(radar_track_topic_);
}

void Radar::Impl::PublishRadarDetectionMsg(
    const RadarDetectionMessage& Radar_msg) {
  std::lock_guard<std::mutex> lock(update_lock_);
  topic_manager_.PublishTopic(radar_detection_topic_, (Message&)Radar_msg);
}

void Radar::Impl::PublishRadarTrackMsg(const RadarTrackMessage& Radar_msg) {
  std::lock_guard<std::mutex> lock(update_lock_);
  topic_manager_.PublishTopic(radar_track_topic_, (Message&)Radar_msg);
}

void Radar::Impl::OnEndUpdate() {
  for (const auto& topic : topics_) {
    topic_manager_.UnregisterTopic(topic);
  }
}

Vector3 Radar::Impl::GetSensorVelocity() const {
  // Radar sensor velocity is needed to calculate relative Doppler velocity to
  // the objects it detects.
  if (kinematics_ == nullptr) {
    logger_.LogWarning(
        name_, "[%s] GetSensorVelocity() was called but kinematics_ is null.",
        id_.c_str());
    return Vector3::Zero();
  } else {
    return kinematics_->twist.linear;
  }
}

// class Radar::loader

Radar::Loader::Loader(Radar::Impl& impl) : impl(impl) {}

void Radar::Loader::Load(const nlohmann::json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'Radar_settings'.",
                          impl.id_.c_str());

  LoadRadarSettings(json);

  impl.is_loaded_ = true;

  impl.logger_.LogVerbose(impl.name_, "[%s] 'Radar_settings' loaded.",
                          impl.id_.c_str());
}

void Radar::Loader::LoadRadarSettings(const nlohmann::json& json) {
  RadarSettings setting;

  LoadOriginSetting(json);

  // Load "fov" dict
  auto fov_settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::fov);
  if (JsonUtils::IsEmpty(fov_settings_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'fov' missing or empty. Using defaults");
  } else {
    impl.radar_settings_.fov_azimuth_max =
        JsonUtils::GetNumber<float>(fov_settings_json, Constant::Config::azimuth_max,
                             setting.fov_azimuth_max);

    impl.radar_settings_.fov_azimuth_min =
        JsonUtils::GetNumber<float>(fov_settings_json, Constant::Config::azimuth_min,
                             setting.fov_azimuth_min);

    impl.radar_settings_.fov_elevation_max =
        JsonUtils::GetNumber<float>(fov_settings_json, Constant::Config::elevation_max,
                             setting.fov_elevation_max);

    impl.radar_settings_.fov_elevation_min =
        JsonUtils::GetNumber<float>(fov_settings_json, Constant::Config::elevation_min,
                             setting.fov_elevation_min);

    impl.radar_settings_.fov_azimuth_resolution = JsonUtils::GetNumber<float>(
        fov_settings_json, Constant::Config::azimuth_resolution,
        setting.fov_azimuth_resolution);

    impl.radar_settings_.fov_elevation_resolution = JsonUtils::GetNumber<float>(
        fov_settings_json, Constant::Config::elevation_resolution,
        setting.fov_elevation_resolution);
  }

  impl.radar_settings_.range_max = JsonUtils::GetNumber<float>(
      json, Constant::Config::range_max, setting.range_max);

  impl.radar_settings_.range_resolution = JsonUtils::GetNumber<float>(
      json, Constant::Config::range_resolution, setting.range_resolution);

  impl.radar_settings_.velocity_max = JsonUtils::GetNumber<float>(
      json, Constant::Config::velocity_max, setting.velocity_max);

  impl.radar_settings_.velocity_resolution = JsonUtils::GetNumber<float>(
      json, Constant::Config::velocity_resolution, setting.velocity_resolution);

  impl.radar_settings_.detection_interval = JsonUtils::GetNumber<float>(
      json, Constant::Config::detection_interval, setting.detection_interval);

  impl.radar_settings_.track_interval = JsonUtils::GetNumber<float>(
      json, Constant::Config::track_interval, setting.track_interval);

  impl.radar_settings_.rcs_adjust_factor = JsonUtils::GetNumber<float>(
      json, Constant::Config::rcs_adjust_factor, setting.rcs_adjust_factor);

  impl.radar_settings_.draw_debug_points = JsonUtils::GetInteger(
      json, Constant::Config::draw_debug_points, setting.draw_debug_points);

  // Load "masks" array
  auto masks_settings_json = JsonUtils::GetArray(json, Constant::Config::masks);
  try {
    std::for_each(
        masks_settings_json.begin(), masks_settings_json.end(),
        [this](auto& cur_json) {
          // Load mask dict into vector of Radar masks
          RadarMask mask;
          mask.azimuth_min = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::azimuth_min, mask.azimuth_min);
          mask.azimuth_max = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::azimuth_max, mask.azimuth_max);

          mask.elevation_min = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::elevation_min, mask.elevation_min);
          mask.elevation_max = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::elevation_max, mask.elevation_max);

          mask.range_min = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::range_min, mask.range_min);
          mask.range_max = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::range_max, mask.range_max);

          mask.velocity_min = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::velocity_min, mask.velocity_min);
          mask.velocity_max = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::velocity_max, mask.velocity_max);

          mask.rcs_sqm_min = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::rcs_sqm_min, mask.rcs_sqm_min);
          mask.rcs_sqm_max = JsonUtils::GetNumber<float>(
              cur_json, Constant::Config::rcs_sqm_max, mask.rcs_sqm_max);

          impl.radar_settings_.masks.push_back(mask);
        });
  } catch (...) {
    impl.radar_settings_.masks.clear();
    throw;
  }
}

void Radar::Loader::LoadOriginSetting(const nlohmann::json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'origin' missing or empty. Using default.");
  } else {
    impl.radar_settings_.origin_setting =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl.logger_.LogVerbose(impl.name_, "'origin' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
