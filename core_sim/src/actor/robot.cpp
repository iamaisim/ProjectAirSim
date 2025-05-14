// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/actor/robot.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "actuators/actuator_impl.hpp"
#include "algorithms.hpp"
#include "constant.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/collision_info_message.hpp"
#include "core_sim/message/kinematics_message.hpp"
#include "core_sim/message/rotor_message.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "json.hpp"
#include "sensors/sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class Robot::Loader {
 public:
  explicit Loader(Robot::Impl& impl);

  void Load(const json& json);

 private:
  void LoadLinks(const json& json);

  Link LoadLink(const json& json);

  void LoadJoints(const json& json);

  Joint LoadJoint(const json& json);

  void LoadSensors(const json& json);

  void LoadActuators(const json& json);

  void LoadPhysicsType(const json& json);

  void LoadController(const json& json);

  Robot::Impl& impl_;
};

class Robot::Impl : public ActorImpl {
 public:
  Impl(const std::string& id, const Transform& origin, const Logger& logger,
       const TopicManager& topic_manager, const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  const std::vector<std::reference_wrapper<Actuator>>& GetActuators();

  const std::vector<Joint>& GetJoints();

  Link* GetLink(const std::string& id);

  const std::vector<Link>& GetLinks();

  const Logger& GetLogger() const;

  const std::vector<std::reference_wrapper<Sensor>>& GetSensors();

  int GetSensorIndex(const std::string& sensor_id) const;

  void InitializeSensors(const Kinematics& kinematics,
                         const Environment& environment);

  void CreateTopics();

  Topic CreateTopic(const std::string& name, TopicType type, int frequency,
                    MessageType message_type,
                    std::function<void(const Topic&, const Message&)> callback);

  void RemoveTopic(const Topic& topic);

  void RegisterServiceMethod(const ServiceMethod& method,
                             MethodHandler method_handler);

  void RegisterServiceMethods();

  KinematicsMessage GetGroundTruthKinematics();

  bool SetGroundTruthKinematics(const KinematicsMessage& kinematics);

  void SetController(std::unique_ptr<IController> controller);

  void OnBeginUpdate() override;

  void PublishRobotPose(const PoseStampedMessage& pose);

  void PublishRobotKinematics(const KinematicsMessage& kinematics);

  void PublishTopic(const Topic& topic, const Message& msg);

  void SetCallbackKinematicsUpdated(const KinematicsCallback& callback);

  void SetCallbackActuatorOutputUpdated(
      const ActuatedTransformsCallback& callback);

  void OnDesiredRobotPoseStampedMessage(const Topic& topic,
                                        const Message& message);

  bool SetPose(const Transform& new_pose, bool reset_kinematics);

  void OnEndUpdate() override;

  const Kinematics& GetKinematics() const;
  const Transform GetGroundTruthPose();
  const GeoPoint GetGroundTruthGeoLocation();
  const Environment& GetEnvironment() const;
  const CollisionInfo& GetCollisionInfo() const;
  const Vector3& GetExternalForce() const;

  void SetActuatedRotations(const ActuatedRotations& actuated_rots,
                            TimeNano external_time_stamp = -1);

  void SetActuatedTransforms(const ActuatedTransforms& actuated_transforms,
                             TimeNano external_time_stamp = -1);

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

  void UpdateCollisionInfo(const CollisionInfo& collision_info);
  void SetHasCollided(bool has_collided);
  void UpdateControlInput();
  void UpdateActuators(const TimeNano sim_time, const TimeNano sim_dt_nanos);
  void UpdateKinematics(const Kinematics& kinematics,
                        TimeNano external_time_stamp = -1);
  void UpdateEnvironment();
  void UpdateSensors(const TimeNano sim_time, const TimeNano sim_dt_nanos);
  void UpdateCameraPose(Camera* camera);
  void SetHomeGeoPoint(const HomeGeoPoint& home_geo_point);

  std::vector<Wheel*> GetWheels() const;
  bool HasWheels() const;

  bool SetExternalForce(const std::vector<float>& ext_force);

  Pose GetCameraRay(const std::string& camera_id, int image_type, int x, int y);

  void RegisterChildTransformNodes(TransformTree& transformtree);
  const TransformTree::RefFrame& GetHomeRefFrame(void) const;
  operator const TransformTree::RefFrame&(void) const;

 private:
  friend class Robot::Loader;

  Robot::Loader loader_;
  std::vector<Link> links_;
  std::vector<Joint> joints_;

  Kinematics kinematics_;
  Environment environment_;
  CollisionInfo collision_info_;
  Vector3 ext_force_ = Vector3::Zero();
  ActuatedTransforms actuated_transforms_;
  TimeNano last_actuated_rotation_simtime_;
  TimeNano kinematics_updated_timestamp_;

  PhysicsType physics_type_;
  std::string physics_connection_settings_;
  std::string control_connection_settings_;
  bool start_landed_;

  std::string controller_type_;
  std::string controller_settings_;

  std::vector<std::unique_ptr<Sensor>> sensors_;
  std::vector<std::reference_wrapper<Sensor>> sensors_ref_;
  std::vector<std::tuple<Tilt*, Rotor*>>
      tilt_targets_;  // List of tilt actuators and their targets
  std::unordered_map<std::string, int> sensor_index_by_id_;

  float total_power_ = 0.0f;

  std::unique_ptr<IController> controller_;

  std::vector<std::unique_ptr<Actuator>> actuators_;
  std::vector<std::reference_wrapper<Actuator>> actuators_ref_;

  Topic actual_robot_pose_topic_;
  Topic desired_robot_pose_topic_;
  Topic collision_info_topic_;
  Topic rotor_info_topic_;
  Topic gt_kinematics_topic_;
  std::vector<std::reference_wrapper<Topic>> topics_;

  KinematicsCallback callback_kinematics_updated_;

  ActuatedTransformsCallback callback_actuated_transforms_updated_;

  TransformTree::IndirectRefFrame
      indirectrefframe_;  // Current pose reference frame
  TransformTree::StaticRefFrame
      staticrefframe_home_;  // Home pose reference frame
};

// -----------------------------------------------------------------------------
// class Robot

Robot::Robot() : Actor(nullptr) {}

Robot::Robot(const std::string& id, const Transform& origin,
             const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
    : Actor(std::shared_ptr<ActorImpl>(
          new Robot::Impl(id, origin, logger, topic_manager, parent_topic_path,
                          service_manager, state_manager))) {}

Robot::Robot(const std::string& id, const Transform& origin,
             const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager, HomeGeoPoint home_geo_point)
    : Actor(std::shared_ptr<ActorImpl>(
          new Robot::Impl(id, origin, logger, topic_manager, parent_topic_path,
                          service_manager, state_manager))) {
  static_cast<Robot::Impl*>(pimpl_.get())->SetHomeGeoPoint(home_geo_point);
}

void Robot::Load(ConfigJson config_json) {
  return static_cast<Robot::Impl*>(pimpl_.get())->Load(config_json);
}

Link* Robot::GetLink(const std::string& id) {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetLink(id);
}

const std::vector<Link>& Robot::GetLinks() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetLinks();
}

const std::vector<Joint>& Robot::GetJoints() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetJoints();
}

