// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actuators/rotor.hpp"

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

class Rotor::Loader {
 public:
  explicit Loader(Rotor::Impl& impl);

  void Load(const json& json);

 private:
  void LoadOriginSetting(const json& json);

  void LoadRotorSetting(const json& json);

  Rotor::Impl& impl_;
};

class Rotor::Impl : public ActuatorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const std::string& child_link, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void CreateTopics();

  const Transform& GetOrigin() const;

  void OnBeginUpdate() override;

  void OnEndUpdate() override;

  const RotorSetting& GetRotorSettings() const;

  const WrenchPoint& GetWrenchPoint() const;

  float GetAngle() const;

  float GetRotatingSpeed() const;

  float GetThrust() const;

  float GetTorque() const;

  const ActuatedRotations& GetActuatedRotations() const;

  const ActuatedTransforms& GetActuatedTransforms() const;

  const float GetPowerConsumption() const;

  void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos);

  void SetAirDensityRatio(float air_density_ratio);

  void SetTilt(Quaternion quat);

  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Rotor::Loader;

  Rotor::Loader loader_;
  RotorSetting rotor_settings_;
  float air_density_ratio_;
  FirstOrderFilter<float> first_order_filter_;
  float thrust_scalar_;
  float torque_scalar_;
  float power_;
  float rotating_speed_;  // rad/s
  WrenchPoint wrench_point_;
  ActuatedRotations actuated_rotations_;  // Mapping from linked child to
                                          // current incremental rotor rotation
  ActuatedTransforms actuated_transforms_;  // Mapping from linked child to
                                            // current rotor transform
  float angle_cur_;                         // Absolute rotor angle
  Quaternion quat_tilt_;                    // Orientation modified by tilt
  TransformTree::TransformRefFrame
      transform_refframe_;  // Our RefFrame with pose from
                            // rotor_settings_.origin

  // TODO make a rotor topic
  // topic scene_image_topic;
  std::vector<Topic> topics_;
};

//------------------------------------------------------------------------------
// class Rotor

Rotor::Rotor() : Actuator(std::shared_ptr<ActuatorImpl>(nullptr)) {}

Rotor::Rotor(const std::string& id, bool is_enabled,
             const std::string& parent_link, const std::string& child_link,
             const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
    : Actuator(std::shared_ptr<ActuatorImpl>(new Rotor::Impl(
          id, is_enabled, parent_link, child_link, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

void Rotor::Load(ConfigJson config_json) {
  static_cast<Rotor::Impl*>(pimpl_.get())->Load(config_json);
}

void Rotor::BeginUpdate() {
  static_cast<Rotor::Impl*>(pimpl_.get())->BeginUpdate();
}

void Rotor::EndUpdate() {
  static_cast<Rotor::Impl*>(pimpl_.get())->EndUpdate();
}

const RotorSetting& Rotor::GetRotorSettings() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetRotorSettings();
}

const float Rotor::GetPowerConsumption() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetPowerConsumption();
}

const WrenchPoint& Rotor::GetWrenchPoint() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetWrenchPoint();
}

float Rotor::GetAngle() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetAngle();
}

float Rotor::GetRotatingSpeed() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetRotatingSpeed();
}

float Rotor::GetThrust() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetThrust();
}

float Rotor::GetTorque() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetTorque();
}

const ActuatedRotations& Rotor::GetActuatedRotations() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetActuatedRotations();
}

const ActuatedTransforms& Rotor::GetActuatedTransforms() const {
  return static_cast<Rotor::Impl*>(pimpl_.get())->GetActuatedTransforms();
}

void Rotor::UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos){
  static_cast<Rotor::Impl*>(pimpl_.get())
      ->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
}

void Rotor::SetAirDensityRatio(float air_density_ratio) {
  static_cast<Rotor::Impl*>(pimpl_.get())
      ->SetAirDensityRatio(air_density_ratio);
}

void Rotor::SetTilt(Quaternion quat) {
  static_cast<Rotor::Impl*>(pimpl_.get())->SetTilt(quat);
}

Rotor::operator TransformTree::RefFrame&(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(
      static_cast<Rotor::Impl*>(pimpl_.get())
          ->
          operator const TransformTree::RefFrame&()));
}

Rotor::operator const TransformTree::RefFrame&(void) const {
  return (static_cast<Rotor::Impl*>(pimpl_.get())
              ->
              operator const TransformTree::RefFrame&());
}

//------------------------------------------------------------------------------
// class Rotor::Impl

