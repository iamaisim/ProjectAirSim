// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "sim_unity_wrapper.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

#include "core_sim/actor/robot.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/geodetic_converter.hpp"
#include "core_sim/message/image_message.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/service_method.hpp"
#include "nng/nng.h"
#include "tile_manager.hpp"

namespace projectairsim = microsoft::projectairsim;
namespace rendering = microsoft::projectairsim::rendering::scene;

void LoggingCallback(const std::string& component,
                     projectairsim::LogLevel level,
                     const std::string& message) {
  std::cout << "[" << component << "]"
            << " " << message << std::endl;
}

bool LoadServer(int topics_port, int services_port) {
  if (simserver != nullptr) {
    delete simserver;
    simserver = nullptr;
  }

  simserver = new projectairsim::SimServer(LoggingCallback,
                                           projectairsim::LogLevel::kVerbose);

  std::cout << "Loading simulator with topics_port=" << topics_port
            << ", services_port=" << services_port << "..." << std::endl;
  simserver->LoadSimulator(topics_port, services_port);

  std::cout << "Loading default scene..." << std::endl;
  simserver->LoadScene();

  return true;
}

bool StartServer() {
  if (simserver == nullptr) return false;

  std::cout << "Starting simulator..." << std::endl;
  simserver->StartSimulator();

  std::cout << "Starting scene..." << std::endl;
  simserver->StartScene();

  std::cout << "Simulation is now running." << std::endl;
  return true;
}

bool StopServer() {
  if (simserver == nullptr) return false;

  std::cout << "Stopping scene..." << std::endl;
  simserver->StopScene();

  std::cout << "Stopping simulator..." << std::endl;
  simserver->StopSimulator();

  // Deallocate static simserver object pointer
  delete simserver;
  simserver = nullptr;

  // Release all NNG resources to prevent hanging on application close
  nng_fini();

  std::cout << "Simulation is now stopped." << std::endl;
  return true;
}

void SetCallbackLoadExternalScene(void (*load_unreal_scene_func)()) {
  if (simserver == nullptr) return;
  simserver->SetCallbackLoadExternalScene(load_unreal_scene_func);
}

void SetCallbackStartExternalScene(void (*start_unreal_scene_func)()) {
  if (simserver == nullptr) return;
  simserver->SetCallbackStartExternalScene(start_unreal_scene_func);
}

void SetCallbackStopExternalScene(void (*stop_unreal_scene_func)()) {
  if (simserver == nullptr) return;
  simserver->SetCallbackStopExternalScene(stop_unreal_scene_func);
}

void SetCallbackUnloadExternalScene(void (*unload_unreal_scene_func)()) {
  if (simserver == nullptr) return;
  simserver->SetCallbackUnloadExternalScene(unload_unreal_scene_func);
}

const char* GetSceneConfigJSON() {
  if (simserver == nullptr) return "";

  const std::string& scene_config = simserver->GetSceneConfigJSON();
  return scene_config.c_str();
}

int GetActorIndex(const char* actor_name) {
  if (simserver == nullptr) {
    // TODO Log warning
    return -1;
  }

  auto& scene = simserver->GetScene();
  int actor_idx = scene.GetActorIndex(actor_name);
  return actor_idx;
}

bool GetRobotByActorIndex(int actor_index, projectairsim::Robot& out_robot) {
  if (simserver == nullptr) {
    // TODO Log warning
    return false;
  }

  auto& actors = simserver->GetScene().GetActors();
  if (actor_index < 0 || actor_index >= actors.size()) {
    // TODO Log warning
    return false;
  }

  auto& actor = actors[actor_index];
  if (actor.get().GetType() == projectairsim::ActorType::kRobot) {
    out_robot = static_cast<projectairsim::Robot&>(actor.get());
    return true;
  }

  // TODO Log warning
  return false;
}

int GetSensorIndex(int actor_index, const char* sensor_name) {
  if (simserver == nullptr) {
    // TODO Log warning
    return -1;
  }

  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    return robot.GetSensorIndex(sensor_name);
  } else {
    // TODO Log warning
    return -1;
  }
}

void InvokeCollisionDetection(
    int actor_index, UnityInterop::InteropCollisionInfo collision_info) {
  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    projectairsim::CollisionInfo collisionInfo =
        UnityInterop::ToSimCollisionInfo(collision_info);
    robot.UpdateCollisionInfo(collisionInfo);
  } else {
    // TODO Log warning
  }
}

