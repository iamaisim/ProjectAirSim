// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "core_sim/sensors/lidar.hpp"

#include <map>
#include <memory>
#include <string>

#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/logger.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

// forward declarations

class Lidar::Loader {
 public:
  explicit Loader(Lidar::Impl& impl);

  void Load(const nlohmann::json& json);

 private:
  void InitializeLidarSettings(){};
  void LoadLidarSettings(const nlohmann::json& json);
  void LoadLidarKindSetting(const nlohmann::json& json);
  void LoadOriginSetting(const nlohmann::json& json);

  Lidar::Impl& impl;
};

class Lidar::Impl : public SensorImpl {
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

  const LidarSettings& GetLidarSetting() const;

  const Transform& GetOrigin() const;

  void OnBeginUpdate() override;

  void PublishLidarMsg(const LidarMessage& lidar_msg);

  void OnEndUpdate() override;

 private:
  friend class Lidar::Loader;

  Lidar::Loader loader;

  LidarSettings lidar_settings;

  Topic lidar_topic;
  std::vector<Topic> topics;
};

// Default LIDAR settings for different types

// Mapping from LIDAR type ID string to settings
static const struct LidarKindEntry {
  std::string id;  // LIDAR type ID string
  const LidarSettings
      lidar_settings_default;  // Default settings for the type ID
} rglidar_kind_entry[] = {
    {Constant::Config::
         generic_cylindrical,  // Default settings for generic cylindrical scan
                               // pattern, based on Velodyne Puck (VLP-16),
                               // https://velodynelidar.com/products/puck
     LidarSettings(LidarKind::kGenericCylindrical,
                   Transform(Vector3(0, 0, -1),
                             Quaternion::Identity()),  // origin
                   Quaternion::Identity(),             // scan_orientation
                   16,                                 // number_of_channels
                   100.0f,                             // range
                   100000,                             // points_per_second
                   0.0f,                               // report_frequency
                   10.0f,   // horizontal_rotation_frequency
                   0.0f,    // horizontal_fov_start_deg
                   360.0f,  // horizontal_fov_end_deg
                   0.0f,    // vertical_rotation_frequency (ignored)
                   -45.0f,  // vertical_fov_lower_deg
                   -15.0f,  // vertical_fov_upper_deg
                   1.0f,    // radial_scaling
                   false,   // disable_self_hits
                   true,   // report_no_return_points
                   Vector3(0.0f, 0.0f, 0.0f),  // no_return_point_value
                   false                       // draw_debug_points
                   )},
    {Constant::Config::
         gpu_cylindrical,  // Default settings for GPU cylindrical scan
                           // pattern, based on Velodyne Puck (VLP-16),
                           // https://velodynelidar.com/products/puck
     LidarSettings(LidarKind::kGPUCylindrical,
                   Transform(Vector3(0, 0, -1),
                             Quaternion::Identity()),  // origin
                   Quaternion::Identity(),             // scan_orientation
                   16,                                 // number_of_channels
                   100.0f,                             // range
                   100000,                             // points_per_second
                   0.0f,                               // report_frequency
                   10.0f,   // horizontal_rotation_frequency
                   -45.0f,  // horizontal_fov_start_deg
                   45.0f,   // horizontal_fov_end_deg
                   0.0f,    // vertical_rotation_frequency (ignored)
                   -15.0f,  // vertical_fov_lower_deg
                   15.0f,   // vertical_fov_upper_deg
                   1.0f,    // radial_scaling
                   false,   // disable_self_hits
                   false,   // report_no_return_points
                   Vector3(0.0f, 0.0f, 0.0f),  // no_return_point_value
                   false                       // draw_debug_points
                   )},
    {Constant::Config::generic_rosette,  // Default settings for generic rosette
                                         // scan pattern
     LidarSettings(LidarKind::kGenericRosette,
                   Transform(Vector3(0, 0, -1),
                             Quaternion::Identity()),  // origin
                   Quaternion::Identity(),             // scan_orientation
                   1,                                  // number_of_channels
                   100.0f,                             // range
                   100000,                             // points_per_second
                   0.0f,                               // report_frequency
                   10.0f * 1.1f,   // horizontal_rotation_frequency (1.1 major
                                   // circle revolutions per 0.1 second)
                   0.0f,           // horizontal_fov_start_deg
                   360.0f,         // horizontal_fov_end_deg
                   10.0f * 10.0f,  // vertical_rotation_frequency (10 minor
                                   // circle revolutions per 0.1 sec)
                   -45.0f,         // vertical_fov_lower_deg
                   -15.0f,         // vertical_fov_upper_deg
                   1.0f,           // radial_scaling
                   false,          // disable_self_hits
                   false,          // report_no_return_points
                   Vector3(0.0f, 0.0f, 0.0f),  // no_return_point_value
                   false                       // draw_debug_points
                   )},
    {Constant::Config::livox_mid70,  // Default settings for Livox Mid-70
                                     // (https://www.livoxtech.com/mid-70)
     LidarSettings(
         LidarKind::kGenericRosette,
         Transform(Vector3(0, 0, -1),
                   Quaternion::Identity()),  // origin
         TransformUtils::ToQuaternion(
             0, MathUtils::deg2Rad(90),
             0),  // scan_orientation (rosette facing forward along +X axis)
         1,       // number_of_channels
         200.0f,  // range
         100000,  // points_per_second
         10.0f,   // report_frequency
         10.0f * (1 + 1.0f / 6) -
             1.0f / 360.0f,  // horizontal_rotation_frequency: 1-1/6
                             // major circle revolutions every 0.1 sec
                             // minus 1 degree per sec precession
         0.0f,               // horizontal_fov_start_deg
         360.0f,             // horizontal_fov_end_deg
         10.0f * 17.0f,      // vertical_rotation_frequency: 17 minor
                             // circle revolutions every 0.1 sec to
         // produce 18 scan "petals" per revolution
         -90.0f,  // vertical_fov_lower_deg
         -54.8f,  // vertical_fov_upper_deg
         0.375f,  // radial_scaling: produce elongated scan "petals"
                  // rather than circular
         false,   // disable_self_hits
         true,    // report_no_return_points
         Vector3(0.0f, 0.0f, 0.0f),  // no_return_point_value
         false                       // draw_debug_points
         )},
    {Constant::Config::livox_avia,  // Default settings for Livox Avia
                                    // (https://www.livoxtech.com/avia)
     LidarSettings(
         LidarKind::kGenericRosette,
         Transform(Vector3(0, 0, -1),
                   Quaternion::Identity()),  // origin
         TransformUtils::ToQuaternion(
             0, MathUtils::deg2Rad(90),
             0),  // scan_orientation (rosette facing forward along +X axis)
         6,       // number_of_channels
         400.0f,  // range
         240000,  // points_per_second
         10.0f,   // report_frequency
         10.0f * (1 + 1.0f / 6) -
             1.0f / 360.0f,  // horizontal_rotation_frequency: 1-1/6
                             // major circle revolutions every 0.1 sec
                             // minus 1 degree per sec precession
         0.0f,               // horizontal_fov_start_deg
         360.0f,             // horizontal_fov_end_deg
         10.0f * 17.0f,      // vertical_rotation_frequency: 17 minor
                             // circle revolutions every 0.1 sec to
         // produce 18 scan "petals" per revolution
         -90.0f,  // vertical_fov_lower_deg
         -54.8f,  // vertical_fov_upper_deg
         0.375f,  // radial_scaling: produce elongated scan "petals"
                  // rather than circular
         false,   // disable_self_hits
         true,    // report_no_return_points
         Vector3(0.0f, 0.0f, 0.0f),  // no_return_point_value
         false,                      // draw_debug_points
         0.0f,                       // distance_between_lasers (cm)
         0.15f,                      // angle_between_lasers_pitch_max (degrees)
         0.01f,                      // angle_between_lasers_pitch_min (degrees)
         1.1f,                       // angle_between_lasers_yaw_max (degrees)
         0.9f                        // angle_between_lasers_yaw_min (degrees)
         )},
};

