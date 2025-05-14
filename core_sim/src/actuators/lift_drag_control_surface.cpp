// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/actuators/lift_drag_control_surface.hpp"

#include <memory>
#include <string>

#include "actuator_impl.hpp"
#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/first_order_filter.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/physics_common_types.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

//------------------------------------------------------------------------------
// Forward declarations

class LiftDragControlSurface::Loader {
 public:
  explicit Loader(LiftDragControlSurface::Impl& impl);

  void Load(const json& json);

 private:
  void LoadSettings(const json& json);

  LiftDragControlSurface::Impl& impl_;
};

class LiftDragControlSurface::Impl : public ActuatorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const std::string& child_link, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void CreateTopics();

  void OnBeginUpdate() override;

  void OnEndUpdate() override;

  const LiftDragControlSurfaceSettings& GetSettings() const;

  const float& GetControlAngle() const;

  void UpdateActuatorOutput(std::vector<float> && control_signals, const TimeNano sim_dt_nanos);

 private:
  friend class LiftDragControlSurface::Loader;

  LiftDragControlSurface::Loader loader_;
  LiftDragControlSurfaceSettings settings_;

  FirstOrderFilter<float> first_order_filter_;
  float control_angle_;

  // TODO make a topic
  // topic control_surface_topic;
  std::vector<Topic> topics_;
};

//------------------------------------------------------------------------------
// class LiftDragControlSurface

LiftDragControlSurface::LiftDragControlSurface()
    : Actuator(std::shared_ptr<ActuatorImpl>(nullptr)) {}

LiftDragControlSurface::LiftDragControlSurface(
    const std::string& id, bool is_enabled, const std::string& parent_link,
    const std::string& child_link, const Logger& logger,
    const TopicManager& topic_manager, const std::string& parent_topic_path,
    const ServiceManager& service_manager, const StateManager& state_manager)
    : Actuator(std::shared_ptr<ActuatorImpl>(new LiftDragControlSurface::Impl(
          id, is_enabled, parent_link, child_link, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

void LiftDragControlSurface::Load(ConfigJson config_json) {
  static_cast<LiftDragControlSurface::Impl*>(pimpl_.get())->Load(config_json);
}

void LiftDragControlSurface::BeginUpdate() {
  static_cast<LiftDragControlSurface::Impl*>(pimpl_.get())->BeginUpdate();
}

void LiftDragControlSurface::EndUpdate() {
  static_cast<LiftDragControlSurface::Impl*>(pimpl_.get())->EndUpdate();
}

const LiftDragControlSurfaceSettings& LiftDragControlSurface::GetSettings()
    const {
  return static_cast<LiftDragControlSurface::Impl*>(pimpl_.get())
      ->GetSettings();
}

const float& LiftDragControlSurface::GetControlAngle() const {
  return static_cast<LiftDragControlSurface::Impl*>(pimpl_.get())
      ->GetControlAngle();
}

void LiftDragControlSurface::UpdateActuatorOutput(std::vector<float> && control_signals,
  const TimeNano sim_dt_nanos){
  static_cast<LiftDragControlSurface::Impl*>(pimpl_.get())
      ->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
}

//------------------------------------------------------------------------------
// class LiftDragControlSurface::Impl

LiftDragControlSurface::Impl::Impl(
    const std::string& id, bool is_enabled, const std::string& parent_link,
    const std::string& child_link, const Logger& logger,
    const TopicManager& topic_manager, const std::string& parent_topic_path,
    const ServiceManager& service_manager, const StateManager& state_manager)
    :ActuatorImpl(ActuatorType::kLiftDragControlSurface, id, is_enabled,
                   parent_link, child_link, Constant::Component::rotor, logger,
                   topic_manager, parent_topic_path, service_manager,
                   state_manager),
      loader_(*this),
      control_angle_(0.0f) {
  SetTopicPath();
  CreateTopics();
}

void LiftDragControlSurface::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
  first_order_filter_.Initialize(settings_.smoothing_tc, 0.0, 0.0);
}

void LiftDragControlSurface::Impl::CreateTopics() {}

void LiftDragControlSurface::Impl::OnBeginUpdate() {}

void LiftDragControlSurface::Impl::OnEndUpdate() {}

const LiftDragControlSurfaceSettings&
LiftDragControlSurface::Impl::GetSettings() const {
  return settings_;
}

const float& LiftDragControlSurface::Impl::GetControlAngle() const {
  return control_angle_;
}

void LiftDragControlSurface::Impl::UpdateActuatorOutput(std::vector<float> && control_signals,
  const TimeNano sim_dt_nanos){
  // Convert -1.0 ~ +1.0 control signal to control surface angle (like a servo
  // motor but without any dynamics)
  auto control_signal = control_signals[0];
  first_order_filter_.SetInput(control_signal);
  first_order_filter_.UpdateOutput(SimClock::Get()->NanosToSec(sim_dt_nanos));
  control_angle_ = settings_.rotation_rate * first_order_filter_.GetOutput();
}

//------------------------------------------------------------------------------
// class LiftDragControlSurface::Loader

LiftDragControlSurface::Loader::Loader(LiftDragControlSurface::Impl& impl)
    : impl_(impl) {}

void LiftDragControlSurface::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_,
                           "[%s] Loading 'lift_drag_control_surface' actuator.",
                           impl_.id_.c_str());

  LoadSettings(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_,
                           "[%s] 'lift_drag_control_surface' actuator loaded.",
                           impl_.id_.c_str());
}

void LiftDragControlSurface::Loader::LoadSettings(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_,
                           "Loading 'lift_drag_control_surface_settings'.");

  LiftDragControlSurfaceSettings default_settings;
  auto settings_json = JsonUtils::GetJsonObject(
      json, Constant::Config::lift_drag_control_surface_settings);

  impl_.settings_.rotation_rate = JsonUtils::GetNumber<float>(
      settings_json, Constant::Config::rotation_rate,
      default_settings.rotation_rate);

  impl_.settings_.smoothing_tc =
      JsonUtils::GetNumber<float>(settings_json, Constant::Config::smoothing_tc,
                                  default_settings.smoothing_tc);

  impl_.logger_.LogVerbose(impl_.name_,
                           "'lift_drag_control_surface_settings' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
