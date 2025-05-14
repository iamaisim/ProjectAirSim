// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "core_sim/sensors/magnetometer.hpp"

#include <map>
#include <memory>
#include <string>

#include "constant.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/magnetometer_message.hpp"
#include "core_sim/sensors/noise_model_utils.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

class Magnetometer::Loader {
 public:
  explicit Loader(Magnetometer::Impl& impl);

  void Load(const nlohmann::json& json);

 private:
  void LoadMagnetometerSettings(const nlohmann::json& json);
  void LoadOriginSetting(const nlohmann::json& json);

  Magnetometer::Impl& impl_;
};

struct MagnetometerData {
  TimeNano time_stamp;
  Vector3 magnetic_field_body;  // in Gauss
  std::vector<float>
      magnetic_field_covariance;  // 9 elements of the 3x3 covariance matrix
};

class Magnetometer::Impl : public SensorImpl {
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

  void ApplyNoiseModel(MagnetometerData&, const TimeSec&);

  void Update(const TimeNano, const TimeNano);

  MagnetometerMessage getOutput();

  void RegisterServiceMethod();

  void OnEndUpdate() override;

 private:
  friend class Magnetometer::Loader;

  Magnetometer::Loader loader_;

  MagnetometerParams magnetometer_settings_;

  RandomVectorGaussianR noise_vec_;
  Vector3 bias_vec_;
  Vector3 magnetic_field_true_;

  Topic magnetometer_topic_;
  std::vector<std::reference_wrapper<Topic>> topics_;

  struct GroundTruth {
    const Environment* environment;
    const Kinematics* kinematics;
  } ground_truth_;

  MagnetometerData output_;  // To hold latest sensor data
};

