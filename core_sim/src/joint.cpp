// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/joint.hpp"

#include <memory>

#include "component.hpp"
#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class Joint::Loader {
 public:
  Loader(Joint::Impl& impl);

  void Load(const json& json);

 private:
  void LoadID(const json& json);

  void LoadJointType(const json& json);

  void LoadParentLink(const json& json);

  void LoadChildLink(const json& json);

  void LoadOrigin(const json& json);

  void LoadAxis(const json& json);

  void LoadLimit(const json& json);

  void LoadParentDominates(const json& json);

  void LoadSpringConstant(const json& json);

  void LoadDampingConstant(const json& json);

  void LoadMaxForce(const json& json);

  Joint::Impl& impl_;
};

class Joint::Impl : public ComponentWithTopics {
 public:
  Impl(const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path);

  void Load(ConfigJson config_json);

  void CreateTopics();

  const JointType& GetJointType();

  const std::string& GetParentLink();

  const std::string& GetChildLink();

  const Transform& GetOrigin();

  const Vector3& GetAxis();

  float GetLimit();

  bool GetParentDominates();

  float GetSpringConstant();

  float GetDampingConstant();

  float GetMaxForce();

  void OnBeginUpdate() override;

  void PublishJointState(const JointStateMessage& state);

  void SetCallbackJointStateUpdated(
      const std::function<void(const JointStateMessage&)>& callback);

  void OnEndUpdate() override;

  void OnDesiredJointStateMessage(const Topic& topic, const Message& message);

  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Joint::Loader;

  Joint::Loader loader_;
  JointType type_;
  std::string parent_link_;
  std::string child_link_;
  Transform origin_;
  Vector3 axis_;
  float limit_;
  bool parent_dominates_;
  float spring_constant_;
  float damping_constant_;
  float max_force_;
  std::function<void(const JointStateMessage&)> joint_state_updated_callback_;
  Topic actual_state_topic_;
  Topic desired_state_topic_;
  std::vector<Topic> topics_;
  TransformTree::TransformRefFrame
      transformrefframe_;  // Inertial reference frame's transform tree node
};

// -----------------------------------------------------------------------------
// class Joint

Joint::Joint() : pimpl_(std::shared_ptr<Joint::Impl>(nullptr)) {}

Joint::Joint(const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path)
    : pimpl_(std::shared_ptr<Joint::Impl>(
          new Joint::Impl(logger, topic_manager, parent_topic_path))) {}

void Joint::Load(ConfigJson config_json) { return pimpl_->Load(config_json); }

bool Joint::IsLoaded() const { return pimpl_->IsLoaded(); }

const std::string& Joint::GetID() const { return pimpl_->GetID(); }

const JointType& Joint::GetJointType() const { return pimpl_->GetJointType(); }

const std::string& Joint::GetParentLink() const {
  return pimpl_->GetParentLink();
}

const std::string& Joint::GetChildLink() const {
  return pimpl_->GetChildLink();
}

const Transform& Joint::GetOrigin() const { return pimpl_->GetOrigin(); }

const Vector3& Joint::GetAxis() const { return pimpl_->GetAxis(); }

float Joint::GetLimit() const { return pimpl_->GetLimit(); }

bool Joint::GetParentDominates() const { return pimpl_->GetParentDominates(); }

float Joint::GetSpringConstant() const { return pimpl_->GetSpringConstant(); }

float Joint::GetDampingConstant() const { return pimpl_->GetDampingConstant(); }

float Joint::GetMaxForce() const { return pimpl_->GetMaxForce(); }

void Joint::BeginUpdate() { pimpl_->BeginUpdate(); }

void Joint::PublishJointState(const JointStateMessage& state) {
  pimpl_->PublishJointState(state);
}

void Joint::SetCallbackJointStateUpdated(
    const std::function<void(const JointStateMessage&)>& callback) {
  pimpl_->SetCallbackJointStateUpdated(callback);
}

void Joint::EndUpdate() { pimpl_->EndUpdate(); }

Joint::operator TransformTree::RefFrame&(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(
      pimpl_->operator const TransformTree::RefFrame&()));
}

