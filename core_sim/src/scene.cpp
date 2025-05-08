// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/scene.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <unordered_map>

#include "algorithms.hpp"
#include "component.hpp"
#include "constant.hpp"
#include "core_sim/actor/env_actor.hpp"
#include "core_sim/actor/env_actor_grounded.hpp"
#include "core_sim/actor/env_car.hpp"
#include "core_sim/actor/env_object.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/error.hpp"
#include "core_sim/file_utils.hpp"
#include "core_sim/geodetic_converter.hpp"
#include "core_sim/service_method.hpp"
#include "core_sim/viewport_camera.hpp"
#include "json.hpp"
#include "message/common_utils.hpp"


namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class Scene::Loader {
 public:
  explicit Loader(Scene::Impl& impl);

  void LoadSceneWithJSON(const json& json);

  void UnloadScene();

 private:
  void LoadID(const json& json);

  void LoadActorsWithJSON(const json& json);

  std::unique_ptr<Actor> LoadActorWithJSON(const json& json);

  void LoadEnvActorsWithJSON(const json& json);

  std::unique_ptr<EnvActor> LoadEnvActorWithJSON(const json& json);

  void LoadEnvObjectsWithJSON(const json& json);

  std::unique_ptr<EnvObject> LoadEnvObjectWithJSON(const json& json);

  void LoadClockSettings(const json& json);

  void LoadHomeGeoPoint(const json& json);

  void LoadWindSettings(const json& json);

  void LoadSegmentationSettings(const json& json);

  void LoadGISSettings(const json& json);

  void LoadDistributedSimSettings(const json& json);

  void LoadVRMode(const json& json);

  std::string GetActorID(const json& json);

  std::string GetActorType(const json& json, const std::string& actor_id);

  Transform GetActorOrigin(const json& json, const std::string& actor_id);

  Scene::Impl& impl_;
};

