// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/actuators/tilt.hpp"

#include <memory>
#include <string>

#include "actuator_impl.hpp"
#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/control_mapper.hpp"
#include "core_sim/first_order_filter.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/physics_common_types.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

//------------------------------------------------------------------------------
// Forward declarations

class Tilt::Loader {
 public:
  explicit Loader(Tilt::Impl& impl);

  void Load(const json& json);

 private:
  void LoadSettings(const json& json);

  Tilt::Impl& impl_;
};

class Tilt::Impl : public ActuatorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const std::string& child_link, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void CreateTopics(void);

  void OnBeginUpdate(void) override;

  void OnEndUpdate(void) override;

  const ActuatedTransforms& GetActuatedTransforms() const;

  const Quaternion& GetControlRotation(void) const;

  const TiltSettings& GetSettings(void) const;

  const std::string& GetTargetID(void) const { return (settings_.target_id); }

  void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos);

 private:
  friend class Tilt::Loader;

  ActuatedTransforms actuated_transforms_;      // Mapping from linked child to
                                                // current rotor rotation
  ControlMapper control_mapper_;                // Input control mapping
  FirstOrderFilter<float> first_order_filter_;  // Output angle low-pass filter
  Quaternion quat_output_;  // Output rotation from this actuator
  TiltSettings settings_;   // Actuator settings

  // TODO make a topic
  // topic control_surface_topic;
  std::vector<Topic> topics_;

  Tilt::Loader loader_;  // Actuator loader
};

//------------------------------------------------------------------------------
// class Tilt

Tilt::Tilt(void) : Actuator(std::shared_ptr<ActuatorImpl>(nullptr)) {}

Tilt::Tilt(const std::string& id, bool is_enabled,
           const std::string& parent_link, const std::string& child_link,
           const Logger& logger, const TopicManager& topic_manager,
           const std::string& parent_topic_path,
           const ServiceManager& service_manager,
           const StateManager& state_manager)
    : Actuator(std::shared_ptr<ActuatorImpl>(new Tilt::Impl(
          id, is_enabled, parent_link, child_link, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

void Tilt::Load(ConfigJson config_json) {
  static_cast<Tilt::Impl*>(pimpl_.get())->Load(config_json);
}

void Tilt::BeginUpdate(void) {
  static_cast<Tilt::Impl*>(pimpl_.get())->BeginUpdate();
}

void Tilt::EndUpdate(void) {
  static_cast<Tilt::Impl*>(pimpl_.get())->EndUpdate();
}

const ActuatedTransforms& Tilt::GetActuatedTransforms(void) const {
  return static_cast<Tilt::Impl*>(pimpl_.get())->GetActuatedTransforms();
}

const Quaternion& Tilt::GetControlRotation(void) const {
  return static_cast<Tilt::Impl*>(pimpl_.get())->GetControlRotation();
}

const TiltSettings& Tilt::GetSettings(void) const {
  return static_cast<Tilt::Impl*>(pimpl_.get())->GetSettings();
}

const std::string& Tilt::GetTargetID(void) const {
  return static_cast<Tilt::Impl*>(pimpl_.get())->GetTargetID();
}

void Tilt::UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos){
  static_cast<Tilt::Impl*>(pimpl_.get())
      -> UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
}

//------------------------------------------------------------------------------
// class Tilt::Impl

Tilt::Impl::Impl(const std::string& id, bool is_enabled,
                 const std::string& parent_link, const std::string& child_link,
                 const Logger& logger, const TopicManager& topic_manager,
                 const std::string& parent_topic_path,
                 const ServiceManager& service_manager,
                 const StateManager& state_manager)
    : ActuatorImpl(ActuatorType::kTilt, id, is_enabled, parent_link, child_link,
                   Constant::Component::rotor, logger, topic_manager,
                   parent_topic_path, service_manager, state_manager),
      actuated_transforms_(),
      control_mapper_(),
      first_order_filter_(),
      quat_output_(),
      settings_(),
      topics_(),
      loader_(*this) {
  SetTopicPath();
  CreateTopics();
}

void Tilt::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
  first_order_filter_.Initialize(settings_.smoothing_tc, 0.0, 0.0);
}

void Tilt::Impl::CreateTopics() {}

void Tilt::Impl::OnBeginUpdate() {}

void Tilt::Impl::OnEndUpdate() {}

const TiltSettings& Tilt::Impl::GetSettings() const { return settings_; }

const Quaternion& Tilt::Impl::GetControlRotation() const {
  return quat_output_;
}

const ActuatedTransforms& Tilt::Impl::GetActuatedTransforms() const {
  return actuated_transforms_;
}

void Tilt::Impl::UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos){
  float radians;
  Quaternion quat;

  // Convert -1.0 ~ +1.0 control signal to control surface angle (like a servo
  // motor but without any dynamics)
  auto control_signal = control_signals[0];
  first_order_filter_.SetInput(control_mapper_(control_signal));
  first_order_filter_.UpdateOutput(SimClock::Get()->NanosToSec(sim_dt_nanos));
  radians = settings_.radians_min +
            (settings_.dradians) * first_order_filter_.GetOutput();
  quat = AngleAxis(radians, settings_.vec3_axis);

  quat_output_ = quat;
  actuated_transforms_[child_link_] =
      ActuatedTransform(Affine3(quat.toRotationMatrix()));
}

//------------------------------------------------------------------------------
// class Tilt::Loader

Tilt::Loader::Loader(Tilt::Impl& impl) : impl_(impl) {}

void Tilt::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'tilt' actuator.",
                           impl_.id_.c_str());

  LoadSettings(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'tilt' actuator loaded.",
                           impl_.id_.c_str());
}

void Tilt::Loader::LoadSettings(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'tilt_settings'.");

  TiltSettings default_settings;
  auto settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::tilt_settings);
  float radians_max;

  impl_.settings_.radians_min = JsonUtils::GetNumber<float>(
      settings_json, Constant::Config::angle_min, default_settings.radians_min);

  radians_max = JsonUtils::GetNumber<float>(
      settings_json, Constant::Config::angle_max,
      default_settings.radians_min + default_settings.dradians);
  impl_.settings_.dradians = radians_max - impl_.settings_.radians_min;

  impl_.settings_.smoothing_tc =
      JsonUtils::GetNumber<float>(settings_json, Constant::Config::smoothing_tc,
                                  default_settings.smoothing_tc);

  impl_.settings_.vec3_axis = JsonUtils::GetVector3(
      settings_json, Constant::Config::axis, default_settings.vec3_axis);
  impl_.settings_.vec3_axis.normalize();

  impl_.settings_.target_id = JsonUtils::GetString(
      settings_json, Constant::Config::target, default_settings.target_id);
  if (impl_.settings_.target_id.empty()) {
    impl_.logger_.LogError(impl_.name_,
                           "[%s] tilt actuator '%s' target_id cannot be empty.",
                           impl_.id_.c_str(), impl_.GetID().c_str());
    throw Error("Invalid target actuator ID for tilt actuator.");
  }

  // Load control input map
  impl_.control_mapper_.Load(
      JsonUtils::GetJsonObject(settings_json, Constant::Config::input_map));

  impl_.logger_.LogVerbose(impl_.name_, "'tilt_settings' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