Rotor::Impl::Impl(const std::string& id, bool is_enabled,
                  const std::string& parent_link, const std::string& child_link,
                  const Logger& logger, const TopicManager& topic_manager,
                  const std::string& parent_topic_path,
                  const ServiceManager& service_manager,
                  const StateManager& state_manager)
    : ActuatorImpl(ActuatorType::kRotor, id, is_enabled, parent_link,
                   child_link, Constant::Component::rotor, logger,
                   topic_manager, parent_topic_path, service_manager,
                   state_manager),
      loader_(*this),
      air_density_ratio_(1.0f),
      thrust_scalar_(0.0f),
      torque_scalar_(0.0f),
      rotating_speed_(0.0f),
      actuated_rotations_(),
      actuated_transforms_(),
      angle_cur_(0.0f),
      quat_tilt_(Quaternion::Identity()),
      transform_refframe_(std::string("AR ") + id,
                          &rotor_settings_.origin_setting) {
  SetTopicPath();
  CreateTopics();
  ActuatorImpl::RegisterServiceMethods();
}

void Rotor::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
  first_order_filter_.Initialize(rotor_settings_.smoothing_tc, 0.0, 0.0);
}

void Rotor::Impl::CreateTopics() {
  // TODO make rotor topic
  // rotor_topic = topic("rotor", topic_path, topic_type::published,
  //                           60, message_type::???);

  // topics.push_back(rotor_topic);
}

void Rotor::Impl::OnBeginUpdate() {
  // topic_manager.RegisterTopic(rotor_topic);
}

void Rotor::Impl::OnEndUpdate() {
  // topic_manager.UnregisterTopic(rotor_topic);
}

const RotorSetting& Rotor::Impl::GetRotorSettings() const {
  return rotor_settings_;
}

const WrenchPoint& Rotor::Impl::GetWrenchPoint() const { return wrench_point_; }

float Rotor::Impl::GetAngle() const { return angle_cur_; }

float Rotor::Impl::GetRotatingSpeed() const { return rotating_speed_; }

float Rotor::Impl::GetThrust() const { return thrust_scalar_; }

float Rotor::Impl::GetTorque() const { return torque_scalar_; }

const ActuatedRotations& Rotor::Impl::GetActuatedRotations() const {
  return actuated_rotations_;
}

const ActuatedTransforms& Rotor::Impl::GetActuatedTransforms() const {
  return actuated_transforms_;
}

const float Rotor::Impl::GetPowerConsumption() const { return power_; }

void Rotor::Impl::SetAirDensityRatio(float air_density_ratio) {
  air_density_ratio_ = air_density_ratio;
}

void Rotor::Impl::SetTilt(Quaternion quat) { quat_tilt_ = quat; }

void Rotor::Impl::UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos){
  // This actuator uses one control signal
  auto control_signal = control_signals[0];
  // Apply first order filter to simulate actuator hardware dynamics
  TimeSec dt_sec = sim_dt_nanos / 1.0e9;
  first_order_filter_.SetInput(std::clamp(control_signal, 0.0f, 1.0f));
  first_order_filter_.UpdateOutput(dt_sec);  // do filtering
  auto control_signal_input = first_order_filter_.GetInput();
  auto control_signal_filtered = first_order_filter_.GetOutput();
  // TODO Clamp filtered output to same range as input?

  // Note: rotating_speed_ is set by max_speed_square because
  // control_signal_filtered is proportional to rotor thrust, and thrust is
  // proportional to rotating speed squared. See relationship of rotation speed
  // with thrust: http://physics.stackexchange.com/a/32013/14061
  float rotating_speed_raw =
      std::sqrt(control_signal_filtered * rotor_settings_.max_speed_square);

  // In the past, some clipping of rotations near zero was needed to prevent
  // slow rotation when the propellers were really supposed to be stopped since
  // there are no actual physics like friction to damp out minor actuations, but
  // this depended on other settings like SimpleFlight's min throttle position.
  // As a placeholder for this kind of functionality, just use
  // IsApproximatelyZero() to clip numerical precision error for pure zero
  // rotation, and reconsider later if there is a need to have a more logical
  // clipping threshold based on the controller or actuator parameters.
  rotating_speed_ = MathUtils::IsApproximatelyZero(rotating_speed_raw)
                        ? 0.0f
                        : rotating_speed_raw;

  thrust_scalar_ = control_signal_filtered * rotor_settings_.max_thrust;
  torque_scalar_ = control_signal_filtered * rotor_settings_.max_torque *
                   static_cast<int>(rotor_settings_.turning_direction);

  if (is_fault_injected_) {
    rotating_speed_raw = 0.0f;
    rotating_speed_ = 0.0f;
    thrust_scalar_ = 0.0f;
    torque_scalar_ = 0.0f;
  }

  power_ = abs(torque_scalar_ * rotating_speed_raw);

  // forces and torques are proportional to air density:
  // http://physics.stackexchange.com/a/32013/14061
  Vector3 wrench_force_rotor_frame =
      rotor_settings_.normal_vector * thrust_scalar_ * air_density_ratio_;

  Vector3 wrench_torque_rotor_frame =
      rotor_settings_.normal_vector * torque_scalar_ * air_density_ratio_;

  // Apply tilt rotation
  wrench_point_.wrench.force =
      quat_tilt_._transformVector(wrench_force_rotor_frame);

  wrench_point_.wrench.torque =
      quat_tilt_._transformVector(wrench_torque_rotor_frame);

  // Save the rotor's absolute rotation to be applied to the rotor's output
  // child link to spin the rotor mesh
  auto dangle = rotating_speed_ * dt_sec *
                static_cast<int>(rotor_settings_.turning_direction);

  angle_cur_ += dangle;
  while (angle_cur_ < 0.0f) angle_cur_ += static_cast<float>(2.0 * M_PI);
  while (angle_cur_ > static_cast<float>(2.0 * M_PI))
    angle_cur_ -= static_cast<float>(2.0 * M_PI);
  actuated_transforms_[child_link_] = Affine3(
      (quat_tilt_ * AngleAxis(angle_cur_, rotor_settings_.normal_vector))
          .toRotationMatrix());

  // Save the rotor's angular velocity as an actuated rotation to be applied to
  // the rotor's output child link to spin the rotor mesh
  Vector3 rotor_ang_vel(dangle * rotor_settings_.normal_vector);

  actuated_rotations_[child_link_] = rotor_ang_vel;
}

