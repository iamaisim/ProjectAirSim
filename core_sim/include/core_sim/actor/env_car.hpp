// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_CAR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_CAR_HPP_

#include "env_actor_grounded.hpp"

namespace microsoft {
namespace projectairsim {

class EnvCar : public EnvActorGrounded {
 public:
  typedef microsoft::projectairsim::Transform Pose;

  void SetWheelRadius(float wheel_radius);

  void SetWheelsDistance(float wheels_distance);

  void HasSkeletalMesh(bool is_skeletal_mesh);

  float GetSteeringAngleDeg() const;
  float GetWheelRotationRateDeg() const;

 protected:
  friend class Scene;

  class Impl;

  EnvCar(const std::string& id, const Pose& origin, Logger& logger,
         TopicManager& topic_manager, const std::string& parent_topic_path,
         ServiceManager& service_manager, StateManager& state_manager);

  EnvCar(std::shared_ptr<Impl> pimpl);
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_CAR_HPP_