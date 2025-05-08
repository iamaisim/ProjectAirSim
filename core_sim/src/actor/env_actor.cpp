// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actor/env_actor.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/kinematics_message.hpp"
#include "core_sim/runtime_components.hpp"
#include "env_actor_impl.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;
// -----------------------------------------------------------------------------
// class EnvActor

EnvActor::EnvActor() : Actor(nullptr) {}

EnvActor::EnvActor(const std::string& id, const Pose& origin,
                   const Logger& logger, const TopicManager& topic_manager,
                   const std::string& parent_topic_path,
                   const ServiceManager& service_manager,
                   const StateManager& state_manager)
    : Actor(std::shared_ptr<ActorImpl>(new EnvActor::Impl(
          EnvActorType::kEnvActor, id, origin, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

EnvActor::EnvActor(std::shared_ptr<Impl> pimpl)
    : Actor(std::shared_ptr<ActorImpl>(pimpl)) {}

void EnvActor::Load(ConfigJson config_json) {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->Load(config_json);
}

void EnvActor::SetCallbackLinkRotAnglesUpdated(
    const std::function<
        void(const std::unordered_map<std::string, AngleAxis>&)>& callback) {
  static_cast<EnvActor::Impl*>(pimpl_.get())
      ->SetCallbackLinkRotAnglesUpdated(callback);
}

void EnvActor::SetCallbackLinkRotRatesUpdated(
    const std::function<
        void(const std::unordered_map<std::string, AngleAxis>&)>& callback) {
  static_cast<EnvActor::Impl*>(pimpl_.get())
      ->SetCallbackLinkRotRatesUpdated(callback);
}

const std::vector<Link>& EnvActor::GetLinks() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetLinks();
}

const std::vector<Joint>& EnvActor::GetJoints() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetJoints();
}

std::shared_ptr<Trajectory> EnvActor::GetTrajectoryPtr() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetTrajectoryPtr();
}

void EnvActor::SetTrajectory(const std::shared_ptr<Trajectory> traj_ptr,
                             const bool to_loop, const float time_offset,
                             const float x_offset, const float y_offset,
                             const float z_offset, const float roll_offset,
                             const float pitch_offset, const float yaw_offset) {
  return static_cast<EnvActor::Impl*>(pimpl_.get())
      ->SetTrajectory(traj_ptr, to_loop, time_offset, x_offset, y_offset,
                      z_offset, roll_offset, pitch_offset, yaw_offset);
}

bool EnvActor::SetLinkRotationAngle(const std::string& link_name,
                                    const float rotation_deg) {
  return static_cast<EnvActor::Impl*>(pimpl_.get())
      ->SetLinkRotationAngle(link_name, rotation_deg);
}

bool EnvActor::SetLinkRotationRate(const std::string& link_name,
                                   const float rotation_deg_per_sec) {
  return static_cast<EnvActor::Impl*>(pimpl_.get())
      ->SetLinkRotationRate(link_name, rotation_deg_per_sec);
}

const Kinematics& EnvActor::GetKinematics() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetKinematics();
}

const Kinematics EnvActor::GetKinematicsThreadSafe() {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetKinematicsThreadSafe();
}

const std::unordered_map<std::string, AngleAxis>&
EnvActor::GetLinkRotationAngles() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetLinkRotationAngles();
}

const std::unordered_map<std::string, AngleAxis>&
EnvActor::GetLinkRotationRates() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetLinkRotationRates();
}

const std::unordered_map<std::string, std::vector<std::string>>&
EnvActor::GetLinkChildrenMap() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetLinkChildrenMap();
}

void EnvActor::UpdateLinkRotationAngles() {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->UpdateLinkRotationAngles();
}

void EnvActor::UpdateLinkRotationRates() {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->UpdateLinkRotationRates();
}

void EnvActor::UpdateKinematics(TimeNano curr_time) {
  return static_cast<EnvActor::Impl*>(pimpl_.get())
      ->UpdateKinematics(curr_time);
}

void EnvActor::BeginUpdate() {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->BeginUpdate();
}

void EnvActor::EndUpdate() {
  static_cast<EnvActor::Impl*>(pimpl_.get())->EndUpdate();
}