TransformTree::RefFrame& Robot::GetHomeRefFrame(void) {
  // Call const version to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return const_cast<TransformTree::RefFrame&>(
      static_cast<Robot::Impl*>(pimpl_.get())->GetHomeRefFrame());
}

const TransformTree::RefFrame& Robot::GetHomeRefFrame(void) const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetHomeRefFrame();
}

Robot::operator TransformTree::RefFrame&(void) {
  // Call const function to avoid duplicating it with a non-cost version in the
  // impl--const_cast safe to do because this object is non-const in this call
  return const_cast<TransformTree::RefFrame&>(
      static_cast<Robot::Impl*>(pimpl_.get())
          ->
          operator const TransformTree::RefFrame&());
}

Robot::operator const TransformTree::RefFrame&(void) const {
  return static_cast<Robot::Impl*>(pimpl_.get())
      ->
      operator const TransformTree::RefFrame&();
}

const std::vector<std::reference_wrapper<Sensor>>& Robot::GetSensors() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetSensors();
}

int Robot::GetSensorIndex(const std::string& sensor_id) const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetSensorIndex(sensor_id);
}

const std::vector<std::reference_wrapper<Actuator>>& Robot::GetActuators()
    const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetActuators();
}

const Logger& Robot::GetLogger() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetLogger();
}

void Robot::BeginUpdate() {
  return static_cast<Robot::Impl*>(pimpl_.get())->BeginUpdate();
}

Topic Robot::CreateTopic(
    const std::string& name, TopicType type, int frequency,
    MessageType message_type,
    std::function<void(const Topic&, const Message&)> callback) {
  return static_cast<Robot::Impl*>(pimpl_.get())
      ->CreateTopic(name, type, frequency, message_type, callback);
}

void Robot::RemoveTopic(const Topic& topic) {
  static_cast<Robot::Impl*>(pimpl_.get())->RemoveTopic(topic);
}

void Robot::RegisterChildTransformNodes(TransformTree& transformtree) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->RegisterChildTransformNodes(transformtree);
}

void Robot::RegisterServiceMethod(const ServiceMethod& method,
                                  MethodHandler method_handler) {
  return static_cast<Robot::Impl*>(pimpl_.get())
      ->RegisterServiceMethod(method, method_handler);
}

void Robot::SetController(std::unique_ptr<IController> controller) {
  static_cast<Robot::Impl*>(pimpl_.get())->SetController(std::move(controller));
}

void Robot::PublishRobotPose(const PoseStampedMessage& pose) {
  static_cast<Robot::Impl*>(pimpl_.get())->PublishRobotPose(pose);
}

void Robot::PublishRobotKinematics(const KinematicsMessage& kinematics) {
  static_cast<Robot::Impl*>(pimpl_.get())->PublishRobotKinematics(kinematics);
}

void Robot::PublishTopic(const Topic& topic, const Message& msg) {
  static_cast<Robot::Impl*>(pimpl_.get())->PublishTopic(topic, msg);
}

void Robot::SetCallbackKinematicsUpdated(const KinematicsCallback& callback) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->SetCallbackKinematicsUpdated(callback);
}

void Robot::SetCallbackActuatorOutputUpdated(
    const ActuatedTransformsCallback& callback) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->SetCallbackActuatorOutputUpdated(callback);
}

void Robot::EndUpdate() {
  static_cast<Robot::Impl*>(pimpl_.get())->EndUpdate();
}

const Kinematics& Robot::GetKinematics() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetKinematics();
}

const Environment& Robot::GetEnvironment() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetEnvironment();
}

const Vector3& Robot::GetExternalForce() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetExternalForce();
}

const PhysicsType& Robot::GetPhysicsType() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetPhysicsType();
}

void Robot::SetPhysicsType(const PhysicsType& phys_type) {
  static_cast<Robot::Impl*>(pimpl_.get())->SetPhysicsType(phys_type);
}

const std::string& Robot::GetPhysicsConnectionSettings() const {
  return static_cast<Robot::Impl*>(pimpl_.get())
      ->GetPhysicsConnectionSettings();
}

void Robot::SetPhysicsConnectionSettings(
    const std::string& phys_conn_settings) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->SetPhysicsConnectionSettings(phys_conn_settings);
}

const std::string& Robot::GetControlConnectionSettings() const {
  return static_cast<Robot::Impl*>(pimpl_.get())
      ->GetControlConnectionSettings();
}

void Robot::SetControlConnectionSettings(
    const std::string& control_conn_settings) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->SetControlConnectionSettings(control_conn_settings);
}

bool Robot::GetStartLanded() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetStartLanded();
}

void Robot::SetStartLanded(bool start_landed) {
  static_cast<Robot::Impl*>(pimpl_.get())->SetStartLanded(start_landed);
}

const std::string& Robot::GetControllerType() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetControllerType();
}

const std::string& Robot::GetControllerSettings() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetControllerSettings();
}

const CollisionInfo& Robot::GetCollisionInfo() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetCollisionInfo();
}

void Robot::SetActuatedTransforms(const ActuatedTransforms& actuated_transforms,
                                  TimeNano external_time_stamp) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->SetActuatedTransforms(actuated_transforms, external_time_stamp);
}

