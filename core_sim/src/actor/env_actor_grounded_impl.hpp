// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/actor/env_actor_grounded.hpp"
#include "env_actor_impl.hpp"

namespace microsoft {
namespace projectairsim {

class EnvActorGrounded::Impl : public EnvActor::Impl {
 public:
  Impl(const EnvActorType env_actor_type, const std::string& id,
       const Pose& origin, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager, const StateManager& state_manager)
      : EnvActor::Impl(env_actor_type, id, origin, logger, topic_manager,
                       parent_topic_path, service_manager, state_manager) {}

  void UpdateKinematics(TimeSec curr_time) override;

  void SetGroundLevel(float ground_level);

  void SetPositionZOffset(float position_z_offset_);

 protected:
  TimeSec last_update_time = 0;
  double dt;
  Eigen::Vector3f prev_position;
  float prev_yaw;
  Eigen::Vector3f speed = Eigen::Vector3f::Zero();
  float ground_level = 0;
  float position_z_offset = 0;
};

}  // namespace projectairsim
}  // namespace microsoft