Joint::operator const TransformTree::RefFrame&(void) const {
  return (pimpl_->operator const TransformTree::RefFrame&());
}

// -----------------------------------------------------------------------------
// class Joint::Impl

Joint::Impl::Impl(const Logger& logger, const TopicManager& topic_manager,
                  const std::string& parent_topic_path)
    : ComponentWithTopics(Constant::Component::joint, logger, topic_manager,
                          parent_topic_path),
      loader_(*this),
      spring_constant_(0.0f),
      damping_constant_(0.0f),
      max_force_(0.0f),
      joint_state_updated_callback_(nullptr),
      transformrefframe_("Joint", &origin_) {}

void Joint::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
  transformrefframe_.SetID(std::string("J ") + GetID());
}

void Joint::Impl::CreateTopics() {
  actual_state_topic_ = Topic("actual", topic_path_, TopicType::kPublished, 60,
                              MessageType::kJointState);
  desired_state_topic_ = Topic("desired", topic_path_, TopicType::kSubscribed,
                               60, MessageType::kJointState);

  topics_.push_back(actual_state_topic_);
  topics_.push_back(desired_state_topic_);
}

const JointType& Joint::Impl::GetJointType() { return type_; }

const std::string& Joint::Impl::GetParentLink() { return parent_link_; }

const std::string& Joint::Impl::GetChildLink() { return child_link_; }

const Transform& Joint::Impl::GetOrigin() { return origin_; }

const Vector3& Joint::Impl::GetAxis() { return axis_; }

float Joint::Impl::GetLimit() { return limit_; }

bool Joint::Impl::GetParentDominates() { return parent_dominates_; }

float Joint::Impl::GetSpringConstant() { return spring_constant_; }

float Joint::Impl::GetDampingConstant() { return damping_constant_; }

float Joint::Impl::GetMaxForce() { return max_force_; }

void Joint::Impl::OnBeginUpdate() {
  if (type_ == JointType::kFixed) {
    return;
  }

  topic_manager_.RegisterTopic(actual_state_topic_);
  topic_manager_.RegisterTopic(desired_state_topic_);

  topic_manager_.SubscribeTopic(
      desired_state_topic_, [this](const Topic& topic, const Message& message) {
        OnDesiredJointStateMessage(topic, message);
      });
}

void Joint::Impl::PublishJointState(const JointStateMessage& state) {
  if (type_ == JointType::kFixed) {
    return;
  }

  std::lock_guard<std::mutex> lock(update_lock_);

  topic_manager_.PublishTopic(actual_state_topic_, state);
}

void Joint::Impl::SetCallbackJointStateUpdated(
    const std::function<void(const JointStateMessage&)>& callback) {
  if (type_ == JointType::kFixed) {
    return;
  }

  std::lock_guard<std::mutex> lock(update_lock_);
  joint_state_updated_callback_ = callback;
}

void Joint::Impl::OnEndUpdate() {
  if (type_ == JointType::kFixed) {
    return;
  }

  joint_state_updated_callback_ = nullptr;

  topic_manager_.UnregisterTopic(actual_state_topic_);
  topic_manager_.UnregisterTopic(desired_state_topic_);
}

void Joint::Impl::OnDesiredJointStateMessage(const Topic& topic,
                                             const Message& message) {
  std::function<void(const JointStateMessage&)> callback = nullptr;
  std::lock_guard<std::mutex> lock(update_lock_);
  callback = joint_state_updated_callback_;

  if (callback != nullptr) {
    callback(static_cast<const JointStateMessage&>(message));
  }
}

Joint::Impl::operator const TransformTree::RefFrame&(void) const {
  return (transformrefframe_);
}

// -----------------------------------------------------------------------------
// class joint::loader

Joint::Loader::Loader(Joint::Impl& impl) : impl_(impl) {}

