// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "core_sim/sensors/barometer.hpp"

#include <map>
#include <memory>
#include <string>

#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/barometer_message.hpp"
#include "core_sim/sensors/noise_model_utils.hpp"
#include "json.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class Barometer::Loader {
 public:
  explicit Loader(Barometer::Impl& impl);

  void Load(const json& json);

 private:
  void LoadBarometerSettings(const json& json);

  Barometer::Impl& impl_;
};

struct BarometerData {
  TimeNano time_stamp;
  float altitude;  // meters
  float pressure;  // pascal
  float qnh;       // HectoPascals(hPa)/Millibars(mbar).
};

class Barometer::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  void OnBeginUpdate() override;

  void CreateTopics();

  void ApplyNoiseModel(BarometerData&, const TimeSec&);

  void Update(const TimeNano, const TimeNano);

  BarometerMessage getOutput();

  void RegisterServiceMethod();

  void OnEndUpdate() override;

 private:
  friend class Barometer::Loader;

  Barometer::Loader loader_;

  BarometerParams barometer_settings_;

  GaussianMarkov pressure_factor_;
  RandomGeneratorGaussianF uncorrelated_noise_;

  Topic barometer_topic_;
  std::vector<std::reference_wrapper<Topic>> topics_;

  struct GroundTruth {
    const Environment* environment;
  } ground_truth_;

  BarometerData output_;  // Holds latest sensor data
};

Barometer::Barometer() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Barometer::Barometer(const std::string& id, bool is_enabled,
                     const std::string& parent_link, const Logger& logger,
                     const TopicManager& topic_manager,
                     const std::string& parent_topic_path,
                     const ServiceManager& service_manager,
                     const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new Barometer::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void Barometer::Load(ConfigJson config_json) {
  return static_cast<Barometer::Impl*>(pimpl.get())->Load(config_json);
}

void Barometer::Initialize(const Kinematics& kinematics,
                           const Environment& environment) {
  static_cast<Barometer::Impl*>(pimpl.get())
      ->Initialize(kinematics, environment);
}

void Barometer::BeginUpdate() {
  static_cast<Barometer::Impl*>(pimpl.get())->BeginUpdate();
}

void Barometer::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Barometer::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

BarometerMessage Barometer::getOutput() const {
  return static_cast<Barometer::Impl*>(pimpl.get())->getOutput();
}

void Barometer::EndUpdate() {
  static_cast<Barometer::Impl*>(pimpl.get())->EndUpdate();
}

Barometer::Impl::Impl(const std::string& id, bool is_enabled,
                      const std::string& parent_link, const Logger& logger,
                      const TopicManager& topic_manager,
                      const std::string& parent_topic_path,
                      const ServiceManager& service_manager,
                      const StateManager& state_manager)
    : SensorImpl(SensorType::kBarometer, id, is_enabled, parent_link,
                 Constant::Component::imu, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader_(*this) {
  SetTopicPath();
  CreateTopics();
}

void Barometer::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

void Barometer::Impl::Initialize(const Kinematics& kinematics,
                                 const Environment& environment) {
  ground_truth_.environment = &environment;

  pressure_factor_.Initialize(barometer_settings_.pressure_factor_tau,
                              barometer_settings_.pressure_factor_sigma, 0);

  uncorrelated_noise_ = RandomGeneratorGaussianF(
      0.0f, barometer_settings_.uncorrelated_noise_sigma);
}

void Barometer::Impl::CreateTopics() {
  barometer_topic_ = Topic("barometer", topic_path_, TopicType::kPublished, 100,
                           MessageType::kBarometer);
  topics_.push_back(barometer_topic_);
}

void Barometer::Impl::OnBeginUpdate() {
  for (const auto& topic_ref : topics_) {
    topic_manager_.RegisterTopic(topic_ref.get());
  }
  RegisterServiceMethod();
}

void Barometer::Impl::Update(const TimeNano sim_time,
                             const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto current_time = sim_time;
  //! Calculate time since last update.
  //! Using sim_dt_nanos supplied by the Scene as it is the fastest ticking
  //! components allowing downscaling for custom frequencies if needed
  TimeSec dt = sim_dt_nanos / 1.0e9;
  // TimeSec dt = (current_time - last_update_time_) / 1.0e9;

  output_.altitude = ground_truth_.environment->env_info.geo_point.altitude;
  output_.pressure =
      ground_truth_.environment->env_info
          .air_pressure;  // EarthUtils::GetStandardPressure(output.altitude);
  output_.qnh = barometer_settings_.qnh;

  //! Perturb barometer sensor data based on the NoiseModel
  ApplyNoiseModel(output_, dt);

  output_.time_stamp = current_time;

  //! Prepare barometer sensor msg to publish
  BarometerMessage barometer_msg(output_.time_stamp, output_.altitude,
                                 output_.pressure, output_.qnh);

  //! Publish barometer output msg
  topic_manager_.PublishTopic(barometer_topic_, (Message&)barometer_msg);
}

BarometerMessage Barometer::Impl::getOutput() {
  return BarometerMessage(output_.time_stamp, output_.altitude,
                          output_.pressure, output_.qnh);
}

void Barometer::Impl::RegisterServiceMethod() {
  //! Create a callable for getOutput and register a service method
  //! Note: The service method endpoint is same as the topic name
  auto unique_method_name = topic_path_ + "/" + "barometer";
  auto get_barometer_data = ServiceMethod(unique_method_name, {""});
  auto get_barometer_data_handler = get_barometer_data.CreateMethodHandler(
      &Barometer::Impl::getOutput, *this);
  service_manager_.RegisterMethod(get_barometer_data,
                                  get_barometer_data_handler);
}
void Barometer::Impl::ApplyNoiseModel(BarometerData& barometer_data,
                                      const TimeSec& dt) {
  // Add drift in pressure, about 10m change per hour
  pressure_factor_.Update(dt);
  barometer_data.pressure +=
      barometer_data.pressure * pressure_factor_.getOutput();

  // Add noise in pressure (about 0.2m sigma)
  barometer_data.pressure += uncorrelated_noise_.next();
  barometer_data.pressure = barometer_data.pressure -
                            EarthUtils::kSeaLevelPressure +
                            barometer_settings_.qnh * 100.0f;

  // apply altimeter formula
  // https://en.wikipedia.org/wiki/Pressure_altitude
  barometer_data.altitude =
      EarthUtils::GetPressureAltitude(barometer_data.pressure);
  barometer_data.qnh = barometer_settings_.qnh;
}

void Barometer::Impl::OnEndUpdate() {
  for (const auto& topic : topics_) {
    topic_manager_.UnregisterTopic(topic);
  }
}

Barometer::Loader::Loader(Barometer::Impl& impl) : impl_(impl) {}

void Barometer::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'barometer_settings'.",
                           impl_.id_.c_str());

  LoadBarometerSettings(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'barometer_settings' loaded.",
                           impl_.id_.c_str());
}

