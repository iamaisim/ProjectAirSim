// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "core_sim/sensors/gps.hpp"

#include "constant.hpp"
#include "core_sim/first_order_filter.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/gps_message.hpp"
#include "core_sim/topic.hpp"
#include "json.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;

class Gps::Loader {
 public:
  explicit Loader(Gps::Impl& impl);

  void Load(const json& json);

 private:
  void LoadGpsSettings(const json& json);

  Gps::Impl& impl_;
};

enum NavSatStatusType : char {
  STATUS_NO_FIX = 80,   // unable to fix position
  STATUS_FIX = 0,       // unaugmented fix
  STATUS_SBAS_FIX = 1,  // with satellite-based augmentation
  STATUS_GBAS_FIX = 2   // with ground-based augmentation
};

enum NavSatStatusServiceType : unsigned short int {
  SERVICE_GPS = 1,
  SERVICE_GLONASS = 2,
  SERVICE_COMPASS = 4,  // includes BeiDou.
  SERVICE_GALILEO = 8
};

struct NavSatStatus {
  NavSatStatusType status;
  NavSatStatusServiceType service;
};

enum PositionCovarianceType : unsigned char {
  COVARIANCE_TYPE_UNKNOWN = 0,
  COVARIANCE_TYPE_APPROXIMATED = 1,
  COVARIANCE_TYPE_DIAGONAL_KNOWN = 2,
  COVARIANCE_TYPE_KNOWN = 3
};

enum GnssFixType : unsigned char {
  GNSS_FIX_NO_FIX = 0,
  GNSS_FIX_TIME_ONLY = 1,
  GNSS_FIX_2D_FIX = 2,
  GNSS_FIX_3D_FIX = 3
};

struct GpsData {
  GeoPoint geo_point;
  float eph, epv;  // GPS HDOP/VDOP horizontal/vertical dilution of position
                   // (unitless), 0-100%
  PositionCovarianceType position_cov_type =
      PositionCovarianceType::COVARIANCE_TYPE_UNKNOWN;
  Vector3 velocity;
  GnssFixType fix_type = GnssFixType::GNSS_FIX_NO_FIX;
  NavSatStatus nav_sat_status;

  TimeNano time_stamp = 0;
  TimeMilli time_utc_millis = 0;
};

class Gps::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  const GpsParams& GetGpsSettings() const;

  void OnBeginUpdate() override;

  void CreateTopics();

  void ApplyNoiseModel(GpsData&, const TimeSec&);

  void Update(const TimeNano, const TimeNano);

  GpsMessage getOutput();

  void RegisterServiceMethod();

  void OnEndUpdate() override;

 private:
  friend class Gps::Loader;

  Gps::Loader loader_;

  GpsParams gps_settings_;

  Topic gps_topic_;

  std::vector<std::reference_wrapper<Topic>> topics_;

  struct GroundTruth {
    const Kinematics* kinematics;
    const Environment* environment;
  } ground_truth_;

  FirstOrderFilter<float> epv_filter, eph_filter;

  TimeNano last_update_time_;

  GpsData output_;  // To hold latest sensor data
};

Gps::Gps() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Gps::Gps(const std::string& id, bool is_enabled, const std::string& parent_link,
         const Logger& logger, const TopicManager& topic_manager,
         const std::string& parent_topic_path,
         const ServiceManager& service_manager,
         const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(
          new Gps::Impl(id, is_enabled, parent_link, logger, topic_manager,
                        parent_topic_path, service_manager, state_manager))) {}

void Gps::Load(ConfigJson config_json) {
  return static_cast<Gps::Impl*>(pimpl.get())->Load(config_json);
}

void Gps::Initialize(const Kinematics& kinematics,
                     const Environment& environment) {
  static_cast<Gps::Impl*>(pimpl.get())->Initialize(kinematics, environment);
}

const GpsParams& Gps::GetGpsSettings() const {
  return static_cast<Gps::Impl*>(pimpl.get())->GetGpsSettings();
}

void Gps::BeginUpdate() { static_cast<Gps::Impl*>(pimpl.get())->BeginUpdate(); }

void Gps::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Gps::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

GpsMessage Gps::getOutput() const {
  return static_cast<Gps::Impl*>(pimpl.get())->getOutput();
}

void Gps::EndUpdate() { static_cast<Gps::Impl*>(pimpl.get())->EndUpdate(); }

