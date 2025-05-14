// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_ACTOR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_ACTOR_HPP_

#include <memory>
#include <unordered_map>
#include <vector>

#include "core_sim/actor.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/environment.hpp"
#include "core_sim/joint.hpp"
#include "core_sim/link.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/trajectory.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;
class ServiceManager;
class StateManager;

enum EnvActorType { kEnvActor = 0, kEnvCar = 1, kEnvHuman = 2 };
class EnvActor : public Actor {
 public:
  EnvActor();

  typedef microsoft::projectairsim::Transform Pose;

  //---------------------------------------------------------------------------
  // Static config/model
  //
  const std::vector<Link>& GetLinks() const;

  const std::vector<Joint>& GetJoints() const;

  //---------------------------------------------------------------------------
  // Runtime
  //

  void SetCallbackLinkRotAnglesUpdated(
      const std::function<
          void(const std::unordered_map<std::string, AngleAxis>&)>& callback);

  void SetCallbackLinkRotRatesUpdated(
      const std::function<
          void(const std::unordered_map<std::string, AngleAxis>&)>& callback);

  void SetTrajectory(const std::shared_ptr<Trajectory> traj_ptr,
                     const bool to_loop = false, const float time_offset = 0,
                     const float x_offset = 0, const float y_offset = 0,
                     const float z_offset = 0, const float roll_offset = 0,
                     const float pitch_offset = 0, const float yaw_offset = 0);

  bool SetLinkRotationAngle(const std::string& link_name,
                            const float rotation_deg);

  bool SetLinkRotationRate(const std::string& link_name,
                           const float rotation_deg_per_sec);

  const Kinematics& GetKinematics() const;

  const Kinematics GetKinematicsThreadSafe();

  const std::unordered_map<std::string, AngleAxis>& GetLinkRotationAngles()
      const;

  const std::unordered_map<std::string, AngleAxis>& GetLinkRotationRates()
      const;

  const std::unordered_map<std::string, std::vector<std::string>>&
  GetLinkChildrenMap() const;

  void UpdateLinkRotationAngles();

  void UpdateLinkRotationRates();

  virtual void UpdateKinematics(TimeNano curr_time);

  void BeginUpdate();
  void EndUpdate();

  EnvActorType GetEnvActorType() const;

 protected:
  friend class Scene;
  class Impl;
  class Loader;

  EnvActor(const std::string& id, const Pose& origin, const Logger& logger,
           const TopicManager& topic_manager,
           const std::string& parent_topic_path,
           const ServiceManager& service_manager,
           const StateManager& state_manager);
  EnvActor(std::shared_ptr<Impl> pimpl);
  std::shared_ptr<Trajectory> GetTrajectoryPtr() const;

  void Load(ConfigJson config_json) override;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ENV_ACTOR_HPP_