void Robot::SetActuatedRotations(const ActuatedRotations& actuated_rots,
                                 TimeNano external_time_stamp) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->SetActuatedRotations(actuated_rots, external_time_stamp);
}

void Robot::InitializeSensors(const Kinematics& kinematics,
                              const Environment& environment) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->InitializeSensors(kinematics, environment);
}

void Robot::UpdateCollisionInfo(const CollisionInfo& collision_info) {
  static_cast<Robot::Impl*>(pimpl_.get())->UpdateCollisionInfo(collision_info);
}

void Robot::SetHasCollided(bool has_collided) {
  static_cast<Robot::Impl*>(pimpl_.get())->SetHasCollided(has_collided);
}

void Robot::UpdateControlInput() {
  static_cast<Robot::Impl*>(pimpl_.get())->UpdateControlInput();
}

void Robot::UpdateActuators(const TimeNano sim_time,
                            const TimeNano sim_dt_nanos) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->UpdateActuators(sim_time, sim_dt_nanos);
}

void Robot::UpdateKinematics(const Kinematics& kinematics,
                             TimeNano external_time_stamp) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->UpdateKinematics(kinematics, external_time_stamp);
}

void Robot::UpdateEnvironment() {
  static_cast<Robot::Impl*>(pimpl_.get())->UpdateEnvironment();
}

void Robot::UpdateSensors(const TimeNano sim_time,
                          const TimeNano sim_dt_nanos) {
  static_cast<Robot::Impl*>(pimpl_.get())
      ->UpdateSensors(sim_time, sim_dt_nanos);
}

std::vector<Wheel*> Robot::GetWheels() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->GetWheels();
}

bool Robot::HasWheels() const {
  return static_cast<Robot::Impl*>(pimpl_.get())->HasWheels();
}

// -----------------------------------------------------------------------------
// class Robot::Impl

Robot::Impl::Impl(const std::string& id, const Transform& origin,
                  const Logger& logger, const TopicManager& topic_manager,
                  const std::string& parent_topic_path,
                  const ServiceManager& service_manager,
                  const StateManager& state_manager)
    : ActorImpl(ActorType::kRobot, id, origin, Constant::Component::robot,
                logger, topic_manager, parent_topic_path, service_manager,
                state_manager),
      loader_(*this),
      callback_kinematics_updated_(nullptr),
      callback_actuated_transforms_updated_(nullptr),
      indirectrefframe_(std::string("R ") + id, &kinematics_.pose),
      staticrefframe_home_(std::string("R ") + id + "_home", kinematics_.pose),
      last_actuated_rotation_simtime_(0),
      start_landed_(false) {
  SetTopicPath();
  CreateTopics();
  RegisterServiceMethods();
}

void Robot::Impl::Load(ConfigJson config_json) {
  // Load robot from JSON config
  json json = config_json;
  loader_.Load(json);

  // Initialize kinematics from loaded origin
  kinematics_.pose.position = origin_.translation_;
  kinematics_.pose.orientation = origin_.rotation_;

  // Save home pose
  staticrefframe_home_.SetLocalPose(kinematics_.pose);

  // Update initial environment from initial kinematics position (home geo point
  // should have been set during robot construction in the scene).
  UpdateEnvironment();

  //! Initialize Sensors
  InitializeSensors(GetKinematics(), GetEnvironment());

  // Create tilt actuator target list
  {
    std::vector<Tilt*> vecpactuatorTilt;

    // Find all tilt actuators
    for (auto& actuator : actuators_) {
      if (actuator->GetType() == ActuatorType::kTilt) {
        vecpactuatorTilt.push_back(static_cast<Tilt*>(actuator.get()));
      }
    }

    // Find tilt actuator targets and add to target list
    for (auto& ptilt : vecpactuatorTilt) {
      bool fFoundTarget = false;
      auto target_id = ptilt->GetTargetID();

      assert(!target_id.empty());  // Tilt actuator load should have caught this
      for (auto& actuator : actuators_) {
        if (actuator->GetId() == target_id) {
          // We only support rotors as targets...for now
          if (actuator->GetType() != ActuatorType::kRotor) {
            logger_.LogError(
                name_,
                "[%s] tilt actuator '%s' target actuator '%s' is not a rotor.",
                id_.c_str(), ptilt->GetId().c_str(), target_id.c_str());
            throw Error("Invalid target actuator type for tilt actuator.");
          }

          fFoundTarget = true;
          tilt_targets_.push_back(
              std::make_tuple(ptilt, static_cast<Rotor*>(actuator.get())));
          break;
        }
      }

      if (!fFoundTarget) {
        logger_.LogError(
            name_, "[%s] tilt actuator '%s' target actuator '%s' not found.",
            id_.c_str(), ptilt->GetId().c_str(), target_id.c_str());
        throw Error("Unknown target actuator ID for tilt actuator.");
      }
    }
  }
}

void Robot::Impl::CreateTopics() {
  actual_robot_pose_topic_ =
      Topic("actual_pose", topic_path_, TopicType::kPublished, 0,
            MessageType::kPosestamped);

  desired_robot_pose_topic_ =
      Topic("desired_pose", topic_path_, TopicType::kSubscribed, 0,
            MessageType::kPosestamped);

  collision_info_topic_ =
      Topic("collision_info", topic_path_, TopicType::kPublished, 0,
            MessageType::kCollisionInfo);

  rotor_info_topic_ = Topic("rotor_info", topic_path_, TopicType::kPublished, 0,
                            MessageType::kRotorInfo);
  
  // Ground truth pose and kinematics topic
  gt_kinematics_topic_ = Topic("gt_kinematics", topic_path_, TopicType::kPublished, 0,
                            MessageType::kKinematics);

  topics_.push_back(actual_robot_pose_topic_);
  topics_.push_back(desired_robot_pose_topic_);
  topics_.push_back(collision_info_topic_);
  topics_.push_back(rotor_info_topic_);
  topics_.push_back(gt_kinematics_topic_);
}

Topic Robot::Impl::CreateTopic(
    const std::string& name, TopicType type, int frequency,
    MessageType message_type,
    std::function<void(const Topic&, const Message&)> callback) {
  auto topic = Topic(name, topic_path_, type, frequency, message_type);
  topic_manager_.RegisterTopic(topic);

  if (type == TopicType::kSubscribed && callback != nullptr) {
    topic_manager_.SubscribeTopic(topic, callback);
  } else {
    // TODO - need to support other topic types, if needed
    logger_.LogWarning(name_, "[%s] Unsupported Topic configuration",
                       id_.c_str());
  }
  return topic;
}