// class Lidar

Lidar::Lidar() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Lidar::Lidar(const std::string& id, bool is_enabled,
             const std::string& parent_link, const Logger& logger,
             const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new Lidar::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void Lidar::Load(ConfigJson config_json) {
  static_cast<Lidar::Impl*>(pimpl.get())->Load(config_json);
}

void Lidar::Initialize(const Kinematics& kinematics,
                       const Environment& environment) {
  static_cast<Lidar::Impl*>(pimpl.get())->Initialize(kinematics, environment);
}

void Lidar::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Lidar::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

const LidarSettings& Lidar::GetLidarSettings() const {
  return static_cast<Lidar::Impl*>(pimpl.get())->GetLidarSetting();
}

void Lidar::BeginUpdate() {
  static_cast<Lidar::Impl*>(pimpl.get())->BeginUpdate();
}

void Lidar::PublishLidarMsg(const LidarMessage& lidar_msg) {
  static_cast<Lidar::Impl*>(pimpl.get())->PublishLidarMsg(lidar_msg);
}

void Lidar::EndUpdate() { static_cast<Lidar::Impl*>(pimpl.get())->EndUpdate(); }

// class lidar::impl

Lidar::Impl::Impl(const std::string& id, bool is_enabled,
                  const std::string& parent_link, const Logger& logger,
                  const TopicManager& topic_manager,
                  const std::string& parent_topic_path,
                  const ServiceManager& service_manager,
                  const StateManager& state_manager)
    : SensorImpl(SensorType::kLidar, id, is_enabled, parent_link,
                 Constant::Component::lidar, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader(*this) {
  SetTopicPath();
  CreateTopics();
}

void Lidar::Impl::Load(ConfigJson config_json) {
  nlohmann::json json = config_json;
  loader.Load(json);
}

void Lidar::Impl::Initialize(const Kinematics& kinematics,
                             const Environment& environment) {
  //! Ground-truth kinematics and environment info insn't used
  //! by the lidar impl
}

void Lidar::Impl::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  //! Lidar updates on render ticks from the Rendering engine
  //! (UE) and therefore does not execute any op specifically at scene/physics
  //! ticks
}

void Lidar::Impl::CreateTopics() {
  lidar_topic = Topic("lidar", topic_path_, TopicType::kPublished, 60,
                      MessageType::kLidar);

  topics.push_back(lidar_topic);
}

const LidarSettings& Lidar::Impl::GetLidarSetting() const {
  return lidar_settings;
}

void Lidar::Impl::OnBeginUpdate() { topic_manager_.RegisterTopic(lidar_topic); }

void Lidar::Impl::PublishLidarMsg(const LidarMessage& lidar_msg) {
  std::lock_guard<std::mutex> lock(update_lock_);

  topic_manager_.PublishTopic(lidar_topic, (Message&)lidar_msg);
}

void Lidar::Impl::OnEndUpdate() { topic_manager_.UnregisterTopic(lidar_topic); }

// class lidar::loader

Lidar::Loader::Loader(Lidar::Impl& impl) : impl(impl) {}

void Lidar::Loader::Load(const nlohmann::json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'Lidar_settings'.",
                          impl.id_.c_str());

  LoadLidarSettings(json);

  impl.is_loaded_ = true;

  impl.logger_.LogVerbose(impl.name_, "[%s] 'Lidar_settings' loaded.",
                          impl.id_.c_str());
}