Gps::Impl::Impl(const std::string& id, bool is_enabled,
                const std::string& parent_link, const Logger& logger,
                const TopicManager& topic_manager,
                const std::string& parent_topic_path,
                const ServiceManager& service_manager,
                const StateManager& state_manager)
    : SensorImpl(SensorType::kGps, id, is_enabled, parent_link,
                 Constant::Component::gps, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader_(*this) {
  SetTopicPath();
  CreateTopics();
}

void Gps::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

void Gps::Impl::Initialize(const Kinematics& kinematics,
                           const Environment& environment) {
  this->ground_truth_.kinematics = &kinematics;
  this->ground_truth_.environment = &environment;

  epv_filter.Initialize(gps_settings_.epv_time_constant,
                        gps_settings_.epv_final, gps_settings_.epv_initial);
  eph_filter.Initialize(gps_settings_.eph_time_constant,
                        gps_settings_.eph_final, gps_settings_.eph_initial);
  epv_filter.Reset();  // (re)set input_ & output_ for the filter
  eph_filter.Reset();
}

void Gps::Impl::CreateTopics() {
  gps_topic_ =
      Topic("gps", topic_path_, TopicType::kPublished, 100, MessageType::kGps);
  topics_.push_back(gps_topic_);
}

const GpsParams& Gps::Impl::GetGpsSettings() const { return gps_settings_; }

void Gps::Impl::OnBeginUpdate() {
  for (const auto& topic_ref : topics_) {
    topic_manager_.RegisterTopic(topic_ref.get());
  }

  RegisterServiceMethod();
}

void Gps::Impl::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto current_time = sim_time;

  TimeSec dt = sim_dt_nanos / 1.0e9;

  output_.time_stamp = current_time;
  output_.time_utc_millis =
      static_cast<uint64_t>(current_time / 1.0e3);  // in millisec
  output_.geo_point = ground_truth_.environment->env_info.geo_point;
  output_.velocity = ground_truth_.kinematics->twist.linear;

  //! Apply filter order filter for HDOP & VDOP & update GPS fix type
  ApplyNoiseModel(output_, dt);

  //! Prepare GPS sensor msg to publish
  GpsMessage gps_msg(output_.time_stamp, output_.time_utc_millis,
                     output_.geo_point.latitude, output_.geo_point.longitude,
                     output_.geo_point.altitude, output_.epv, output_.eph,
                     output_.position_cov_type, output_.fix_type,
                     output_.velocity);

  //! Publish GPS sensor output msg
  topic_manager_.PublishTopic(gps_topic_, (Message&)gps_msg);
}

void Gps::Impl::ApplyNoiseModel(GpsData& gps_data, const TimeSec& dt) {
  // Update eph & epv using first order filter
  epv_filter.UpdateOutput(dt);
  eph_filter.UpdateOutput(dt);
  gps_data.epv = epv_filter.GetOutput();
  gps_data.eph = eph_filter.GetOutput();

  // Determine GPS fix type
  gps_data.fix_type =
      gps_data.eph <= gps_settings_.eph_min_3d   ? GnssFixType::GNSS_FIX_3D_FIX
      : gps_data.eph <= gps_settings_.eph_min_2d ? GnssFixType::GNSS_FIX_2D_FIX
                                                 : GnssFixType::GNSS_FIX_NO_FIX;
}

GpsMessage Gps::Impl::getOutput() {
  return GpsMessage(output_.time_stamp, output_.time_utc_millis,
                    output_.geo_point.latitude, output_.geo_point.longitude,
                    output_.geo_point.altitude, output_.epv, output_.eph,
                    output_.position_cov_type, output_.fix_type,
                    output_.velocity);
}

void Gps::Impl::RegisterServiceMethod() {
  //! Create a callable for "getOutput" & register as a service method
  //! Note: The service method endpoint is same as the topic name
  auto unique_method_name = topic_path_ + "/" + "gps";
  auto get_gps_data = ServiceMethod(unique_method_name, {""});
  auto get_gps_data_handler =
      get_gps_data.CreateMethodHandler(&Gps::Impl::getOutput, *this);
  service_manager_.RegisterMethod(get_gps_data, get_gps_data_handler);
}

void Gps::Impl::OnEndUpdate() {
  for (const auto& topic : topics_) {
    topic_manager_.UnregisterTopic(topic);
  }
}

Gps::Loader::Loader(Gps::Impl& impl) : impl_(impl) {}

void Gps::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'GPS_settings'.",
                           impl_.id_.c_str());
  LoadGpsSettings(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'GPS_settings' loaded.",
                           impl_.id_.c_str());
}

void Gps::Loader::LoadGpsSettings(const json& json) {
  impl_.gps_settings_.eph_time_constant =
      JsonUtils::GetNumber<float>(json, Constant::Config::eph_time_constant,
                           impl_.gps_settings_.eph_time_constant);
  impl_.gps_settings_.epv_time_constant =
      JsonUtils::GetNumber<float>(json, Constant::Config::epv_time_constant,
                           impl_.gps_settings_.epv_time_constant);

  impl_.gps_settings_.eph_initial = JsonUtils::GetNumber<float>(
      json, Constant::Config::eph_initial, impl_.gps_settings_.eph_initial);

  impl_.gps_settings_.epv_initial = JsonUtils::GetNumber<float>(
      json, Constant::Config::epv_initial, impl_.gps_settings_.epv_initial);

  impl_.gps_settings_.eph_final = JsonUtils::GetNumber<float>(
      json, Constant::Config::eph_final, impl_.gps_settings_.eph_final);

  impl_.gps_settings_.epv_final = JsonUtils::GetNumber<float>(
      json, Constant::Config::epv_final, impl_.gps_settings_.epv_final);

  impl_.gps_settings_.eph_min_3d = JsonUtils::GetNumber<float>(
      json, Constant::Config::eph_min_3d, impl_.gps_settings_.eph_min_3d);

  impl_.gps_settings_.eph_min_2d = JsonUtils::GetNumber<float>(
      json, Constant::Config::eph_min_2d, impl_.gps_settings_.eph_min_2d);

  impl_.logger_.LogVerbose(impl_.name_, "Loaded GPS settings");
}

}  // namespace projectairsim
}  // namespace microsoft
