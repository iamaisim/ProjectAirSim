// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "core_sim/sensors/imu.hpp"

#include <map>
#include <memory>
#include <string>

#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/sensors/noise_model_utils.hpp"
#include "json.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;

class Imu::Loader {
 public:
  explicit Loader(Imu::Impl& impl);

  void Load(const json& json);

 private:
  void LoadImuSettings(const json& json);
  void LoadAccelerometerSettings(const json& json);
  void LoadGyroSettings(const json& json);

  Imu::Impl& impl;
};

struct ImuData {
  TimeNano time_stamp;
  Quaternion orientation;
  Vector3 angular_velocity;
  Vector3 linear_acceleration;
};

class Imu::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  const ImuParams& GetImuSettings() const;

  void OnBeginUpdate() override;

  void CreateTopics();

  void ApplyNoiseModel(ImuData&, const TimeSec&);

  void Update(const TimeNano, const TimeNano);

  ImuMessage getOutput();

  void RegisterServiceMethod();

  void OnEndUpdate() override;

 private:
  friend class Imu::Loader;

  Imu::Loader loader;

  ImuParams imu_settings;

  Topic imu_kinematics_topic;
  std::vector<Topic> topics;

  struct GroundTruth {
    const Kinematics* kinematics;
    const Environment* environment;
  } ground_truth_;

  TimeNano last_update_time_;

  RandomVectorGaussianR gaussian_distrib = RandomVectorGaussianR(0, 1);

  struct State {
    Vector3 accelerometer_bias;
    Vector3 gyroscope_bias;
  } state_;

  RealT accelerometer_bias_stability_norm, gyroscope_bias_stability_norm;
  ImuData output_;
};

Imu::Imu() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Imu::Imu(const std::string& id, bool is_enabled, const std::string& parent_link,
         const Logger& logger, const TopicManager& topic_manager,
         const std::string& parent_topic_path,
         const ServiceManager& service_manager,
         const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(
          new Imu::Impl(id, is_enabled, parent_link, logger, topic_manager,
                        parent_topic_path, service_manager, state_manager))) {}

void Imu::Load(ConfigJson config_json) {
  return static_cast<Imu::Impl*>(pimpl.get())->Load(config_json);
}

void Imu::Initialize(const Kinematics& kinematics,
                     const Environment& environment) {
  static_cast<Imu::Impl*>(pimpl.get())->Initialize(kinematics, environment);
}

const ImuParams& Imu::GetImuSettings() const {
  return static_cast<Imu::Impl*>(pimpl.get())->GetImuSettings();
}

void Imu::BeginUpdate() { static_cast<Imu::Impl*>(pimpl.get())->BeginUpdate(); }

void Imu::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Imu::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

ImuMessage Imu::getOutput() const {
  return static_cast<Imu::Impl*>(pimpl.get())->getOutput();
}

void Imu::EndUpdate() { static_cast<Imu::Impl*>(pimpl.get())->EndUpdate(); }

Imu::Impl::Impl(const std::string& id, bool is_enabled,
                const std::string& parent_link, const Logger& logger,
                const TopicManager& topic_manager,
                const std::string& parent_topic_path,
                const ServiceManager& service_manager,
                const StateManager& state_manager)
    : SensorImpl(SensorType::kImu, id, is_enabled, parent_link,
                 Constant::Component::imu, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader(*this) {
  SetTopicPath();
  CreateTopics();
}

void Imu::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader.Load(json);
}

void Imu::Impl::Initialize(const Kinematics& kinematics,
                           const Environment& environment) {
  this->ground_truth_.kinematics = &kinematics;
  this->ground_truth_.environment = &environment;
}

void Imu::Impl::CreateTopics() {
  imu_kinematics_topic = Topic("imu_kinematics", topic_path_,
                               TopicType::kPublished, 100, MessageType::kImu);
  topics.push_back(imu_kinematics_topic);
}

const ImuParams& Imu::Impl::GetImuSettings() const { return imu_settings; }

void Imu::Impl::OnBeginUpdate() {
  topic_manager_.RegisterTopic(imu_kinematics_topic);
  // last_update_time_ = SimClock::Get()->NowNanos();
  RegisterServiceMethod();
}

void Imu::Impl::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  // TimeNano current_time = SimClock::Get()->NowNanos();
  auto current_time = sim_time;
  //! Calculate time since last update
  //! Using sim_dt_nanos supplied by the Scene as it is the fastest ticking
  //! components allowing downscaling for custom frequencies if needed
  TimeSec dt = sim_dt_nanos / 1.0e9;
  // TimeSec dt = (current_time - last_update_time_) / 1.0e9;

  output_.angular_velocity = ground_truth_.kinematics->twist.angular;

  output_.linear_acceleration = PhysicsUtils::TransformVectorToBodyFrame(
      ground_truth_.kinematics->accels.linear -
          ground_truth_.environment->env_info.gravity,
      ground_truth_.kinematics->pose.orientation);

  output_.orientation = ground_truth_.kinematics->pose.orientation;

  //! Apply IMU noise model based on the user-configured parameters
  ApplyNoiseModel(output_, dt);

  output_.time_stamp = current_time;

  //! Prepare imu_msg to publish
  ImuMessage imu_msg(output_.time_stamp, output_.orientation,
                     output_.angular_velocity, output_.linear_acceleration);

  //! Publish IMU output msg
  topic_manager_.PublishTopic(imu_kinematics_topic, (Message&)imu_msg);
  //! Cache last update time if dt is calculated locally
  // last_update_time_ = current_time;
}