void Lidar::Loader::LoadLidarSettings(const nlohmann::json& json) {
  LoadLidarKindSetting(json);

  LoadOriginSetting(json);

  impl.lidar_settings.scan_orientation = JsonUtils::GetQuaternion(
      json, Constant::Config::scan_rpy, impl.lidar_settings.scan_orientation);

  impl.lidar_settings.number_of_channels =
      JsonUtils::GetInteger(json, Constant::Config::number_of_channels,
                            impl.lidar_settings.number_of_channels);

  impl.lidar_settings.range = JsonUtils::GetNumber<float>(
      json, Constant::Config::range, impl.lidar_settings.range);

  impl.lidar_settings.points_per_second =
      JsonUtils::GetInteger(json, Constant::Config::points_per_second,
                            impl.lidar_settings.points_per_second);

  impl.lidar_settings.report_frequency =
      JsonUtils::GetNumber<float>(json, Constant::Config::report_frequency,
                                  impl.lidar_settings.report_frequency);

  impl.lidar_settings.horizontal_rotation_frequency =
      JsonUtils::GetNumber<float>(
          json, Constant::Config::horizontal_rotation_frequency,
          impl.lidar_settings.horizontal_rotation_frequency);

  impl.lidar_settings.horizontal_fov_start_deg = JsonUtils::GetNumber<float>(
      json, Constant::Config::horizontal_fov_start_deg,
      impl.lidar_settings.horizontal_fov_start_deg);

  impl.lidar_settings.horizontal_fov_end_deg = JsonUtils::GetNumber<float>(
      json, Constant::Config::horizontal_fov_end_deg,
      impl.lidar_settings.horizontal_fov_end_deg);

  impl.lidar_settings.vertical_rotation_frequency = JsonUtils::GetNumber<float>(
      json, Constant::Config::vertical_rotation_frequency,
      impl.lidar_settings.vertical_rotation_frequency);

  impl.lidar_settings.vertical_fov_upper_deg = JsonUtils::GetNumber<float>(
      json, Constant::Config::vertical_fov_upper_deg,
      impl.lidar_settings.vertical_fov_upper_deg);

  impl.lidar_settings.vertical_fov_lower_deg = JsonUtils::GetNumber<float>(
      json, Constant::Config::vertical_fov_lower_deg,
      impl.lidar_settings.vertical_fov_lower_deg);

  impl.lidar_settings.disable_self_hits =
      JsonUtils::GetInteger(json, Constant::Config::disable_self_hits,
                            impl.lidar_settings.disable_self_hits);

  if (JsonUtils::HasKey(json, Constant::Config::no_return_point_value)) {
    impl.lidar_settings.no_return_point_value =
        JsonUtils::GetVector3(json, Constant::Config::no_return_point_value,
                              impl.lidar_settings.no_return_point_value);
    impl.lidar_settings.report_no_return_points =
        true;  // If no-return-point-value is specified, assume
               // report-no-return-points is true (will be overridden by an
               // explicit report-no-return-points)
  }

  impl.lidar_settings.report_no_return_points =
      JsonUtils::GetInteger(json, Constant::Config::report_no_return_points,
                            impl.lidar_settings.report_no_return_points);

  impl.lidar_settings.draw_debug_points =
      JsonUtils::GetInteger(json, Constant::Config::draw_debug_points,
                            impl.lidar_settings.draw_debug_points);

  impl.lidar_settings.distance_between_lasers = JsonUtils::GetNumber<float>(
      json, Constant::Config::distance_between_lasers,
      impl.lidar_settings.distance_between_lasers);

  impl.lidar_settings.angle_between_lasers_yaw_max =
      JsonUtils::GetNumber<float>(
          json, Constant::Config::angle_between_lasers_yaw_max,
          impl.lidar_settings.angle_between_lasers_yaw_max);

  impl.lidar_settings.angle_between_lasers_yaw_min =
      JsonUtils::GetNumber<float>(
          json, Constant::Config::angle_between_lasers_yaw_min,
          impl.lidar_settings.angle_between_lasers_yaw_min);

  impl.lidar_settings.angle_between_lasers_pitch_max =
      JsonUtils::GetNumber<float>(
          json, Constant::Config::angle_between_lasers_pitch_max,
          impl.lidar_settings.angle_between_lasers_pitch_max);

  impl.lidar_settings.angle_between_lasers_pitch_min =
      JsonUtils::GetNumber<float>(
          json, Constant::Config::angle_between_lasers_pitch_min,
          impl.lidar_settings.angle_between_lasers_pitch_min);

  impl.lidar_settings.report_point_cloud =
      JsonUtils::GetInteger(json, Constant::Config::report_point_cloud,
                            impl.lidar_settings.report_point_cloud);

  impl.lidar_settings.report_azimuth_elevation_range = JsonUtils::GetInteger(
      json, Constant::Config::report_azimuth_elevation_range,
      impl.lidar_settings.report_azimuth_elevation_range);
}

void Lidar::Loader::LoadLidarKindSetting(const nlohmann::json& json) {
  auto str_lidar_type =
      JsonUtils::GetString(json, Constant::Config::lidar_type);
  bool found_entry = false;

  if (!str_lidar_type.empty()) {
    for (auto& lidar_kind_entry : rglidar_kind_entry) {
      if (str_lidar_type == lidar_kind_entry.id) {
        impl.lidar_settings = lidar_kind_entry.lidar_settings_default;
        found_entry = true;
        break;
      }
    }
  }

  if (!found_entry)
    impl.logger_.LogError(impl.name_,
                          "Unrecognized 'lidar_type' value '%s'--resetting "
                          "to '%s'.",
                          str_lidar_type.c_str(),
                          Constant::Config::generic_cylindrical);
}

void Lidar::Loader::LoadOriginSetting(const nlohmann::json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'origin' missing or empty. Using default.");
  } else {
    impl.lidar_settings.origin =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl.logger_.LogVerbose(impl.name_, "'origin' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
