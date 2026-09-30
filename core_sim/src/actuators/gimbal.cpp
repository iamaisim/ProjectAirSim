// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "core_sim/actuators/gimbal.hpp"

#include <memory>
#include <mutex>
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

  void RegisterServiceMethods();

  const Transform& GetOrigin() const;

  void OnBeginUpdate() override;

  void OnEndUpdate() override;

  const ActuatedTransforms& GetActuatedTransforms() const;

  void UpdateActuatorOutput(std::vector<float>&& control_signals,
                            const TimeNano sim_dt_nanos);

  void SetCommand(const GimbalCommand& command);

  bool SetCommandFromJson(json command_json);

  void UpdateGimbal(const TimeNano sim_dt_nanos);

  GimbalState GetGimbalState() const;

  json GetStateAsJson();

  double calcSetpoint(double dt, double lastSetpoint, double newSetpoint,
                      double newRateSetpoint);

 private:
  friend class Gimbal::Loader;
  ActuatedTransforms actuated_transforms_;

  Gimbal::Loader loader_;
  GimbalCommand command_;
  GimbalState gimbal_state_;
  mutable std::mutex state_mutex_;

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

void Gimbal::SetCommand(const GimbalCommand& command) {
  static_cast<Gimbal::Impl*>(pimpl_.get())->SetCommand(command);
}

void Gimbal::UpdateGimbal(const TimeNano sim_dt_nanos) {
  static_cast<Gimbal::Impl*>(pimpl_.get())->UpdateGimbal(sim_dt_nanos);
}

GimbalState Gimbal::GetGimbalState() const {
  return static_cast<Gimbal::Impl*>(pimpl_.get())->GetGimbalState();
}

void Gimbal::UpdateActuatorOutput(std::vector<float>&& control_signals,
                                  const TimeNano sim_dt_nanos) {
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
  RegisterServiceMethods();
}

void Gimbal::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

void Gimbal::Impl::CreateTopics() {}

void Gimbal::Impl::RegisterServiceMethods() {
  ActuatorImpl::RegisterServiceMethods();

  auto set_command =
      ServiceMethod(topic_path_ + "/SetCommand", {"command"});
  auto set_command_handler = set_command.CreateMethodHandler(
      &Gimbal::Impl::SetCommandFromJson, *this);
  service_manager_.RegisterMethod(set_command, set_command_handler);

  auto get_state = ServiceMethod(topic_path_ + "/GetState", {""});
  auto get_state_handler =
      get_state.CreateMethodHandler(&Gimbal::Impl::GetStateAsJson, *this);
  service_manager_.RegisterMethod(get_state, get_state_handler);
}

void Gimbal::Impl::OnBeginUpdate() {}

void Gimbal::Impl::OnEndUpdate() {}

GimbalState Gimbal::Impl::GetGimbalState() const {
  std::lock_guard<std::mutex> guard(state_mutex_);
  return gimbal_state_;
}

void Gimbal::Impl::SetCommand(const GimbalCommand& command) {
  std::lock_guard<std::mutex> guard(state_mutex_);
  command_ = command;
}

bool Gimbal::Impl::SetCommandFromJson(json command_json) {
  GimbalCommand command;
  auto get_optional_float = [&command_json](const char* key) {
    if (!command_json.contains(key) || command_json.at(key).is_null()) {
      return NAN;
    }
    return command_json.at(key).get<float>();
  };

  command.roll = get_optional_float("roll");
  command.pitch = get_optional_float("pitch");
  command.yaw = get_optional_float("yaw");
  command.roll_rate = get_optional_float("roll_rate");
  command.pitch_rate = get_optional_float("pitch_rate");
  command.yaw_rate = get_optional_float("yaw_rate");

  {
    std::lock_guard<std::mutex> guard(state_mutex_);
    command.roll_lock = command_json.value("roll_lock", command_.roll_lock);
    command.pitch_lock = command_json.value("pitch_lock", command_.pitch_lock);
    command.yaw_lock = command_json.value("yaw_lock", command_.yaw_lock);
    command_ = command;
  }
  return true;
}

json Gimbal::Impl::GetStateAsJson() {
  auto state = GetGimbalState();
  return {{"roll", state.roll},
          {"pitch", state.pitch},
          {"yaw", state.yaw},
          {"roll_rate", state.roll_rate},
          {"pitch_rate", state.pitch_rate},
          {"yaw_rate", state.yaw_rate},
          {"roll_lock", state.roll_lock},
          {"pitch_lock", state.pitch_lock},
          {"yaw_lock", state.yaw_lock}};
}

void Gimbal::Impl::UpdateGimbal(const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> guard(state_mutex_);
  const TimeSec dt = SimClock::Get()->NanosToSec(sim_dt_nanos);
  auto roll = calcSetpoint(dt, gimbal_state_.roll, command_.roll,
                           command_.roll_rate);
  auto pitch = calcSetpoint(dt, gimbal_state_.pitch, command_.pitch,
                            command_.pitch_rate);
  auto yaw = calcSetpoint(dt, gimbal_state_.yaw, command_.yaw,
                          command_.yaw_rate);

  gimbal_state_.yaw_lock = command_.yaw_lock;
  gimbal_state_.roll_lock = command_.roll_lock;
  gimbal_state_.pitch_lock = command_.pitch_lock;

  if (dt > 0.0) {
    gimbal_state_.roll_rate = (roll - gimbal_state_.roll) / dt;
    gimbal_state_.pitch_rate = (pitch - gimbal_state_.pitch) / dt;
    gimbal_state_.yaw_rate = (yaw - gimbal_state_.yaw) / dt;
  } else {
    gimbal_state_.roll_rate = 0.0f;
    gimbal_state_.pitch_rate = 0.0f;
    gimbal_state_.yaw_rate = 0.0f;
  }

  if (roll == gimbal_state_.roll && pitch == gimbal_state_.pitch &&
      yaw == gimbal_state_.yaw) {
    return;
  }

  gimbal_state_.roll = roll;
  gimbal_state_.pitch = pitch;
  gimbal_state_.yaw = yaw;
  const auto output_quaternion = TransformUtils::ToQuaternion(
      gimbal_state_.roll, gimbal_state_.pitch, gimbal_state_.yaw);

  actuated_transforms_[child_link_] =
      ActuatedTransform(Affine3(output_quaternion.toRotationMatrix()));
}

void Gimbal::Impl::UpdateActuatorOutput(std::vector<float>&&,
                                        const TimeNano) {}

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

    std::lock_guard<std::mutex> guard(impl_.state_mutex_);
    impl_.gimbal_state_.roll = rpy.x();
    impl_.gimbal_state_.pitch = rpy.y();
    impl_.gimbal_state_.yaw = rpy.z();
  }
  impl_.logger_.LogVerbose(impl_.name_, "'gimbal origin settings' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
