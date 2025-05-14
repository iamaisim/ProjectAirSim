// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "core_sim/sensors/airspeed.hpp"

#include <map>
#include <memory>
#include <string>

#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/airspeed_message.hpp"
#include "core_sim/sensors/noise_model_utils.hpp"
#include "json.hpp"
#include "sensor_impl.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

// Sensor configuration loader
class AirspeedSensor::Loader {
 public:
  explicit Loader(AirspeedSensor::Impl& impl);

  void Load(const json& json);

 private:
  void LoadAirspeedSensorSettings(const json& json);

  AirspeedSensor::Impl& impl_;  // Pointer to sensor implementation object
};                              // class AirspeedSensor::Loader

// Sensor data
struct AirspeedSensorData {
  TimeNano time_stamp;  // When the sensor was read
  float pressure;       // Differential pressure read from the sensor (Pascals)
};                      // struct AirspeedSensorData

class AirspeedSensor::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void ApplyNoiseModel(AirspeedSensorData&, const TimeSec&);

  void CreateTopics();

  AirspeedMessage getOutput();

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  void OnBeginUpdate() override;

  void OnEndUpdate() override;

  void RegisterServiceMethod();

  void Update(const TimeNano, const TimeNano);

 private:
  friend class AirspeedSensor::Loader;

 private:
  // Externally set state we take as the truth
  struct GroundTruth {
    const Kinematics* kinematics;    // Vehicle's kinetic state
    const Environment* environment;  // Environment's state
  };                                 // struct GroundTruth

 private:
  AirspeedSensorParams airspeedsensor_settings_;  // Sensor parameters
  Topic airspeed_topic_;                          // Sensor data stream topic
  GroundTruth ground_truth_;        // Internal vehicle and environment state
  AirspeedSensor::Loader loader_;   // Sensor parameter loader
  AirspeedSensorData output_;       // Holds latest sensor data
  Vector3 position_last_;           // Vehicle's position from the last update
  GaussianMarkov pressure_factor_;  // Pressure drift simulation
  std::vector<std::reference_wrapper<Topic>> topics_;  // All topics we publish
  RandomGeneratorGaussianF uncorrelated_noise_;  // Pressure noise simulation
};                                               // class AirspeedSensor::Impl

AirspeedSensor::AirspeedSensor()
    : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