void Barometer::Loader::LoadBarometerSettings(const json& json) {
  // Load Barometer settings
  impl_.barometer_settings_.qnh = JsonUtils::GetNumber<float>(
      json, Constant::Config::qnh, impl_.barometer_settings_.qnh);

  impl_.barometer_settings_.pressure_factor_sigma =
      JsonUtils::GetNumber<float>(json, Constant::Config::pressure_factor_sigma,
                           impl_.barometer_settings_.pressure_factor_sigma);

  impl_.barometer_settings_.pressure_factor_tau =
      JsonUtils::GetNumber<float>(json, Constant::Config::pressure_factor_tau,
                           impl_.barometer_settings_.pressure_factor_tau);

  impl_.barometer_settings_.uncorrelated_noise_sigma =
      JsonUtils::GetNumber<float>(json, Constant::Config::uncorrelated_noise_sigma,
                           impl_.barometer_settings_.uncorrelated_noise_sigma);

  impl_.barometer_settings_.update_latency =
      JsonUtils::GetNumber<float>(json, Constant::Config::update_latency,
                           impl_.barometer_settings_.update_latency);

  impl_.barometer_settings_.update_frequency =
      JsonUtils::GetNumber<float>(json, Constant::Config::update_frequency,
                           impl_.barometer_settings_.update_frequency);

  impl_.barometer_settings_.startup_delay =
      JsonUtils::GetNumber<float>(json, Constant::Config::startup_delay,
                           impl_.barometer_settings_.startup_delay);

  impl_.logger_.LogVerbose(impl_.name_, "Loaded Barometer settings");
}

}  // namespace projectairsim
}  // namespace microsoft
