// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/link.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class Link::Loader {
 public:
  explicit Loader(Link::Impl& impl);

  void Load(const json& json);

 private:
  void LoadID(const json& json);

  void LoadInertial(const json& json);

  void LoadCollision(const json& json);

  void LoadVisual(const json& json);

  Link::Impl& impl_;
};

class Link::Impl : public ComponentWithTopics {
 public:
  Impl(const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path);

  void Load(ConfigJson config_json);

  const Inertial& GetInertial() const;

  const Collision& GetCollision() const;

  const Visual& GetVisual() const;

  const TransformTree::RefFrame& GetVisualRefFrame(void) const;
  operator const TransformTree::RefFrame&(void) const;

  void EnableGroundCollisionDetection(bool enable) {
    is_ground_collision_detection_enabled_ = enable;
  }
  bool IsGroundCollisionDetectionEnabled(void) const {
    return is_ground_collision_detection_enabled_;
  }

 private:
  friend class Link::Loader;

  Collision collision_settings_;
  bool is_ground_collision_detection_enabled_;  // If true, the rendering
                                                 // engine should  perform
                                                 // ground collision checks for
                                                 // this link
  Inertial inertial_settings_;
  Link::Loader loader_;
  Visual visual_settings_;
};

// -----------------------------------------------------------------------------
// class Link

Link::Link() : pimpl_(std::shared_ptr<Link::Impl>(nullptr)) {}

Link::Link(const Logger& logger, const TopicManager& topic_manager,
           const std::string& parent_topic_path)
    : pimpl_(std::shared_ptr<Link::Impl>(
          new Link::Impl(logger, topic_manager, parent_topic_path))) {}

void Link::Load(ConfigJson config_json) { return pimpl_->Load(config_json); }

bool Link::IsLoaded() { return pimpl_->IsLoaded(); }

const std::string& Link::GetID() const { return pimpl_->GetID(); }

const Inertial& Link::GetInertial() const { return pimpl_->GetInertial(); }

const Collision& Link::GetCollision() const { return pimpl_->GetCollision(); }

const Visual& Link::GetVisual() const { return pimpl_->GetVisual(); }

TransformTree::RefFrame& Link::GetVisualRefFrame(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(pimpl_->GetVisualRefFrame()));
}

const TransformTree::RefFrame& Link::GetVisualRefFrame(void) const {
  return (pimpl_->GetVisualRefFrame());
}

Link::operator TransformTree::RefFrame&(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return (const_cast<TransformTree::RefFrame&>(
      pimpl_->
      operator const microsoft::projectairsim::TransformTree::RefFrame&()));
}

Link::operator const TransformTree::RefFrame&(void) const {
  return (pimpl_->
          operator const microsoft::projectairsim::TransformTree::RefFrame&());
}

void Link::EnableGroundCollisionDetection(bool enable) {
  pimpl_->EnableGroundCollisionDetection(enable);
}

bool Link::IsGroundCollisionDetectionEnabled(void) const {
  return (pimpl_->IsGroundCollisionDetectionEnabled());
}

// -----------------------------------------------------------------------------
// class Link::Impl

Link::Impl::Impl(const Logger& logger, const TopicManager& topic_manager,
                 const std::string& parent_topic_path)
    : ComponentWithTopics(Constant::Component::link, logger, topic_manager,
                          parent_topic_path),
      collision_settings_(logger),
      is_ground_collision_detection_enabled_(false),
      inertial_settings_(logger),
      loader_(*this),
      visual_settings_(logger) {}

void Link::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
  static_cast<TransformTree::RefFrame&>(inertial_settings_)
      .SetID(std::string("L ") + GetID() + " inertial");
  static_cast<TransformTree::RefFrame&>(visual_settings_)
      .SetID(std::string("L ") + GetID() + " visual");
}

const Inertial& Link::Impl::GetInertial() const { return inertial_settings_; }

const Collision& Link::Impl::GetCollision() const {
  return collision_settings_;
}

const Visual& Link::Impl::GetVisual() const { return visual_settings_; }

const TransformTree::RefFrame& Link::Impl::GetVisualRefFrame(void) const {
  return (visual_settings_);
}

Link::Impl::operator const TransformTree::RefFrame&(void) const {
  return (inertial_settings_);
}

// -----------------------------------------------------------------------------
// class Link::Loader

Link::Loader::Loader(Link::Impl& impl) : impl_(impl) {}

void Link::Loader::Load(const json& json) {
  LoadID(json);
  LoadInertial(json);
  LoadCollision(json);
  LoadVisual(json);

  impl_.is_loaded_ = true;
}

void Link::Loader::LoadID(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'id'.");

  impl_.id_ = JsonUtils::GetIdentifier(json, Constant::Config::name);
  impl_.SetTopicPath();

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'id' loaded.", impl_.id_.c_str());
}

void Link::Loader::LoadInertial(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'inertial'.",
                           impl_.id_.c_str());

  auto inertial_json =
      JsonUtils::GetJsonObject(json, Constant::Config::inertial);
  impl_.inertial_settings_.Load(inertial_json);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'inertial' loaded.",
                           impl_.id_.c_str());
}

void Link::Loader::LoadCollision(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'collision'.",
                           impl_.id_.c_str());

  auto collision_json =
      JsonUtils::GetJsonObject(json, Constant::Config::collision);
  impl_.collision_settings_.Load(collision_json);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'collision' loaded.",
                           impl_.id_.c_str());
}

void Link::Loader::LoadVisual(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s]Loading 'visual'.",
                           impl_.id_.c_str());

  auto visual_json = JsonUtils::GetJsonObject(json, Constant::Config::visual);
  impl_.visual_settings_.Load(visual_json);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'visual' loaded.",
                           impl_.id_.c_str());
}

}  // namespace projectairsim
}  // namespace microsoft
