// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "core_sim/sensors/distance_sensor.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

// forward declarations

class DistanceSensor::Loader {
 public:
  explicit Loader(DistanceSensor::Impl& impl);

  void Load(const nlohmann::json& json);

 private:
  void InitializeDistanceSensorSettings(){};
  void LoadDistanceSensorSettings(const nlohmann::json& json);
  void LoadOriginSetting(const nlohmann::json& json);

  DistanceSensor::Impl& impl;
};

class DistanceSensor::Impl : public SensorImpl {
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

  const DistanceSensorSettings& GetDistanceSensorSetting() const;

  const Transform& GetOrigin() const;

  void OnBeginUpdate() override;

  void PublishDistanceSensorMsg(
      const DistanceSensorMessage& distance_sensor_msg);

  void OnEndUpdate() override;

  DistanceSensorMessage getOutput();

 private:
  friend class DistanceSensor::Loader;

  DistanceSensor::Loader loader;

  DistanceSensorSettings distance_sensor_settings;

  DistanceSensorMessage lastMessage;

  Topic distance_sensor_topic;
  std::vector<Topic> topics;
};

// class DistanceSensor

DistanceSensor::DistanceSensor()
    : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

DistanceSensor::DistanceSensor(const std::string& id, bool is_enabled,
                               const std::string& parent_link,
                               const Logger& logger,
                               const TopicManager& topic_manager,
                               const std::string& parent_topic_path,
                               const ServiceManager& service_manager,
                               const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new DistanceSensor::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void DistanceSensor::Load(ConfigJson config_json) {
  static_cast<DistanceSensor::Impl*>(pimpl.get())->Load(config_json);
}

void DistanceSensor::Initialize(const Kinematics& kinematics,
                                const Environment& environment) {
  static_cast<DistanceSensor::Impl*>(pimpl.get())
      ->Initialize(kinematics, environment);
}

void DistanceSensor::Update(const TimeNano sim_time,
                            const TimeNano sim_dt_nanos) {
  static_cast<DistanceSensor::Impl*>(pimpl.get())
      ->Update(sim_time, sim_dt_nanos);
}

const DistanceSensorSettings& DistanceSensor::GetDistanceSensorSettings()
    const {
  return static_cast<DistanceSensor::Impl*>(pimpl.get())
      ->GetDistanceSensorSetting();
}

void DistanceSensor::BeginUpdate() {
  static_cast<DistanceSensor::Impl*>(pimpl.get())->BeginUpdate();
}

void DistanceSensor::PublishDistanceSensorMsg(
    const DistanceSensorMessage& distance_sensor_msg) {
  static_cast<DistanceSensor::Impl*>(pimpl.get())
      ->PublishDistanceSensorMsg(distance_sensor_msg);
}

void DistanceSensor::EndUpdate() {
  static_cast<DistanceSensor::Impl*>(pimpl.get())->EndUpdate();
}

DistanceSensorMessage DistanceSensor::getOutput() {
  return static_cast<DistanceSensor::Impl*>(pimpl.get())->getOutput();
}

// class DistanceSensor::impl

DistanceSensor::Impl::Impl(const std::string& id, bool is_enabled,
                           const std::string& parent_link, const Logger& logger,
                           const TopicManager& topic_manager,
                           const std::string& parent_topic_path,
                           const ServiceManager& service_manager,
                           const StateManager& state_manager)
    : SensorImpl(SensorType::kDistanceSensor, id, is_enabled, parent_link,
                 Constant::Component::distance_sensor, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader(*this) {
  SetTopicPath();
  CreateTopics();
}

void DistanceSensor::Impl::Load(ConfigJson config_json) {
  nlohmann::json json = config_json;
  loader.Load(json);
}

void DistanceSensor::Impl::Initialize(const Kinematics& kinematics,
                                      const Environment& environment) {
  //! Ground-truth kinematics and environment info insn't used
  //! by the DistanceSensor impl
}

void DistanceSensor::Impl::Update(const TimeNano sim_time,
                                  const TimeNano sim_dt_nanos) {
  //! DistanceSensor updates on render ticks from the Rendering engine
  //! and therefore does not execute any op specifically at scene/physics
  //! ticks
}

void DistanceSensor::Impl::CreateTopics() {
  distance_sensor_topic =
      Topic("distance_sensor", topic_path_, TopicType::kPublished, 60,
            MessageType::kDistanceSensor);

  topics.push_back(distance_sensor_topic);
}

const DistanceSensorSettings& DistanceSensor::Impl::GetDistanceSensorSetting()
    const {
  return distance_sensor_settings;
}

void DistanceSensor::Impl::OnBeginUpdate() {
  topic_manager_.RegisterTopic(distance_sensor_topic);
}

void DistanceSensor::Impl::PublishDistanceSensorMsg(
    const DistanceSensorMessage& distance_sensor_msg) {
  std::lock_guard<std::mutex> lock(update_lock_);
  lastMessage = distance_sensor_msg;
  topic_manager_.PublishTopic(distance_sensor_topic,
                              (Message&)distance_sensor_msg);
}

void DistanceSensor::Impl::OnEndUpdate() {
  topic_manager_.UnregisterTopic(distance_sensor_topic);
}

DistanceSensorMessage DistanceSensor::Impl::getOutput() {
  // Lock updates coming from renderer while the output message is copied and
  // returned
  std::lock_guard<std::mutex> lock(update_lock_);
  return DistanceSensorMessage(lastMessage);
}

// class DistanceSensor::loader

DistanceSensor::Loader::Loader(DistanceSensor::Impl& impl) : impl(impl) {}

void DistanceSensor::Loader::Load(const nlohmann::json& json) {
  impl.logger_.LogVerbose(
      impl.name_, "[%s] Loading 'distance_sensor_settings'.", impl.id_.c_str());

  LoadDistanceSensorSettings(json);

  impl.is_loaded_ = true;

  impl.logger_.LogVerbose(impl.name_, "[%s] 'distance_sensor_settings' loaded.",
                          impl.id_.c_str());
}

void DistanceSensor::Loader::LoadDistanceSensorSettings(
    const nlohmann::json& json) {
  LoadOriginSetting(json);

  impl.distance_sensor_settings.report_frequency =
      JsonUtils::GetNumber(json, Constant::Config::report_frequency,
                           impl.distance_sensor_settings.report_frequency);

  impl.distance_sensor_settings.min_distance =
      JsonUtils::GetNumber(json, Constant::Config::min_distance,
                           impl.distance_sensor_settings.min_distance);

  impl.distance_sensor_settings.max_distance =
      JsonUtils::GetNumber(json, Constant::Config::max_distance,
                           impl.distance_sensor_settings.max_distance);

  impl.distance_sensor_settings.draw_debug_points =
      JsonUtils::GetInteger(json, Constant::Config::draw_debug_points,
                            impl.distance_sensor_settings.draw_debug_points);
}

void DistanceSensor::Loader::LoadOriginSetting(const nlohmann::json& json) {
  impl.logger_.LogVerbose(impl.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl.logger_.LogVerbose(impl.name_,
                            "'origin' missing or empty. Using default.");
  } else {
    impl.distance_sensor_settings.origin =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl.logger_.LogVerbose(impl.name_, "'origin' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