void SetRobotKinematicsCallback(
    int actor_index,
    bool (*set_unity_robot_kinematics_func)(UnityInterop::InteropKinematics kin,
                                            int64_t timestamp)) {
  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    robot.SetCallbackKinematicsUpdated(
        [set_unity_robot_kinematics_func](const projectairsim::Kinematics& kin,
                                          TimeNano timestamp) {
          if (set_unity_robot_kinematics_func == nullptr) return;
          set_unity_robot_kinematics_func(kin, timestamp);
        });
  } else {
    // TODO Log warning
  }
}

void SetActuatedTransformCallback(
    int actor_index,
    bool (*set_unity_robot_actuated_transform_func)(
        UnityInterop::InteropActuatedTransform actuated_transform,
        int64_t timestamp)) {
  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    robot.SetCallbackActuatorOutputUpdated(
        [set_unity_robot_actuated_transform_func](
            const projectairsim::ActuatedTransforms& actuated_transforms,
            TimeNano timestamp) {
          if (set_unity_robot_actuated_transform_func == nullptr) return;

          for (auto& pair : actuated_transforms) {
            UnityInterop::InteropActuatedTransform interop_actuated_transform;
            auto translation = pair.second.affine3.translation();
            auto& pos = interop_actuated_transform.actuated_transform.position;
            pos.x = translation.x();
            pos.y = translation.y();
            pos.z = translation.z();

            auto quat =
                projectairsim::Quaternion(pair.second.affine3.rotation());
            auto& ori =
                interop_actuated_transform.actuated_transform.orientation;
            ori.w = quat.w();
            ori.x = quat.x();
            ori.y = quat.y();
            ori.z = quat.z();

            interop_actuated_transform.apply_order = pair.second.applyorder;

            interop_actuated_transform.actuated_link_id = pair.first.c_str();

            set_unity_robot_actuated_transform_func(interop_actuated_transform,
                                                    timestamp);
          }
        });
  } else {
    // TODO Log warning
  }
}

void SetRobotHasCollided(int actor_index, bool has_collided) {
  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    robot.SetHasCollided(has_collided);
  } else {
    // TODO Log warning
  }
}

bool GetCameraByIndex(int actor_index, int sensor_index,
                      projectairsim::Camera& out_camera) {
  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    auto& sensors = robot.GetSensors();
    if (sensor_index < 0 || sensor_index >= sensors.size()) {
      return false;
    }
    auto& sensor = sensors[sensor_index];
    if (sensor.get().GetType() == projectairsim::SensorType::kCamera) {
      out_camera = static_cast<projectairsim::Camera&>(sensor.get());
      return true;
    }
  }
  return false;
}

template <typename TSensorClass>
bool GetSensorByIndex(int actor_index, int sensor_index,
                      TSensorClass& out_sensor) {
  projectairsim::Robot robot;
  if (GetRobotByActorIndex(actor_index, robot)) {
    auto& sensors = robot.GetSensors();
    if (sensor_index < 0 || sensor_index >= sensors.size()) {
      return false;
    }

    auto& sensor = sensors[sensor_index];
    out_sensor = static_cast<TSensorClass&>(sensor.get());
    return true;
  }

  return false;
}

// TODO: support other kinds of captures
void PublishImages(int actor_index, int sensor_index,
                   UnityInterop::InteropImageMessage* interop_image_messages) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::map<projectairsim::ImageType, projectairsim::ImageMessage> imageMessages;
  TimeNano timestamp;

  for (int i = 0; i < projectairsim::MathUtils::ToNumeric(
                          projectairsim::ImageType::kCount);
       ++i) {
    if (interop_image_messages[i].image_data_uint) {  // if data's not empty
      auto imageType =
          projectairsim::MathUtils::ToEnum<projectairsim::ImageType>(i);
      auto sim_image_message =
          UnityInterop::ToSimImageMessage(interop_image_messages[i], imageType);
      imageMessages.emplace(imageType, sim_image_message);
      timestamp = sim_image_message.GetTimestamp();
    }
  }

  projectairsim::Camera camera;
  if (GetCameraByIndex(actor_index, sensor_index, camera)) {
    camera.PublishImages(std::move(imageMessages));
  } else {
    // TODO: log warning
  }
}

void AddCameraRequestToCaptureQueue(int actor_index, int sensor_index,
                                    int64_t captured_time) {
  projectairsim::Camera camera;
  if (GetCameraByIndex(actor_index, sensor_index, camera)) {
    camera.AddRequestToCaptureQueue(captured_time);
  } else {
    // TODO: log warning
  }
}