class Scene::Impl : public ComponentWithTopicsAndServiceMethods {
 public:
  Impl(const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void LoadSceneWithJSON(ConfigJson config_json);

  void UnloadScene();

  const std::vector<std::reference_wrapper<Actor>>& GetActors() const;

  const std::vector<std::reference_wrapper<EnvActor>>& GetEnvActors() const;

  const std::vector<std::reference_wrapper<EnvObject>>& GetEnvObjects() const;

  int GetActorIndex(const std::string& actor_id);

  int GetEnvActorIndex(const std::string& env_actor_name);

  int GetEnvObjectIndex(const std::string& env_object_name);

  std::shared_ptr<Trajectory> GetTrajectoryPtrByName(
      const std::string& traj_name);

  void SetCallbackWindVelocityUpdated(
      const std::function<void(const Vector3&)>& callback);

  void SetCallbackPhysicsStart(const std::function<void()>& callback);

  void SetCallbackPhysicsSetWrenches(const std::function<void()>& callback);

  void SetCallbackPhysicsStep(
      const std::function<void(TimeNano dt_nanos)>& callback);

  void SetCallbackPhysicsStop(const std::function<void()>& callback);

  void SetCallbackTopicPublished(
      const std::function<void(const std::string&, const MessageType&,
                               const std::string&)>& callback);

  const ClockSettings& GetClockSettings() const;

  const SegmentationSettings& GetSegmentationSettings() const;

  const bool GetVRMode() const;

  const HomeGeoPoint& GetHomeGeoPoint() const;

  std::shared_ptr<ViewportCamera>& GetViewportCamera();

  const SceneType GetSceneType() const;

  const bool IsSimTopicCallbackEnabled() const;

  const std::string GetTilesDirectory() const;

  const std::string GetHorizonTilesDirectory() const;

  const float GetTilesAltitudeOffset() const;

  const int GetTilesLodMax() const;

  const int GetTilesLodMin() const;

  void AddNEDTrajectory(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& pose_x, const std::vector<float>& pose_y,
      const std::vector<float>& pose_z, const std::vector<float>& pose_roll,
      const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
      const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
      const std::vector<float>& vel_z_lin);

  TransformTree* GetTransformTree();

  std::shared_ptr<GeodeticConverter> GetGeodeticConverter();

  void StartSceneTick();

  void StopSceneTick();

  void OnBeginUpdate() override;

  void OnEndUpdate() override;

  // Service call methods
  std::string SimGetClockType();  // TODO Should client get full clock settings?
  TimeNano SimGetTime();
  std::string SimPause(bool do_pause);
  bool IsSimPaused();
  TimeNano ContinueForSimTime(TimeNano delta_time,
                              bool wait_until_complete = false);
  TimeNano ContinueUntilSimTime(TimeNano target_time,
                                bool wait_until_complete = false);
  TimeNano ContinueForNSteps(int n_steps, bool wait_until_complete = false);
  TimeNano ContinueForSingleStep(bool wait_until_complete = false);
  std::vector<std::string> SimGetActors();

  bool SetWindVelocity(float v_x, float v_y, float v_z);
  Vector3 GetWindVelocity();
  void UpdateWindVelocity();

  std::string CallBackAfter(int t_secs);

  void RegisterServiceMethod(const ServiceMethod& method,
                             MethodHandler method_handler);

  void RegisterServiceMethods();

  void UnregisterAllServiceMethods();

  bool SceneTick();

  void EnableViewportCamera(bool enable);

  // set enable_unreal_viewport_camera_callback_
  void SetCallbackEnableUnrealViewportCamera(
      const std::function<void(bool)>& callback);

 private:
  friend class Scene::Loader;

  Scene::Loader loader_;
  std::vector<std::unique_ptr<Actor>> actors_;
  std::vector<std::reference_wrapper<Actor>> actors_ref_;
  std::vector<std::unique_ptr<EnvActor>> env_actors_;
  std::vector<std::reference_wrapper<EnvActor>> env_actors_ref_;
  std::vector<std::unique_ptr<EnvObject>> env_objects_;
  std::vector<std::reference_wrapper<EnvObject>> env_objects_ref_;
  std::unordered_map<std::string, int> env_actor_index_by_id_;
  std::unordered_map<std::string, int> actor_index_by_id_;
  std::unordered_map<std::string, int> env_object_index_by_id_;
  std::function<void(const Vector3&)> wind_velocity_updated_callback_;
  std::function<void(bool)> enable_unreal_viewport_camera_callback_;
  std::function<void()> physics_start_callback_;
  std::function<void()> physics_set_wrenches_callback_;
  std::function<void(TimeNano)> physics_step_callback_;
  std::function<void()> physics_stop_callback_;
  ScheduledExecutor executor_;
  TimeNano sim_time_;
  ClockSettings clock_settings_;
  SegmentationSettings segmentation_settings_;
  HomeGeoPoint home_geo_point_;
  bool vr_mode_;
  SceneType scene_type_;
  std::string tiles_dir_;
  std::string horizon_tiles_dir_;
  float tiles_altitude_offset_;
  int tiles_lod_max_;
  int tiles_lod_min_;
  TransformTree transform_tree_;
  std::vector<std::shared_ptr<Trajectory>> trajectories_;
  std::shared_ptr<ViewportCamera> viewport_camera_;
  std::shared_ptr<GeodeticConverter> geodetic_converter_;
};

// -----------------------------------------------------------------------------
// class Scene

Scene::Scene() : pimpl_(std::shared_ptr<Impl>(nullptr)) {}

Scene::Scene(const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
    : pimpl_(std::make_shared<Impl>(logger, topic_manager, parent_topic_path,
                                    service_manager, state_manager)) {}

void Scene::LoadWithJSON(ConfigJson config_json) {
  pimpl_->LoadSceneWithJSON(config_json);
}

void Scene::Unload() { pimpl_->UnloadScene(); }

bool Scene::IsLoaded() { return pimpl_->IsLoaded(); }

const std::string& Scene::GetID() const { return pimpl_->GetID(); }

const std::vector<std::reference_wrapper<Actor>>& Scene::GetActors() const {
  return pimpl_->GetActors();
}

const std::vector<std::reference_wrapper<EnvActor>>& Scene::GetEnvActors()
    const {
  return pimpl_->GetEnvActors();
}

const std::vector<std::reference_wrapper<EnvObject>>& Scene::GetEnvObjects()
    const {
  return pimpl_->GetEnvObjects();
}

int Scene::GetActorIndex(const std::string& actor_id) {
  return pimpl_->GetActorIndex(actor_id);
}

int Scene::GetEnvActorIndex(const std::string& env_actor_name) {
  return pimpl_->GetEnvActorIndex(env_actor_name);
}

int Scene::GetEnvObjectIndex(const std::string& env_object_name) {
  return pimpl_->GetEnvObjectIndex(env_object_name);
}

std::shared_ptr<Trajectory> Scene::GetTrajectoryPtrByName(
    const std::string& traj_name) {
  return pimpl_->GetTrajectoryPtrByName(traj_name);
}

TransformTree* Scene::GetTransformTree() { return pimpl_->GetTransformTree(); }

std::shared_ptr<GeodeticConverter> Scene::GetGeodeticConverter() {
  return pimpl_->GetGeodeticConverter();
}

void Scene::UpdateWindVelocity() {
  return static_cast<Scene::Impl*>(pimpl_.get())->UpdateWindVelocity();
}

void Scene::SetCallbackWindVelocityUpdated(
    const std::function<void(const Vector3&)>& callback) {
  static_cast<Scene::Impl*>(pimpl_.get())
      ->SetCallbackWindVelocityUpdated(callback);
}

void Scene::SetCallbackPhysicsStart(const std::function<void()>& callback) {
  pimpl_->SetCallbackPhysicsStart(callback);
}

void Scene::SetCallbackPhysicsSetWrenches(
    const std::function<void()>& callback) {
  pimpl_->SetCallbackPhysicsSetWrenches(callback);
}

void Scene::SetCallbackPhysicsStep(
    const std::function<void(TimeNano)>& callback) {
  pimpl_->SetCallbackPhysicsStep(callback);
}

void Scene::SetCallbackPhysicsStop(const std::function<void()>& callback) {
  pimpl_->SetCallbackPhysicsStop(callback);
}

void Scene::SetCallbackTopicPublished(
    const std::function<void(const std::string&, const MessageType&,
                             const std::string&)>& callback) {
  pimpl_->SetCallbackTopicPublished(callback);
}

// SetCallbackEnableUnrealViewportCamera
void Scene::SetCallbackEnableUnrealViewportCamera(
    const std::function<void(bool)>& callback) {
  pimpl_->SetCallbackEnableUnrealViewportCamera(callback);
}

// EnableViewportCamera
void Scene::EnableViewportCamera(bool enable) {
  pimpl_->EnableViewportCamera(enable);
}

const ClockSettings& Scene::GetClockSettings() const {
  return pimpl_->GetClockSettings();
}

const SegmentationSettings& Scene::GetSegmentationSettings() const {
  return pimpl_->GetSegmentationSettings();
}

const bool Scene::GetVRMode() const { return pimpl_->GetVRMode(); }

const HomeGeoPoint& Scene::GetHomeGeoPoint() const {
  return pimpl_->GetHomeGeoPoint();
}

std::shared_ptr<ViewportCamera>& Scene::GetViewportCamera() {
  return pimpl_->GetViewportCamera();
}

const SceneType Scene::GetSceneType() const { return pimpl_->GetSceneType(); }

const bool Scene::IsSimTopicCallbackEnabled() const {
  return pimpl_->IsSimTopicCallbackEnabled();
}

const std::string Scene::GetTilesDirectory() const {
  return pimpl_->GetTilesDirectory();
}

const std::string Scene::GetHorizonTilesDirectory() const {
  return pimpl_->GetHorizonTilesDirectory();
}

const float Scene::GetTilesAltitudeOffset() const {
  return pimpl_->GetTilesAltitudeOffset();
}

const int Scene::GetTilesLodMax() const { return pimpl_->GetTilesLodMax(); }

const int Scene::GetTilesLodMin() const { return pimpl_->GetTilesLodMin(); }

const Vector3 Scene::GetWindVelocity() const {
  return pimpl_->GetWindVelocity();
}

void Scene::AddNEDTrajectory(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& pose_x, const std::vector<float>& pose_y,
    const std::vector<float>& pose_z, const std::vector<float>& pose_roll,
    const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
    const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
    const std::vector<float>& vel_z_lin) {
  pimpl_->AddNEDTrajectory(traj_name, time, pose_x, pose_y, pose_z, pose_roll,
                           pose_pitch, pose_yaw, vel_x_lin, vel_y_lin,
                           vel_z_lin);
}

void Scene::RegisterServiceMethod(const ServiceMethod& method,
                                  MethodHandler method_handler) {
  return static_cast<Scene::Impl*>(pimpl_.get())
      ->RegisterServiceMethod(method, method_handler);
}

void Scene::Start() {
  pimpl_->RegisterServiceMethods();
  pimpl_->BeginUpdate();     // begin all robot updates (do this before starting
                             // scene tick to make sure it finishes before the
                             // scene tick starts processing robot updates on it
                             // own thread)
  pimpl_->StartSceneTick();  // begin scene ticks
}

void Scene::Stop() {
  pimpl_->EndUpdate();  // stop all robot updates (do this before stopping scene
                        // tick to give robots a chance to unblock any wait
                        // loops like an external Simulink controller connection
                        // before trying stop and join scene tick thread that
                        // they are being updated on)
  pimpl_->StopSceneTick();  // stop scene ticks
  pimpl_->UnregisterAllServiceMethods();
  // TODO Scene doesn't currently manage any topics, but should consider if
  // scene should do an UnregisterAllTopics() to ensure final clean up in case
  // each component didn't unregister its own topics in its OnEndUpate().
}

// -----------------------------------------------------------------------------
// class Scene::Impl

Scene::Impl::Impl(const Logger& logger, const TopicManager& topic_manager,
                  const std::string& parent_topic_path,
                  const ServiceManager& service_manager,
                  const StateManager& state_manager)
    : ComponentWithTopicsAndServiceMethods(Constant::Component::scene, logger,
                                           topic_manager, parent_topic_path,
                                           service_manager, state_manager),
      loader_(*this),
      geodetic_converter_(std::make_shared<GeodeticConverter>()) {}

void Scene::Impl::LoadSceneWithJSON(ConfigJson config_json) {
  const json& json = config_json;
  loader_.LoadSceneWithJSON(json);
}

void Scene::Impl::UnloadScene() { loader_.UnloadScene(); }

const std::vector<std::reference_wrapper<Actor>>& Scene::Impl::GetActors()
    const {
  return actors_ref_;
}

const std::vector<std::reference_wrapper<EnvActor>>& Scene::Impl::GetEnvActors()
    const {
  return env_actors_ref_;
}

const std::vector<std::reference_wrapper<EnvObject>>&
Scene::Impl::GetEnvObjects() const {
  return env_objects_ref_;
}

int Scene::Impl::GetActorIndex(const std::string& actor_id) {
  auto actor_idx_itr = actor_index_by_id_.find(actor_id);
  if (actor_idx_itr != actor_index_by_id_.end()) {
    return actor_idx_itr->second;
  } else {
    return -1;
  }
}

int Scene::Impl::GetEnvActorIndex(const std::string& env_actor_name) {
  auto env_actor_idx_itr = env_actor_index_by_id_.find(env_actor_name);
  if (env_actor_idx_itr != env_actor_index_by_id_.end()) {
    return env_actor_idx_itr->second;
  } else {
    return -1;
  }
}

int Scene::Impl::GetEnvObjectIndex(const std::string& env_object_name) {
  auto env_object_idx_itr = env_object_index_by_id_.find(env_object_name);
  if (env_object_idx_itr != env_object_index_by_id_.end()) {
    return env_object_idx_itr->second;
  } else {
    return -1;
  }
}

std::shared_ptr<Trajectory> Scene::Impl::GetTrajectoryPtrByName(
    const std::string& traj_name) {
  for (auto trajectory : trajectories_) {
    if (trajectory->GetName() == traj_name) return trajectory;
  }
  return nullptr;
}

void Scene::Impl::SetCallbackWindVelocityUpdated(
    const std::function<void(const Vector3&)>& callback) {
  wind_velocity_updated_callback_ = callback;
}

void Scene::Impl::UpdateWindVelocity() {
  std::function<void(const Vector3&)> wind_velocity_updated_func = nullptr;

  wind_velocity_updated_func = wind_velocity_updated_callback_;
  if (wind_velocity_updated_func != nullptr) {
    wind_velocity_updated_func(GetWindVelocity());
  }
}

void Scene::Impl::SetCallbackPhysicsStart(
    const std::function<void()>& callback) {
  physics_start_callback_ = callback;
}

void Scene::Impl::SetCallbackPhysicsSetWrenches(
    const std::function<void()>& callback) {
  physics_set_wrenches_callback_ = callback;
}

void Scene::Impl::SetCallbackPhysicsStep(
    const std::function<void(TimeNano)>& callback) {
  physics_step_callback_ = callback;
}

void Scene::Impl::SetCallbackPhysicsStop(
    const std::function<void()>& callback) {
  physics_stop_callback_ = callback;
}

void Scene::Impl::SetCallbackTopicPublished(
    const std::function<void(const std::string&, const MessageType&,
                             const std::string&)>& callback) {
  topic_manager_.SetCallbackTopicPublished(callback);
}

void Scene::Impl::SetCallbackEnableUnrealViewportCamera(
    const std::function<void(bool)>& callback) {
  enable_unreal_viewport_camera_callback_ = callback;
}

void Scene::Impl::EnableViewportCamera(bool enable) {
  if (enable && !viewport_camera_) {
    auto topic_path_ = parent_topic_path_ + "/interactive/sensors";
    viewport_camera_ = std::make_shared<ViewportCamera>(
        "viewport_camera", "ViewportCamera", logger_, topic_manager_,
        topic_path_, service_manager_, state_manager_);
    enable_unreal_viewport_camera_callback_(true);
  } else if (!enable && viewport_camera_) {
    viewport_camera_ = nullptr;
    enable_unreal_viewport_camera_callback_(false);
  }
}

const ClockSettings& Scene::Impl::GetClockSettings() const {
  return clock_settings_;
}

const SegmentationSettings& Scene::Impl::GetSegmentationSettings() const {
  return segmentation_settings_;
}

const bool Scene::Impl::GetVRMode() const { return vr_mode_; }

const HomeGeoPoint& Scene::Impl::GetHomeGeoPoint() const {
  return home_geo_point_;
}

std::shared_ptr<ViewportCamera>& Scene::Impl::GetViewportCamera() {
  return viewport_camera_;
}

const SceneType Scene::Impl::GetSceneType() const { return scene_type_; }

const bool Scene::Impl::IsSimTopicCallbackEnabled() const {
  return topic_manager_.IsTopicPublishedCallbackEnabled();
}

const std::string Scene::Impl::GetTilesDirectory() const { return tiles_dir_; }

const std::string Scene::Impl::GetHorizonTilesDirectory() const {
  return horizon_tiles_dir_;
}

const float Scene::Impl::GetTilesAltitudeOffset() const {
  return tiles_altitude_offset_;
}

const int Scene::Impl::GetTilesLodMax() const { return tiles_lod_max_; }

const int Scene::Impl::GetTilesLodMin() const { return tiles_lod_min_; }

void Scene::Impl::AddNEDTrajectory(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& pose_x, const std::vector<float>& pose_y,
    const std::vector<float>& pose_z, const std::vector<float>& pose_roll,
    const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
    const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
    const std::vector<float>& vel_z_lin) {
  std::shared_ptr<Trajectory> trajectory(new Trajectory(logger_));
  trajectory->Load(traj_name, time, pose_x, pose_y, pose_z, pose_roll,
                   pose_pitch, pose_yaw, vel_x_lin, vel_y_lin, vel_z_lin);
  trajectories_.push_back(trajectory);
}

TransformTree* Scene::Impl::GetTransformTree() { return &transform_tree_; }

std::shared_ptr<GeodeticConverter> Scene::Impl::GetGeodeticConverter() {
  return geodetic_converter_;
}

void Scene::Impl::StartSceneTick() {
  // Start StateManager before scene tick to be ready to connect on first tick
  state_manager_.Start();

  std::function<void()> start_physics_func = nullptr;
  start_physics_func = physics_start_callback_;
  if (start_physics_func != nullptr) {
    start_physics_func();
  }

  // Bind callback with implicit argument "this"
  executor_.Initialize(std::bind(&Scene::Impl::SceneTick, this),
                       clock_settings_.scene_tick_period);

  sim_time_ = SimClock::Get()->NowSimNanos();
  executor_.Start();
}

void Scene::Impl::StopSceneTick() {
  // Stop StateManager before scene tick to break NNG receive loops
  state_manager_.Stop();

  std::function<void()> stop_physics_func = nullptr;
  stop_physics_func = physics_stop_callback_;
  if (stop_physics_func != nullptr) {
    stop_physics_func();
  }

  executor_.Stop();
}

void Scene::Impl::OnBeginUpdate() {
  for (auto& actor : actors_) {
    auto& robot = static_cast<Robot&>(*actor);
    robot.BeginUpdate();
  }

  for (auto& env_actor : env_actors_) {
    env_actor->BeginUpdate();
  }
}

void Scene::Impl::OnEndUpdate() {
  for (auto& actor : actors_) {
    if (actor->GetType() == ActorType::kRobot) {
      auto& robot = static_cast<Robot&>(*actor);
      robot.EndUpdate();
    }
  }

  for (auto& env_actor : env_actors_) {
    if (env_actor->GetType() == ActorType::kEnvActor) {
      env_actor->EndUpdate();
    }
  }
}

std::string Scene::Impl::SimGetClockType() {
  switch (clock_settings_.type) {
    case ClockType::kSteppable:
      return Constant::Config::steppable;
    case ClockType::kRealTime:
      return Constant::Config::real_time;
    default:
      return "unknown";
  }
}

TimeNano Scene::Impl::SimGetTime() { return sim_time_; }

std::string Scene::Impl::SimPause(bool do_pause) {
  if (clock_settings_.type != ClockType::kSteppable) {
    logger_.LogError(name_, "This clock type doesn't support SimPause.");
    throw Error("This clock type doesn't support SimPause.");
  }
  // TODO Should this wait to get confirmation that all external components
  // (like Unreal) have also paused before returning?
  TimeNano result_time;
  if (do_pause) {
    SimClock::Get()->SimPause(true);
    result_time = SimClock::Get()->NowSimNanos();
    return "Sim time paused at t = " + std::to_string(result_time);
  } else {
    result_time = SimClock::Get()->NowSimNanos();
    SimClock::Get()->SimPause(false);
    return "Sim time resumed from t = " + std::to_string(result_time);
  }
}

bool Scene::Impl::IsSimPaused() { return SimClock::Get()->IsPaused(); }

TimeNano Scene::Impl::ContinueForSimTime(TimeNano delta_time,
                                         bool wait_until_complete) {
  if (clock_settings_.type != ClockType::kSteppable) {
    logger_.LogError(name_,
                     "This clock type doesn't support ContinueForSimTime.");
    throw Error("This clock type doesn't support ContinueForSimTime.");
  }

  std::string info_msg = "Continuing sim time for " +
                         std::to_string(delta_time) +
                         " ns, and then will pause.";
  logger_.LogVerbose(name_, info_msg.c_str());

  SimClock::Get()->ContinueForSimTime(delta_time);
  if (wait_until_complete) {
    // TODO Add a thread check to prevent this wait loop causing a deadlock if
    // this method was called from the scene tick thread.
    logger_.LogVerbose(name_,
                       "Waiting for target sim time duration to complete.");
    while (!IsSimPaused()) {
      std::this_thread::yield();
    }
  }

  return SimClock::Get()->NowSimNanos();
}

TimeNano Scene::Impl::ContinueUntilSimTime(TimeNano target_time,
                                           bool wait_until_complete) {
  if (clock_settings_.type != ClockType::kSteppable) {
    logger_.LogError(name_,
                     "This clock type doesn't support ContinueUntilSimTime.");
    throw Error("This clock type doesn't support ContinueUntilSimTime.");
  }

  std::string info_msg = "Continuing sim time until " +
                         std::to_string(target_time) +
                         " ns, and then will pause.";
  logger_.LogVerbose(name_, info_msg.c_str());

  SimClock::Get()->ContinueUntilSimTime(target_time);

  if (wait_until_complete) {
    // TODO Add a thread check to prevent this wait loop causing a deadlock if
    // this method was called from the scene tick thread.
    logger_.LogVerbose(name_, "Waiting to reach target sim time.");
    while (!IsSimPaused()) {
      std::this_thread::yield();
    }
  }

  return SimClock::Get()->NowSimNanos();
}

TimeNano Scene::Impl::ContinueForNSteps(int n_steps, bool wait_until_complete) {
  if (clock_settings_.type != ClockType::kSteppable) {
    logger_.LogError(name_,
                     "This clock type doesn't support ContinueForNSteps.");
    throw Error("This clock type doesn't support ContinueForNSteps.");
  }
  std::string info_msg = "Continuing sim time for " + std::to_string(n_steps) +
                         " steps, and then will pause.";
  logger_.LogVerbose(name_, info_msg.c_str());

  SimClock::Get()->ContinueForNBaseSteps(n_steps);

  if (wait_until_complete) {
    // TODO Add a thread check to prevent this wait loop causing a deadlock if
    // this method was called from the scene tick thread.
    logger_.LogVerbose(name_, "Waiting to reach target sim step.");
    while (!IsSimPaused()) {
      std::this_thread::yield();
    }
  }

  return SimClock::Get()->NowSimNanos();
}

TimeNano Scene::Impl::ContinueForSingleStep(bool wait_until_complete) {
  if (clock_settings_.type != ClockType::kSteppable) {
    logger_.LogError(name_,
                     "This clock type doesn't support ContinueForSingleStep.");
    throw Error("This clock type doesn't support ContinueForSingleStep.");
  }

  logger_.LogVerbose(name_, "Single step sim time and then pause.");

  SimClock::Get()->ContinueForSingleStep();

  if (wait_until_complete) {
    // TODO Add a thread check to prevent this wait loop causing a deadlock if
    // this method was called from the scene tick thread.
    logger_.LogVerbose(name_, "Waiting for sim step to complete.");
    while (!IsSimPaused()) {
      std::this_thread::yield();
    }
  }

  return SimClock::Get()->NowSimNanos();
}

std::vector<std::string> Scene::Impl::SimGetActors() {
  std::vector<std::string> actor_ids = {};
  for (const auto& actor : actors_) {
    const auto actor_id = actor->GetID();
    actor_ids.push_back(actor_id);
  }
  return actor_ids;
}

bool Scene::Impl::SetWindVelocity(float v_x, float v_y, float v_z) {
  Environment::wind_velocity = Vector3(v_x, v_y, v_z);
  UpdateWindVelocity();
  return true;
}

Vector3 Scene::Impl::GetWindVelocity() { return Environment::wind_velocity; }

std::string Scene::Impl::CallBackAfter(int t_secs) {
  std::this_thread::sleep_for(std::chrono::seconds(t_secs));
  const auto& msg = "Returning after" + std::to_string(t_secs) + " seconds.";
  return msg;
}

//! Helper method to register internal and external Service Methods
void Scene::Impl::RegisterServiceMethod(const ServiceMethod& method,
                                        MethodHandler method_handler) {
  auto method_name = method.GetName();
  auto params_list = method.GetParamsList();
  // Re-use topic path to create a unique name for the
  // service method; Added to keep the naming consistent throughout even if
  // there will atmost be only one instance of Scene
  auto unique_method_name = topic_path_ + "/" + method_name;
  auto unique_method = ServiceMethod(unique_method_name, params_list);

  service_manager_.RegisterMethod(unique_method, method_handler);
}

//! Register Service Methods offered by the Scene Class
void Scene::Impl::RegisterServiceMethods() {
  auto sim_get_clock_type = ServiceMethod("GetSimClockType", {""});
  auto sim_get_clock_type_handler = sim_get_clock_type.CreateMethodHandler(
      &Scene::Impl::SimGetClockType, *this);
  RegisterServiceMethod(sim_get_clock_type, sim_get_clock_type_handler);

  auto sim_get_time = ServiceMethod("GetSimTime", {""});
  auto sim_get_time_handler =
      sim_get_time.CreateMethodHandler(&Scene::Impl::SimGetTime, *this);
  RegisterServiceMethod(sim_get_time, sim_get_time_handler);

  auto sim_pause = ServiceMethod("Pause", {"do_pause"});
  auto sim_pause_handler =
      sim_pause.CreateMethodHandler(&Scene::Impl::SimPause, *this);
  RegisterServiceMethod(sim_pause, sim_pause_handler);

  auto is_sim_paused = ServiceMethod("IsPaused", {""});
  auto is_sim_paused_handler =
      is_sim_paused.CreateMethodHandler(&Scene::Impl::IsSimPaused, *this);
  RegisterServiceMethod(is_sim_paused, is_sim_paused_handler);

  auto continue_for_simtime = ServiceMethod(
      "ContinueForSimTime", {"delta_time", "wait_until_complete"});
  auto continue_for_simtime_handler = continue_for_simtime.CreateMethodHandler(
      &Scene::Impl::ContinueForSimTime, *this);
  RegisterServiceMethod(continue_for_simtime, continue_for_simtime_handler);

  auto continue_until_simtime = ServiceMethod(
      "ContinueUntilSimTime", {"target_time", "wait_until_complete"});
  auto continue_until_simtime_handler =
      continue_until_simtime.CreateMethodHandler(
          &Scene::Impl::ContinueUntilSimTime, *this);
  RegisterServiceMethod(continue_until_simtime, continue_until_simtime_handler);

  auto continue_for_n_steps =
      ServiceMethod("ContinueForNSteps", {"n_steps", "wait_until_complete"});
  auto continue_for_n_steps_handler = continue_for_n_steps.CreateMethodHandler(
      &Scene::Impl::ContinueForNSteps, *this);
  RegisterServiceMethod(continue_for_n_steps, continue_for_n_steps_handler);

  auto single_step =
      ServiceMethod("ContinueForSingleStep", {"wait_until_complete"});
  auto single_step_handler = single_step.CreateMethodHandler(
      &Scene::Impl::ContinueForSingleStep, *this);
  RegisterServiceMethod(single_step, single_step_handler);

  auto sim_get_actors = ServiceMethod("ListActors", {""});
  auto sim_get_actors_handler =
      sim_get_actors.CreateMethodHandler(&Scene::Impl::SimGetActors, *this);
  RegisterServiceMethod(sim_get_actors, sim_get_actors_handler);

  auto set_wind_vel = ServiceMethod("SetWindVelocity", {"v_x", "v_y", "v_z"});
  auto set_wind_vel_handler =
      set_wind_vel.CreateMethodHandler(&Scene::Impl::SetWindVelocity, *this);
  RegisterServiceMethod(set_wind_vel, set_wind_vel_handler);

  auto get_wind_vel = ServiceMethod("GetWindVelocity", {""});
  auto get_wind_vel_handler =
      get_wind_vel.CreateMethodHandler(&Scene::Impl::GetWindVelocity, *this);
  RegisterServiceMethod(get_wind_vel, get_wind_vel_handler);
}

void Scene::Impl::UnregisterAllServiceMethods() {
  service_manager_.UnregisterAllMethods();
}

// Main scene tick loop to move simulation robots over each sim clock time step
bool Scene::Impl::SceneTick() {
  if (state_manager_.IsDistributed() && !state_manager_.IsConnected()) {
    state_manager_.ConnectSims();
  }

  // 1. Step sim clock to start process of moving sim time forward
  if (clock_settings_.type == ClockType::kSteppable &&
      state_manager_.IsDistributed() && !state_manager_.IsClockSource()) {
    TimeNano sim_time = state_manager_.ReceiveSimTime();
    SimClock::Get()->SetNextSimTime(sim_time);
  }

  SimClock::Get()->Step();
  TimeNano new_sim_time = SimClock::Get()->NowSimNanos();
  TimeNano sim_dt_nanos = new_sim_time - sim_time_;
  sim_time_ = new_sim_time;

  if (clock_settings_.type == ClockType::kSteppable &&
      state_manager_.IsDistributed() && state_manager_.IsClockSource()) {
    state_manager_.SendSimTime(sim_time_);
  }

  // If sim time has advanced by a new dt, the world state is now "dirty" and
  // the physics world needs to advance to catch up to the new sim time.
  if (sim_dt_nanos > 0) {
    try {
      // Update the kinematics of each env_actor before robots so that they will
      // be in the right positions for the robots to sense them.
      for (auto& env_actor : env_actors_) {
        if (env_actor->GetType() == ActorType::kEnvActor) {
          env_actor->UpdateKinematics(sim_time_);
        }
      }

      for (auto& env_object : env_objects_) {
        if (env_object->GetType() == ActorType::kEnvObject) {
          TimeSec curr_time = SimClock::Get()->NanosToSec(sim_time_);
        }
      }

    } catch (const Error& e) {
      logger_.LogError(name_, "Error while updating EnvActor kinematics: %s",
                       e.what());
    } catch (const std::exception& e) {
      logger_.LogError(name_, "Error while updating EnvActor kinematics: %s",
                       e.what());
    } catch (...) {
      logger_.LogError(name_,
                       "Unknown exception while updating EnvActor kinematics");
    }

    try {
      // 2. Step physics:
      // Update each actor's corresponding physics body's kinematics to catch
      // up to the new sim time based on the wrenches that had been previously
      // set and applied over this dt period.
      std::function<void(TimeNano dt_nanos)> step_physics_func = nullptr;
      step_physics_func = physics_step_callback_;
      if (step_physics_func != nullptr) {
        // Physics Body's Wrench -> Kinematics
        step_physics_func(sim_dt_nanos);
      }

      // Send/receive physics state between distributed instances
      if (state_manager_.IsDistributed()) {
        state_manager_.SynchronizePhysicsState(sim_time_);
      }

      // 3. Environment and sensors:
      // After kinematics have been updated, update each actor's environment
      // to match the new state, and then update the sensor state to match the
      // new environments.
      for (auto& actor : actors_) {
        if (actor->GetType() == ActorType::kRobot) {
          auto& robot = static_cast<Robot&>(*actor);

          // Skip processing any non-local robots for distributed instances
          if (state_manager_.IsDistributed() &&
              !state_manager_.IsLocalRobot(robot.GetID())) {
            continue;
          }

          // Kinematics -> Environment Conditions
          // Don't pass time params, no time dependency for environment calcs
          robot.UpdateEnvironment();

          // Kin/Env -> Sensor Values
          // Pass sim_time_ for timestamps, sim_dt_nanos_ for filtering/noise
          robot.UpdateSensors(sim_time_, sim_dt_nanos);
        }
      }

      // After updating physics, environment, and sensor states, the world
      // state is now "clean" at t = sim_time_.

      // 4. Control and actuators:
      // After sensor outputs have been updated, the controllers can calculate
      // new commands to be applied at t = sim_time_ for the next dt period.
      // After control commands are updated, set the actuator outputs to
      // match.
      for (auto& actor : actors_) {
        if (actor->GetType() == ActorType::kRobot) {
          auto& robot = static_cast<Robot&>(*actor);

          // Skip processing any non-local robots for distributed instances
          if (state_manager_.IsDistributed() &&
              !state_manager_.IsLocalRobot(robot.GetID())) {
            continue;
          }

          // Sensor Values -> FC -> PWM
          // Don't pass time params, controllers generally manage their own time
          robot.UpdateControlInput();

          // PWM/Environment -> Thrust -> Wrench
          // Pass sim_time_ for timestamps, sim_dt_nanos_ for filtering/noise
          robot.UpdateActuators(sim_time_, sim_dt_nanos);
        }
      }

      // 5. Set wrenches on physics bodies
      // TODO Rename this to "SetActuationOutputs"
      // After the actuator outputs are updated, set their wrenches on the
      // physics bodies to be applied at the next physics step.
      std::function<void()> set_wrenches_physics_func = nullptr;
      set_wrenches_physics_func = physics_set_wrenches_callback_;
      if (step_physics_func != nullptr) {
        set_wrenches_physics_func();  // Store Wrench on physics bodies
      }

      // // This is just for test logging the ticks
      // if (sim_dt_nanos > 0) {
      //   auto& robot1 = static_cast<Robot&>(*(actors_[0]));
      //   auto kin = robot1.GetKinematics();
      //   auto rpy = TransformUtils::ToRPY(kin.pose.orientation);
      //   logger_.LogVerbose(
      //       name_,
      //       "simtime: %lld, (%f), "
      //       "xyz: %f, %f, %f, rpy: %f, %f, %f, "
      //       "vlin_xyz: %f, %f, %f, vang_xyz: %f, %f, %f",
      //       sim_time_, executor_.GetSleepTimeAve(), kin.pose.position.x(),
      //       kin.pose.position.y(), kin.pose.position.z(),
      //       TransformUtils::ToDegrees(rpy.x()),
      //       TransformUtils::ToDegrees(rpy.y()),
      //       TransformUtils::ToDegrees(rpy.z()), kin.twist.linear.x(),
      //       kin.twist.linear.y(), kin.twist.linear.z(),
      //       kin.twist.angular.x(), kin.twist.angular.y(),
      //       kin.twist.angular.z());
      // }  // end logging
    } catch (const Error& e) {
      logger_.LogError(name_, "Error occured while ticking scene: %s",
                       e.what());
      return false;  // respond as abnormal operation to scheduled executor
    } catch (const std::exception& e) {
      logger_.LogError(name_, "std::exception occured while ticking scene: %s",
                       e.what());
      return false;  // respond as abnormal operation to scheduled executor
    } catch (...) {
      logger_.LogError(name_, "Unknown exception occurred while ticking scene");
      return false;  // respond as abnormal operation to scheduled executor
    }
  }

  return true;  // respond as normal operation to scheduled executor
}

// -----------------------------------------------------------------------------
// class Scene::Loader

Scene::Loader::Loader(Scene::Impl& impl) : impl_(impl) {}

// Loads scene and actors straight from provided json instead of off the disk

void Scene::Loader::LoadSceneWithJSON(const json& json) {
  // Load scene ID
  LoadID(json);

  // Load home geo point for load_actors() to use it below
  LoadHomeGeoPoint(json);
  impl_.geodetic_converter_->setHome(impl_.home_geo_point_.geo_point.latitude,
                                     impl_.home_geo_point_.geo_point.longitude,
                                     impl_.home_geo_point_.geo_point.altitude);

  LoadWindSettings(json);
  // Load scene clock settings and reset singleton before actors use it
  LoadClockSettings(json);
  if (impl_.clock_settings_.type == ClockType::kRealTime) {
    // Initialize a real-time clock
    SimClock::Get(std::make_shared<RealTimeClock>());
  } else if (impl_.clock_settings_.type == ClockType::kSteppable) {
    // Initialize a steppable clock
    SimClock::Get(std::make_shared<SteppableClock>(impl_.clock_settings_.step));
    if (impl_.clock_settings_.pause_on_start) {
      SimClock::Get()->SimPause(true);
    }
  } else {
    // Default to steppable clock
    SimClock::Get(std::make_shared<SteppableClock>());
  }

  // Load segmentation settings for initializing scene object segmentation IDs
  LoadSegmentationSettings(json);

  // Load is-gis-scene, to know whether GIS tiles should be rendered
  LoadGISSettings(json);

  // Load all of the actors and env actors in the scene from their config JSON
  // files
  LoadActorsWithJSON(json);

  LoadEnvActorsWithJSON(json);

  LoadEnvObjectsWithJSON(json);

  LoadDistributedSimSettings(json);

  LoadVRMode(json);

  // TODO Move to LoadSimTopicCallbackSettings() if adding ability to configure
  // a set of topic paths to enable
  bool enable_sim_topic_callback = JsonUtils::GetInteger(
      json, Constant::Config::enable_sim_topic_callback, false);
  impl_.topic_manager_.SetTopicPublishedCallbackEnabled(
      enable_sim_topic_callback);

  impl_.is_loaded_ = true;
}

void Scene::Loader::UnloadScene() {
  impl_.topic_manager_.SetTopicPublishedCallbackEnabled(false);
  impl_.topic_manager_.SetCallbackTopicPublished(nullptr);

  // TODO Confirm all of robot objects are properly deleted
  impl_.actors_.clear();
  impl_.actors_ref_.clear();
  impl_.actor_index_by_id_.clear();
  impl_.env_actors_.clear();
  impl_.env_actors_ref_.clear();
  impl_.env_actor_index_by_id_.clear();
  impl_.trajectories_.clear();
  impl_.is_loaded_ = false;
}

void Scene::Loader::LoadID(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'id'.");

  impl_.id_ = JsonUtils::GetIdentifier(json, Constant::Config::id);
  impl_.SetTopicPath();

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'id' loaded.", impl_.id_.c_str());
}

void Scene::Loader::LoadActorsWithJSON(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'actors'.",
                           impl_.id_.c_str());
  auto actors_json = JsonUtils::GetArray(json, Constant::Config::actors);
  if (JsonUtils::IsEmptyArray(actors_json)) {
    impl_.logger_.LogVerbose(impl_.name_, "[%s] 'actors' missing or empty.",
                             impl_.id_.c_str());
  }

  std::transform(actors_json.begin(), actors_json.end(),
                 std::back_inserter(impl_.actors_),
                 [this](auto& json) { return LoadActorWithJSON(json); });

  impl_.actors_ref_ = ToRefs(impl_.actors_);

  for (int i = 0; i < impl_.actors_.size(); ++i) {
    impl_.actor_index_by_id_.emplace(impl_.actors_[i]->GetID(), i);
  }
}

void Scene::Loader::LoadEnvActorsWithJSON(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'Enviroment Actors'.",
                           impl_.id_.c_str());
  auto env_actors_json =
      JsonUtils::GetArray(json, Constant::Config::env_actors);
  if (JsonUtils::IsEmptyArray(env_actors_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s] 'environment-actors' missing or empty.",
                             impl_.id_.c_str());
  }
  std::transform(env_actors_json.begin(), env_actors_json.end(),
                 std::back_inserter(impl_.env_actors_),
                 [this](auto& json) { return LoadEnvActorWithJSON(json); });

  impl_.env_actors_ref_ = ToRefs(impl_.env_actors_);

  for (int i = 0; i < impl_.env_actors_.size(); ++i) {
    impl_.env_actor_index_by_id_.emplace(impl_.env_actors_[i]->GetID(), i);
  }
}