Magnetometer::Magnetometer() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Magnetometer::Magnetometer(const std::string& id, bool is_enabled,
                           const std::string& parent_link, const Logger& logger,
                           const TopicManager& topic_manager,
                           const std::string& parent_topic_path,
                           const ServiceManager& service_manager,
                           const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new Magnetometer::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void Magnetometer::Load(ConfigJson config_json) {
  return static_cast<Magnetometer::Impl*>(pimpl.get())->Load(config_json);
}

void Magnetometer::Initialize(const Kinematics& kinematics,
                              const Environment& environment) {
  static_cast<Magnetometer::Impl*>(pimpl.get())
      ->Initialize(kinematics, environment);
}

void Magnetometer::BeginUpdate() {
  static_cast<Magnetometer::Impl*>(pimpl.get())->BeginUpdate();
}

void Magnetometer::Update(const TimeNano sim_time,
                          const TimeNano sim_dt_nanos) {
  static_cast<Magnetometer::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

MagnetometerMessage Magnetometer::getOutput() const {
  return static_cast<Magnetometer::Impl*>(pimpl.get())->getOutput();
}

void Magnetometer::EndUpdate() {
  static_cast<Magnetometer::Impl*>(pimpl.get())->EndUpdate();
}

Magnetometer::Impl::Impl(const std::string& id, bool is_enabled,
                         const std::string& parent_link, const Logger& logger,
                         const TopicManager& topic_manager,
                         const std::string& parent_topic_path,
                         const ServiceManager& service_manager,
                         const StateManager& state_manager)
    : SensorImpl(SensorType::kMagnetometer, id, is_enabled, parent_link,
                 Constant::Component::imu, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader_(*this) {
  SetTopicPath();
  CreateTopics();
}

void Magnetometer::Impl::Load(ConfigJson config_json) {
  nlohmann::json json = config_json;
  loader_.Load(json);
}

void Magnetometer::Impl::Initialize(const Kinematics& kinematics,
                                    const Environment& environment) {
  ground_truth_.environment = &environment;
  ground_truth_.kinematics = &kinematics;
  noise_vec_ = RandomVectorGaussianR(Vector3::Zero(),
                                     magnetometer_settings_.noise_sigma);
  bias_vec_ = RandomVectorGaussianR(-magnetometer_settings_.noise_bias,
                                    magnetometer_settings_.noise_bias)
                  .next();
}

void Magnetometer::Impl::CreateTopics() {
  magnetometer_topic_ =
      Topic("magnetometer", topic_path_, TopicType::kPublished, 100,
            MessageType::kMagnetometer);
  topics_.push_back(magnetometer_topic_);
}

void Magnetometer::Impl::OnBeginUpdate() {
  for (const auto& topic_ref : topics_) {
    topic_manager_.RegisterTopic(topic_ref.get());
  }
  RegisterServiceMethod();
}

void Magnetometer::Impl::Update(const TimeNano sim_time,
                                const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto current_time = sim_time;
  //! Calculate time since last update.
  //! Using sim_dt_nanos supplied by the Scene as it is the fastest ticking
  //! components allowing downscaling for custom frequencies if needed
  TimeSec dt = sim_dt_nanos / 1.0e9;
  // TimeSec dt = (current_time - last_update_time_) / 1.0e9;

  switch (magnetometer_settings_.ref_source) {
    case MagnetometerParams::ReferenceSource::kReferenceSourceConstant:
      // Constant magnetic field for Seattle
      // TODO: This can also be configurable?
      magnetic_field_true_ = Vector3(0.34252f, 0.9805f, 0.93438f);
      break;
    case MagnetometerParams::ReferenceSource::kReferenceSourceDipoleModel:
      magnetic_field_true_ =
          EarthUtils::GetMagneticField(
              ground_truth_.environment->env_info.geo_point) *
          1e4f;  // Convert units from Tesla to Gauss
      break;
    default:
      throw Error("Magnetometer sensor ReferenceSource is invalid");
  }

  // TODO Use transform tree to convert magnetic_field_true_ from world frame to
  // sensor frame instead of manually rotating by (body kinematic orientation +
  // sensor origin rotation).
  Quaternion sensor_quat = ground_truth_.kinematics->pose.orientation *
                           magnetometer_settings_.origin_setting.rotation_;
  output_.magnetic_field_body = PhysicsUtils::TransformVectorToBodyFrame(
                                    magnetic_field_true_, sensor_quat, true) *
                                magnetometer_settings_.scale_factor;

  //! Perturb magnetometer sensor data based on the NoiseModel
  ApplyNoiseModel(output_, dt);

  output_.time_stamp = current_time;

  //! Prepare magnetometer sensor msg to publish
  MagnetometerMessage magnetometer_msg(output_.time_stamp,
                                       output_.magnetic_field_body,
                                       output_.magnetic_field_covariance);

  //! Publish magnetometer output msg
  topic_manager_.PublishTopic(magnetometer_topic_, (Message&)magnetometer_msg);
}

void Magnetometer::Impl::ApplyNoiseModel(MagnetometerData& magnetometer_data,
                                         const TimeSec& dt) {
  // Add magnetic field noise to magnetometer data
  magnetometer_data.magnetic_field_body += noise_vec_.next() + bias_vec_;
}

MagnetometerMessage Magnetometer::Impl::getOutput() {
  return MagnetometerMessage(output_.time_stamp, output_.magnetic_field_body,
                             output_.magnetic_field_covariance);
}

void Magnetometer::Impl::RegisterServiceMethod() {
  //! Create a callable & register as a service method
  //! Note: The service method endpoint is same as the topic name
  auto unique_method_name = topic_path_ + "/" + "magnetometer";
  auto get_magnetometer_data = ServiceMethod(unique_method_name, {""});
  auto get_magnetometer_data_handler =
      get_magnetometer_data.CreateMethodHandler(&Magnetometer::Impl::getOutput,
                                                *this);
  service_manager_.RegisterMethod(get_magnetometer_data,
                                  get_magnetometer_data_handler);
}

void Magnetometer::Impl::OnEndUpdate() {
  for (const auto& topic : topics_) {
    topic_manager_.UnregisterTopic(topic);
  }
}

Magnetometer::Loader::Loader(Magnetometer::Impl& impl) : impl_(impl) {}

void Magnetometer::Loader::Load(const nlohmann::json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'magnetometer_settings'.",
                           impl_.id_.c_str());

  LoadMagnetometerSettings(json);
  LoadOriginSetting(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'magnetometer_settings' loaded.",
                           impl_.id_.c_str());
}

void Magnetometer::Loader::LoadMagnetometerSettings(
    const nlohmann::json& json) {
  // Load Magnetometer settings
  impl_.magnetometer_settings_.scale_factor =
      JsonUtils::GetNumber<float>(json, Constant::Config::scale_factor,
                           impl_.magnetometer_settings_.scale_factor);

  impl_.magnetometer_settings_.noise_sigma =
      JsonUtils::GetVector3(json, Constant::Config::noise_sigma,
                            impl_.magnetometer_settings_.noise_sigma);

  impl_.magnetometer_settings_.noise_bias =
      JsonUtils::GetVector3(json, Constant::Config::noise_bias,
                            impl_.magnetometer_settings_.noise_bias);

  impl_.logger_.LogVerbose(impl_.name_, "Loaded Magnetometer settings");
}

void Magnetometer::Loader::LoadOriginSetting(const nlohmann::json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "'origin' missing or empty. Using default.");
  } else {
    impl_.magnetometer_settings_.origin_setting =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl_.logger_.LogVerbose(impl_.name_, "'origin' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
