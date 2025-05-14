// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/actuators/wheel.hpp"

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

class Wheel::Loader {
 public:
  explicit Loader(Wheel::Impl& impl);

  void Load(const json& json);

 private:
  void LoadOriginSetting(const json& json);

  void LoadWheelSetting(const json& json);

  Wheel::Impl& impl_;
};

class Wheel::Impl : public ActuatorImpl {
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

  const WheelSetting& GetWheelSettings() const;

  const WrenchPoint& GetWrenchPoint() const;

  float GetAngle() const;

  float GetSteering() const;

  float GetRotatingSpeed() const;

  float GetSteeringSpeed() const;

  bool IsEngineConnected() const;

  bool IsBrakeConnected() const;

  bool IsSteeringConnected() const;

  float GetTorque() const;

  float GetRadius() const;

  const ActuatedRotations& GetActuatedRotations() const;

  const ActuatedTransforms& GetActuatedTransforms() const;

  const float GetPowerConsumption() const;

  void UpdateActuatorOutput(std::vector<float>&& control_signals,
                            const TimeNano sim_dt_nanos);

  void SetTilt(Quaternion quat);

  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Wheel::Loader;

  float steering_value_;
  bool engine_connected_;
  bool steering_connected_;
  bool brake_connected_;

  Wheel::Loader loader_;
  WheelSetting wheel_settings_;
  FirstOrderFilter<float> engine_filter_;
  FirstOrderFilter<float> steering_filter_;
  FirstOrderFilter<float> brake_filter_;
  float force_scalar_;
  float torque_scalar_;
  float power_;
  float rotating_speed_;  // rad/s
  float steering_speed_;  // rad/s
  float brake_value_;     // rad/s
  float radius_;          // rad/s
  WrenchPoint wrench_point_;
  ActuatedRotations actuated_rotations_;  // Mapping from linked child to
                                          // current incremental wheel rotation
  ActuatedTransforms actuated_transforms_;  // Mapping from linked child to
                                            // current wheel transform
  float angle_cur_;                         // Absolute wheel angle
  TransformTree::TransformRefFrame
      transform_refframe_;  // Our RefFrame with pose from
                            // wheel_settings_.origin

  // TODO make a wheel topic
  // topic scene_image_topic;
  std::vector<Topic> topics_;
};

//------------------------------------------------------------------------------
// class Wheel

Wheel::Wheel() : Actuator(std::shared_ptr<ActuatorImpl>(nullptr)) {}

Wheel::Wheel(const std::string& id, bool is_enabled,
             const std::string& parent_link, const std::string& child_link,
             const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
    : Actuator(std::shared_ptr<ActuatorImpl>(new Wheel::Impl(
          id, is_enabled, parent_link, child_link, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

void Wheel::Load(ConfigJson config_json) {
  static_cast<Wheel::Impl*>(pimpl_.get())->Load(config_json);
}

void Wheel::BeginUpdate() {
  static_cast<Wheel::Impl*>(pimpl_.get())->BeginUpdate();
}

void Wheel::EndUpdate() {
  static_cast<Wheel::Impl*>(pimpl_.get())->EndUpdate();
}

const WheelSetting& Wheel::GetWheelSettings() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetWheelSettings();
}

const float Wheel::GetPowerConsumption() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetPowerConsumption();
}

const WrenchPoint& Wheel::GetWrenchPoint() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetWrenchPoint();
}

float Wheel::GetAngle() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetAngle();
}

float Wheel::GetSteering() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetSteering();
}

float Wheel::GetRotatingSpeed() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetRotatingSpeed();
}

float Wheel::GetSteeringSpeed() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetSteeringSpeed();
}

bool Wheel::IsEngineConnected() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->IsEngineConnected();
}

bool Wheel::IsBrakeConnected() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->IsBrakeConnected();
}

bool Wheel::IsSteeringConnected() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->IsSteeringConnected();
}

float Wheel::GetRadius() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetRadius();
}

float Wheel::GetTorque() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetTorque();
}

const ActuatedRotations& Wheel::GetActuatedRotations() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetActuatedRotations();
}

const ActuatedTransforms& Wheel::GetActuatedTransforms() const {
  return static_cast<Wheel::Impl*>(pimpl_.get())->GetActuatedTransforms();
}

void Wheel ::UpdateActuatorOutput(std::vector<float>&& control_signals,
                                  const TimeNano sim_dt_nanos) {
  static_cast<Wheel::Impl*>(pimpl_.get())
      ->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
}