void Robot::Impl::RemoveTopic(const Topic& topic) {
  topic_manager_.UnregisterTopic(topic);
}

void Robot::Impl::RegisterChildTransformNodes(TransformTree& transformtree) {
  if (links_.empty()) return;

  auto& robot_id = this->GetID();

  // Register first link as directly connected to the robot
  transformtree.Register(links_[0], *this);

  // Register child joints and links references by the joints
  for (auto& joint : joints_) {
    // Register joint as child of parent link
    {
      auto& parent_link_id = joint.GetParentLink();
      TransformTree::RefFrame* prefframeParent;

      if (parent_link_id == robot_id)
        prefframeParent = &const_cast<TransformTree::RefFrame&>(
            static_cast<const TransformTree::RefFrame&>(*this));
      else {
        auto plink = GetLink(parent_link_id);

        if (plink == nullptr)
          throw std::logic_error(std::string("Unrecognized parent link \"") +
                                 parent_link_id + "\" specified by joint \"" +
                                 joint.GetID() + "\"");

        prefframeParent = &static_cast<TransformTree::RefFrame&>(*plink);
      }

      transformtree.Register(joint, *prefframeParent);
    }

    // Register child link inertial and visual as children of joint
    {
      // Get child link
      auto& child_link_id = joint.GetChildLink();
      auto plink = GetLink(child_link_id);

      if (plink == nullptr)
        throw std::logic_error(std::string("Unrecognized child link \"") +
                               child_link_id + "\" specified by joint \"" +
                               joint.GetID() + "\"");

      transformtree.Register(*plink, joint);
      transformtree.Register(plink->GetVisualRefFrame(), joint);
    }
  }

  // Register rotor actuators
  for (auto& actuator : actuators_) {
    if (actuator->GetType() == ActuatorType::kRotor) {
      transformtree.Register(*static_cast<Rotor*>(actuator.get()), *this);
    }
    if (actuator->GetType() == ActuatorType::kWheel) {
      transformtree.Register(*static_cast<Wheel*>(actuator.get()), *this);
    }
  }
}

void Robot::Impl::RegisterServiceMethod(const ServiceMethod& method,
                                        MethodHandler method_handler) {
  // Register external Service Methods through the Robot Class
  auto method_name = method.GetName();
  auto params_list = method.GetParamsList();
  // Re-use topic path to create a unique (instance-specific) name for the
  // service method
  auto unique_method_name = topic_path_ + "/" + method_name;
  auto unique_method = ServiceMethod(unique_method_name, params_list);
  service_manager_.RegisterMethod(unique_method, method_handler);
}

void Robot::Impl::RegisterServiceMethods() {
  // Register internal Service Methods offered by the Robot Class
  auto get_gt_kinematics =
      ServiceMethod(topic_path_ + "/GetGroundTruthKinematics", {""});
  auto get_gt_kinematics_handler = get_gt_kinematics.CreateMethodHandler(
      &Robot::Impl::GetGroundTruthKinematics, *this);
  service_manager_.RegisterMethod(get_gt_kinematics, get_gt_kinematics_handler);

  auto set_gt_kinematics =
      ServiceMethod(topic_path_ + "/SetGroundTruthKinematics", {"kinematics"});
  auto set_gt_kinematics_handler = set_gt_kinematics.CreateMethodHandler(
      &Robot::Impl::SetGroundTruthKinematics, *this);
  service_manager_.RegisterMethod(set_gt_kinematics, set_gt_kinematics_handler);

  auto get_pose = ServiceMethod(topic_path_ + "/GetGroundTruthPose", {""});
  auto get_pose_handler =
      get_pose.CreateMethodHandler(&Robot::Impl::GetGroundTruthPose, *this);
  service_manager_.RegisterMethod(get_pose, get_pose_handler);

  auto get_geo_location =
      ServiceMethod(topic_path_ + "/GetGroundTruthGeoLocation", {""});
  auto get_geo_location_handler = get_geo_location.CreateMethodHandler(
      &Robot::Impl::GetGroundTruthGeoLocation, *this);
  service_manager_.RegisterMethod(get_geo_location, get_geo_location_handler);

  auto set_pose =
      ServiceMethod(topic_path_ + "/SetPose", {"pose", "reset_kinematics"});
  auto set_pose_handler =
      set_pose.CreateMethodHandler(&Robot::Impl::SetPose, *this);
  service_manager_.RegisterMethod(set_pose, set_pose_handler);

  auto set_ext_force =
      ServiceMethod(topic_path_ + "/SetExternalForce", {"ext_force"});
  auto set_ext_force_handler =
      set_ext_force.CreateMethodHandler(&Robot::Impl::SetExternalForce, *this);
  service_manager_.RegisterMethod(set_ext_force, set_ext_force_handler);

  auto get_camera_ray =
      // ServiceMethod(topic_path_ + "/GetCameraRay", {"camera_id",
      // "image_type", "position"});
      ServiceMethod(topic_path_ + "/GetCameraRay",
                    {"camera_id", "image_type", "x", "y"});
  auto get_camera_ray_handler =
      get_camera_ray.CreateMethodHandler(&Robot::Impl::GetCameraRay, *this);
  service_manager_.RegisterMethod(get_camera_ray, get_camera_ray_handler);
}

bool Robot::Impl::SetExternalForce(const std::vector<float>& ext_force) {
  ext_force_ = Vector3(ext_force[0], ext_force[1], ext_force[2]);
  return true;
}

KinematicsMessage Robot::Impl::GetGroundTruthKinematics() {
  KinematicsMessage kin_msg(SimClock::Get()->NowSimNanos(), kinematics_);
  return kin_msg;
}

bool Robot::Impl::SetGroundTruthKinematics(
    const KinematicsMessage& kinematics) {
  Kinematics new_kin = kinematics.GetKinematics();
  SetHasCollided(false);
  UpdateKinematics(new_kin);

  return true;
}

void Robot::Impl::SetController(std::unique_ptr<IController> controller) {
  controller_ = std::move(controller);
}