EnvActorType EnvActor::GetEnvActorType() const {
  return static_cast<EnvActor::Impl*>(pimpl_.get())->GetEnvActorType();
}
// -----------------------------------------------------------------------------
// class EnvActor::Impl

EnvActor::Impl::Impl(EnvActorType env_actor_type, const std::string& id,
                     const Pose& origin, const Logger& logger,
                     const TopicManager& topic_manager,
                     const std::string& parent_topic_path,
                     const ServiceManager& service_manager,
                     const StateManager& state_manager)
    : ActorImpl(ActorType::kEnvActor, id, origin,
                Constant::Component::env_actor, logger, topic_manager,
                parent_topic_path, service_manager, state_manager),
      loader_(*this),
      callback_link_rot_angles_updated_(nullptr),
      callback_link_rot_rates_updated_(nullptr) {
  traj_ptr_ = nullptr;
  env_actor_type_ = env_actor_type;
  SetTopicPath();
  CreateTopics();
}

void EnvActor::Impl::CreateTopics() {
  env_actor_kinematic_topic =
      Topic("actual_kinematics", topic_path_, TopicType::kPublished, 0,
            MessageType::kKinematics);

  topics_.push_back(env_actor_kinematic_topic);
}

void EnvActor::Impl::Load(ConfigJson config_json) {
  // Load EnvActor from JSON config
  json json = config_json;
  loader_.Load(json);

  // Initialize kinematics from loaded origin
  LoadKinematics();
}

void EnvActor::Impl::SetCallbackLinkRotAnglesUpdated(
    const std::function<
        void(const std::unordered_map<std::string, AngleAxis>&)>& callback) {
  std::lock_guard<std::mutex> lock(update_lock_);
  callback_link_rot_angles_updated_ = callback;
}

void EnvActor::Impl::SetCallbackLinkRotRatesUpdated(
    const std::function<
        void(const std::unordered_map<std::string, AngleAxis>&)>& callback) {
  std::lock_guard<std::mutex> lock(update_lock_);
  callback_link_rot_rates_updated_ = callback;
}

const std::vector<Link>& EnvActor::Impl::GetLinks() { return links_; }

const std::vector<Joint>& EnvActor::Impl::GetJoints() { return joints_; }

std::shared_ptr<Trajectory> EnvActor::Impl::GetTrajectoryPtr() const {
  return traj_ptr_;
}

void EnvActor::Impl::SetTrajectory(const std::shared_ptr<Trajectory> traj_ptr,
                                   const bool to_loop, const float time_offset,
                                   const float x_offset, const float y_offset,
                                   const float z_offset,
                                   const float roll_offset,
                                   const float pitch_offset,
                                   const float yaw_offset) {
  if (traj_ptr) {
    SetTrajectoryKinematics(to_loop, time_offset, x_offset, y_offset, z_offset,
                            roll_offset, pitch_offset, yaw_offset);
    traj_ptr_ = traj_ptr;
  }
}

bool EnvActor::Impl::SetLinkRotationAngle(const std::string& link_name,
                                          const float angle_deg) {
  if (link_rotation_angles_.find(link_name) == link_rotation_angles_.end()) {
    logger_.LogWarning(
        "SetLinkRotationAngle() called for "
        "invalid link: %s",
        link_name.c_str());
    return false;
  }
  link_rotation_angles_.at(link_name).angle() = MathUtils::deg2Rad(angle_deg);
  UpdateLinkRotationAngles();
  return true;
}

bool EnvActor::Impl::SetLinkRotationRate(const std::string& link_name,
                                         const float rotation_deg_per_sec) {
  if (link_rotation_rates_.find(link_name) == link_rotation_rates_.end()) {
    logger_.LogWarning(
        "SetLinkRotationRate() called for "
        "invalid link: %s",
        link_name.c_str());
    return false;
  }
  link_rotation_rates_.at(link_name).angle() =
      MathUtils::deg2Rad(rotation_deg_per_sec);
  UpdateLinkRotationRates();
  return true;
}

