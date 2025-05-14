// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/actuators/gimbal.hpp"

#include <memory>
#include <string>

#include "actuator_impl.hpp"
#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/physics_common_types.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

//------------------------------------------------------------------------------
// Forward declarations

class Gimbal::Loader {
 public:
  explicit Loader(Gimbal::Impl& impl);

  void Load(const json& json);

 private:
  void LoadOriginSettings(const json& json);

  Gimbal::Impl& impl_;
};

class Gimbal::Impl : public ActuatorImpl {
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

  const ActuatedTransforms& GetActuatedTransforms() const;

   void UpdateActuatorOutput(std::vector<float> && control_signals,
                            const TimeNano sim_dt_nanos);

  void UpdateGimbal(IController::GimbalState& new_state,
                    const TimeNano sim_dt_nanos);

  const IController::GimbalState& GetGimbalState() const;

  double calcSetpoint(double dt, double lastSetpoint, double newSetpoint,
                      double newRateSetpoint);

 private:
  friend class Gimbal::Loader;
  ActuatedTransforms actuated_transforms_;

  Gimbal::Loader loader_;
  IController::GimbalState gimbal_state_;

  std::vector<Topic> topics_;
};

//------------------------------------------------------------------------------
// class Gimbal

Gimbal::Gimbal(void) : Actuator(std::shared_ptr<ActuatorImpl>(nullptr)) {}

