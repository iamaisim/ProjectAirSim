#ifndef CORE_SIM_ACTOR_ENV_ACTOR_IMPL_HPP_
#define CORE_SIM_ACTOR_ENV_ACTOR_IMPL_HPP_

#include "actor_impl.hpp"
#include "core_sim/actor/env_actor.hpp"
#include "env_actor_loader.hpp"

namespace microsoft {
namespace projectairsim {

class EnvActor::Impl : public ActorImpl {
 public:
  typedef microsoft::projectairsim::Transform Pose;

  Impl(EnvActorType env_actor_type, const std::string& id, const Pose& origin,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  virtual void Load(ConfigJson config_json);

  void SetCallbackLinkRotAnglesUpdated(
      const std::function<
          void(const std::unordered_map<std::string, AngleAxis>&)>& callback);

  void SetCallbackLinkRotRatesUpdated(
      const std::function<
          void(const std::unordered_map<std::string, AngleAxis>&)>& callback);

  const std::vector<Link>& GetLinks();

  const std::vector<Joint>& GetJoints();

  std::shared_ptr<Trajectory> GetTrajectoryPtr() const;

  void SetTrajectory(const std::shared_ptr<Trajectory> traj_ptr,
                     const bool to_loop, const float time_offset,
                     const float x_offset, const float y_offset,
                     const float z_offset, const float roll_offset,
                     const float pitch_offset, const float yaw_offset);

  virtual bool SetLinkRotationAngle(const std::string& link_name,
                                    const float rotation_deg);

  virtual bool SetLinkRotationRate(const std::string& link_name,
                                   const float rotation_deg_per_sec);

  void UpdateLinkRotationAnglesCallback();

  void SetTrajectoryKinematics(const bool to_loop, const float time_offset,
                               const float x_offset, const float y_offset,
                               const float z_offset, const float roll_offset,
                               const float pitch_offset,
                               const float yaw_offset);

  void LoadKinematics();

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

  virtual void UpdateKinematics(TimeSec curr_time);

  void CreateTopics();

  void OnBeginUpdate() override;

  void OnEndUpdate() override;

  EnvActorType GetEnvActorType() const;

 protected:
  friend class EnvActor;

  EnvActorType env_actor_type_;
  std::vector<Link> links_;
  std::vector<Joint> joints_;

  EnvActor::Loader loader_;

  std::mutex kinematics_lock_;
  Topic env_actor_kinematic_topic;
  std::vector<std::reference_wrapper<Topic>> topics_;

  std::shared_ptr<Trajectory> traj_ptr_;
  bool auto_start_enabled_;  // not used currently

  TrajectoryKinematics traj_kinematics_;
  TimeSec time_at_load = 0.0f; // time when trajectory was loaded
  Kinematics kinematics_at_load; // kinematics when trajectory was loaded

  std::unordered_map<std::string, AngleAxis> link_rotation_angles_;
  std::unordered_map<std::string, AngleAxis> link_rotation_rates_;
  std::unordered_map<std::string, std::vector<std::string>> link_children_map_;

  std::function<void(const std::unordered_map<std::string, AngleAxis>&)>
      callback_link_rot_angles_updated_;

  std::function<void(const std::unordered_map<std::string, AngleAxis>&)>
      callback_link_rot_rates_updated_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_ACTOR_ENV_ACTOR_IMPL_HPP_