void Scene::Loader::LoadEnvObjectsWithJSON(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'Enviroment Objects'.",
                           impl_.id_.c_str());
  auto env_objects_json =
      JsonUtils::GetArray(json, Constant::Config::env_objects);
  if (JsonUtils::IsEmptyArray(env_objects_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s] 'environment-actors' missing or empty.",
                             impl_.id_.c_str());
  }
  std::transform(env_objects_json.begin(), env_objects_json.end(),
                 std::back_inserter(impl_.env_objects_),
                 [this](auto& json) { return LoadEnvObjectWithJSON(json); });

  impl_.env_objects_ref_ = ToRefs(impl_.env_objects_);

  for (int i = 0; i < impl_.env_objects_.size(); ++i) {
    impl_.env_object_index_by_id_.emplace(impl_.env_objects_[i]->GetID(), i);
  }

}

void Scene::Loader::LoadClockSettings(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'clock' settings.",
                           impl_.id_.c_str());

  auto clock_json = JsonUtils::GetJsonObject(json, Constant::Config::clock);
  if (JsonUtils::IsEmpty(clock_json)) {
    impl_.logger_.LogVerbose(
        impl_.name_,
        "[%s] 'clock' missing or empty. Using default clock settings.",
        impl_.id_.c_str());
    // Use clock_settings_ default values set from the ClockSettings struct
    // definition in clock.hpp
    return;
  }

  try {
    auto clock_type = JsonUtils::GetIdentifier(
        clock_json, Constant::Config::type, Constant::Config::steppable);

    if (clock_type == Constant::Config::steppable) {
      impl_.clock_settings_.type = ClockType::kSteppable;
    } else if (clock_type == Constant::Config::real_time) {
      impl_.clock_settings_.type = ClockType::kRealTime;
    } else {
      impl_.logger_.LogWarning(
          impl_.name_,
          "[%s] invalid 'clock' type. Using default clock settings.",
          impl_.id_.c_str());
      // Use clock_settings_ default values set from the ClockSettings struct
      // definition in clock.hpp
      return;
    }

    impl_.clock_settings_.step = JsonUtils::GetInteger(
        clock_json, Constant::Config::step, ClockBase::kDefaultStepNanos);

    impl_.clock_settings_.scene_tick_period = JsonUtils::GetInteger(
        clock_json, Constant::Config::real_time_update_rate,
        ClockBase::kDefaultStepNanos);

    impl_.clock_settings_.pause_on_start =
        JsonUtils::GetInteger(clock_json, Constant::Config::pause_on_start, 0);
  } catch (...) {
    throw;
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'clock' settings loaded.",
                           impl_.id_.c_str());
}