bool IsCameraCaptureQueueFull(int actor_index, int sensor_index) {
  projectairsim::Camera camera;
  if (GetCameraByIndex(actor_index, sensor_index, camera)) {
    return camera.IsCaptureQueueFull();
  } else {
    // TODO: log warning
    return false;
  }
}

int64_t GetSimTimeNanos() {
  if (simserver == nullptr) {
    // TODO Log warning
    return -1;
  }

  return projectairsim::SimClock::Get()->NowSimNanos();
}

void PublishLidarData(int actor_index, int sensor_index,
                      UnityInterop::InteropLidarMessage interop_lidar_message) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  projectairsim::Lidar lidar;
  if (GetSensorByIndex<projectairsim::Lidar>(actor_index, sensor_index,
                                             lidar)) {
    projectairsim::LidarMessage lidar_msg =
        UnityInterop::ToSimLidarMessage(interop_lidar_message);
    lidar.PublishLidarMsg(lidar_msg);
  } else {
    // TODO: log warning
  }
}

void EcefToNed(const double x, const double y, const double z, double* north,
               double* east, double* down) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  auto geo_converter = simserver->GetScene().GetGeodeticConverter();
  geo_converter->ecef2Ned(x, y, z, north, east, down);
}

void GetHomeGeoPointInECEF(double* x, double* y, double* z) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  auto home = simserver->GetScene().GetHomeGeoPoint().geo_point;
  auto geo_converter = simserver->GetScene().GetGeodeticConverter();
  geo_converter->geodetic2Ecef(home.latitude, home.longitude, home.altitude, x,
                               y, z);
}

double* GetEcefToNeuRotationMatrix() {
  if (simserver == nullptr) {
    // TODO Log warning
    return nullptr;
  }

  auto home = simserver->GetScene().GetHomeGeoPoint().geo_point;
  auto geo_converter = simserver->GetScene().GetGeodeticConverter();
  auto sourceArr = geo_converter->getEcefToNeuRotationMatrix().data();
  auto destArr = new double[9];
  std::copy_n(sourceArr, 9, destArr);
  return destArr;
}

int* GetTileKeysToRender(int* size) {
  if (simserver == nullptr) {
    // TODO Log warning
    return 0;
  }

  auto tileManager = simserver->GetTileManager();
  if (tileManager == nullptr) {
    *size = 0;
    return nullptr;
  }

  auto tileKeysVector = tileManager->GetTileKeysToRender();
  const int tileKeysArrSize = tileKeysVector.size() * 3;
  auto outTileKeys = new int[tileKeysArrSize];

  if (outTileKeys) {
    int arrayIndex = 0;
    for (const auto& tileKey : tileKeysVector) {
      outTileKeys[arrayIndex++] = tileKey.x;
      outTileKeys[arrayIndex++] = tileKey.y;
      outTileKeys[arrayIndex++] = tileKey.lod;
    }
  }

  *size = tileKeysArrSize;
  return outTileKeys;
}

bool CameraIsPoseUpdatePending(int actor_index, int sensor_index) {
  if (simserver == nullptr) {
    // TODO Log warning
    return false;
  }

  projectairsim::Robot robot;
  projectairsim::Camera camera;
  if (GetRobotByActorIndex(actor_index, robot)) {
    if (GetSensorByIndex<projectairsim::Camera>(actor_index, sensor_index, camera))
      return camera.IsPoseUpdatePending();
  }
  
  // TODO Log warning
  return false;
}

const char* CameraGetLookAtObject(int actor_index, int sensor_index) {
   if (simserver == nullptr) {
    // TODO Log warning
    return "";
  }

  projectairsim::Robot robot;
  projectairsim::Camera camera;
  if (GetRobotByActorIndex(actor_index, robot)) {
    if (GetSensorByIndex<projectairsim::Camera>(actor_index, sensor_index, camera))
    {
      const char* result = camera.GetLookAtObject().c_str();
      return result;
    }
  }
  
  // TODO Log warning
  return "";
}

bool CameraResetPose(int actor_index, int sensor_index, bool wait_for_pose_update) {
  if (simserver == nullptr) {
    // TODO Log warning
    return false;
  }

  projectairsim::Robot robot;
  projectairsim::Camera camera;
  if (GetRobotByActorIndex(actor_index, robot)) {
    if (GetSensorByIndex<projectairsim::Camera>(actor_index, sensor_index, camera))
      return camera.ResetCameraPose(wait_for_pose_update);
  }
  
  // TODO Log warning
  return false;
}

