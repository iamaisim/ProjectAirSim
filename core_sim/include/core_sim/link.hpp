// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_LINK_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_LINK_HPP_

#include <memory>
#include <string>

#include "core_sim/actor.hpp"
#include "core_sim/link/collision.hpp"
#include "core_sim/link/inertial.hpp"
#include "core_sim/link/visual.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;

class Link {
 public:
  Link();

  bool IsLoaded();

  const std::string& GetID() const;

  const Inertial& GetInertial() const;

  const Collision& GetCollision() const;

  const Visual& GetVisual() const;

  // TransformTree methods--the conversion operators allow this object to be
  // passed directly to TransformTree methods
  TransformTree::RefFrame& GetVisualRefFrame(void);
  const TransformTree::RefFrame& GetVisualRefFrame(void) const;
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

  // The rendering engine may only perform collision checking on root links
  // only.  For ground contact links like wheels, we can request explicit
  // checks for collisions with the ground.
  void EnableGroundCollisionDetection(bool enable);
  bool IsGroundCollisionDetectionEnabled(void) const;

 private:
  friend class Robot;

  friend class EnvActor;

  friend class EnvObject;

  Link(const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path);

  void Load(ConfigJson config_json);

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_LINK_HPP_
