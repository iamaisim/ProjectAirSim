// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once

#include <regex>
#include <string>

#include "Components/InputComponent.h"
#include "CoreMinimal.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerInput.h"
#include "IImageWrapperModule.h"
#include "LightActorBase.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "TextureShuffleActor.h"
#include "UnrealHelpers.h"
#include "UnrealScene.h"
#include "UnrealTransforms.h"
#include "World/TimeofDay.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/scene.hpp"
#include "core_sim/service_method.hpp"
#include "json.hpp"

enum class WeatherParameter {
  Enabled = 0,
  Rain = 1,
  RoadWetness = 2,
  Snow = 3,
  RoadSnow = 4,
  MapleLeaf = 5,
  RoadLeaf = 6,
  Dust = 7,
  Fog = 8
};

class WorldSimApi {
 public:
  WorldSimApi(UWorld* unreal_world, const std::shared_ptr<TimeOfDay>& tod,
              microsoft::projectairsim::Scene* sim_scene)
      : unreal_world_(unreal_world), sim_scene_(sim_scene), tod_(tod) {
    UnrealHelpers::GenerateActorMap(unreal_world, scene_object_map_);
    UnrealHelpers::GenerateAssetRegistryMap(asset_map_);
    UnrealHelpers::GenerateBlueprintRegistryMap(blueprint_map_);
    UnrealHelpers::GenerateSkeletalMeshRegistryMap(skeletal_map_);
    InitSegmentationIDs();
    RegisterServiceMethods();
  }

  typedef microsoft::projectairsim::Transform Pose;
  typedef microsoft::projectairsim::Vector3 Vector3;

  Vector3 HitTest(const Pose& pose_from);

  bool setTimeofDay(bool is_enabled, const std::string& start_datetime,
                    bool is_start_datetime_dst, float celestial_clock_speed,
                    float update_interval_secs, bool move_sun);

  std::string getSimTimeofDay();

  bool setSunPositionFromDateTime(const std::string& datetime,
                                  const std::string& format, bool is_dst);

  std::string spawnObject(const std::string& object_name,
                          const std::string& asset_path, const Pose& pose,
                          const std::vector<float>& scale, bool enable_physics);

  std::string spawnObjectFromFile(const std::string& object_name,
                                  const std::string& file_format,
                                  const std::vector<uint8_t>& byte_array,
                                  bool is_binary, const Pose& pose,
                                  const std::vector<float>& scale,
                                  bool enable_physics);

  std::string spawnObjectFromFileServiceMethod(
      const std::string& object_name, const std::string& file_format,
      const nlohmann::json::binary_t& byte_array, bool is_binary,
      const Pose& pose, const std::vector<float>& scale, bool enable_physics);

  std::string spawnObjectFromFileAtGeo(
      const std::string& object_name, const std::string& file_format,
      const std::vector<uint8_t>& byte_array, bool is_binary, const double lat,
      const double lon, const float alt, const std::vector<float>& rotation,
      const std::vector<float>& scale, bool enable_physics);

  std::string spawnObjectFromFileAtGeoServiceMethod(
      const std::string& object_name, const std::string& file_format,
      const nlohmann::json::binary_t& byte_array, bool is_binary,
      const double lat, const double lon, const float alt,
      const std::vector<float>& rotation, const std::vector<float>& scale,
      bool enable_physics);

  std::string spawnObjectAtGeo(
      const std::string& object_name, const std::string& asset_path,
      const double latitude, const double longitude, const float altitude,
      const std::vector<float>&
          rotation,  // TODO: fix Quaternion type json deserialization
      const std::vector<float>& scale, bool enable_physics);

  bool destroyObject(const std::string& object_name);

  bool destroyAllSpawnedObjects();

  Pose getObjectPose(const std::string& object_name);

  std::vector<Pose> getObjectPoses(
      const std::vector<std::string>& object_names);

  bool setObjectPose(const std::string& object_name, const Pose& pose,
                     bool teleport);

  std::vector<float> getObjectScale(const std::string& object_name);

  bool setObjectScale(const std::string& object_name,
                      const std::vector<float>& scale);

  std::vector<std::string> listSceneObjects(
      const std::string& name_regex = ".*");

  std::vector<std::string> listSimAssets(const std::string& name_regex = ".*");

  bool enableWeather(bool enable);

  bool resetWeatherEffect();

  bool setWeatherParameter(WeatherParameter param, float val);

  std::unordered_map<std::string, float> getWeatherParameters();

  bool setMaterial(const std::string& object_name,
                   const std::string& material_path);

  bool setTextureFromUrl(const std::string& object_name,
                         const std::string& url);

  bool setTextureFromFile(const std::string& object_name,
                          const std::vector<uint8_t>& raw_img);

  bool setTextureFromFileServiceMethod(const std::string& object_name,
                                       const nlohmann::json::binary_t& raw_img);

  bool setTextureFromPackagedAsset(const std::string& object_name,
                                   const std::string& texture_path);

  std::vector<std::string> swapTextures(const std::string& tag, int tex_id,
                                        int component_id, int material_id);

  bool setLightIntensity(const std::string& object_name, float new_intensity);

  bool setLightColor(const std::string& object_name,
                     const std::vector<float>& color_rgb);

  bool setLightRadius(const std::string& object_name, float new_radius);

  bool setSunLightIntensity(float intensity);

  bool setCloudShadowStrength(float strength);

  float getCloudShadowStrength();

  float getSunLightIntensity();

  bool SetSegmentationIDByName(const std::string& mesh_name,
                               int segmentation_id, bool is_name_regex,
                               bool use_owner_name);