Gimbal::Gimbal(const std::string& id, bool is_enabled,
               const std::string& parent_link, const std::string& child_link,
               const Logger& logger, const TopicManager& topic_manager,
               const std::string& parent_topic_path,
               const ServiceManager& service_manager,
               const StateManager& state_manager)
    : Actuator(std::shared_ptr<ActuatorImpl>(new Gimbal::Impl(
          id, is_enabled, parent_link, child_link, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

void Gimbal::Load(ConfigJson config_json) {
  static_cast<Gimbal::Impl*>(pimpl_.get())->Load(config_json);
}

void Gimbal::BeginUpdate() {
  static_cast<Gimbal::Impl*>(pimpl_.get())->BeginUpdate();
}

void Gimbal::EndUpdate() {
  static_cast<Gimbal::Impl*>(pimpl_.get())->EndUpdate();
}

const ActuatedTransforms& Gimbal::GetActuatedTransforms() const {
  return static_cast<Gimbal::Impl*>(pimpl_.get())->GetActuatedTransforms();
}

void Gimbal::UpdateGimbal(IController::GimbalState& new_state,
                          const TimeNano sim_dt_nanos) {
  return static_cast<Gimbal::Impl*>(pimpl_.get())
      ->UpdateGimbal(new_state, sim_dt_nanos);
}

const IController::GimbalState& Gimbal::GetGimbalState() const {
  return static_cast<Gimbal::Impl*>(pimpl_.get())->GetGimbalState();
};

void Gimbal::UpdateActuatorOutput(std::vector<float> && control_signals, const TimeNano sim_dt_nanos){
  static_cast<Gimbal::Impl*>(pimpl_.get())
      ->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
}

//------------------------------------------------------------------------------
// class Gimbal::Impl

Gimbal::Impl::Impl(const std::string& id, bool is_enabled,
                   const std::string& parent_link,
                   const std::string& child_link, const Logger& logger,
                   const TopicManager& topic_manager,
                   const std::string& parent_topic_path,
                   const ServiceManager& service_manager,
                   const StateManager& state_manager)
    : ActuatorImpl(ActuatorType::kGimbal, id, is_enabled, parent_link,
                   child_link, Constant::Component::gimbal, logger,
                   topic_manager, parent_topic_path, service_manager,
                   state_manager),
      loader_(*this),
      actuated_transforms_() {
  SetTopicPath();
  CreateTopics();
}

void Gimbal::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

void Gimbal::Impl::CreateTopics() {}

void Gimbal::Impl::OnBeginUpdate() {}

void Gimbal::Impl::OnEndUpdate() {}
const IController::GimbalState& Gimbal::Impl::GetGimbalState() const {
  return gimbal_state_;
};

void Gimbal::Impl::UpdateGimbal(IController::GimbalState& new_state,
                                const TimeNano sim_dt_nanos) {
  TimeSec dt = SimClock::Get()->NanosToSec(sim_dt_nanos);
  auto roll =
      calcSetpoint(dt, gimbal_state_.roll, new_state.roll, new_state.roll_vel);
  auto pitch = calcSetpoint(dt, gimbal_state_.pitch, new_state.pitch,
                            new_state.pitch_vel);
  auto yaw =
      calcSetpoint(dt, gimbal_state_.yaw, new_state.yaw, new_state.yaw_vel);

  // Need to pass these on to Unreal?
  gimbal_state_.yaw_lock = new_state.yaw_lock;
  gimbal_state_.roll_lock = new_state.roll_lock;
  gimbal_state_.pitch_lock = new_state.pitch_lock;

  gimbal_state_.yaw_vel = new_state.yaw_vel;
  gimbal_state_.roll_vel = new_state.roll_vel;
  gimbal_state_.pitch_vel = new_state.pitch_vel;

  if (roll == gimbal_state_.roll && pitch == gimbal_state_.pitch &&
      yaw == gimbal_state_.yaw) {
    return;
  } else {
    gimbal_state_.roll = roll;
    gimbal_state_.pitch = pitch;
    gimbal_state_.yaw = yaw;
    auto quat_output_ = TransformUtils::ToQuaternion(
        gimbal_state_.roll, gimbal_state_.pitch, gimbal_state_.yaw);

    actuated_transforms_[child_link_] =
        ActuatedTransform(Affine3(quat_output_.toRotationMatrix()));
  }
}

void Gimbal::Impl::UpdateActuatorOutput(std::vector<float> && control_signals, const TimeNano sim_dt_nanos) {
  return;
}

const ActuatedTransforms& Gimbal::Impl::GetActuatedTransforms() const {
  return actuated_transforms_;
}

double Gimbal::Impl::calcSetpoint(double dt, double lastSetpoint,
                                  double newSetpoint, double newRateSetpoint) {
  const bool setpointValid = std::isfinite(newSetpoint);
  const bool rateSetpointValid = std::isfinite(newRateSetpoint);

  if (rateSetpointValid) {
    const double rateDiff = dt * newRateSetpoint;
    const double setpointFromRate = lastSetpoint + rateDiff;

    if (setpointValid) {
      // In this case angle and rate are valid, so we use the rate but constrain
      // it by the angle.
      if (rateDiff > 0.0) {
        return std::min(newSetpoint, setpointFromRate);
      } else {
        return std::max(newSetpoint, setpointFromRate);
      }

    } else {
      // Only the rate is valid, so we just use it.
      return setpointFromRate;
    }

  } else if (setpointValid) {
    // Only the angle is valid.
    return newSetpoint;
  }
  return lastSetpoint;
}

//------------------------------------------------------------------------------
// class Gimbal::Loader

Gimbal::Loader::Loader(Gimbal::Impl& impl) : impl_(impl) {}

void Gimbal::Loader::Load(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'gimbal'.",
                           impl_.id_.c_str());

  impl_.is_loaded_ = true;
  LoadOriginSettings(json);
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'gimbal loaded.",
                           impl_.id_.c_str());
}
void Gimbal::Loader::LoadOriginSettings(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading gimbal origin settings.");
  auto origin_json = JsonUtils::GetJsonObject(json, Constant::Config::origin);
  if (JsonUtils::IsEmpty(origin_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "'origin' missing or empty. Using default.");
  } else {
    auto origin = JsonUtils::GetTransform(json, Constant::Config::origin);
    auto rpy = TransformUtils::ToRPY(origin.rotation_);

    impl_.gimbal_state_.roll = rpy.x();
    impl_.gimbal_state_.pitch = rpy.y();
    impl_.gimbal_state_.yaw = rpy.z();
  }
  impl_.logger_.LogVerbose(impl_.name_, "'gimbal origin settings' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