Link* Robot::Impl::GetLink(const std::string& id) {
  for (auto& link : links_) {
    if (link.GetID() == id) return (&link);
  }

  return (nullptr);
}

const std::vector<Link>& Robot::Impl::GetLinks() { return links_; }

const std::vector<Joint>& Robot::Impl::GetJoints() { return joints_; }

const TransformTree::RefFrame& Robot::Impl::GetHomeRefFrame(void) const {
  return this->staticrefframe_home_;
}

Robot::Impl::operator const TransformTree::RefFrame&(void) const {
  return indirectrefframe_;
}

const std::vector<std::reference_wrapper<Sensor>>& Robot::Impl::GetSensors() {
  return sensors_ref_;
}

int Robot::Impl::GetSensorIndex(const std::string& sensor_id) const {
  auto sensor_idx_itr = sensor_index_by_id_.find(sensor_id);
  if (sensor_idx_itr != sensor_index_by_id_.end()) {
    return sensor_idx_itr->second;
  } else {
    return -1;
  }
}

const std::vector<std::reference_wrapper<Actuator>>&
Robot::Impl::GetActuators() {
  return actuators_ref_;
}

const Logger& Robot::Impl::GetLogger() const { return logger_; }

void Robot::Impl::OnBeginUpdate() {
  // Register all topics to be ready for publish/subscribe calls
  for (const auto& topic_ref : topics_) {
    topic_manager_.RegisterTopic(topic_ref.get());
  }

  // Subscribe to topics and bind their callbacks
  topic_manager_.SubscribeTopic(
      desired_robot_pose_topic_,
      [this](const Topic& topic, const Message& message) {
        OnDesiredRobotPoseStampedMessage(topic, message);
      });

  if (controller_ != nullptr) {
    // BeginUpdate on controller must happen first before other calls.
    controller_->BeginUpdate();
    controller_->SetKinematics(&GetKinematics());
  }

  for (auto& sensor : sensors_) {
    sensor->BeginUpdate();
  }

  // Publish the initial pose
  TimeNano time_stamp = SimClock::Get()->NowSimNanos();
  auto pose_msg = PoseStampedMessage(time_stamp, kinematics_.pose.position,
                                     kinematics_.pose.orientation);
  topic_manager_.PublishTopic(actual_robot_pose_topic_, pose_msg);
  
  // Publish initial kinematics
  auto kinematics_msg = KinematicsMessage(time_stamp, kinematics_);
  topic_manager_.PublishTopic(gt_kinematics_topic_, kinematics_msg);
}

void Robot::Impl::PublishRobotPose(const PoseStampedMessage& pose) {
  std::lock_guard<std::mutex> lock(update_lock_);

  topic_manager_.PublishTopic(actual_robot_pose_topic_, pose);
}

void Robot::Impl::PublishRobotKinematics(const KinematicsMessage& kinematics) {
  std::lock_guard<std::mutex> lock(update_lock_);

  topic_manager_.PublishTopic(gt_kinematics_topic_, kinematics);
}

void Robot::Impl::PublishTopic(const Topic& topic, const Message& msg) {
  std::lock_guard<std::mutex> lock(update_lock_);

  topic_manager_.PublishTopic(topic, msg);
}

void Robot::Impl::SetCallbackKinematicsUpdated(
    const KinematicsCallback& callback) {
  std::lock_guard<std::mutex> lock(update_lock_);
  callback_kinematics_updated_ = callback;
}

void Robot::Impl::SetCallbackActuatorOutputUpdated(
    const ActuatedTransformsCallback& callback) {
  std::lock_guard<std::mutex> lock(update_lock_);
  callback_actuated_transforms_updated_ = callback;
}

void Robot::Impl::OnEndUpdate() {
  // Clear all topic callbacks to nullptr
  callback_kinematics_updated_ = nullptr;
  callback_actuated_transforms_updated_ = nullptr;

  // Unregister all topics
  for (const auto& topic_ref : topics_) {
    topic_manager_.UnregisterTopic(topic_ref.get());
  }

  if (controller_ != nullptr) {
    controller_->EndUpdate();
  }

  for (auto& sensor : sensors_) {
    sensor->EndUpdate();
  }
}

void Robot::Impl::OnDesiredRobotPoseStampedMessage(const Topic& topic,
                                                   const Message& message) {
  // Copy current base kinematics and overwrite new position/orientation
  auto pose_msg = static_cast<const PoseStampedMessage&>(message);
  Kinematics desired_kin = kinematics_;
  desired_kin.pose.position = pose_msg.GetPosition();
  desired_kin.pose.orientation = pose_msg.GetOrientation();

  // Call UpdateKinematics to set the new pose on the robot, which will also
  // update the UnrealRobot's pose to match
  UpdateKinematics(desired_kin);
}

bool Robot::Impl::SetPose(const Transform& new_pose, bool reset_kinematics) {
  Kinematics new_kin;  // start with empty kinematics
  if (reset_kinematics == false) {
    // If request is to not reset kinematics, copy existing kinematic values
    // to preserve velocities and accels
    new_kin = kinematics_;
  }

  // Set kinematics pose to values from new_pose
  new_kin.pose.position = new_pose.translation_;
  new_kin.pose.orientation = new_pose.rotation_;

  // Call UpdateKinematics to set the new pose on the robot, which will also
  // update the UnrealRobot's pose to match
  UpdateKinematics(new_kin);

  return true;
}

const Kinematics& Robot::Impl::GetKinematics() const { return kinematics_; }

const Transform Robot::Impl::GetGroundTruthPose() {
  Transform current_pose;  // Start with an empty Transform
  current_pose.translation_ =
      kinematics_.pose.position;  // Add in the position and orientation
  current_pose.rotation_ = kinematics_.pose.orientation;
  current_pose.timestamp_ = kinematics_updated_timestamp_;
  return current_pose;
}

const GeoPoint Robot::Impl::GetGroundTruthGeoLocation() {
  return environment_.env_info.geo_point;
}

const Environment& Robot::Impl::GetEnvironment() const { return environment_; }

const Vector3& Robot::Impl::GetExternalForce() const { return ext_force_; }

const CollisionInfo& Robot::Impl::GetCollisionInfo() const {
  return collision_info_;
}