void Scene::Loader::LoadWindSettings(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'wind-settings'.",
                           impl_.id_.c_str());
  auto wind_settings = JsonUtils::GetJsonObject(json, Constant::Config::wind);
  if (JsonUtils::IsEmpty(wind_settings)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s] 'wind-settings' missing or empty. Using "
                             "default wind setting.",
                             impl_.id_.c_str());
    Environment::wind_velocity = Vector3::Zero();
    return;
  }

  try {
    auto wind_velocity =
        JsonUtils::GetVector3(wind_settings, Constant::Config::velocity);
    Environment::wind_velocity = wind_velocity;
  } catch (...) {
    throw;
  }
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'wind-settings' settings loaded.",
                           impl_.id_.c_str());
}
void Scene::Loader::LoadHomeGeoPoint(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'home-geo-point'.",
                           impl_.id_.c_str());

  auto home_pt_json =
      JsonUtils::GetJsonObject(json, Constant::Config::home_geo_point);
  if (JsonUtils::IsEmpty(home_pt_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s] 'home-geo-point' missing or empty. Using "
                             "default home geo point.",
                             impl_.id_.c_str());

    impl_.home_geo_point_ = HomeGeoPoint(GeoPoint(47.642101, -122.137001, 0.f));

    return;
  }

  try {
    auto latitude = JsonUtils::GetNumber<double>(
        home_pt_json, Constant::Config::latitude, 47.642101);

    auto longitude = JsonUtils::GetNumber<double>(
        home_pt_json, Constant::Config::longitude, -122.137001);

    auto altitude = JsonUtils::GetNumber<float>(
        home_pt_json, Constant::Config::altitude, 0.f);

    impl_.home_geo_point_ =
        HomeGeoPoint(GeoPoint(latitude, longitude, altitude));
  } catch (...) {
    throw;
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'clock' settings loaded.",
                           impl_.id_.c_str());
}