  int GetSegmentationIDByName(const std::string& mesh_name,
                              bool use_owner_name);

  nlohmann::json GetSegmentationIDMap();

  /* Server-side only */
  AActor* FindActor(const std::string& object_name);

  std::vector<int> createVoxelGrid(const WorldSimApi::Pose& position,
                                   const float x_size, const float y_size,
                                   const float z_size, const float res, 
                                   const int n_z_resolution,
                                   std::vector<std::string> actors_to_ignore,
                                   bool use_segmentation);

  bool debugFlushPersistentMarkers();

  bool debugPlotPoints(const std::vector<std::vector<float>>& points,
                       const std::vector<float>& color_rgba, float size,
                       float duration, bool is_persistent);

  bool debugPlotSolidLine(const std::vector<std::vector<float>>& points,
                          const std::vector<float>& color_rgba, float thickness,
                          float duration, bool is_persistent);

  bool debugPlotDashedLine(const std::vector<std::vector<float>>& points,
                           const std::vector<float>& color_rgba,
                           float thickness, float duration, bool is_persistent);

  bool debugPlotArrows(const std::vector<std::vector<float>>& points_start,
                       const std::vector<std::vector<float>>& points_end,
                       const std::vector<float>& color_rgba, float thickness,
                       float arrow_size, float duration, bool is_persistent);

  bool debugPlotStrings(const std::vector<std::string>& strings,
                        const std::vector<std::vector<float>>& positions,
                        float scale, const std::vector<float>& color_rgba,
                        float duration);

  bool debugPlotTransforms(const std::vector<Pose>& poses, float scale,
                           float thickness, float duration, bool is_persistent);

  bool debugPlotTransformsWithNames(const std::vector<Pose>& poses,
                                    const std::vector<std::string>& names,
                                    float tf_scale, float tf_thickness,
                                    float text_scale,
                                    const std::vector<float>& text_color_rgba,
                                    float duration);

  bool ImportNEDTrajectory(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& pose_x, const std::vector<float>& pose_y,
      const std::vector<float>& pose_z, const std::vector<float>& pose_roll,
      const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
      const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
      const std::vector<float>& vel_z_lin);

  bool ImportGeoTrajectory(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& latitudes, const std::vector<float>& longitudes,
      const std::vector<float>& altitudes, const std::vector<float>& pose_roll,
      const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
      const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
      const std::vector<float>& vel_z_lin);

  bool ImportGeoCoordinates(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& latitudes, const std::vector<float>& longitudes,
      const std::vector<float>& altitudes, const std::vector<float>& vel_x_lin,
      const std::vector<float>& vel_y_lin, const std::vector<float>& vel_z_lin);

  void GetNEDFromGeoCoords(const std::vector<float>& latitudes,
                           const std::vector<float>& longitudes,
                           const std::vector<float>& altitudes,
                           std::vector<float>& x, std::vector<float>& y,
                           std::vector<float>& z);

  bool SetEnvActorTrajectory(const std::string& env_actor_name,
                             const std::string& traj_name,
                             const float time_offset, const float x_offset,
                             const float y_offset, const float z_offset,
                             const float roll_offset, const float pitch_offset,
                             const float yaw_offset, const bool to_loop);

  bool SetEnvActorLinkRotAngle(const std::string& env_actor_name,
                               const std::string& link_name,
                               const float angle_deg);

  bool SetEnvActorLinkRotRate(const std::string& env_actor_name,
                              const std::string& link_name,
                              const float rotation_deg_per_sec);

  float GetZAtPoint(const float x, const float y);

 protected:
 private:
  void GetRotations(const std::vector<float>& x, const std::vector<float>& y,
                    const std::vector<float>& z, std::vector<float>& roll,
                    std::vector<float>& pitch, std::vector<float>& yaw);

  std::string getUniqueName(const std::string& object_name) const;

  Pose GetPoseFromGeoPoint(const double lat, const double lon, const float alt,
                           const std::vector<float>& rotation);

  template <typename T>
  T* createNewActor(const FActorSpawnParameters& spawn_params,
                    const FTransform& actor_transform,
                    const std::vector<float>& scale, UStaticMesh* static_mesh);

  template <typename T>
  T* createNewSkeletalActor(const FActorSpawnParameters& spawn_params,
                            const FTransform& actor_transform,
                            const std::vector<float>& scale,
                            USkeletalMesh* static_mesh);

  template <typename T>
  T* createNewBlueprintActor(const FActorSpawnParameters& spawn_params,
                             const FTransform& actor_transform,
                             const std::vector<float>& scale,
                             UClass* bp_generated_class);

  void InitSegmentationIDs();

  void setObjectTexture(AActor* actor, UTexture2D* texture,
                        UMaterial* textureSwapMat);

  UMaterial* getTextureSwapMaterial() const;

  void setMaterialProperties(UMaterialInstanceDynamic* dynamic_material,
                             UTexture2D* texture);
  FHitResult GetHitResult(const FVector& start, const FVector& end,
                          FCollisionQueryParams& TraceParams);

  UWorld* unreal_world_;
  microsoft::projectairsim::Scene* sim_scene_;
  TMap<FString, AActor*> scene_object_map_;
  TMap<FString, AActor*> scene_spawned_object_map_;
  TMap<FString, FAssetData> asset_map_;
  TMap<FString, FAssetData> blueprint_map_;
  TMap<FString, FAssetData> skeletal_map_;
  TMap<FString, int> seg_map_;

  const std::shared_ptr<TimeOfDay> tod_;

  void RegisterServiceMethods();

  float nan = std::numeric_limits<float>::quiet_NaN();

  std::vector<int> voxel_grid_;
};