void Robot::Impl::SetActuatedRotations(const ActuatedRotations& actuated_rots,
                                       TimeNano external_time_stamp) {
  std::lock_guard<std::mutex> lock(update_lock_);

  // Copy new actuated rotations to robot's actuated_rotations_
  actuated_transforms_.clear();
  for (const auto& [output_link, rotor_ang_vel] : actuated_rots) {
    auto transform =
        TransformUtils::ToQuaternion(rotor_ang_vel).toRotationMatrix();

    actuated_transforms_.insert(std::pair(output_link, Affine3(transform)));
  }

  TimeNano time_stamp = external_time_stamp == -1
                            ? SimClock::Get()->NowSimNanos()
                            : external_time_stamp;

  TimeNano delta_nanos = time_stamp - last_actuated_rotation_simtime_;

  // Process callback for updated actuated rotations
  auto func = callback_actuated_transforms_updated_;
  if (func != nullptr) {
    func(actuated_transforms_, delta_nanos);
  }

  last_actuated_rotation_simtime_ = time_stamp;
}

void Robot::Impl::SetActuatedTransforms(
    const ActuatedTransforms& actuated_transforms,
    TimeNano external_time_stamp) {
  std::lock_guard<std::mutex> lock(update_lock_);

  // Copy new actuated rotations to robot's actuated_rotations_
  actuated_transforms_ = actuated_transforms;

  TimeNano time_stamp = external_time_stamp == -1
                            ? SimClock::Get()->NowSimNanos()
                            : external_time_stamp;

  TimeNano delta_nanos = time_stamp - last_actuated_rotation_simtime_;

  // Process callback for updated actuated rotations
  auto func = callback_actuated_transforms_updated_;
  if (func != nullptr) {
    func(actuated_transforms_, delta_nanos);
  }

  last_actuated_rotation_simtime_ = time_stamp;
}

const PhysicsType& Robot::Impl::GetPhysicsType() const { return physics_type_; }

void Robot::Impl::SetPhysicsType(const PhysicsType& phys_type) {
  physics_type_ = phys_type;
}

const std::string& Robot::Impl::GetPhysicsConnectionSettings() const {
  return physics_connection_settings_;
}

void Robot::Impl::SetPhysicsConnectionSettings(
    const std::string& phys_conn_settings) {
  physics_connection_settings_ = phys_conn_settings;
}

const std::string& Robot::Impl::GetControlConnectionSettings() const {
  return control_connection_settings_;
}

void Robot::Impl::SetControlConnectionSettings(
    const std::string& control_conn_settings) {
  control_connection_settings_ = control_conn_settings;
}

bool Robot::Impl::GetStartLanded() const { return start_landed_; }

void Robot::Impl::SetStartLanded(bool start_landed) {
  start_landed_ = start_landed;
}

const std::string& Robot::Impl::GetControllerType() const {
  return controller_type_;
}

const std::string& Robot::Impl::GetControllerSettings() const {
  return controller_settings_;
}

Pose Robot::Impl::GetCameraRay(const std::string& camera_id, int image_type,
                               int x, int y) {
  auto isensor = GetSensorIndex(camera_id);
  Pose pose_ret(Vector3(std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::quiet_NaN()),
                Quaternion::Identity());

  if (isensor >= 0) {
    auto& sensor = GetSensors()[isensor];

    if (sensor.get().GetType() == SensorType::kCamera) {
      auto& camera = static_cast<Camera&>(sensor.get());

      pose_ret =
          camera.GetRay(static_cast<ImageType>(image_type), Vector2(x, y));
    }
  }

  return (pose_ret);
}

void Robot::Impl::InitializeSensors(const Kinematics& kinematics,
                                    const Environment& environment) {
  //! Iterate over loaded sensors & initialize with Ground-truth pointers
  for (auto& sensor : sensors_) {
    sensor->Initialize(kinematics, environment);
  }
}

void Robot::Impl::UpdateCollisionInfo(const CollisionInfo& collision_info) {
  std::lock_guard<std::mutex> lock(update_lock_);
  collision_info_ = collision_info;

  // Publish a collision info topic every time a collision is detected
  if (collision_info_.has_collided) {
    CollisionInfoMessage collision_info_msg(collision_info_);
    topic_manager_.PublishTopic(collision_info_topic_, collision_info_msg);
  }
}

void Robot::Impl::SetHasCollided(bool has_collided) {
  std::lock_guard<std::mutex> lock(update_lock_);
  collision_info_.has_collided = has_collided;
}

void Robot::Impl::UpdateControlInput() {
  // Don't lock the robot's update_lock_ mutex since the controller is only
  // updating itself and not changing the robot's data directly. This is needed
  // to prevent a deadlock on sim stopping when using an external controller
  // like Simulink that has a blocking wait loop for passing messages.
  if (controller_ != nullptr) {
    controller_->Update();
  }
}