void Scene::Loader::LoadSegmentationSettings(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'segmentation' settings.",
                           impl_.id_.c_str());

  auto seg_json =
      JsonUtils::GetJsonObject(json, Constant::Config::segmentation);
  if (JsonUtils::IsEmpty(seg_json)) {
    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s] 'segmentation' missing or empty. Using "
                             "default segmentation settings.",
                             impl_.id_.c_str());
    return;
  }

  try {
    impl_.segmentation_settings_.initialize_ids = JsonUtils::GetBoolean(
        seg_json, Constant::Config::initialize_ids, false);

    impl_.segmentation_settings_.ignore_existing = JsonUtils::GetBoolean(
        seg_json, Constant::Config::ignore_existing, false);

    impl_.segmentation_settings_.use_owner_name =
        JsonUtils::GetBoolean(seg_json, Constant::Config::use_owner_name, true);
  } catch (...) {
    throw;
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'segmentation' settings loaded.",
                           impl_.id_.c_str());
}

void Scene::Loader::LoadGISSettings(const json& json) {
  // Resolve scene type
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'scene-type'.");
  // Get json value directly it uses the macro to deserialize into the enum
  impl_.scene_type_ =
      json.value(Constant::Config::scene_type, SceneType::kUnrealNative);
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'scene-type' loaded.",
                           impl_.id_.c_str());

  // Get tile directory if any
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'tiles-dir'.");
  impl_.tiles_dir_ =
      JsonUtils::GetString(json, Constant::Config::tiles_dir, "");
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'tiles-dir' loaded.",
                           impl_.id_.c_str());

  // Get horizon/DEM tile directory if any
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'horizon-tiles-dir'.");
  impl_.horizon_tiles_dir_ =
      JsonUtils::GetString(json, Constant::Config::horizon_tiles_dir, "");
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'horizon-tiles-dir' loaded.",
                           impl_.id_.c_str());

  // Get tile altitude offset if any
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'tiles-altitude-offset'.");
  impl_.tiles_altitude_offset_ = JsonUtils::GetNumber<float>(
      json, Constant::Config::tiles_altitude_offset, 0.f);
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'tiles-altitude-offset' loaded.",
                           impl_.id_.c_str());

  // Get tiles lod max if any
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'tiles-lod-max'.");
  impl_.tiles_lod_max_ =
      JsonUtils::GetInteger(json, Constant::Config::tiles_lod_max, 19);
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'tiles-lod-max' loaded.",
                           impl_.id_.c_str());

  // Get tiles lod min if any
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'tiles-lod-min'.");
  impl_.tiles_lod_min_ =
      JsonUtils::GetInteger(json, Constant::Config::tiles_lod_min, 13);
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'tiles-lod-min' loaded.",
                           impl_.id_.c_str());
}