void Imu::Impl::ApplyNoiseModel(ImuData& imu_data, const TimeSec& dt) {
  //! See docs/imu for the math
  auto sqrt_dt = static_cast<float>(
      sqrt(std::max<TimeSec>(dt, imu_settings.min_sample_time)));

  //! Model Accelerometer noise
  //! 1. Convert velocity random walk to stddev
  RealT accelerometer_sigma_vel_rand_walk =
      imu_settings.accelerometer.velocity_random_walk / sqrt_dt;
  imu_data.linear_acceleration +=
      gaussian_distrib.next() * accelerometer_sigma_vel_rand_walk +
      state_.accelerometer_bias;
  //! 2. Update bias random walk
  RealT accelerometer_sigma_bias = accelerometer_bias_stability_norm * sqrt_dt;
  state_.accelerometer_bias +=
      gaussian_distrib.next() * accelerometer_sigma_bias;

  //! Model Gyroscope noise
  //! 1. Convert angle random walk to stddev
  RealT gyroscope_sigma_angle_rand_walk =
      imu_settings.gyro.angle_random_walk / sqrt_dt;
  imu_data.angular_velocity +=
      gaussian_distrib.next() * gyroscope_sigma_angle_rand_walk +
      state_.gyroscope_bias;
  //! 2. Update bias random walk
  RealT gyroscope_sigma_bias = gyroscope_bias_stability_norm * sqrt_dt;
  state_.gyroscope_bias += gaussian_distrib.next() * gyroscope_sigma_bias;
}

ImuMessage Imu::Impl::getOutput() {
  return ImuMessage(output_.time_stamp, output_.orientation,
                    output_.angular_velocity, output_.linear_acceleration);
}

void Imu::Impl::RegisterServiceMethod() {
  //! Note: The service method endpoint is same as the topic name
  auto unique_method_name = topic_path_ + "/" + "imu_kinematics";
  auto get_imu_data = ServiceMethod(unique_method_name, {""});
  auto get_imu_data_handler =
      get_imu_data.CreateMethodHandler(&Imu::Impl::getOutput, *this);
  service_manager_.RegisterMethod(get_imu_data, get_imu_data_handler);
}

void Imu::Impl::OnEndUpdate() {
  topic_manager_.UnregisterTopic(imu_kinematics_topic);
}

Imu::Loader::Loader(Imu::Impl& impl) : impl(impl) {}

void Imu::Loader::Load(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'imu_settings'.",
                          impl.id_.c_str());

  LoadImuSettings(json);

  impl.is_loaded_ = true;

  impl.logger_.LogVerbose(impl.name_, "[%s] 'imu_settings' loaded.",
                          impl.id_.c_str());
}

void Imu::Loader::LoadImuSettings(const json& json) {
  // Load Accelerometer settings
  auto accelerometer_settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::accelerometer);
  if (JsonUtils::IsEmpty(accelerometer_settings_json)) {
    impl.logger_.LogVerbose(
        impl.name_, "'accelerometer' missing or empty. Using defaults.");
  } else {
    LoadAccelerometerSettings(accelerometer_settings_json);
  }

  // Load Gyroscope settings
  auto gyro_settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::gyroscope);
  if (JsonUtils::IsEmpty(gyro_settings_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'gyroscope' missing or empty. Using defaults.");
  } else {
    LoadGyroSettings(gyro_settings_json);
  }

  impl.logger_.LogVerbose(impl.name_, "Loaded IMU");
}

void Imu::Loader::LoadAccelerometerSettings(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading Accelerometer.");
  impl.imu_settings.accelerometer.gravity = JsonUtils::GetNumber<float>(
      json, Constant::Config::gravity, impl.imu_settings.accelerometer.gravity);
  impl.imu_settings.accelerometer.velocity_random_walk = JsonUtils::GetNumber<float>(
      json, Constant::Config::velocity_random_walk,
      impl.imu_settings.accelerometer.velocity_random_walk);
  impl.imu_settings.accelerometer.tau = JsonUtils::GetNumber<float>(
      json, Constant::Config::tau, impl.imu_settings.accelerometer.tau);
  impl.imu_settings.accelerometer.bias_stability =
      JsonUtils::GetNumber<float>(json, Constant::Config::bias_stability,
                           impl.imu_settings.accelerometer.bias_stability);
  impl.imu_settings.accelerometer.turn_on_bias =
      JsonUtils::GetVector3(json, Constant::Config::turn_on_bias);

  //! Initialize internal state
  impl.state_.accelerometer_bias = impl.imu_settings.accelerometer.turn_on_bias;
  impl.accelerometer_bias_stability_norm =
      impl.imu_settings.accelerometer.bias_stability /
      sqrt(impl.imu_settings.accelerometer.tau);

  impl.logger_.LogVerbose(impl.name_, "Accelerometer Loaded.");
}

void Imu::Loader::LoadGyroSettings(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading Gyroscope.");
  impl.imu_settings.gyro.angle_random_walk =
      JsonUtils::GetNumber<float>(json, Constant::Config::angle_random_walk,
                           impl.imu_settings.gyro.angle_random_walk);
  impl.imu_settings.gyro.tau = JsonUtils::GetNumber<float>(json, Constant::Config::tau,
                                                    impl.imu_settings.gyro.tau);
  impl.imu_settings.gyro.bias_stability =
      JsonUtils::GetNumber<float>(json, Constant::Config::bias_stability,
                           impl.imu_settings.gyro.bias_stability);
  impl.imu_settings.gyro.turn_on_bias =
      JsonUtils::GetVector3(json, Constant::Config::turn_on_bias);

  //! Initialize internal state
  impl.state_.gyroscope_bias = impl.imu_settings.gyro.turn_on_bias;
  impl.gyroscope_bias_stability_norm =
      impl.imu_settings.gyro.bias_stability / sqrt(impl.imu_settings.gyro.tau);

  impl.logger_.LogVerbose(impl.name_, "Gyroscope loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