Rotor::Impl::operator const TransformTree::RefFrame&(void) const {
  return (transform_refframe_);
}

//------------------------------------------------------------------------------
// class Rotor::Loader

Rotor::Loader::Loader(Rotor::Impl& impl) : impl_(impl) {}

void Rotor::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'rotor_settings'.",
                           impl_.id_.c_str());

  LoadOriginSetting(json);
  LoadRotorSetting(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'rotor_settings' loaded.",
                           impl_.id_.c_str());
}

void Rotor::Loader::LoadOriginSetting(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "'origin' missing or empty. Using default.");
  } else {
    impl_.rotor_settings_.origin_setting =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl_.logger_.LogVerbose(impl_.name_, "'origin' loaded.");
}

void Rotor::Loader::LoadRotorSetting(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'rotor_settings'.");

  RotorSetting default_rotor_setting;
  auto rotor_settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::rotor_settings);

  if (JsonUtils::IsEmpty(rotor_settings_json)) {
    impl_.logger_.LogVerbose(
        impl_.name_, "'rotor-settings' missing or empty. Using default.");
  } else {
    impl_.rotor_settings_.normal_vector = JsonUtils::GetVector3(
        rotor_settings_json, Constant::Config::normal_vector);

    auto turning_dir = JsonUtils::GetString(rotor_settings_json,
                                            Constant::Config::turning_direction,
                                            Constant::Config::clock_wise);
    if (turning_dir == Constant::Config::counter_clock_wise) {
      impl_.rotor_settings_.turning_direction =
          RotorTurningDirection::kRotorTurningDirectionCCW;
    } else {
      impl_.rotor_settings_.turning_direction =
          RotorTurningDirection::kRotorTurningDirectionCW;
    }

    impl_.rotor_settings_.coeff_of_thrust = JsonUtils::GetNumber<float>(
        rotor_settings_json, Constant::Config::coeff_of_thrust,
        default_rotor_setting.coeff_of_thrust);

    impl_.rotor_settings_.coeff_of_torque = JsonUtils::GetNumber<float>(
        rotor_settings_json, Constant::Config::coeff_of_torque,
        default_rotor_setting.coeff_of_torque);

    impl_.rotor_settings_.max_rpm = JsonUtils::GetNumber<float>(
        rotor_settings_json, Constant::Config::max_rpm,
        default_rotor_setting.max_rpm);

    impl_.rotor_settings_.propeller_diameter = JsonUtils::GetNumber<float>(
        rotor_settings_json, Constant::Config::propeller_diameter,
        default_rotor_setting.propeller_diameter);

    impl_.rotor_settings_.smoothing_tc = JsonUtils::GetNumber<float>(
        rotor_settings_json, Constant::Config::smoothing_tc,
        default_rotor_setting.smoothing_tc);
  }

  impl_.rotor_settings_.CalcMaxThrustAndTorque();

  impl_.logger_.LogVerbose(impl_.name_, "'rotor_settings' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