std::unique_ptr<Actor> Scene::Loader::LoadActorWithJSON(const json& jsonIn) {
  auto id = GetActorID(jsonIn);
  auto type = GetActorType(jsonIn, id);
  auto origin = GetActorOrigin(jsonIn, id);
  // Read ref JSON data and write it as a string
  json robot_config;

  try {
    robot_config = JsonUtils::GetArray(jsonIn, Constant::Config::robot_config);
    impl_.logger_.LogVerbose(
        impl_.name_,
        "[%s][%s] Loading 'Enviroment Actor'. is_array [%d], is_object [%d]",
        impl_.id_.c_str(), id.c_str(), robot_config.is_array(),
        robot_config.is_object());
    //
    //  If env_actor_config is an array, update the first object using all the
    //  other objects, and then set env_actor_config to the final object
    //
    if (robot_config.is_array()) {
      auto final_config = robot_config[0];
      for (int i = 1; i < robot_config.size(); i++) {
        final_config.update(robot_config[i]);
      }
      robot_config = final_config;
      impl_.logger_.LogVerbose(
          impl_.name_,
          "[%s][%s] Merged 'Robot Actor'. is_array [%d], is_object [%d]",
          impl_.id_.c_str(), id.c_str(), robot_config.is_array(),
          robot_config.is_object());
      std::string test_output = robot_config.dump();
      impl_.logger_.LogVerbose(
          impl_.name_, "[%s][%s] Merged 'Robot Actor'. contents [%s]",
          impl_.id_.c_str(), id.c_str(), test_output.c_str());
    }
  } catch (...) {
    robot_config =
        JsonUtils::GetJsonObject(jsonIn, Constant::Config::robot_config);
  }

  auto physics_connection_json =
      JsonUtils::GetJsonObject(jsonIn, Constant::Config::physics_connection);

  auto control_connection_json =
      JsonUtils::GetJsonObject(jsonIn, Constant::Config::control_connection);

  bool start_landed_flag =
      JsonUtils::GetBoolean(jsonIn, Constant::Config::start_landed, false);

  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] Loading 'actor'.",
                           impl_.id_.c_str(), id.c_str());

  if (type == Constant::Config::robot) {
    auto robot =
        new Robot(id, origin, impl_.logger_, impl_.topic_manager_,
                  impl_.topic_path_ + "/robots", impl_.service_manager_,
                  impl_.state_manager_, impl_.home_geo_point_);
    robot->Load(robot_config);
    robot->SetPhysicsConnectionSettings(physics_connection_json.dump());
    robot->SetControlConnectionSettings(control_connection_json.dump());
    robot->SetStartLanded(start_landed_flag);

    // Register robot's current, home, and child reference frames
    impl_.GetTransformTree()->Register(*robot, TransformTree::kRefFrameGlobal);
    impl_.GetTransformTree()->Register(robot->GetHomeRefFrame(),
                                       TransformTree::kRefFrameGlobal);
    robot->RegisterChildTransformNodes(*impl_.GetTransformTree());
    // impl_.GetTransformTree()->Dump();

    return std::unique_ptr<Actor>(robot);

  } else {
    impl_.logger_.LogError(impl_.name_, "[%s] Invalid actor type '%s'.",
                           impl_.id_.c_str(), type.c_str());
    throw Error("Invalid actor type.");
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] 'actor' loaded.",
                           impl_.id_.c_str(), id.c_str());
}

