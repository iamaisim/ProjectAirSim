// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actor/env_car.hpp"

#include <vector>

#include "env_actor_grounded_impl.hpp"

namespace microsoft {
namespace projectairsim {

class EnvCar::Impl : public EnvActorGrounded::Impl {
 public:
  Impl(const std::string& id, const Pose& origin, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager, const StateManager& state_manager)
      : EnvActorGrounded::Impl(EnvActorType::kEnvCar, id, origin, logger,
                               topic_manager, parent_topic_path,
                               service_manager, state_manager) {}

  void UpdateKinematics(TimeSec curr_time) override;

  bool SetLinkRotationAngle(const std::string& link_name,
                            const float rotation_deg) override;

  bool SetLinkRotationRate(const std::string& link_name,
                           const float rotation_deg_per_sec) override;

  void SetWheelRadius(float wheel_radius);

  void SetWheelsDistance(float wheels_distance);

  void HasSkeletalMesh(bool is_skeletal_mesh);

  float GetSteeringAngleDeg() const;
  float GetWheelRotationRateDeg() const;
  float steering_angle_deg = 0;
  float wheel_rotation_rate_deg = 0;

 private:
  float wheel_radius = 0;
  float wheels_distance = 0;
  bool is_skeletal_mesh = false;

};

EnvCar::EnvCar(const std::string& id, const Pose& origin, Logger& logger,
               TopicManager& topic_manager,
               const std::string& parent_topic_path,
               ServiceManager& service_manager, StateManager& state_manager)
    : EnvActorGrounded(std::shared_ptr<EnvActorGrounded::Impl>(
          new EnvCar::Impl(id, origin, logger, topic_manager, parent_topic_path,
                           service_manager, state_manager))) {}

EnvCar::EnvCar(std::shared_ptr<Impl> pimpl) : EnvActorGrounded(pimpl) {}

void EnvCar::Impl::UpdateKinematics(TimeSec curr_time) {
  // Update kinematics with father's method
  EnvActorGrounded::Impl::UpdateKinematics(curr_time);

  // Get the speed of the car
  speed.x() =
      (traj_kinematics_.kinematics.pose.position.x() - prev_position.x()) / dt;
  speed.y() =
      (traj_kinematics_.kinematics.pose.position.y() - prev_position.y()) / dt;

  float speed_module = sqrt(speed.x() * speed.x() + speed.y() * speed.y());
  float speed_angle = atan2(speed.y(), speed.x());

  float yaw = traj_kinematics_.kinematics.pose.orientation.z();
  float speed_direction = 1.0;
  // If the angle is greater than pi, the car is going backwards
  if (abs(speed_angle - yaw * M_PI) > M_PI / 2) speed_direction = -1.0;

  float wheel_rotation_rate = speed_module / wheel_radius * speed_direction;
  wheel_rotation_rate_deg = wheel_rotation_rate * 180 / M_PI;

  float L = 2.4;  // Distance between the front and rear wheels
  if (is_skeletal_mesh) {
    L = wheels_distance;
  } else {
    for (auto link : links_) {
      if (link.GetID() == "RL_Wheel") {
        L = link.GetVisual().GetOrigin().translation_.x() * -2;
        break;
      }
    }
  }
  float yaw_dot = (yaw - prev_yaw) / dt;                       // Yaw rate
  float steering_angle = atan(yaw_dot * L) * speed_direction;  // Steering angle
  // Correct the angle to be into the range [0, 2*pi]
  while (steering_angle > 2 * M_PI) {
    steering_angle -= 2 * M_PI;
  }
  while (steering_angle < 0) {
    steering_angle += 2 * M_PI;
  }
  steering_angle_deg =
      steering_angle * 180 / M_PI;  // Steering angle in degrees
  if(!is_skeletal_mesh) {
    // Update the wheels rotation rate and angle
    SetLinkRotationAngle("FL_Normal", steering_angle_deg);
    SetLinkRotationAngle("FR_Normal", steering_angle_deg);
    SetLinkRotationRate("FL_Wheel", wheel_rotation_rate_deg);
    SetLinkRotationRate("RL_Wheel", wheel_rotation_rate_deg);
    SetLinkRotationRate("FR_Wheel", -wheel_rotation_rate_deg);
    SetLinkRotationRate("RR_Wheel", -wheel_rotation_rate_deg);
  }
  // Publish kinematics as a topic
  KinematicsMessage kine_msg(curr_time, GetKinematics());
  topic_manager_.PublishTopic(env_actor_kinematic_topic, kine_msg);
}

void EnvCar::SetWheelRadius(float wheel_radius) {
  std::static_pointer_cast<EnvCar::Impl>(pimpl_)->SetWheelRadius(wheel_radius);
}

void EnvCar::Impl::SetWheelRadius(float wheel_radius_) {
  wheel_radius = wheel_radius_;
}

void EnvCar::SetWheelsDistance(float wheels_distance) {
  std::static_pointer_cast<EnvCar::Impl>(pimpl_)->SetWheelsDistance(wheels_distance);
}

void EnvCar::Impl::SetWheelsDistance(float wheels_distance_) {
  wheels_distance = wheels_distance_;
}

void EnvCar::HasSkeletalMesh(bool is_skeletal_mesh) {
  std::static_pointer_cast<EnvCar::Impl>(pimpl_)->HasSkeletalMesh(is_skeletal_mesh);
}

void EnvCar::Impl::HasSkeletalMesh(bool is_skeletal_mesh_) {
  is_skeletal_mesh = is_skeletal_mesh_;
}

float EnvCar::GetSteeringAngleDeg() const {
  return std::static_pointer_cast<EnvCar::Impl>(pimpl_)->steering_angle_deg;
}

float EnvCar::Impl::GetSteeringAngleDeg() const {
  return steering_angle_deg;
}

float EnvCar::GetWheelRotationRateDeg() const {
  return std::static_pointer_cast<EnvCar::Impl>(pimpl_)->wheel_rotation_rate_deg;
}
float EnvCar::Impl::GetWheelRotationRateDeg() const {
  return wheel_rotation_rate_deg;
}


bool EnvCar::Impl::SetLinkRotationAngle(const std::string& link_name,
                                        const float rotation_deg) {
  bool found = false;
  for (auto const& link : links_) {
    if (link.GetID() == link_name) {
      found = true;
      break;
    }
  }

  // if the link is not found, log a warning and return false
  if (!found) {
    logger_.LogWarning(
        "SetLinkRotationAngle() called for "
        "invalid link: %s",
        link_name.c_str());
    return false;
  }
  float rotation_rad = MathUtils::deg2Rad(rotation_deg);
  // if the link exists but not in the map, add it
  if (link_rotation_angles_.find(link_name) == link_rotation_angles_.end()) {
    // lock the map while adding the new entry with mutex
    std::lock_guard<std::mutex> lock(update_lock_);
    link_rotation_angles_.insert(
        {link_name, AngleAxis(rotation_rad, Vector3::UnitZ())});
  }

  // if the link exists in the map, update the angle
  else {
    // lock the map while updating the entry with mutex
    std::lock_guard<std::mutex> lock(update_lock_);
    link_rotation_angles_.at(link_name).angle() = rotation_rad;
  }

  UpdateLinkRotationAngles();
  return true;
}

bool EnvCar::Impl::SetLinkRotationRate(const std::string& link_name,
                                       const float rotation_deg_per_sec) {
  bool found = false;
  for (auto const& link : links_) {
    if (link.GetID() == link_name) {
      found = true;
      break;
    }
  }

  // if the link is not found, log a warning and return false
  if (!found) {
    logger_.LogWarning(
        "SetLinkRotationRate() called for "
        "invalid link: %s",
        link_name.c_str());
    return false;
  }

  // if the link exists but not in the map, add it
  if (link_rotation_rates_.find(link_name) == link_rotation_rates_.end()) {
    // lock the map while adding the new entry with mutex
    std::lock_guard<std::mutex> lock(update_lock_);
    AngleAxis angle_axis(MathUtils::deg2Rad(rotation_deg_per_sec),
                         Vector3::UnitY());
    link_rotation_rates_.insert({link_name, angle_axis});
  }

  // if the link exists in the map, update the angle
  else {
    // lock the map while updating the entry with mutex
    std::lock_guard<std::mutex> lock(update_lock_);
    link_rotation_rates_.at(link_name).angle() =
        MathUtils::deg2Rad(rotation_deg_per_sec);
  }

  UpdateLinkRotationRates();
  return true;
}

}  // namespace projectairsim
}  // namespace microsoft