void CameraMarkPoseUpdateAsCompleted(int actor_index, int sensor_index) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  projectairsim::Robot robot;
  projectairsim::Camera camera;
  if (GetRobotByActorIndex(actor_index, robot)) {
    if (GetSensorByIndex<projectairsim::Camera>(actor_index, sensor_index, camera))
      camera.MarkPoseUpdateAsCompleted();
  }
  
  // TODO Log warning
}

UnityInterop::InteropPose CameraGetDesiredPose(int actor_index, int sensor_index) {
  if (simserver == nullptr) {
    // TODO Log warning
    return UnityInterop::InteropPose();
  }

  projectairsim::Robot robot;
  projectairsim::Camera camera;
  if (GetRobotByActorIndex(actor_index, robot)) {
    if (GetSensorByIndex<projectairsim::Camera>(actor_index, sensor_index, camera))
    {
      const projectairsim::Transform& transformPose = camera.GetDesiredPose();
      UnityInterop::InteropPose pose = UnityInterop::InteropPose(transformPose);
      return pose;
    }
  }
  
  // TODO Log warning
  return UnityInterop::InteropPose();
}

void SetSpawnObjectCallback(const char* (*spawn_object_unity)(
    const char* object_name, const char* asset_path,
    UnityInterop::InteropPose pose, UnityInterop::InteropVector3 scale)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<const char*(const std::string&, const std::string&,
                            const projectairsim::Transform&,
                            const std::vector<float>&, bool)>
      spawn_object_wrapper =
          [spawn_object_unity](
              const std::string& object_name, const std::string& asset_path,
              const projectairsim::Transform& pose,
              const std::vector<float>& scale, bool enable_physics) {
            return spawn_object_unity(
                object_name.c_str(), asset_path.c_str(),
                UnityInterop::InteropPose(pose),
                UnityInterop::InteropVector3(scale[0], scale[1], scale[2]));
          };

  auto& scene = simserver->GetScene();
  auto spawn_object_method = projectairsim::ServiceMethod(
      "SpawnObject",
      {"object_name", "asset_path", "pose", "scale", "enable_physics"});
  auto spawn_object_handler =
      spawn_object_method.CreateMethodHandler(spawn_object_wrapper);
  scene.RegisterServiceMethod(spawn_object_method, spawn_object_handler);
}

void SetSpawnObjectAtGeoCallback(const char* (*spawn_object_unity)(
    const char* object_name, const char* asset_path,
    UnityInterop::InteropPose pose, UnityInterop::InteropVector3 scale)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  auto& scene = simserver->GetScene();
  auto geo_converter = scene.GetGeodeticConverter();
  std::function<const char*(
      const std::string&, const std::string&, const double, const double,
      const float, const std::vector<float>&, const std::vector<float>&, bool)>
      spawn_object_at_geo_wrapper =
          [spawn_object_unity, geo_converter](
              const std::string& object_name, const std::string& asset_path,
              const double lat, const double lon, const float alt,
              const std::vector<float>& rotation,
              const std::vector<float>& scale, bool enable_physics) {
            // Convert geo coordinate to pose
            double n, e, d;
            geo_converter->geodetic2Ned(lat, lon, alt, &n, &e, &d);
            auto translation = projectairsim::Vector3(n, e, d);
            auto pose = projectairsim::Pose(
                translation,
                projectairsim::Quaternion(rotation[0], rotation[1], rotation[2],
                                          rotation[3]));

            return spawn_object_unity(
                object_name.c_str(), asset_path.c_str(),
                UnityInterop::InteropPose(pose),
                UnityInterop::InteropVector3(scale[0], scale[1], scale[2]));
          };

  auto spawn_object_at_geo_method = projectairsim::ServiceMethod(
      "spawnObjectAtGeo", {"object_name", "asset_path", "latitude", "longitude",
                           "altitude", "rotation", "scale", "enable_physics"});
  auto spawn_object_at_geo_handler =
      spawn_object_at_geo_method.CreateMethodHandler(
          spawn_object_at_geo_wrapper);
  scene.RegisterServiceMethod(spawn_object_at_geo_method,
                              spawn_object_at_geo_handler);
}

void SetDestroyObjectCallback(
    bool (*destroy_object_unity)(const char* object_name)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(const std::string&)> destroy_object_wrapper =
      [destroy_object_unity](const std::string& object_name) {
        return destroy_object_unity(object_name.c_str());
      };

  auto& scene = simserver->GetScene();
  auto destroy_object_method =
      projectairsim::ServiceMethod("DestroyObject", {"object_name"});
  auto destroy_object_handler =
      destroy_object_method.CreateMethodHandler(destroy_object_wrapper);
  scene.RegisterServiceMethod(destroy_object_method, destroy_object_handler);
}