std::unique_ptr<EnvActor> Scene::Loader::LoadEnvActorWithJSON(
    const json& jsonIn) {
  auto id = GetActorID(jsonIn);
  auto type = GetActorType(jsonIn, id);
  auto origin = GetActorOrigin(jsonIn, id);
  json env_actor_config;

  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] Loading 'Enviroment Actor'.",
                           impl_.id_.c_str(), id.c_str());

  try {
    env_actor_config =
        JsonUtils::GetArray(jsonIn, Constant::Config::env_actor_config);
    impl_.logger_.LogVerbose(
        impl_.name_,
        "[%s][%s] Loading 'Enviroment Actor'. is_array [%d], is_object [%d]",
        impl_.id_.c_str(), id.c_str(), env_actor_config.is_array(),
        env_actor_config.is_object());
    //
    //  If env_actor_config is an array, update the first object using all the
    //  other objects, and then set env_actor_config to the final object
    //
    if (env_actor_config.is_array()) {
      auto final_config = env_actor_config[0];
      for (int i = 1; i < env_actor_config.size(); i++) {
        final_config.update(env_actor_config[i]);
      }
      env_actor_config = final_config;
      impl_.logger_.LogVerbose(
          impl_.name_,
          "[%s][%s] Merged 'Enviroment Actor'. is_array [%d], is_object [%d]",
          impl_.id_.c_str(), id.c_str(), env_actor_config.is_array(),
          env_actor_config.is_object());
      std::string test_output = env_actor_config.dump();
      impl_.logger_.LogVerbose(
          impl_.name_, "[%s][%s] Merged 'Enviroment Actor'. contents [%s]",
          impl_.id_.c_str(), id.c_str(), test_output.c_str());
    }
  } catch (...) {
    env_actor_config =
        JsonUtils::GetJsonObject(jsonIn, Constant::Config::env_actor_config);
  }

  if (type == Constant::Config::env_actor) {
    auto env_actor =
        new EnvActor(id, origin, impl_.logger_, impl_.topic_manager_,
                     impl_.topic_path_ + "/env_actors", impl_.service_manager_,
                     impl_.state_manager_);

    env_actor->Load(env_actor_config);

    std::shared_ptr<Trajectory> traj_ptr = env_actor->GetTrajectoryPtr();
    if (traj_ptr) impl_.trajectories_.push_back(traj_ptr);

    return std::unique_ptr<EnvActor>(env_actor);
  } else if (type == Constant::Config::env_car) {
    auto env_car = new EnvCar(id, origin, impl_.logger_, impl_.topic_manager_,
                              impl_.topic_path_ + "/env_actors",
                              impl_.service_manager_, impl_.state_manager_);

    env_car->Load(env_actor_config);

    std::shared_ptr<Trajectory> traj_ptr = env_car->GetTrajectoryPtr();
    if (traj_ptr) impl_.trajectories_.push_back(traj_ptr);

    return std::unique_ptr<EnvActor>(env_car);
  } else if (type == Constant::Config::env_human) {
    auto env_human =
        new EnvActorGrounded(id, origin, impl_.logger_, impl_.topic_manager_,
                             impl_.topic_path_ + "/env_actors",
                             impl_.service_manager_, impl_.state_manager_);

    env_human->Load(env_actor_config);

    std::shared_ptr<Trajectory> traj_ptr = env_human->GetTrajectoryPtr();
    if (traj_ptr) impl_.trajectories_.push_back(traj_ptr);

    return std::unique_ptr<EnvActor>(env_human);
  } else {
    impl_.logger_.LogError(impl_.name_,
                           "[%s] Invalid Enviroment Actor type '%s'.",
                           impl_.id_.c_str(), type.c_str());
    throw Error("Invalid actor type.");

    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s][%s] 'Environment Actor' loaded.",
                             impl_.id_.c_str(), id.c_str());
  }
}