void Robot::Impl::UpdateActuators(const TimeNano sim_time,
                                  const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  if (actuators_.empty()) return;

  // Update tilt actuator first since they affect other actuators
  for (auto& actuator : actuators_) {
    if (actuator->GetType() == ActuatorType::kTilt) {
      // Call actuator to update its output for its current control signal
      std::vector<float> control_signals =
          controller_->GetControlSignals(actuator->GetId());

      actuator->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
    }
  }

  // Update tilt targets
  for (auto& tuple : tilt_targets_) {
    std::get<1>(tuple)->SetTilt(std::get<0>(tuple)->GetControlRotation());
  }

  // Initialize container to publish rotor parameters
  std::vector<RotorInfo> rotor_info_vec;

  // Update non-tilt actuators
  for (auto& actuator : actuators_) {
    if (actuator->GetType() == ActuatorType::kGimbal) {
      auto gimbal_signal = controller_->GetGimbalSignal(actuator->GetId());
      auto gimbal = static_cast<Gimbal&>(*actuator);
      gimbal.UpdateGimbal(gimbal_signal, sim_dt_nanos);
    } else if (actuator->GetType() != ActuatorType::kTilt) {
      // Do pre-processing specific to actuator type
      if (actuator->GetType() == ActuatorType::kRotor) {
        // For rotor actuators, update their air_density from current
        // environment
        auto& rotor_ref = static_cast<Rotor&>(*actuator);
        float air_density_ratio =
            environment_.env_info.air_density / EarthUtils::kSeaLevelAirDensity;

        rotor_ref.SetAirDensityRatio(air_density_ratio);
      }

      // Call actuator to update its output for its current control signal
      std::vector<float> control_signals =
          controller_->GetControlSignals(actuator->GetId());

      actuator->UpdateActuatorOutput(std::move(control_signals), sim_dt_nanos);
    }

    // Do post-processing specific to actuator type
    {
      ActuatedTransforms actuated_transforms;

      if (actuator->GetType() == ActuatorType::kRotor) {
        auto& rotor = static_cast<Rotor&>(*actuator);

        // Add rotor actuator data to container for publishing
        rotor_info_vec.push_back(
            RotorInfo(actuator->GetId(), rotor.GetRotatingSpeed(),
                      rotor.GetAngle(), rotor.GetTorque(), rotor.GetThrust()));

        actuated_transforms = rotor.GetActuatedTransforms();

        // Get power consumption
        total_power_ += rotor.GetPowerConsumption();
      } else if (actuator->GetType() == ActuatorType::kTilt) {
        auto& tilt = static_cast<Tilt&>(*actuator);

        actuated_transforms = tilt.GetActuatedTransforms();
        // TODO: This can be used when we implement gimbal using transform tree.
        // Right now, this transform is incapable of isolating gimbal movement
        // from the movement of underlying robot - which is an integral part to
        // implement Gimbal locks. So we are relying on Unreal to do this for
        // us.
        //} else if (actuator->GetType() == ActuatorType::kGimbal) {
        //  auto& gimbal = static_cast<Gimbal&>(*actuator);

        //  actuated_transforms = gimbal.GetActuatedTransforms();
      } else if (actuator->GetType() == ActuatorType::kWheel) {
        auto& wheel = static_cast<Wheel&>(*actuator);
        actuated_transforms = wheel.GetActuatedTransforms();
      }
      if (!actuated_transforms.empty()) {
        auto func = callback_actuated_transforms_updated_;
        if (func != nullptr) {
          func(actuated_transforms, sim_dt_nanos);
        }
      }
    }
  }
  // Publish rotor topic
  if (!rotor_info_vec.empty()) {
    auto rotor_info =
        RotorMessage(SimClock::Get()->NowSimNanos(), rotor_info_vec);
    topic_manager_.PublishTopic(rotor_info_topic_, rotor_info);
  }

  last_actuated_rotation_simtime_ = sim_time;
}

void Robot::Impl::UpdateKinematics(const Kinematics& kinematics,
                                   TimeNano external_time_stamp) {
  std::lock_guard<std::mutex> lock(update_lock_);

  // Copy new kinematics to robot's kinematics_
  kinematics_ = kinematics;

  TimeNano time_stamp = external_time_stamp == -1
                            ? SimClock::Get()->NowSimNanos()
                            : external_time_stamp;

  // Process callback for updated kinematics
  auto func = callback_kinematics_updated_;
  if (func != nullptr) {
    func(kinematics_, time_stamp);
  }

  kinematics_updated_timestamp_ = time_stamp;

  // Publish actual pose topic
  PoseStampedMessage pose_msg(time_stamp, kinematics_.pose.position,
                              kinematics_.pose.orientation);
  topic_manager_.PublishTopic(actual_robot_pose_topic_, pose_msg);

  // Publish kinematics topic
  KinematicsMessage kinematics_msg(time_stamp, kinematics_);
  topic_manager_.PublishTopic(gt_kinematics_topic_, kinematics_msg);

  // logger_.LogTrace(name_, "[%s] Pose at simtime=%lld xyz=%f, %f, %f",
  //                  id_.c_str(), time_stamp,
  //                  kinematics_.pose.position.x(),
  //                  kinematics_.pose.position.y(),
  //                  kinematics_.pose.position.z());
}

void Robot::Impl::UpdateEnvironment() {
  std::lock_guard<std::mutex> lock(update_lock_);
  environment_.SetPosition(kinematics_.pose.position);
}

void Robot::Impl::UpdateSensors(const TimeNano sim_time,
                                const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);
  //! Update every loaded and initialized sensor that is enabled on the
  //! robot Note: Not all sensors update in-sync with the scene/physics
  //! tick
  for (auto& sensor : sensors_) {
    if (sensor->IsEnabled()) {
      if (sensor->GetType() == SensorType::kBattery) {
        auto& battery_ref = static_cast<Battery&>(*sensor);
        battery_ref.Update(sim_time, sim_dt_nanos, total_power_);
        total_power_ = 0;
      } else {
        // TODO: Comment this out when Gimbal is implemented using Transform
        // tree
        if (sensor->GetType() == SensorType::kCamera) {
          auto camera = static_cast<Camera*>(sensor.get());
          UpdateCameraPose(camera);
        }
        // Maybe we need to make this Update method more generalized?
        sensor->Update(sim_time, sim_dt_nanos);
      }
    }
  }
}

// If we want, we can probably make this generalized to all sensors
// but not sure if Gimbals are useful/used for other sensors.
void Robot::Impl::UpdateCameraPose(Camera* camera) {
  if (camera->HasGimbal()) {
    auto gimbal_id = camera->GetGimbalId();
    for (auto& actuator : actuators_) {
      if (actuator->GetId() == gimbal_id) {
        auto gimbal = static_cast<Gimbal&>(*actuator);
        auto& state = gimbal.GetGimbalState();
        auto new_pose =
            TransformUtils::ToQuaternion(state.roll, state.pitch, state.yaw);
        auto& curr_pose = camera->GetDesiredPose();
        // Since this is an internal update driven by the robot's
        // UpdateSensors() tick, don't wait for the renderer frame update to
        // set each camera's new gimbal pose
        camera->SetPose(Transform(curr_pose.translation_, new_pose), false);
      }
    }
  }
}

void Robot::Impl::SetHomeGeoPoint(const HomeGeoPoint& home_geo_point) {
  std::lock_guard<std::mutex> lock(update_lock_);
  environment_.home_geo_point = home_geo_point;
}