void EnvActor::Impl::SetTrajectoryKinematics(
    const bool to_loop, const float time_offset, const float x_offset,
    const float y_offset, const float z_offset, const float roll_offset,
    const float pitch_offset, const float yaw_offset) {
  traj_kinematics_.traj_params.time_offset = time_offset;
  traj_kinematics_.traj_params.x_offset = x_offset;
  traj_kinematics_.traj_params.y_offset = y_offset;
  traj_kinematics_.traj_params.z_offset = z_offset;
  traj_kinematics_.traj_params.roll_offset = roll_offset;
  traj_kinematics_.traj_params.pitch_offset = pitch_offset;
  traj_kinematics_.traj_params.yaw_offset = yaw_offset;
  traj_kinematics_.traj_params.to_loop = to_loop;
  traj_kinematics_.traj_params.num_loops = 0;
  LoadKinematics();
}

void EnvActor::Impl::LoadKinematics() {
  traj_kinematics_.kinematics.pose.position = origin_.translation_;
  traj_kinematics_.kinematics.pose.orientation = origin_.rotation_;
  time_at_load = SimClock::Get()->NowSimSec();
  kinematics_at_load = traj_kinematics_.kinematics;
}

const Kinematics& EnvActor::Impl::GetKinematics() const {
  return traj_kinematics_.kinematics;
}

const Kinematics EnvActor::Impl::GetKinematicsThreadSafe() {
  std::lock_guard<std::mutex> lock(kinematics_lock_);
  return traj_kinematics_.kinematics;
}

const std::unordered_map<std::string, AngleAxis>&
EnvActor::Impl::GetLinkRotationAngles() const {
  return link_rotation_angles_;
}

const std::unordered_map<std::string, AngleAxis>&
EnvActor::Impl::GetLinkRotationRates() const {
  return link_rotation_rates_;
}

const std::unordered_map<std::string, std::vector<std::string>>&
EnvActor::Impl::GetLinkChildrenMap() const {
  return link_children_map_;
}

void EnvActor::Impl::UpdateLinkRotationAngles() {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto func = callback_link_rot_angles_updated_;
  if (func != nullptr) {
    func(link_rotation_angles_);
  }
}

void EnvActor::Impl::UpdateLinkRotationRates() {
  std::lock_guard<std::mutex> lock(update_lock_);

  auto func = callback_link_rot_rates_updated_;
  if (func != nullptr) {
    func(link_rotation_rates_);
  }
}

void EnvActor::Impl::UpdateKinematics(TimeSec curr_time) {
  if (traj_ptr_) {
    std::unique_lock<std::mutex> lock(kinematics_lock_);
    traj_ptr_->KinematicsUpdate(curr_time, traj_kinematics_, time_at_load,
                                kinematics_at_load);
    lock.unlock();

    // Publish kinematics as a topic
    KinematicsMessage kine_msg(curr_time, GetKinematics());
    topic_manager_.PublishTopic(env_actor_kinematic_topic, kine_msg);
  }
}

EnvActorType EnvActor::Impl::GetEnvActorType() const { return env_actor_type_; }

void EnvActor::Impl::OnBeginUpdate() {
  // Register all topics to be ready for publish/subscribe calls
  for (const auto& topic_ref : topics_) {
    topic_manager_.RegisterTopic(topic_ref.get());
  }
}

void EnvActor::Impl::OnEndUpdate() {
  // Clear all callbacks to nullptr
  callback_link_rot_angles_updated_ = nullptr;
  callback_link_rot_rates_updated_ = nullptr;

  // Unregister all topics
  for (const auto& topic_ref : topics_) {
    topic_manager_.UnregisterTopic(topic_ref.get());
  }
}

// class EnvActor::Loader
EnvActor::Loader::Loader(EnvActor::Impl& impl) : impl_(impl) {}

void EnvActor::Loader::Load(const json& json) {
  LoadLinks(json);
  LoadJoints(json);
  LoadEnvActorScript(json);

  impl_.is_loaded_ = true;
}

void EnvActor::Loader::LoadLinks(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'links'.",
                           impl_.id_.c_str());

  auto links_json = JsonUtils::GetArray(json, Constant::Config::links);
  if (JsonUtils::IsEmptyArray(links_json)) {
    impl_.logger_.LogWarning(impl_.name_, "[%s] 'links' missing or empty.",
                             impl_.id_.c_str());
  }

  try {
    std::transform(links_json.begin(), links_json.end(),
                   std::back_inserter(impl_.links_),
                   [this](auto& json) { return LoadLink(json); });

  } catch (...) {
    impl_.links_.clear();
    throw;
  }
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'links' loaded.",
                           impl_.id_.c_str());
}