Wheel::operator TransformTree::RefFrame&(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(
      static_cast<Wheel::Impl*>(pimpl_.get())
          ->
          operator const TransformTree::RefFrame&()));
}

Wheel::operator const TransformTree::RefFrame&(void) const {
  return (static_cast<Wheel::Impl*>(pimpl_.get())
              ->
              operator const TransformTree::RefFrame&());
}

//------------------------------------------------------------------------------
// class Wheel::Impl

Wheel::Impl::Impl(const std::string& id, bool is_enabled,
                  const std::string& parent_link, const std::string& child_link,
                  const Logger& logger, const TopicManager& topic_manager,
                  const std::string& parent_topic_path,
                  const ServiceManager& service_manager,
                  const StateManager& state_manager)
    : ActuatorImpl(ActuatorType::kWheel, id, is_enabled, parent_link,
                   child_link, Constant::Component::wheel, logger,
                   topic_manager, parent_topic_path, service_manager,
                   state_manager),
      steering_value_(0.0f),
      engine_connected_(true),
      steering_connected_(true),
      brake_connected_(true),
      loader_(*this),
      wheel_settings_(),
      engine_filter_(),
      steering_filter_(),
      brake_filter_(),
      force_scalar_(0.0f),
      torque_scalar_(0.0f),
      rotating_speed_(0.0f),
      steering_speed_(0.0f),
      brake_value_(0.0f),
      radius_(wheel_settings_.radius_),
      actuated_rotations_(),
      actuated_transforms_(),
      angle_cur_(0.0f),
      transform_refframe_(std::string("AR ") + id,
                          &wheel_settings_.origin_setting) {
  SetTopicPath();
  CreateTopics();
  ActuatorImpl::RegisterServiceMethods();
}

void Wheel::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
  engine_filter_.Initialize(wheel_settings_.smoothing_tc, 0.0, 0.0);
  steering_filter_.Initialize(wheel_settings_.smoothing_tc, 0.0, 0.0);
  brake_filter_.Initialize(wheel_settings_.smoothing_tc, 0.0, 0.0);
}

void Wheel::Impl::CreateTopics() {}

void Wheel::Impl::OnBeginUpdate() {}

void Wheel::Impl::OnEndUpdate() {}

const WheelSetting& Wheel::Impl::GetWheelSettings() const {
  return wheel_settings_;
}

const WrenchPoint& Wheel::Impl::GetWrenchPoint() const { return wrench_point_; }

float Wheel::Impl::GetAngle() const { return angle_cur_; }

float Wheel::Impl::GetRotatingSpeed() const { return rotating_speed_; }

float Wheel::Impl::GetSteeringSpeed() const { return steering_speed_; }

float Wheel::Impl::GetTorque() const { return torque_scalar_; }

float Wheel::Impl::GetRadius() const { return radius_; }

float Wheel::Impl::GetSteering() const { return steering_value_; }

bool Wheel::Impl::IsEngineConnected() const { return engine_connected_; }

bool Wheel::Impl::IsBrakeConnected() const { return brake_connected_; }

bool Wheel::Impl::IsSteeringConnected() const { return steering_connected_; }

const ActuatedRotations& Wheel::Impl::GetActuatedRotations() const {
  return actuated_rotations_;
}

const ActuatedTransforms& Wheel::Impl::GetActuatedTransforms() const {
  return actuated_transforms_;
}

const float Wheel::Impl::GetPowerConsumption() const { return power_; }