void SetEnableWeatherVisualEffectsCallback(
    bool (*enable_weather_effects_unity)(bool)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(bool)> enable_weather_effects_wrapper =
      [enable_weather_effects_unity](bool status) {
        return enable_weather_effects_unity(status);
      };

  auto& scene = simserver->GetScene();
  auto enable_weather_effects_method = projectairsim::ServiceMethod(
      "SimSetWeatherVisualEffectsStatus", {"status"});
  auto enable_weather_effects_handler =
      enable_weather_effects_method.CreateMethodHandler(
          enable_weather_effects_wrapper);
  scene.RegisterServiceMethod(enable_weather_effects_method,
                              enable_weather_effects_handler);
}

void SetSetWeatherVisualEffectsParamCallback(
    bool (*set_weather_param_unity)(int param, float value)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(int, float)> set_weather_param_wrapper =
      [set_weather_param_unity](int param, float value) {
        return set_weather_param_unity(param, value);
      };

  auto& scene = simserver->GetScene();
  auto set_weather_param_method = projectairsim::ServiceMethod(
      "SetWeatherVisualEffectsParameter", {"param", "value"});
  auto set_weather_param_handler =
      set_weather_param_method.CreateMethodHandler(set_weather_param_wrapper);
  scene.RegisterServiceMethod(set_weather_param_method,
                              set_weather_param_handler);
}

void SetResetWeatherEffectsCallback(bool (*reset_weather_effects_unity)()) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(void)> reset_weather_effects_wrapper =
      [reset_weather_effects_unity]() { return reset_weather_effects_unity(); };

  auto& scene = simserver->GetScene();
  auto reset_weather_effects_method =
      projectairsim::ServiceMethod("ResetWeatherEffects", {});
  auto reset_weather_effects_handler =
      reset_weather_effects_method.CreateMethodHandler(
          reset_weather_effects_wrapper);
  scene.RegisterServiceMethod(reset_weather_effects_method,
                              reset_weather_effects_handler);
}

void SetSetSunPositionFromDateTimeCallback(bool (*set_sun_pos_unity)(
    const char* datetime, const char* format, bool is_dst)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(const std::string&, const std::string&, bool)>
      set_sun_pos_wrapper = [set_sun_pos_unity](const std::string& datetime,
                                                const std::string& format,
                                                bool is_dst) {
        return set_sun_pos_unity(datetime.c_str(), format.c_str(), is_dst);
      };

  auto& scene = simserver->GetScene();
  auto set_sun_pos_method = projectairsim::ServiceMethod(
      "SetSunPositionFromDateTime", {"datetime", "format", "is_dst"});
  auto set_sun_pos_handler =
      set_sun_pos_method.CreateMethodHandler(set_sun_pos_wrapper);
  scene.RegisterServiceMethod(set_sun_pos_method, set_sun_pos_handler);
}

void SetSetCloudShadowStrengthCallback(
    bool (*set_cloud_shadows_unity)(float strength)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(float)> set_cloud_shadows_wrapper =
      [set_cloud_shadows_unity](float strength) {
        return set_cloud_shadows_unity(strength);
      };

  auto& scene = simserver->GetScene();
  auto set_cloud_shadows_method =
      projectairsim::ServiceMethod("SetCloudShadowStrength", {"strength"});
  auto set_cloud_shadows_handler =
      set_cloud_shadows_method.CreateMethodHandler(set_cloud_shadows_wrapper);
  scene.RegisterServiceMethod(set_cloud_shadows_method,
                              set_cloud_shadows_handler);
}

void SetSetSegmentationIDByNameCallback(
    bool (*set_seg_id_unity)(const char* mesh_name, int seg_id,
                             bool is_name_regex, bool use_owner_name)) {
  if (simserver == nullptr) {
    // TODO Log warning
    return;
  }

  std::function<bool(const std::string&, int, bool, bool)> set_seg_id_wrapper =
      [set_seg_id_unity](const std::string& mesh_name, int seg_id,
                         bool is_name_regex, bool use_owner_name) {
        return set_seg_id_unity(mesh_name.c_str(), seg_id, is_name_regex,
                                use_owner_name);
      };

  auto& scene = simserver->GetScene();
  auto set_seg_id_method = projectairsim::ServiceMethod(
      "SetSegmentationIDByName",
      {"mesh_name", "segmentation_id", "is_name_regex", "use_owner_name"});
  auto set_seg_id_handler =
      set_seg_id_method.CreateMethodHandler(set_seg_id_wrapper);
  scene.RegisterServiceMethod(set_seg_id_method, set_seg_id_handler);
}