std::vector<Wheel*> Robot::Impl::GetWheels() const {
  std::vector<Wheel*> result;
  for (auto& actuator : actuators_) {
    if (actuator->GetType() == ActuatorType::kWheel) {
      result.emplace_back(static_cast<Wheel*>(actuator.get()));
    }
  }

  return result;
}

bool Robot::Impl::HasWheels() const {
  for (auto& actuator : actuators_) {
    if (actuator->GetType() == ActuatorType::kWheel) {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
// class Robot::Loader

Robot::Loader::Loader(Robot::Impl& impl) : impl_(impl) {}

void Robot::Loader::Load(const json& json) {
  LoadPhysicsType(json);
  LoadLinks(json);
  LoadJoints(json);
  LoadSensors(json);
  LoadActuators(json);
  LoadController(json);

  impl_.is_loaded_ = true;
}

void Robot::Loader::LoadLinks(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'links'.",
                           impl_.id_.c_str());

  auto links_json = JsonUtils::GetArray(json, Constant::Config::links);
  if (JsonUtils::IsEmptyArray(links_json)) {
    impl_.logger_.LogError(impl_.name_, "[%s] 'links' missing or empty.",
                           impl_.id_.c_str());
    throw Error("Robot must have at least one link.");
  }

  try {
    std::transform(links_json.begin(), links_json.end(),
                   std::back_inserter(impl_.links_),
                   [this](auto& json) { return LoadLink(json); });
  } catch (...) {
    impl_.links_.clear();
    throw;
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'links' loaded.",
                           impl_.id_.c_str());
}

Link Robot::Loader::LoadLink(const json& json) {
  Link link(impl_.logger_, impl_.topic_manager_, impl_.topic_path_ + "/links");
  link.Load(json);
  return link;
}

void Robot::Loader::LoadJoints(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'joints'.",
                           impl_.id_.c_str());

  auto joints_json = JsonUtils::GetArray(json, Constant::Config::joints);
  if (JsonUtils::IsEmptyArray(joints_json)) {
    impl_.logger_.LogWarning(impl_.name_, "[%s] 'joints' missing or empty.",
                             impl_.id_.c_str());
  }

  try {
    std::transform(joints_json.begin(), joints_json.end(),
                   std::back_inserter(impl_.joints_),
                   [this](auto& json) { return LoadJoint(json); });
  } catch (...) {
    impl_.joints_.clear();
    throw;
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'joints' loaded.",
                           impl_.id_.c_str());
}

Joint Robot::Loader::LoadJoint(const json& json) {
  Joint joint(impl_.logger_, impl_.topic_manager_,
              impl_.topic_path_ + "/joints");
  joint.Load(json);
  return joint;
}

void Robot::Loader::LoadSensors(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'sensors'.",
                           impl_.id_.c_str());

  try {
    SensorImpl::LoadSensors(json, impl_.sensors_, impl_.logger_, impl_.name_,
                            impl_.id_, impl_.topic_manager_,
                            impl_.topic_path_ + "/sensors",
                            impl_.service_manager_, impl_.state_manager_);
    impl_.sensors_ref_ = ToRefs(impl_.sensors_);
    for (int i = 0; i < impl_.sensors_.size(); ++i) {
      impl_.sensor_index_by_id_.emplace(impl_.sensors_[i]->GetId(), i);
    }
  } catch (...) {
    impl_.sensors_.clear();
    impl_.sensors_ref_.clear();
    impl_.sensor_index_by_id_.clear();
    throw;
  }
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'sensors' loaded.",
                           impl_.id_.c_str());
}

void Robot::Loader::LoadActuators(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'actuators'.",
                           impl_.id_.c_str());

  try {
    ActuatorImpl::LoadActuators(json, impl_.actuators_, impl_.logger_,
                                impl_.name_, impl_.id_, impl_.topic_manager_,
                                impl_.topic_path_ + "/actuators",
                                impl_.service_manager_, impl_.state_manager_);
    impl_.actuators_ref_ = ToRefs(impl_.actuators_);

    for (auto& actuator : impl_.actuators_) {
      if (actuator->GetType() == ActuatorType::kWheel) {
        // Enable child link of wheel to detect ground collisions
        // that will be reported to the main physics body
        auto linkid_child = actuator->GetChildLink();
        auto plink = impl_.GetLink(linkid_child);

        if (plink != nullptr) plink->EnableGroundCollisionDetection(true);
      }
    }
  } catch (...) {
    impl_.actuators_.clear();
    impl_.actuators_ref_.clear();
    throw;
  }
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'actuators' loaded.",
                           impl_.id_.c_str());
}

void Robot::Loader::LoadPhysicsType(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'physics-type'.",
                           impl_.id_.c_str());
  auto physics_type = JsonUtils::GetIdentifier(
      json, Constant::Config::physics_type, Constant::Config::non_physics);

  if (physics_type == Constant::Config::non_physics) {
    impl_.physics_type_ = PhysicsType::kNonPhysics;
  } else if (physics_type == Constant::Config::fast_physics) {
    impl_.physics_type_ = PhysicsType::kFastPhysics;
  } else if (physics_type == Constant::Config::matlab_physics) {
    impl_.physics_type_ = PhysicsType::kMatlabPhysics;
  } else if (physics_type == Constant::Config::unreal_physics) {
    impl_.physics_type_ = PhysicsType::kUnrealPhysics;
  } else {
    impl_.physics_type_ = PhysicsType::kNonPhysics;
    impl_.logger_.LogWarning(
        impl_.name_,
        "[%s] invalid 'physics-type'. Using default non-physics type.",
        impl_.id_.c_str());
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'physics-type' loaded.",
                           impl_.id_.c_str());
}

void Robot::Loader::LoadController(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'controller'.",
                           impl_.id_.c_str());

  auto controller_json =
      JsonUtils::GetJsonObject(json, Constant::Config::controller);

  if (JsonUtils::IsEmpty(controller_json)) {
    impl_.logger_.LogWarning(impl_.name_, "[%s] 'controller' missing or empty.",
                             impl_.id_.c_str());
    return;
  }

  impl_.controller_type_ =
      JsonUtils::GetIdentifier(controller_json, Constant::Config::type);
  impl_.controller_settings_ = controller_json.dump();

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'controller' loaded.",
                           impl_.id_.c_str());
}

}  // namespace projectairsim
}  // namespace microsoft
