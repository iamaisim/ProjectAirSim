// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/actor/env_actor_grounded.hpp"

#include "env_actor_grounded_impl.hpp"

namespace microsoft {
namespace projectairsim {

EnvActorGrounded::EnvActorGrounded() : EnvActor() {}

EnvActorGrounded::EnvActorGrounded(const std::string& id, const Pose& origin,
                                   Logger& logger, TopicManager& topic_manager,
                                   const std::string& parent_topic_path,
                                   ServiceManager& service_manager,
                                   StateManager& state_manager)
    : EnvActor(std::shared_ptr<EnvActor::Impl>(new EnvActorGrounded::Impl(
          EnvActorType::kEnvHuman, id, origin, logger, topic_manager,
          parent_topic_path, service_manager, state_manager))) {}

EnvActorGrounded::EnvActorGrounded(std::shared_ptr<Impl> pimpl)
    : EnvActor(std::shared_ptr<EnvActor::Impl>(pimpl)) {}

void EnvActorGrounded::SetGroundLevel(float ground_level_) {
  std::static_pointer_cast<EnvActorGrounded::Impl>(pimpl_)->SetGroundLevel(
      ground_level_);
}

void EnvActorGrounded::SetPositionZOffset(float position_z_offset_) {
  std::static_pointer_cast<EnvActorGrounded::Impl>(pimpl_)->SetPositionZOffset(
      position_z_offset_);
}

void EnvActorGrounded::Impl::SetGroundLevel(float ground_level_) {
  ground_level = ground_level_;
}

void EnvActorGrounded::Impl::SetPositionZOffset(float position_z_offset_) {
  position_z_offset = position_z_offset_;
}

void EnvActorGrounded::Impl::UpdateKinematics(TimeSec curr_time) {
  // Update kinematics
  // delta time between function calls
  TimeSec time_sec = SimClock::Get()->NanosToSec(curr_time);
  dt = time_sec - last_update_time;
  last_update_time = time_sec;

  // Save last position
  prev_position.x() = traj_kinematics_.kinematics.pose.position.x();
  prev_position.y() = traj_kinematics_.kinematics.pose.position.y();
  // Save last yaw
  prev_yaw = traj_kinematics_.kinematics.pose.orientation.z();

  std::unique_lock<std::mutex> lock(kinematics_lock_);
  if (traj_ptr_) {
    // Set the position to the trajectory point
    traj_ptr_->KinematicsUpdate(time_sec, traj_kinematics_, time_at_load,
                                kinematics_at_load);
    traj_kinematics_.kinematics.pose.position.z() =
        prev_position.z();  // This avoid traj to change altitude
  }
  // If the envActor is over the ground
  if (traj_kinematics_.kinematics.pose.position.z() <
      ground_level - position_z_offset) {
    // Calculate gravity
    speed.z() += 9.8 * dt;
    // Update speed and position with gravity
    traj_kinematics_.kinematics.twist.linear.z() = speed.z();

    traj_kinematics_.kinematics.pose.position.z() +=
        traj_kinematics_.kinematics.twist.linear.z() * dt;
  } else {
    // If the envActor is underground, set the position to the ground level
    traj_kinematics_.kinematics.pose.position.z() =
        ground_level - position_z_offset;
    traj_kinematics_.kinematics.twist.linear.z() = 0;
    speed.z() = 0;
  }
  lock.unlock();

  // Save last position, the one affected with gravity
  prev_position.z() = traj_kinematics_.kinematics.pose.position.z();

  // Publish kinematics as a topic
  KinematicsMessage kine_msg(curr_time, GetKinematics());
  topic_manager_.PublishTopic(env_actor_kinematic_topic, kine_msg);
}

}  // namespace projectairsim
}  // namespace microsoft