AirspeedSensor::AirspeedSensor(const std::string& id, bool is_enabled,
                               const std::string& parent_link,
                               const Logger& logger,
                               const TopicManager& topic_manager,
                               const std::string& parent_topic_path,
                               const ServiceManager& service_manager,
                               const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new AirspeedSensor::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void AirspeedSensor::Load(ConfigJson config_json) {
  return static_cast<AirspeedSensor::Impl*>(pimpl.get())->Load(config_json);
}

void AirspeedSensor::Initialize(const Kinematics& kinematics,
                                const Environment& environment) {
  static_cast<AirspeedSensor::Impl*>(pimpl.get())
      ->Initialize(kinematics, environment);
}

void AirspeedSensor::BeginUpdate() {
  static_cast<AirspeedSensor::Impl*>(pimpl.get())->BeginUpdate();
}

void AirspeedSensor::Update(const TimeNano sim_time,
                            const TimeNano sim_dt_nanos) {
  static_cast<AirspeedSensor::Impl*>(pimpl.get())
      ->Update(sim_time, sim_dt_nanos);
}

AirspeedMessage AirspeedSensor::getOutput() const {
  return static_cast<AirspeedSensor::Impl*>(pimpl.get())->getOutput();
}

void AirspeedSensor::EndUpdate() {
  static_cast<AirspeedSensor::Impl*>(pimpl.get())->EndUpdate();
}

AirspeedSensor::Impl::Impl(const std::string& id, bool is_enabled,
                           const std::string& parent_link, const Logger& logger,
                           const TopicManager& topic_manager,
                           const std::string& parent_topic_path,
                           const ServiceManager& service_manager,
                           const StateManager& state_manager)
    : SensorImpl(SensorType::kAirspeed, id, is_enabled, parent_link,
                 Constant::Component::imu, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      airspeedsensor_settings_(),
      airspeed_topic_(),
      loader_(*this),
      ground_truth_(),
      output_(),
      position_last_(0, 0, 0),
      pressure_factor_(),
      topics_(),
      uncorrelated_noise_() {
  SetTopicPath();
  CreateTopics();
}

void AirspeedSensor::Impl::ApplyNoiseModel(
    AirspeedSensorData& airspeedsensor_data, const TimeSec& dtSec) {
  // Add drift in pressure, about 10m change per hour
  pressure_factor_.Update(dtSec);
  airspeedsensor_data.pressure +=
      airspeedsensor_data.pressure * pressure_factor_.getOutput();

  // Add noise in pressure (about 0.2m sigma)
  airspeedsensor_data.pressure += uncorrelated_noise_.next();
}

void AirspeedSensor::Impl::CreateTopics() {
  airspeed_topic_ = Topic("airspeed", topic_path_, TopicType::kPublished, 100,
                          MessageType::kAirspeed);
  topics_.push_back(airspeed_topic_);
}

AirspeedMessage AirspeedSensor::Impl::getOutput() {
  return AirspeedMessage(output_.time_stamp, output_.pressure);
}

void AirspeedSensor::Impl::Initialize(const Kinematics& kinematics,
                                      const Environment& environment) {
  this->ground_truth_.environment = &environment;
  this->ground_truth_.kinematics = &kinematics;
  this->position_last_ = this->ground_truth_.kinematics->pose.position;

  this->pressure_factor_.Initialize(
      airspeedsensor_settings_.pressure_factor_tau,
      airspeedsensor_settings_.pressure_factor_sigma, 0);

  this->uncorrelated_noise_ = RandomGeneratorGaussianF(
      0.0f, airspeedsensor_settings_.uncorrelated_noise_sigma);
}

void AirspeedSensor::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

void AirspeedSensor::Impl::OnBeginUpdate() {
  for (const auto& topic_ref : topics_) {
    topic_manager_.RegisterTopic(topic_ref.get());
  }
  RegisterServiceMethod();
}

void AirspeedSensor::Impl::OnEndUpdate() {
  for (const auto& topic : topics_) {
    topic_manager_.UnregisterTopic(topic);
  }
}

void AirspeedSensor::Impl::RegisterServiceMethod() {
  //! Create a callable for getOutput and register a service method
  //! Note: The service method endpoint is same as the topic name
  auto unique_method_name = topic_path_ + "/" + "airspeed";
  auto get_AirspeedSensor_data = ServiceMethod(unique_method_name, {""});
  auto get_AirspeedSensor_data_handler =
      get_AirspeedSensor_data.CreateMethodHandler(
          &AirspeedSensor::Impl::getOutput, *this);
  service_manager_.RegisterMethod(get_AirspeedSensor_data,
                                  get_AirspeedSensor_data_handler);
}

void AirspeedSensor::Impl::Update(const TimeNano sim_time,
                                  const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  //! Calculate time since last update.
  //! Using sim_dt_nanos supplied by the Scene as it is the fastest ticking
  //! components allowing downscaling for custom frequencies if needed
  TimeSec dtSec = sim_dt_nanos / 1.0e9;

  auto position = ground_truth_.kinematics->pose.position;
  float speed_in_air_true;
  float speed_ratio;
  Vector3f velocity_in_air_true;

  // TODO Use ground_truth_.kinematics->twist.linear_velocity instead
  // of (position - position_last_) / dtSec?
  velocity_in_air_true =
      (position - position_last_) / dtSec -
      ground_truth_.environment->wind_velocity;  // Get true air velocity
  position_last_ = position;  // Save position for next update

  // Rotate forward dir in sensor frame to world frame using body orientation
  // TODO This should use transform tree from sensor orientation to world frame
  auto forward_world = PhysicsUtils::TransformVectorToWorldFrame(
      airspeedsensor_settings_.forward_xyz,
      ground_truth_.kinematics->pose.orientation);

  // Get air speed in forward direction of vehicle
  speed_in_air_true = velocity_in_air_true.dot(forward_world);

  // Don't output negative air speed
  speed_in_air_true = std::max(speed_in_air_true, 0.0f);

  // logger_.LogVerbose(
  //     name_, "[%s] forward_world (x,y,z) = %f, %f, %f speed_in_air_true =
  //     %f", id_.c_str(), forward_world.x(), forward_world.y(),
  //     forward_world.z(), speed_in_air_true);

  speed_ratio = speed_in_air_true /
                EarthUtils::kSeaLevelSpeedOfSound;  // Calculate air speed as
                                                    // ratio to speed of sound
                                                    // at standard sea level

  // Calculate differential pressure (in Pascal units) from true air speed
  // See: https://en.wikipedia.org/wiki/True_airspeed
  output_.pressure = ground_truth_.environment->env_info.air_pressure *
                     (pow((EarthUtils::kSeaLevelTemperature / 5.0f /
                           ground_truth_.environment->env_info.temperature) *
                                  speed_ratio * speed_ratio +
                              1,
                          7.0f / 2.0f) -
                      1);

  //! Perturb airspeed sensor data based on the NoiseModel
  ApplyNoiseModel(output_, dtSec);

  output_.time_stamp = sim_time;

  //! Prepare airspeed sensor msg to publish
  {
    AirspeedMessage airspeed_msg(output_.time_stamp, output_.pressure);

    //! Publish airspeed output msg
    topic_manager_.PublishTopic(airspeed_topic_, (Message&)airspeed_msg);
  }
}

AirspeedSensor::Loader::Loader(AirspeedSensor::Impl& impl) : impl_(impl) {}

void AirspeedSensor::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'airspeed_settings'.",
                           impl_.id_.c_str());

  LoadAirspeedSensorSettings(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'airspeed_settings' loaded.",
                           impl_.id_.c_str());
}

void AirspeedSensor::Loader::LoadAirspeedSensorSettings(const json& json) {
  // Load AirspeedSensor settings
  impl_.airspeedsensor_settings_.pressure_factor_sigma = JsonUtils::GetNumber<float>(
      json, Constant::Config::pressure_factor_sigma,
      impl_.airspeedsensor_settings_.pressure_factor_sigma);

  impl_.airspeedsensor_settings_.pressure_factor_tau =
      JsonUtils::GetNumber<float>(json, Constant::Config::pressure_factor_tau,
                           impl_.airspeedsensor_settings_.pressure_factor_tau);

  impl_.airspeedsensor_settings_.uncorrelated_noise_sigma =
      JsonUtils::GetNumber<float>(
          json, Constant::Config::uncorrelated_noise_sigma,
          impl_.airspeedsensor_settings_.uncorrelated_noise_sigma);

  impl_.airspeedsensor_settings_.forward_xyz =
      JsonUtils::GetVector3(json, Constant::Config::forward_xyz,
                            impl_.airspeedsensor_settings_.forward_xyz);

  impl_.logger_.LogVerbose(impl_.name_, "Loaded airspeed settings");
}

}  // namespace projectairsim
}  // namespace microsoft