Link EnvActor::Loader::LoadLink(const json& json) {
  Link link(impl_.logger_, impl_.topic_manager_, impl_.topic_path_ + "/links");
  link.Load(json);

  if (json.contains(Constant::Config::axis)) {
    try {
      LoadLinkRotationSettings(json, link.GetID());
    } catch (...) {
      throw;
    }
  }
  return link;
}

void EnvActor::Loader::LoadLinkRotationSettings(const json& json,
                                                const std::string& link_name) {
  // Load parent link info to rotate links in lieu of transform tree
  std::string parent_link =
      JsonUtils::GetString(json, Constant::Config::parent_link, "");
  if (!parent_link.empty()) {
    impl_.link_children_map_[parent_link].emplace_back(link_name);
  } else {
    impl_.logger_.LogVerbose(impl_.name_,
                             "'parent_link' missing or empty. Using default.");
  }

  // Load axis of rotation
  auto vec3_axis = JsonUtils::GetVector3(json, Constant::Config::axis);
  vec3_axis.normalize();

  // Load initial angle or rotation frequency depending on the type of link
  if (json.contains(Constant::Config::initial_angle)) {
    AngleAxis angle_axis(
        MathUtils::deg2Rad(json.value(Constant::Config::initial_angle, 0.f)),
        vec3_axis);
    impl_.link_rotation_angles_.insert({link_name, angle_axis});

    if (json.contains(Constant::Config::revolutions_per_sec)) {
      impl_.logger_.LogVerbose(
          impl_.name_,
          "A link can contain either an 'initial-angle' field or a "
          "'revolutions-per-sec' field, not both. Only the initial angle has "
          "been processed.");
    }
  } else if (json.contains(Constant::Config::revolutions_per_sec)) {
    AngleAxis angle_axis(
        MathUtils::deg2Rad(
            json.value(Constant::Config::revolutions_per_sec, 0.f) * 360),
        vec3_axis);
    impl_.link_rotation_rates_.insert({link_name, angle_axis});
  }
}

void EnvActor::Loader::LoadJoints(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'joints'.",
                           impl_.id_.c_str());

  auto joints_json = JsonUtils::GetArray(json, Constant::Config::joints);
  if (JsonUtils::IsEmptyArray(joints_json)) {
    impl_.logger_.LogWarning(impl_.name_, "[%s] 'joints' missing or empty.",
                             impl_.id_.c_str());
  }

  try {
    std::transform(joints_json.begin(), joints_json.end(),
                   std::back_inserter(impl_.joints_),
                   [this](auto& json) { return LoadJoint(json); });
  } catch (...) {
    impl_.joints_.clear();
    throw;
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'joints' loaded.",
                           impl_.id_.c_str());
}

Joint EnvActor::Loader::LoadJoint(const json& json) {
  Joint joint(impl_.logger_, impl_.topic_manager_,
              impl_.topic_path_ + "/joints");
  joint.Load(json);
  return joint;
}

// Loads env_actor trajectory and bool params
void EnvActor::Loader::LoadEnvActorScript(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'script'.",
                           impl_.id_.c_str());

  auto script = JsonUtils::GetJsonObject(json, Constant::Config::script);

  if (!JsonUtils::IsEmpty(script)) {
    impl_.traj_ptr_ =
        std::shared_ptr<Trajectory>(new Trajectory(impl_.logger_));
    auto trajectory_json =
        JsonUtils::GetJsonObject(script, Constant::Config::trajectory);
    impl_.traj_ptr_->Load(trajectory_json);

    impl_.traj_kinematics_.traj_params.to_loop =
        JsonUtils::GetInteger(script, Constant::Config::loop, 0);

    impl_.auto_start_enabled_ =
        JsonUtils::GetInteger(script, Constant::Config::auto_start, 1);

    impl_.logger_.LogVerbose(impl_.name_, "[%s] 'script' loaded.",
                             impl_.id_.c_str());
  }
}

}  // namespace projectairsim
}  // namespace microsoft