void Joint::Loader::Load(const json& json) {
  LoadID(json);
  LoadJointType(json);
  LoadParentLink(json);
  LoadChildLink(json);
  LoadOrigin(json);
  LoadAxis(json);
  LoadLimit(json);
  LoadParentDominates(json);
  LoadSpringConstant(json);
  LoadDampingConstant(json);
  LoadMaxForce(json);
  impl_.CreateTopics();

  impl_.is_loaded_ = true;
}

void Joint::Loader::LoadID(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'id'.");

  impl_.id_ = JsonUtils::GetIdentifier(json, Constant::Config::id);
  impl_.SetTopicPath();

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'id' loaded.", impl_.id_.c_str());
}

void Joint::Loader::LoadJointType(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'type'.",
                           impl_.id_.c_str());

  auto type = JsonUtils::GetIdentifier(json, Constant::Config::type);

  if (type == Constant::Config::fixed) {
    impl_.type_ = JointType::kFixed;
  } else if (type == Constant::Config::revolute) {
    impl_.type_ = JointType::kRevolute;
  } else if (type == Constant::Config::continuous) {
    impl_.type_ = JointType::kContinuous;
  } else {
    impl_.logger_.LogError(impl_.name_, "Invalid joint type '%s'.",
                           type.c_str());
    throw Error("Invalid joint type.");
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'type' loaded.",
                           impl_.id_.c_str());
}

void Joint::Loader::LoadParentLink(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'parent-link'.",
                           impl_.id_.c_str());

  impl_.parent_link_ =
      JsonUtils::GetIdentifier(json, Constant::Config::parent_link);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'parent-link' loaded.",
                           impl_.id_.c_str());
}

void Joint::Loader::LoadChildLink(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'child-link'.",
                           impl_.id_.c_str());

  impl_.child_link_ =
      JsonUtils::GetIdentifier(json, Constant::Config::child_link);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'child-link' loaded.",
                           impl_.id_.c_str());
}

void Joint::Loader::LoadOrigin(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'origin'.");

  impl_.origin_ = JsonUtils::GetTransform(json, Constant::Config::origin);

  impl_.logger_.LogVerbose(impl_.name_, "'origin' loaded.");
}

void Joint::Loader::LoadAxis(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'axis'.");

  auto axis = JsonUtils::GetVector3(json, Constant::Config::axis);
  if (axis != Vector3::UnitX() && axis != Vector3::UnitY() &&
      axis != Vector3::UnitZ()) {
    impl_.logger_.LogError(impl_.name_, "Invalid axis '[%f %f %f]'.", axis.x(),
                           axis.y(), axis.z());
    throw Error("Invalid joint axis type.");
  }
  impl_.axis_ = axis;

  impl_.logger_.LogVerbose(impl_.name_, "'axis' loaded.");
}

void Joint::Loader::LoadLimit(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'limit'.");

  impl_.limit_ = JsonUtils::GetNumber<float>(json, Constant::Config::limit);

  impl_.logger_.LogVerbose(impl_.name_, "'limit' loaded.");
}

void Joint::Loader::LoadParentDominates(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'parent-dominates'.");

  impl_.parent_dominates_ =
      JsonUtils::GetInteger(json, Constant::Config::parent_dominates, 0);

  impl_.logger_.LogVerbose(impl_.name_, "'parent-dominates' loaded.");
}

void Joint::Loader::LoadSpringConstant(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'spring-constant'.");

  impl_.spring_constant_ =
      JsonUtils::GetNumber<float>(json, Constant::Config::spring_constant, 0);

  impl_.logger_.LogVerbose(impl_.name_, "'spring-constant' loaded.");
}

void Joint::Loader::LoadDampingConstant(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'damping-constant'.");

  impl_.damping_constant_ =
      JsonUtils::GetNumber<float>(json, Constant::Config::damping_constant, 0);

  impl_.logger_.LogVerbose(impl_.name_, "'damping-constant' loaded.");
}

void Joint::Loader::LoadMaxForce(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'max-force'.");

  impl_.max_force_ =
      JsonUtils::GetNumber<float>(json, Constant::Config::max_force, 0);

  impl_.logger_.LogVerbose(impl_.name_, "'max-force' loaded.");
}

}  // namespace projectairsim
}  // namespace microsoft
