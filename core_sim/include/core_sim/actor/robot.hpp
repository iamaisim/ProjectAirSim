// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ROBOT_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ROBOT_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core_sim/actor.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/wheel.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/environment.hpp"
#include "core_sim/joint.hpp"
#include "core_sim/link.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/int8_message.hpp"
#include "core_sim/message/int32_message.hpp"
#include "core_sim/message/pose_stamped_message.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "core_sim/service_method.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_tree.hpp"


namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;
class ServiceManager;
class StateManager;

class Robot : public Actor {
 public:
  typedef std::function<void(const Kinematics&, TimeNano)> KinematicsCallback;

  typedef std::function<void(const ActuatedTransforms&, TimeNano)>
      ActuatedTransformsCallback;

 public:
  Robot();

  //---------------------------------------------------------------------------
  // Static config/model
  //
  Link* GetLink(const std::string& id);
  const std::vector<Link>& GetLinks() const;

  const std::vector<Joint>& GetJoints() const;

  const std::vector<std::reference_wrapper<Sensor>>& GetSensors() const;

  int GetSensorIndex(const std::string& sensor_id) const;

  const std::vector<std::reference_wrapper<Actuator>>& GetActuators() const;

  const PhysicsType& GetPhysicsType() const;
  void SetPhysicsType(const PhysicsType& phys_type);
  const std::string& GetPhysicsConnectionSettings() const;
  void SetPhysicsConnectionSettings(const std::string& phys_conn_settings);
  const std::string& GetControlConnectionSettings() const;
  void SetControlConnectionSettings(const std::string& control_conn_settings);
  bool GetStartLanded() const;
  void SetStartLanded(bool start_landed);

  const std::string& GetControllerType() const;
  const std::string& GetControllerSettings() const;

  //---------------------------------------------------------------------------
  // Runtime
  //
  const Logger& GetLogger() const;

  void BeginUpdate();
  void EndUpdate();

  Topic CreateTopic(const std::string& name, TopicType type, int frequency,
                    MessageType message_type,
                    std::function<void(const Topic&, const Message&)> callback);

  void RemoveTopic(const Topic& topic);

  void RegisterServiceMethod(const ServiceMethod& method,
                             MethodHandler method_handler);

  void SetController(std::unique_ptr<IController> controller);

  void PublishRobotPose(const PoseStampedMessage& pose);

  void PublishRobotKinematics(const KinematicsMessage& kinematics);

  void PublishTopic(const Topic& topic, const Message& msg);

  void SetCallbackKinematicsUpdated(const KinematicsCallback& callback);

  void SetCallbackActuatorOutputUpdated(
      const ActuatedTransformsCallback& callback);

  const Kinematics& GetKinematics() const;
  const Environment& GetEnvironment() const;
  const CollisionInfo& GetCollisionInfo() const;
  const Vector3& GetExternalForce() const;

  // Manually sets actuated rotations on the robot links (such as spinning
  // propeller link meshes) that are not moved through the physics model
  void SetActuatedRotations(const ActuatedRotations& actuated_rots,
                            TimeNano external_time_stamp = -1);
  void SetActuatedTransforms(const ActuatedTransforms& actuated_transforms,
                             TimeNano external_time_stamp = -1);

  void InitializeSensors(const Kinematics&, const Environment&);

  void UpdateCollisionInfo(const CollisionInfo& collision_info);
  void SetHasCollided(bool has_collided);
  void UpdateControlInput();
  void UpdateActuators(const TimeNano sim_time, const TimeNano sim_dt_nanos);
  // TODO Rename to UpdateKinematics() to SetKinematics() since this is set by
  // external components like the physics model or API calls?
  void UpdateKinematics(const Kinematics& kinematics,
                        TimeNano external_time_stamp = -1);
  void UpdateEnvironment();
  void UpdateSensors(const TimeNano sim_time, const TimeNano sim_dt_nanos);

  std::vector<Wheel*> GetWheels() const;
  bool HasWheels() const;

  // TranformTree methods--the conversion operators allow this object to be
  // passed directly to TransformTree methods
  void RegisterChildTransformNodes(TransformTree& transformtree);
  TransformTree::RefFrame& GetHomeRefFrame(void);
  const TransformTree::RefFrame& GetHomeRefFrame(void) const;
  operator TransformTree::RefFrame&(
      void);  // Allows the Robot instance to be passed directly to
              // TreeTransform methods as the robot's current RefFrame
  operator const TransformTree::RefFrame&(void) const;

  // Note: No data should be stored in robot because it will be decoupled when
  // robot references are copied in external places like UE plugin or physics
  // lib. Instead, all robot data needs to be in robot::impl so any robot copy
  // will have the shared ptr to the base robot::impl (actor::impl) object.
 private:
  friend class Scene;

  Robot(const std::string& id, const Transform& origin, const Logger& logger,
        const TopicManager& topic_manager, const std::string& parent_topic_path,
        const ServiceManager& service_manager,
        const StateManager& state_manager);

  Robot(const std::string& id, const Transform& origin, const Logger& logger,
        const TopicManager& topic_manager, const std::string& parent_topic_path,
        const ServiceManager& service_manager,
        const StateManager& state_manager, HomeGeoPoint home_geo_point);

  void Load(ConfigJson config_json) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ACTOR_ROBOT_HPP_