std::unique_ptr<EnvObject> Scene::Loader::LoadEnvObjectWithJSON(
    const json& jsonIn) {
  auto id = GetActorID(jsonIn);
  auto type = GetActorType(jsonIn, id);
  auto origin = GetActorOrigin(jsonIn, id);
  json env_object_config;

  impl_.logger_.LogVerbose(impl_.name_,
                           "[%s][%s] Loading 'Enviroment Object'.",
                           impl_.id_.c_str(), id.c_str());

  try {
    env_object_config =
        JsonUtils::GetArray(jsonIn, Constant::Config::env_object_config);
    impl_.logger_.LogVerbose(
        impl_.name_,
        "[%s][%s] Loading 'Enviroment Actor'. is_array [%d], is_object [%d]",
        impl_.id_.c_str(), id.c_str(), env_object_config.is_array(),
        env_object_config.is_object());
    //
    //  If env_object_config is an array, update the first object using all
    //  the other objects, and then set env_object_config to the final object
    //
    if (env_object_config.is_array()) {
      auto final_config = env_object_config[0];
      for (int i = 1; i < env_object_config.size(); i++) {
        final_config.update(env_object_config[i]);
      }
      env_object_config = final_config;
      impl_.logger_.LogVerbose(
          impl_.name_,
          "[%s][%s] Merged 'Enviroment Object'. is_array [%d], is_object [%d]",
          impl_.id_.c_str(), id.c_str(), env_object_config.is_array(),
          env_object_config.is_object());
      std::string test_output = env_object_config.dump();
      impl_.logger_.LogVerbose(
          impl_.name_, "[%s][%s] Merged 'Enviroment Object'. contents [%s]",
          impl_.id_.c_str(), id.c_str(), test_output.c_str());
    }
  } catch (...) {
    env_object_config =
        JsonUtils::GetJsonObject(jsonIn, Constant::Config::env_object_config);
  }

  if (type == Constant::Config::env_particle_effect) {
    auto env_object =
        new EnvObject(id, origin, impl_.logger_, impl_.topic_manager_,
                       impl_.topic_path_ + "/env_objects",
                       impl_.service_manager_, impl_.state_manager_);

    env_object->Load(env_object_config);

    return std::unique_ptr<EnvObject>(env_object);
  } else {
    impl_.logger_.LogError(impl_.name_,
                           "[%s] Invalid Enviroment Object type '%s'.",
                           impl_.id_.c_str(), type.c_str());
    throw Error("Invalid actor type.");

    impl_.logger_.LogVerbose(impl_.name_,
                             "[%s][%s] 'Environment Object' loaded.",
                             impl_.id_.c_str(), id.c_str());
  }
}

void Scene::Loader::LoadDistributedSimSettings(const json& json) {
  impl_.state_manager_.Load(json);
}

std::string Scene::Loader::GetActorID(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'actor.name'.",
                           impl_.id_.c_str());
  auto name = JsonUtils::GetIdentifier(json, Constant::Config::name);
  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] 'actor.name' loaded.",
                           impl_.id_.c_str(), name.c_str());

  return name;
}

void Scene::Loader::LoadVRMode(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Loading 'vr-mode'.",
                           impl_.id_.c_str());
  impl_.vr_mode_ = JsonUtils::GetBoolean(json, Constant::Config::vr_mode);
  impl_.logger_.LogVerbose(impl_.name_, "[%s][%d] 'vr-mode' loaded.",
                           impl_.id_.c_str(), impl_.vr_mode_);
}

std::string Scene::Loader::GetActorType(const json& json,
                                        const std::string& actor_id) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] Loading 'actor.type'.",
                           impl_.id_.c_str(), actor_id.c_str());
  auto type = JsonUtils::GetIdentifier(json, Constant::Config::type);
  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] 'actor.type' loaded.",
                           impl_.id_.c_str(), actor_id.c_str());

  return type;
}

Transform Scene::Loader::GetActorOrigin(const json& json,
                                        const std::string& actor_id) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] Loading 'actor.origin'.",
                           impl_.id_.c_str(), actor_id.c_str());
  auto origin = JsonUtils::GetTransform(json, Constant::Config::origin,
                                        impl_.home_geo_point_.geo_point);
  impl_.logger_.LogVerbose(impl_.name_, "[%s][%s] 'actor.origin' loaded.",
                           impl_.id_.c_str(), actor_id.c_str());

  return origin;
}

}  // namespace projectairsim
}  // namespace microsoft
