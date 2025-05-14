// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef UNITY_SIM_UNITY_WRAPPER_INCLUDE_SIM_UNITY_WRAPPER_HPP_
#define UNITY_SIM_UNITY_WRAPPER_INCLUDE_SIM_UNITY_WRAPPER_HPP_

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

#include "simserver.hpp"
#include "tile_info.hpp"
#include "unity_interop.hpp"

static microsoft::projectairsim::SimServer* simserver = nullptr;

extern "C" EXPORT bool LoadServer(int topics_port, int services_port);

extern "C" EXPORT bool StartServer();

extern "C" EXPORT bool StopServer();

extern "C" EXPORT void SetCallbackLoadExternalScene(
    void (*load_unreal_scene_func)());

extern "C" EXPORT void SetCallbackStartExternalScene(
    void (*start_unreal_scene_func)());

extern "C" EXPORT void SetCallbackStopExternalScene(
    void (*stop_unreal_scene_func)());

extern "C" EXPORT void SetCallbackUnloadExternalScene(
    void (*unload_unreal_scene_func)());

extern "C" EXPORT const char* GetSceneConfigJSON();

extern "C" EXPORT int GetActorIndex(const char* actor_name);

extern "C" EXPORT int GetSensorIndex(int actor_index, const char* sensor_name);

extern "C" EXPORT void InvokeCollisionDetection(
    int actor_index, UnityInterop::InteropCollisionInfo collision_info);

extern "C" EXPORT void SetRobotKinematicsCallback(
    int actor_index,
    bool (*set_unity_robot_kinematics_func)(UnityInterop::InteropKinematics kin,
                                            int64_t timestamp));

extern "C" EXPORT void SetActuatedTransformCallback(
    int actor_index,
    bool (*set_unity_robot_actuated_transform_func)(
        UnityInterop::InteropActuatedTransform actuated_transform,
        int64_t timestamp));

extern "C" EXPORT void SetRobotHasCollided(int actor_index, bool has_collided);

extern "C" EXPORT void PublishImages(
    int actor_index, int sensor_index,
    UnityInterop::InteropImageMessage* interop_image_messages);

extern "C" EXPORT void AddCameraRequestToCaptureQueue(int actor_index,
                                                      int sensor_index,
                                                      int64_t captured_time);

extern "C" EXPORT bool IsCameraCaptureQueueFull(int actor_index,
                                                int sensor_index);

extern "C" EXPORT int64_t GetSimTimeNanos();

extern "C" EXPORT void PublishLidarData(
    int actor_index, int sensor_index,
    UnityInterop::InteropLidarMessage interop_lidar_message);

extern "C" EXPORT void EcefToNed(const double x, const double y, const double z,
                                 double* north, double* east, double* down);

extern "C" EXPORT void GetHomeGeoPointInECEF(double* x, double* y, double* z);

extern "C" EXPORT double* GetEcefToNeuRotationMatrix();

extern "C" EXPORT int* GetTileKeysToRender(int* size);

extern "C" EXPORT bool CameraIsPoseUpdatePending(int actor_index, int sensor_index);

extern "C" EXPORT const char* CameraGetLookAtObject(int actor_index, int sensor_index);

extern "C" EXPORT bool CameraResetPose(int actor_index, int sensor_index, bool wait_for_pose_update);

extern "C" EXPORT void CameraMarkPoseUpdateAsCompleted(int actor_index, int sensor_index);

extern "C" EXPORT UnityInterop::InteropPose CameraGetDesiredPose(int actor_index, int sensor_index);

////////////////////////////// WorldSimApi ///////////////////////////////////

extern "C" EXPORT void SetSpawnObjectCallback(const char* (*spawn_object_func)(
    const char* object_name, const char* asset_path,
    UnityInterop::InteropPose pose, UnityInterop::InteropVector3 scale));

extern "C" EXPORT void SetSpawnObjectAtGeoCallback(const char* (
    *spawn_object_func)(const char* object_name, const char* asset_path,
                        UnityInterop::InteropPose pose,
                        UnityInterop::InteropVector3 scale));

extern "C" EXPORT void SetDestroyObjectCallback(
    bool (*destroy_object_func)(const char* object_name));

extern "C" EXPORT void SetEnableWeatherVisualEffectsCallback(
    bool (*enable_weather_effects_func)(bool));

extern "C" EXPORT void SetSetWeatherVisualEffectsParamCallback(
    bool (*set_weather_param_func)(int param, float value));

extern "C" EXPORT void SetResetWeatherEffectsCallback(
    bool (*reset_weather_effects_func)());

extern "C" EXPORT void SetSetSunPositionFromDateTimeCallback(bool (
    *set_sun_pos_func)(const char* datetime, const char* format, bool is_dst));

extern "C" EXPORT void SetSetCloudShadowStrengthCallback(
    bool (*set_cloud_shadow_strength_func)(float strength));

extern "C" EXPORT void SetSetSegmentationIDByNameCallback(
    bool (*set_segmentation_id_func)(const char* mesh_name, int seg_id,
                                     bool is_name_regex, bool use_owner_name));

//////////////////////////// End: WorldSimApi ////////////////////////////////

#endif  // UNITY_SIM_UNITY_WRAPPER_SRC_SIM_UNITY_WRAPPER_HPP_
