// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_JOINT_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_JOINT_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core_sim/actor.hpp"
#include "core_sim/message/joint_state_message.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/topic.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_tree.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;

enum class JointType { kFixed = 0, kRevolute = 1, kContinuous = 2 };

class Joint {
 public:
  Joint();

  bool IsLoaded() const;

  const std::string& GetID() const;

  const JointType& GetJointType() const;

  const std::string& GetParentLink() const;

  const std::string& GetChildLink() const;

  const Transform& GetOrigin() const;

  const Vector3& GetAxis() const;

  float GetLimit() const;

  bool GetParentDominates() const;

  float GetSpringConstant() const;

  float GetDampingConstant() const;

  float GetMaxForce() const;

  void BeginUpdate();

  void PublishJointState(const JointStateMessage& state);

  void SetCallbackJointStateUpdated(
      const std::function<void(const JointStateMessage&)>& callback);

  void EndUpdate();

  // These conversion operators allow this object to be passed directly to
  // TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Robot;

  friend class EnvActor;

  Joint(const Logger& logger, const TopicManager& topic_manager,
        const std::string& parent_topic_path);

  void Load(ConfigJson config_json);

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_JOINT_HPP_
