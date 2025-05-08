// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SCENE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SCENE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/actor.hpp"
#include "core_sim/actor/env_actor.hpp"
#include "core_sim/actor/env_object.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/service_method.hpp"
#include "core_sim/trajectory.hpp"
#include "core_sim/transforms/transform_tree.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class TopicManager;
class ServiceManager;
class GeodeticConverter;
class StateManager;
class Logger;
class ViewportCamera;

struct SegmentationSettings {
  bool initialize_ids = false;
  bool ignore_existing = false;
  bool use_owner_name = true;
};

enum SceneType {
  kUnrealNative = 0,
  kCustomGIS = 1,
  kCesiumGIS = 2,
  kBlackShark = 3
};

NLOHMANN_JSON_SERIALIZE_ENUM(SceneType, {{kUnrealNative, "UnrealNative"},
                                         {kCustomGIS, "CustomGIS"},
                                         {kCesiumGIS, "CesiumGIS"},
                                         {kBlackShark, "BlackShark"}})

class Scene {
 public:
  Scene();

  bool IsLoaded();

  const std::string& GetID() const;

  const std::vector<std::reference_wrapper<Actor>>& GetActors() const;

  int GetActorIndex(const std::string& actor_id);

  int GetEnvActorIndex(const std::string& env_actor_name);

  int GetEnvObjectIndex(const std::string& env_object_name);

  std::shared_ptr<Trajectory> GetTrajectoryPtrByName(
      const std::string& traj_name);

  const std::vector<std::reference_wrapper<EnvActor>>& GetEnvActors() const;

  const std::vector<std::reference_wrapper<EnvObject>>& GetEnvObjects() const;

  void SetCallbackPhysicsStart(const std::function<void()>& callback);

  void SetCallbackPhysicsSetWrenches(const std::function<void()>& callback);

  void SetCallbackPhysicsStep(const std::function<void(TimeNano)>& callback);

  void SetCallbackPhysicsStop(const std::function<void()>& callback);

  void SetCallbackTopicPublished(
      const std::function<void(const std::string&, const MessageType&,
                               const std::string&)>& callback);

  const ClockSettings& GetClockSettings() const;

  const SegmentationSettings& GetSegmentationSettings() const;

  const bool GetVRMode() const;

  const HomeGeoPoint& GetHomeGeoPoint() const;

  std::shared_ptr<ViewportCamera>& GetViewportCamera();

  void EnableViewportCamera(bool enable);

  const SceneType GetSceneType() const;

  const bool IsSimTopicCallbackEnabled() const;

  const std::string GetTilesDirectory() const;

  const std::string GetHorizonTilesDirectory() const;

  const float GetTilesAltitudeOffset() const;

  const int GetTilesLodMax() const;

  const int GetTilesLodMin() const;

  TransformTree* GetTransformTree();

  std::shared_ptr<GeodeticConverter> GetGeodeticConverter();

  const Vector3 GetWindVelocity() const;

  void UpdateWindVelocity();

  void AddNEDTrajectory(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& pose_x, const std::vector<float>& pose_y,
      const std::vector<float>& pose_z, const std::vector<float>& pose_roll,
      const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
      const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
      const std::vector<float>& vel_z_lin);

  void RegisterServiceMethod(const ServiceMethod& method,
                             MethodHandler method_handler);

  void SetCallbackWindVelocityUpdated(
      const std::function<void(const Vector3&)>& callback);
  // SetCallbackEnableUnrealViewportCamera
  void SetCallbackEnableUnrealViewportCamera(
      const std::function<void(bool)>& callback);

 private:
  friend class Simulator;

  Scene(const Logger& logger, const TopicManager& topic_manager,
        const std::string& parent_topic_path,
        const ServiceManager& service_manager,
        const StateManager& state_manager);

  void LoadWithJSON(ConfigJson config_json);

  void Unload();

  void Start();

  void Stop();

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SCENE_HPP_