void Wheel::Impl::UpdateActuatorOutput(std::vector<float>&& control_signals,
                                       const TimeNano sim_dt_nanos) {
  // Getting the control signals necessary for movement
  auto engine_signal = (engine_connected_) ? control_signals[0] : 0;
  auto steering_signal = (steering_connected_) ? control_signals[1] : 0;
  auto brake_signal = (brake_connected_) ? control_signals[2] : 0;
  TimeSec dt_sec = sim_dt_nanos / 1.0e9;

  if (wheel_settings_.coeff_of_friction == 0) {
    steering_signal = 0;
  }

  // TO-DO: Control Signal Filter, if -90 go all the way to left for
  // steering_angle, 90 is to go all the right
  //  This is returned by GetSteering()
  steering_filter_.SetInput(std::clamp(steering_signal, -1.0f, 1.0f));
  steering_filter_.UpdateOutput(dt_sec);  // do filtering
  auto steering_signal_filtered = steering_filter_.GetOutput();

  steering_value_ = steering_signal_filtered * (float)M_PI / 4;
  // if absolute steering value is more than 45 degrees in radians then clamp it
  // and set speed to 0
  if (std::abs(steering_value_) > M_PI / 4) {
    steering_value_ =
        std::clamp(steering_value_, float(-M_PI / 4), float(M_PI / 4));
    steering_speed_ = 0;
  }

  Quaternion quat_steering(AngleAxis(steering_value_, Vector3::UnitZ()));

  // This is returned by GetRotatingSpeed()
  engine_filter_.SetInput(std::clamp(engine_signal, -1.0f, 1.0f));
  engine_filter_.UpdateOutput(dt_sec);  // do filtering
  auto engine_signal_filtered = engine_filter_.GetOutput();
  auto engine_torque = engine_signal_filtered * 100.0f * 0.5f;

  brake_filter_.SetInput(std::clamp(brake_signal, 0.0f, 1.0f));
  brake_filter_.UpdateOutput(dt_sec);  // do filtering
  auto brake_signal_filtered = brake_filter_.GetOutput();

  // torque_scalar_ = 0;
  auto brake_torque = brake_signal_filtered * -GetRotatingSpeed();
  auto friction_torque =
      wheel_settings_.coeff_of_friction * -GetRotatingSpeed();
  auto resulting_torque = engine_torque + brake_torque + friction_torque;

  auto angular_accel = resulting_torque / 0.125;  // Hardcoded inertia moment.
  rotating_speed_ += dt_sec * angular_accel;

  auto dangle = rotating_speed_ * dt_sec;

  angle_cur_ += dangle;
  while (angle_cur_ < 0.0f) angle_cur_ += static_cast<float>(2.0 * M_PI);
  while (angle_cur_ > static_cast<float>(2.0 * M_PI))
    angle_cur_ -= static_cast<float>(2.0 * M_PI);
  actuated_transforms_[child_link_] = Affine3(
      (quat_steering * AngleAxis(angle_cur_, wheel_settings_.normal_vector))
          .toRotationMatrix());

  // Save the rotor's angular velocity as an actuated rotation to be applied to
  // the rotor's output child link to spin the rotor mesh
  Vector3 wheel_ang_vel(dangle * wheel_settings_.normal_vector);
  actuated_rotations_[child_link_] = wheel_ang_vel;
}

Wheel::Loader::Loader(Wheel::Impl& impl) : impl_(impl) {}

void Wheel::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'wheel_settings'.",
                           impl_.id_.c_str());

  LoadOriginSetting(json);
  LoadWheelSetting(json);

  impl_.is_loaded_ = true;

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'wheel_settings' loaded.",
                           impl_.id_.c_str());
}

void Wheel::Loader::LoadOriginSetting(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'origin'.");

  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "'origin' missing or empty. Using default.");
  } else {
    impl_.wheel_settings_.origin_setting =
        JsonUtils::GetTransform(json, Constant::Config::origin);
  }
  impl_.logger_.LogVerbose(impl_.name_, "'origin' loaded.");
}

Wheel::Impl::operator const TransformTree::RefFrame&(void) const {
  return (transform_refframe_);
}

void Wheel::Loader::LoadWheelSetting(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'wheel_settings'.");

  WheelSetting default_wheel_setting;
  auto wheel_settings_json =
      JsonUtils::GetJsonObject(json, Constant::Config::wheel_settings);

  if (JsonUtils::IsEmpty(wheel_settings_json)) {
    impl_.logger_.LogVerbose(
        impl_.name_, "'wheel-settings' missing or empty. Using default.");
  } else {
    impl_.wheel_settings_.normal_vector = JsonUtils::GetVector3(
        wheel_settings_json, Constant::Config::normal_vector);

    auto turning_dir = JsonUtils::GetString(wheel_settings_json,
                                            Constant::Config::turning_direction,
                                            Constant::Config::clock_wise);

    impl_.wheel_settings_.coeff_of_friction = JsonUtils::GetNumber<float>(
        // Need to change this
        wheel_settings_json, Constant::Config::coeff_of_friction,
        default_wheel_setting.coeff_of_friction);

    impl_.wheel_settings_.smoothing_tc = JsonUtils::GetNumber<float>(
        wheel_settings_json, Constant::Config::smoothing_tc,
        default_wheel_setting.smoothing_tc);

    impl_.wheel_settings_.steering_connected_ = JsonUtils::GetBoolean(
        wheel_settings_json, Constant::Config::steering_connected,
        default_wheel_setting.steering_connected_);
  }

  impl_.wheel_settings_.CalcMaxTorqueAndForce();

  impl_.logger_.LogVerbose(impl_.name_, "'wheel_settings' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
