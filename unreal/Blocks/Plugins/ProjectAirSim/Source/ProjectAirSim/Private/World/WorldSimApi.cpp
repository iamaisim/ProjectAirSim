// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "WorldSimApi.h"

#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/OutputDeviceNull.h"
#include "Renderers/AssimpToProcMesh.h"
#include "Renderers/ProcMeshActor.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "WeatherLib.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "core_sim/actor/env_actor.hpp"

namespace projectairsim = microsoft::projectairsim;

void WorldSimApi::RegisterServiceMethods() {
  auto list_scene_objects =
      projectairsim::ServiceMethod("ListObjects", {"name"});
  auto list_scene_objects_handler = list_scene_objects.CreateMethodHandler(
      &WorldSimApi::listSceneObjects, *this);
  sim_scene_->RegisterServiceMethod(list_scene_objects,
                                    list_scene_objects_handler);

  auto list_sim_assets = projectairsim::ServiceMethod("ListAssets", {"name"});
  auto list_sim_assets_handler =
      list_sim_assets.CreateMethodHandler(&WorldSimApi::listSimAssets, *this);
  sim_scene_->RegisterServiceMethod(list_sim_assets, list_sim_assets_handler);

  auto get_object_pose =
      projectairsim::ServiceMethod("GetObjectPose", {"object_name"});
  auto get_object_pose_handler =
      get_object_pose.CreateMethodHandler(&WorldSimApi::getObjectPose, *this);
  sim_scene_->RegisterServiceMethod(get_object_pose, get_object_pose_handler);

  auto get_object_poses =
      projectairsim::ServiceMethod("GetObjectPoses", {"object_names"});
  auto get_object_poses_handler =
      get_object_poses.CreateMethodHandler(&WorldSimApi::getObjectPoses, *this);
  sim_scene_->RegisterServiceMethod(get_object_poses, get_object_poses_handler);

  auto set_object_pose = projectairsim::ServiceMethod(
      "SetObjectPose", {"object_name", "pose", "teleport"});
  auto set_object_pose_handler =
      set_object_pose.CreateMethodHandler(&WorldSimApi::setObjectPose, *this);
  sim_scene_->RegisterServiceMethod(set_object_pose, set_object_pose_handler);

  auto get_object_scale =
      projectairsim::ServiceMethod("GetObjectScale", {"object_name"});
  auto get_object_scale_handler =
      get_object_scale.CreateMethodHandler(&WorldSimApi::getObjectScale, *this);
  sim_scene_->RegisterServiceMethod(get_object_scale, get_object_scale_handler);

  auto set_object_scale =
      projectairsim::ServiceMethod("SetObjectScale", {"object_name", "scale"});
  auto set_object_scale_handler =
      set_object_scale.CreateMethodHandler(&WorldSimApi::setObjectScale, *this);
  sim_scene_->RegisterServiceMethod(set_object_scale, set_object_scale_handler);

  auto spawn_object = projectairsim::ServiceMethod(
      "SpawnObject",
      {"object_name", "asset_path", "pose", "scale", "enable_physics"});
  auto spawn_object_handler =
      spawn_object.CreateMethodHandler(&WorldSimApi::spawnObject, *this);
  sim_scene_->RegisterServiceMethod(spawn_object, spawn_object_handler);

  auto spawn_object_runtime = projectairsim::ServiceMethod(
      "spawnObjectFromFile", {"object_name", "file_format", "byte_array",
                              "is_binary", "pose", "scale", "enable_physics"});
  auto spawn_object_runtime_handler = spawn_object.CreateMethodHandler(
      &WorldSimApi::spawnObjectFromFileServiceMethod, *this);
  sim_scene_->RegisterServiceMethod(spawn_object_runtime,
                                    spawn_object_runtime_handler);

  auto spawn_object_geo = projectairsim::ServiceMethod(
      "spawnObjectAtGeo", {"object_name", "asset_path", "latitude", "longitude",
                           "altitude", "rotation", "scale", "enable_physics"});
  auto spawn_object_geo_handler = spawn_object_geo.CreateMethodHandler(
      &WorldSimApi::spawnObjectAtGeo, *this);
  sim_scene_->RegisterServiceMethod(spawn_object_geo, spawn_object_geo_handler);

  auto destroy_object =
      projectairsim::ServiceMethod("DestroyObject", {"object_name"});
  auto destroy_object_handler =
      destroy_object.CreateMethodHandler(&WorldSimApi::destroyObject, *this);
  sim_scene_->RegisterServiceMethod(destroy_object, destroy_object_handler);

  auto destroy_all_spawned_objects =
      projectairsim::ServiceMethod("DestroyAllSpawnedObjects", {});
  auto destroy_all_spawned_objects_handler =
      destroy_all_spawned_objects.CreateMethodHandler(
          &WorldSimApi::destroyAllSpawnedObjects, *this);
  sim_scene_->RegisterServiceMethod(destroy_all_spawned_objects,
                                    destroy_all_spawned_objects_handler);

  auto spawn_object_from_file_geo = projectairsim::ServiceMethod(
      "spawnObjectFromFileAtGeo",
      {"object_name", "file_format", "byte_array", "is_binary", "latitude",
       "longitude", "altitude", "rotation", "scale", "enable_physics"});
  auto spawn_runtime_geo_handler =
      spawn_object_from_file_geo.CreateMethodHandler(
          &WorldSimApi::spawnObjectFromFileAtGeoServiceMethod, *this);
  sim_scene_->RegisterServiceMethod(spawn_object_from_file_geo,
                                    spawn_runtime_geo_handler);

  auto enable_weather = projectairsim::ServiceMethod(
      "SimSetWeatherVisualEffectsStatus", {"status"});
  auto enable_weather_handler =
      enable_weather.CreateMethodHandler(&WorldSimApi::enableWeather, *this);
  sim_scene_->RegisterServiceMethod(enable_weather, enable_weather_handler);

  auto reset_weather_effect =
      projectairsim::ServiceMethod("ResetWeatherEffects", {});
  auto reset_weather_effect_handler = reset_weather_effect.CreateMethodHandler(
      &WorldSimApi::resetWeatherEffect, *this);
  sim_scene_->RegisterServiceMethod(reset_weather_effect,
                                    reset_weather_effect_handler);

  auto set_weather_param = projectairsim::ServiceMethod(
      "SetWeatherVisualEffectsParameter", {"param", "value"});
  auto set_weather_param_handler = set_weather_param.CreateMethodHandler(
      &WorldSimApi::setWeatherParameter, *this);
  sim_scene_->RegisterServiceMethod(set_weather_param,
                                    set_weather_param_handler);

  auto get_weather_param =
      projectairsim::ServiceMethod("GetWeatherVisualEffectsParameter", {});
  auto get_weather_param_handler = get_weather_param.CreateMethodHandler(
      &WorldSimApi::getWeatherParameters, *this);
  sim_scene_->RegisterServiceMethod(get_weather_param,
                                    get_weather_param_handler);

  auto set_material = projectairsim::ServiceMethod(
      "SetObjectMaterial", {"object_name", "material_path"});
  auto set_material_handler =
      set_material.CreateMethodHandler(&WorldSimApi::setMaterial, *this);
  sim_scene_->RegisterServiceMethod(set_material, set_material_handler);

  auto set_texture_url = projectairsim::ServiceMethod("SetObjectTextureFromUrl",
                                                      {"object_name", "url"});
  auto set_texture_url_handler = set_texture_url.CreateMethodHandler(
      &WorldSimApi::setTextureFromUrl, *this);
  sim_scene_->RegisterServiceMethod(set_texture_url, set_texture_url_handler);

  auto set_texture_from_file = projectairsim::ServiceMethod(
      "SetObjectTextureFromFile", {"object_name", "raw_img"});
  auto set_texture_from_file_handler =
      set_texture_from_file.CreateMethodHandler(
          &WorldSimApi::setTextureFromFileServiceMethod, *this);
  sim_scene_->RegisterServiceMethod(set_texture_from_file,
                                    set_texture_from_file_handler);

  auto set_texture_from_packaged_asset = projectairsim::ServiceMethod(
      "SetObjectTextureFromPackagedAsset", {"object_name", "texture_path"});
  auto set_texture_from_packaged_asset_handler =
      set_texture_from_packaged_asset.CreateMethodHandler(
          &WorldSimApi::setTextureFromPackagedAsset, *this);
  sim_scene_->RegisterServiceMethod(set_texture_from_packaged_asset,
                                    set_texture_from_packaged_asset_handler);

  auto swap_texture = projectairsim::ServiceMethod(
      "SwapObjectTexture", {"tag", "tex_id", "component_id", "material_id"});
  auto swap_texture_handler =
      swap_texture.CreateMethodHandler(&WorldSimApi::swapTextures, *this);
  sim_scene_->RegisterServiceMethod(swap_texture, swap_texture_handler);

  auto set_light_intensity = projectairsim::ServiceMethod(
      "SetLightObjectIntensity", {"object_name", "new_intensity"});
  auto set_light_intensity_handler = set_light_intensity.CreateMethodHandler(
      &WorldSimApi::setLightIntensity, *this);
  sim_scene_->RegisterServiceMethod(set_light_intensity,
                                    set_light_intensity_handler);

  auto set_light_color = projectairsim::ServiceMethod(
      "SetLightObjectColor", {"object_name", "color_rgb"});
  auto set_light_color_handler =
      set_light_color.CreateMethodHandler(&WorldSimApi::setLightColor, *this);
  sim_scene_->RegisterServiceMethod(set_light_color, set_light_color_handler);

  auto set_light_radius = projectairsim::ServiceMethod(
      "SetLightObjectRadius", {"object_name", "new_radius"});
  auto set_light_radius_handler =
      set_light_radius.CreateMethodHandler(&WorldSimApi::setLightRadius, *this);
  sim_scene_->RegisterServiceMethod(set_light_radius, set_light_radius_handler);

  auto set_time_of_day = projectairsim::ServiceMethod(
      "SetTimeOfDay", {"status", "datetime", "is_dst", "clock_speed",
                       "update_interval", "move_sun"});
  auto set_time_of_day_handler =
      set_time_of_day.CreateMethodHandler(&WorldSimApi::setTimeofDay, *this);
  sim_scene_->RegisterServiceMethod(set_time_of_day, set_time_of_day_handler);

  auto set_run_rot = projectairsim::ServiceMethod(
      "SetSunPositionFromDateTime", {"datetime", "format", "is_dst"});
  auto set_run_rot_handler = set_run_rot.CreateMethodHandler(
      &WorldSimApi::setSunPositionFromDateTime, *this);
  sim_scene_->RegisterServiceMethod(set_run_rot, set_run_rot_handler);

  auto get_tod = projectairsim::ServiceMethod("GetSimTimeofDay", {});
  auto get_tod_handler =
      get_tod.CreateMethodHandler(&WorldSimApi::getSimTimeofDay, *this);
  sim_scene_->RegisterServiceMethod(get_tod, get_tod_handler);

  auto set_sunlight_intensity =
      projectairsim::ServiceMethod("SetSunLightIntensity", {"intensity"});
  auto set_sunlight_intensity_handler = set_light_intensity.CreateMethodHandler(
      &WorldSimApi::setSunLightIntensity, *this);
  sim_scene_->RegisterServiceMethod(set_sunlight_intensity,
                                    set_sunlight_intensity_handler);

  auto set_shadow_strength =
      projectairsim::ServiceMethod("SetCloudShadowStrength", {"strength"});
  auto set_shadow_strength_handler = set_shadow_strength.CreateMethodHandler(
      &WorldSimApi::setCloudShadowStrength, *this);
  sim_scene_->RegisterServiceMethod(set_shadow_strength,
                                    set_shadow_strength_handler);

  auto get_shadow_strength =
      projectairsim::ServiceMethod("GetCloudShadowStrength", {});
  auto get_shadow_strength_handler = get_shadow_strength.CreateMethodHandler(
      &WorldSimApi::getCloudShadowStrength, *this);
  sim_scene_->RegisterServiceMethod(get_shadow_strength,
                                    get_shadow_strength_handler);

  auto get_light_intensity =
      projectairsim::ServiceMethod("GetSunLightIntensity", {});
  auto get_light_intensity_handler = get_light_intensity.CreateMethodHandler(
      &WorldSimApi::getSunLightIntensity, *this);
  sim_scene_->RegisterServiceMethod(get_light_intensity,
                                    get_light_intensity_handler);

  auto set_seg_id = projectairsim::ServiceMethod(
      "SetSegmentationIDByName",
      {"mesh_name", "segmentation_id", "is_name_regex", "use_owner_name"});
  auto set_seg_id_handler = set_seg_id.CreateMethodHandler(
      &WorldSimApi::SetSegmentationIDByName, *this);
  sim_scene_->RegisterServiceMethod(set_seg_id, set_seg_id_handler);

  auto get_seg_id = projectairsim::ServiceMethod(
      "GetSegmentationIDByName", {"mesh_name", "use_owner_name"});
  auto get_seg_id_handler = get_seg_id.CreateMethodHandler(
      &WorldSimApi::GetSegmentationIDByName, *this);
  sim_scene_->RegisterServiceMethod(get_seg_id, get_seg_id_handler);

  auto get_seg_map = projectairsim::ServiceMethod("GetSegmentationIDMap", {});
  auto get_seg_map_handler = get_seg_map.CreateMethodHandler(
      &WorldSimApi::GetSegmentationIDMap, *this);
  sim_scene_->RegisterServiceMethod(get_seg_map, get_seg_map_handler);

  auto create_voxel_grid = projectairsim::ServiceMethod(
      "createVoxelGrid",
      {"position", "x_size", "y_size", "z_size", "res", "n_z_resolution",
       "actors_to_ignore", "use_segmentation"});
  auto create_voxel_grid_handler = create_voxel_grid.CreateMethodHandler(
      &WorldSimApi::createVoxelGrid, *this);
  sim_scene_->RegisterServiceMethod(create_voxel_grid,
                                    create_voxel_grid_handler);

  auto debug_flush_persistent_markers =
      projectairsim::ServiceMethod("debugFlushPersistentMarkers", {});
  auto debug_flush_persistent_markers_handler =
      debug_flush_persistent_markers.CreateMethodHandler(
          &WorldSimApi::debugFlushPersistentMarkers, *this);
  sim_scene_->RegisterServiceMethod(debug_flush_persistent_markers,
                                    debug_flush_persistent_markers_handler);

  auto debug_plot_points = projectairsim::ServiceMethod(
      "debugPlotPoints",
      {"points", "color_rgba", "size", "duration", "is_persistent"});
  auto debug_plot_points_handler = debug_plot_points.CreateMethodHandler(
      &WorldSimApi::debugPlotPoints, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_points,
                                    debug_plot_points_handler);

  auto debug_plot_solid_line = projectairsim::ServiceMethod(
      "debugPlotSolidLine",
      {"points", "color_rgba", "thickness", "duration", "is_persistent"});
  auto debug_plot_solid_line_handler =
      debug_plot_solid_line.CreateMethodHandler(
          &WorldSimApi::debugPlotSolidLine, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_solid_line,
                                    debug_plot_solid_line_handler);

  auto debug_plot_dashed_line = projectairsim::ServiceMethod(
      "debugPlotDashedLine",
      {"points", "color_rgba", "thickness", "duration", "is_persistent"});
  auto debug_plot_dashed_line_handler =
      debug_plot_dashed_line.CreateMethodHandler(
          &WorldSimApi::debugPlotDashedLine, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_dashed_line,
                                    debug_plot_dashed_line_handler);

  auto debug_plot_arrows = projectairsim::ServiceMethod(
      "debugPlotArrows",
      {"points_start", "points_end", "color_rgba", "thickness", "arrow_size",
       "duration", "is_persistent"});
  auto debug_plot_arrows_handler = debug_plot_arrows.CreateMethodHandler(
      &WorldSimApi::debugPlotArrows, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_arrows,
                                    debug_plot_arrows_handler);

  auto debug_plot_strings = projectairsim::ServiceMethod(
      "debugPlotStrings",
      {"strings", "positions", "scale", "color_rgba", "duration"});
  auto debug_plot_strings_handler = debug_plot_strings.CreateMethodHandler(
      &WorldSimApi::debugPlotStrings, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_strings,
                                    debug_plot_strings_handler);

  auto debug_plot_transforms = projectairsim::ServiceMethod(
      "debugPlotTransforms",
      {"poses", "scale", "thickness", "duration", "is_persistent"});
  auto debug_plot_transforms_handler =
      debug_plot_transforms.CreateMethodHandler(
          &WorldSimApi::debugPlotTransforms, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_transforms,
                                    debug_plot_transforms_handler);

  auto debug_plot_transforms_with_names = projectairsim::ServiceMethod(
      "debugPlotTransformsWithNames",
      {"poses", "names", "tf_scale", "tf_thickness", "text_scale",
       "text_color_rgba", "duration"});
  auto debug_plot_transforms_with_names_handler =
      debug_plot_transforms.CreateMethodHandler(
          &WorldSimApi::debugPlotTransformsWithNames, *this);
  sim_scene_->RegisterServiceMethod(debug_plot_transforms_with_names,
                                    debug_plot_transforms_with_names_handler);

  auto import_ned_trajectory = projectairsim::ServiceMethod(
      "ImportNEDTrajectory",
      {"traj_name", "time", "pose_x", "pose_y", "pose_z", "pose_roll",
       "pose_pitch", "pose_yaw", "vel_x_lin", "vel_y_lin", "vel_z_lin"});
  auto import_ned_trajectory_handler =
      import_ned_trajectory.CreateMethodHandler(
          &WorldSimApi::ImportNEDTrajectory, *this);
  sim_scene_->RegisterServiceMethod(import_ned_trajectory,
                                    import_ned_trajectory_handler);

  auto import_geo_trajectory = projectairsim::ServiceMethod(
      "ImportGeoTrajectory",
      {"traj_name", "time", "latitudes", "longitudes", "altitudes", "pose_roll",
       "pose_pitch", "pose_yaw", "vel_x_lin", "vel_y_lin", "vel_z_lin"});
  auto import_geo_trajectory_handler =
      import_geo_trajectory.CreateMethodHandler(
          &WorldSimApi::ImportGeoTrajectory, *this);
  sim_scene_->RegisterServiceMethod(import_geo_trajectory,
                                    import_geo_trajectory_handler);

  auto import_geo_coordinates = projectairsim::ServiceMethod(
      "ImportGeoCoordinates",
      {"traj_name", "time", "latitudes", "longitudes", "altitudes", "vel_x_lin",
       "vel_y_lin", "vel_z_lin"});
  auto import_geo_coordinates_handler =
      import_geo_coordinates.CreateMethodHandler(
          &WorldSimApi::ImportGeoCoordinates, *this);
  sim_scene_->RegisterServiceMethod(import_geo_coordinates,
                                    import_geo_coordinates_handler);

  auto set_env_actor_trajectory = projectairsim::ServiceMethod(
      "SetEnvActorTrajectory",
      {"env_actor_name", "traj_name", "time_offset", "x_offset", "y_offset",
       "z_offset", "roll_offset", "pitch_offset", "yaw_offset", "to_loop"});
  auto set_env_actor_trajectory_handler =
      set_env_actor_trajectory.CreateMethodHandler(
          &WorldSimApi::SetEnvActorTrajectory, *this);
  sim_scene_->RegisterServiceMethod(set_env_actor_trajectory,
                                    set_env_actor_trajectory_handler);

  auto set_env_actor_link_rot_angle = projectairsim::ServiceMethod(
      "SetEnvActorLinkRotAngle", {"env_actor_name", "link_name", "angle_deg"});
  auto set_env_actor_link_rot_angle_handler =
      set_env_actor_link_rot_angle.CreateMethodHandler(
          &WorldSimApi::SetEnvActorLinkRotAngle, *this);
  sim_scene_->RegisterServiceMethod(set_env_actor_link_rot_angle,
                                    set_env_actor_link_rot_angle_handler);

  auto set_env_actor_link_rot_rate = projectairsim::ServiceMethod(
      "SetEnvActorLinkRotRate",
      {"env_actor_name", "link_name", "rotation_deg_per_sec"});
  auto set_env_actor_link_rot_rate_handler =
      set_env_actor_link_rot_rate.CreateMethodHandler(
          &WorldSimApi::SetEnvActorLinkRotRate, *this);
  sim_scene_->RegisterServiceMethod(set_env_actor_link_rot_rate,
                                    set_env_actor_link_rot_rate_handler);

  auto hit_test_location = projectairsim::ServiceMethod("HitTest", {"pose"});
  auto hit_test_location_handler =
      hit_test_location.CreateMethodHandler(&WorldSimApi::HitTest, *this);
  sim_scene_->RegisterServiceMethod(hit_test_location,
                                    hit_test_location_handler);

  auto get_z_at_point = projectairsim::ServiceMethod("GetZAtPoint", {"x", "y"});
  auto get_z_at_point_handler =
      get_z_at_point.CreateMethodHandler(&WorldSimApi::GetZAtPoint, *this);
  sim_scene_->RegisterServiceMethod(get_z_at_point, get_z_at_point_handler);
}

WorldSimApi::Vector3 WorldSimApi::HitTest(const Pose& pose) {
  WorldSimApi::Vector3 result;
  result = projectairsim::Vector3(nan, nan, nan);

  static const float kHitTestDistanceMeters = 10000.0;

  FHitResult HitInfo(ForceInit);
  FVector StartTrace = UnrealTransform::NedToUnrealLinear(pose.translation_);
  FRotator RotationToPose = UnrealHelpers::ToFRotator(pose.rotation_);
  FVector EndTrace =
      UnrealTransform::NedToUnrealLinear(pose.rotation_._transformVector(
          projectairsim::Vector3(kHitTestDistanceMeters, 0, 0)));
  FCollisionQueryParams TraceParams;

  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

  HitInfo = GetHitResult(StartTrace, EndTrace, TraceParams);
  result = UnrealTransform::UnrealToNedLinear(HitInfo.ImpactPoint);

  return result;
}

bool WorldSimApi::setTimeofDay(bool is_enabled,
                               const std::string& start_datetime,
                               bool is_start_datetime_dst,
                               float celestial_clock_speed,
                               float update_interval_secs, bool move_sun) {
  return tod_->set(is_enabled, start_datetime, is_start_datetime_dst,
                   celestial_clock_speed, update_interval_secs, move_sun);
}

std::string WorldSimApi::getSimTimeofDay() { return tod_->getTimeofDay(); }

bool WorldSimApi::setSunPositionFromDateTime(const std::string& datetime,
                                             const std::string& format,
                                             bool is_dst) {
  return tod_->setSunPositionFromDateTime(datetime, format, is_dst);
}

bool WorldSimApi::setSunLightIntensity(float intensity) {
  return tod_->setSunLightIntensity(intensity);
}

bool WorldSimApi::setCloudShadowStrength(float strength) {
  return tod_->setCloudShadowStrength(strength);
}

float WorldSimApi::getCloudShadowStrength() {
  return tod_->getCloudShadowStrength();
}

float WorldSimApi::getSunLightIntensity() {
  return tod_->getSunLightIntensity();
}

std::string WorldSimApi::spawnObject(const std::string& object_name,
                                     const std::string& asset_path,
                                     const WorldSimApi::Pose& pose,
                                     const std::vector<float>& scale,
                                     bool enable_physics) {
  // Create struct for Location and Rotation of actor in Unreal
  FTransform actor_transform = UnrealTransform::FromGlobalNed(pose);
  bool found_object = false, spawned_object = false;
  std::string status;
  std::string object_name_temp = object_name;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &asset_path, &object_name_temp, &actor_transform, &found_object,
       &spawned_object, &scale, &enable_physics]() mutable {
        // Check static assets
        enum EAssetType { STATICMESH, SKELETALMESH, BLUEPRINT };

        FAssetData* LoadAsset = asset_map_.Find(FString(asset_path.c_str()));
        EAssetType loadAssetType = STATICMESH;
        // If no matching static asset found, check for skeletal assets
        if (!LoadAsset) {
          LoadAsset = skeletal_map_.Find(FString(asset_path.c_str()));
          loadAssetType = SKELETALMESH;
        }
        // If no matching static or skeletal asset found, check for blueprints
        if (!LoadAsset) {
          LoadAsset = blueprint_map_.Find(FString(asset_path.c_str()));
          loadAssetType = BLUEPRINT;
        }
        // handle the loaded asset
        if (LoadAsset) {
          found_object = true;

          object_name_temp = getUniqueName(object_name_temp);

          FActorSpawnParameters new_actor_spawn_params;
          new_actor_spawn_params.Name = FName(object_name_temp.c_str());

          // Do not spawn (and subsequently result in fatal crash) if desired
          // name cannot be assigned.
          new_actor_spawn_params.NameMode =
              FActorSpawnParameters::ESpawnActorNameMode::Required_ReturnNull;

          AActor* NewActor = nullptr;

          switch (loadAssetType) {
            case STATICMESH: {
              UStaticMesh* LoadObject =
                  Cast<UStaticMesh>(LoadAsset->GetAsset());
              NewActor = this->createNewActor<AActor>(
                  new_actor_spawn_params, actor_transform, scale, LoadObject);
              break;
            }
            case SKELETALMESH: {
              USkeletalMesh* LoadObject =
                  Cast<USkeletalMesh>(LoadAsset->GetAsset());
              NewActor = this->createNewSkeletalActor<AActor>(
                  new_actor_spawn_params, actor_transform, scale, LoadObject);
              break;
            }
            case BLUEPRINT: {
              UClass* BPGeneratedClass = FindObject<UClass>(
                  LoadAsset->GetPackage(),
                  *(LoadAsset->AssetName.ToString() + TEXT("_C")));

              NewActor = this->createNewBlueprintActor<AActor>(
                  new_actor_spawn_params, actor_transform, scale,
                  BPGeneratedClass);
              break;
            }
          }

          if (NewActor != nullptr) {
            spawned_object = true;
            scene_object_map_.Add(FString(object_name_temp.c_str()), NewActor);
            scene_spawned_object_map_.Add(FString(object_name_temp.c_str()),
                                          NewActor);
            UnrealHelpers::setSimulatePhysics(NewActor, enable_physics);
          }
        } else {
          found_object = false;
        }
      },
      true);

  if (!found_object) {
    std::string status_msg = "There were no objects with name " + asset_path +
                             " found in the Registry";
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("There were no objects with name %hs found in the Registry"),
        asset_path.c_str());
    throw projectairsim::Error(status_msg.c_str());
  }
  if (!spawned_object) {
    std::string status_msg = "Engine could not spawn " + object_name;
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("Engine could not spawn %hs"), object_name.c_str());
    throw projectairsim::Error(status_msg.c_str());
  }
  return object_name_temp;
}

std::string WorldSimApi::spawnObjectAtGeo(const std::string& object_name,
                                          const std::string& asset_path,
                                          const double lat, const double lon,
                                          const float alt,
                                          const std::vector<float>& rotation,
                                          const std::vector<float>& scale,
                                          bool enable_physics) {
  auto home_geo_point = sim_scene_->GetHomeGeoPoint().geo_point;

  auto pose = GetPoseFromGeoPoint(lat, lon, alt, rotation);

  return spawnObject(object_name, asset_path, pose, scale, enable_physics);
}

std::string WorldSimApi::spawnObjectFromFile(
    const std::string& object_name, const std::string& file_format,
    const std::vector<uint8_t>& byte_array, bool is_binary, const Pose& pose,
    const std::vector<float>& scale, bool enable_physics) {
  FTransform actor_transform = UnrealTransform::FromGlobalNed(pose);
  bool spawned_object = false;
  std::string status;
  std::string object_name_temp = object_name;

  UnrealHelpers::RunCommandOnGameThread(
      [this, &file_format, &object_name_temp, &actor_transform, &spawned_object,
       &byte_array, &is_binary, &scale, &enable_physics]() mutable {
        Assimp::Importer importer;

        // Set importer to use 100x scale factor to do m->cm conversion for UE
        importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 100.0f);

        int propertiesFlags = aiProcessPreset_TargetRealtime_MaxQuality |
                              aiProcess_GlobalScale | aiProcess_MakeLeftHanded |
                              aiProcess_FlipUVs;

        std::string ai_file_format = "";

        if (file_format == "gltf" || file_format == "glb") {
          if (is_binary) {
            ai_file_format = "glb";
          } else {
            ai_file_format = "gltf";
          }
        }

        const aiScene* scene = importer.ReadFileFromMemory(
            byte_array.data(), byte_array.size(), propertiesFlags,
            ai_file_format.c_str());

        if (scene) {
          object_name_temp = getUniqueName(object_name_temp);

          FActorSpawnParameters new_actor_spawn_params;
          new_actor_spawn_params.Name = FName(object_name_temp.c_str());

          // Do not spawn (and subsequently result in fatal crash) if desired
          // name cannot be assigned.
          new_actor_spawn_params.NameMode =
              FActorSpawnParameters::ESpawnActorNameMode::Required_ReturnNull;

          AProcMeshActor* MainRuntimeActor =
              this->createNewActor<AProcMeshActor>(
                  new_actor_spawn_params, FTransform(), std::vector<float>(),
                  nullptr);
          MainRuntimeActor->Init(scene->mNumMeshes, actor_transform, scale,
                                 enable_physics);
          MainRuntimeActor->SetHidden(false);

          AssimpToProcMesh::UpdateProcMesh(scene, MainRuntimeActor);

          scene_object_map_.Add(FString(object_name_temp.c_str()),
                                MainRuntimeActor);
          scene_spawned_object_map_.Add(FString(object_name_temp.c_str()),
                                        MainRuntimeActor);
        }
      },
      true);

  return object_name_temp;
}

std::string WorldSimApi::spawnObjectFromFileServiceMethod(
    const std::string& object_name, const std::string& file_format,
    const nlohmann::json::binary_t& byte_array, bool is_binary,
    const Pose& pose, const std::vector<float>& scale, bool enable_physics) {
  // Service methods needs explicit nlohmann::json::binary_t type for binary
  // byte array for it to be parsed properly from the JSON binary-type data,
  // but its underlying C++ data type is a vector<uint8_t> so it can be passed
  // through directly.
  return spawnObjectFromFile(object_name, file_format, byte_array, is_binary,
                             pose, scale, enable_physics);
}

std::string WorldSimApi::spawnObjectFromFileAtGeo(
    const std::string& object_name, const std::string& file_format,
    const std::vector<uint8_t>& byte_array, bool is_binary, const double lat,
    const double lon, const float alt, const std::vector<float>& rotation,
    const std::vector<float>& scale, bool enable_physics) {
  auto home_geo_point = sim_scene_->GetHomeGeoPoint().geo_point;
  auto pose = GetPoseFromGeoPoint(lat, lon, alt, rotation);

  return spawnObjectFromFile(object_name, file_format, byte_array, is_binary,
                             pose, scale, enable_physics);
}

std::string WorldSimApi::spawnObjectFromFileAtGeoServiceMethod(
    const std::string& object_name, const std::string& file_format,
    const nlohmann::json::binary_t& byte_array, bool is_binary,
    const double lat, const double lon, const float alt,
    const std::vector<float>& rotation, const std::vector<float>& scale,
    bool enable_physics) {
  // Service methods needs explicit nlohmann::json::binary_t type for binary
  // byte array for it to be parsed properly from the JSON binary-type data,
  // but its underlying C++ data type is a vector<uint8_t> so it can be passed
  // through directly.
  return spawnObjectFromFileAtGeo(object_name, file_format, byte_array,
                                  is_binary, lat, lon, alt, rotation, scale,
                                  enable_physics);
}

bool WorldSimApi::destroyObject(const std::string& object_name) {
  bool destroyed = false;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &destroyed]() {
        AActor* actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()), &scene_object_map_);
        if (actor) {
          actor->Destroy();
          destroyed = (IsValid(actor) == false);
        }
      },
      true);

  UnrealHelpers::ForceUnrealGarbageCollection();

  if (destroyed) {
    scene_object_map_.Remove(UTF8_TO_TCHAR(object_name.c_str()));
    scene_spawned_object_map_.Remove(UTF8_TO_TCHAR(object_name.c_str()));
  } else {
    std::string status_msg = "DestroyObject failed. No objects of name " +
                             object_name + " were found in the world.";
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("DestroyObject failed. No objects of name %hs were "
                           "found in the world."),
                      object_name.c_str());
  }
  return destroyed;
}

WorldSimApi::Pose WorldSimApi::getObjectPose(const std::string& object_name) {
  WorldSimApi::Pose result;
  result.translation_ = projectairsim::Vector3(nan, nan, nan);
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &result]() {
        // AActor* actor = UnrealHelpers::FindActor<AActor>(
        //    scene_, FString(object_name.c_str()));
        AActor* actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()), &scene_object_map_);

        if (actor) {
          result = UnrealTransform::ToGlobalNed(
              FTransform(actor->GetActorRotation(), actor->GetActorLocation()));
        }
      },
      true);
  return result;
}

std::vector<WorldSimApi::Pose> WorldSimApi::getObjectPoses(
    const std::vector<std::string>& object_names) {
  std::vector<WorldSimApi::Pose> results;

  for (const std::string& obj_name : object_names) {
    WorldSimApi::Pose obj_pose = getObjectPose(obj_name);
    results.push_back(obj_pose);
  }

  return results;
}

bool WorldSimApi::destroyAllSpawnedObjects() {
  bool result = true;
  std::vector<std::string> keys;

  for (auto& object : scene_spawned_object_map_) {
    keys.push_back(TCHAR_TO_UTF8(*(object.Key)));
  }

  for (auto& object_name : keys) {
    result &= WorldSimApi::destroyObject(object_name);
  }

  return result;
}

bool WorldSimApi::setObjectPose(const std::string& object_name,
                                const WorldSimApi::Pose& pose, bool teleport) {
  bool actor_found = false, result = false;
  std::string status;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &pose, teleport, &result, &actor_found]() {
        FTransform actor_transform = UnrealTransform::FromGlobalNed(pose);

        AActor* actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()), &scene_object_map_);

        if (actor) {
          actor_found = true;
          if (teleport)
            result = actor->SetActorLocationAndRotation(
                actor_transform.GetLocation(), actor_transform.GetRotation(),
                false, nullptr, ETeleportType::TeleportPhysics);
          else
            result = actor->SetActorLocationAndRotation(
                actor_transform.GetLocation(), actor_transform.GetRotation(),
                true);
        }
      },
      true);

  if (!result) {
    if (!actor_found) {
      std::string status_msg = "SetObjectPose failed. No objects of name " +
                               object_name + " were found in the world.";
      UnrealLogger::Log(projectairsim::LogLevel::kError,
                        TEXT("SetObjectPose failed. No objects of name %hs"
                             "found in the world."),
                        object_name.c_str());
      throw projectairsim::Error(status_msg);
    } else {
      std::string status_msg = "SetObjectPose failed. Unable to move object " +
                               object_name +
                               ", check if object state is movable!";
      UnrealLogger::Log(projectairsim::LogLevel::kError,
                        TEXT("SetObjectPose failed. Unable to move object %hs"
                             ", check if object state is movable!"),
                        object_name.c_str());
      throw projectairsim::Error(status_msg);
    }
  }
  return result;
}

std::vector<float> WorldSimApi::getObjectScale(const std::string& object_name) {
  std::vector<float> result = {nan, nan, nan};
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &result]() {
        AActor* actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()), &scene_object_map_);

        if (actor) {
          result[0] = actor->GetActorScale().X;
          result[1] = actor->GetActorScale().Y;
          result[2] = actor->GetActorScale().Z;
        }
      },
      true);

  return result;
}

bool WorldSimApi::setObjectScale(const std::string& object_name,
                                 const std::vector<float>& scale) {
  bool result = false;
  std::string status;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &scale, &result]() {
        AActor* actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()), &scene_object_map_);
        if (actor) {
          actor->SetActorScale3D(
              FVector(scale.at(0), scale.at(1), scale.at(2)));
          result = true;
        }
      },
      true);

  if (!result) {
    std::string status_msg = "SetObjectScale failed. No objects of name " +
                             object_name + " were found in the world.";
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("SetObjectScale failed. No objects of name %hs"
                           "found in the world."),
                      object_name.c_str());
    throw projectairsim::Error(status_msg);
  }
  return result;
}

std::vector<std::string> WorldSimApi::listSceneObjects(
    const std::string& name_regex) {
  std::vector<std::string> result;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &name_regex, &result]() {
        result = UnrealHelpers::ListMatchingActors(unreal_world_, name_regex);
      },
      true);
  return result;
}

std::vector<std::string> WorldSimApi::listSimAssets(
    const std::string& name_regex) {
  std::vector<std::string> result;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &name_regex, &result]() {
        result = UnrealHelpers::ListMatchingAssets(unreal_world_, name_regex,
                                                   asset_map_, blueprint_map_,
                                                   skeletal_map_);
      },
      true);
  return result;
}

bool WorldSimApi::enableWeather(bool enable) {
  bool status = UWeatherLib::setWeatherEnabled(unreal_world_, enable);
  return status;
}

bool WorldSimApi::resetWeatherEffect() {
  UnrealHelpers::RunCommandOnGameThread(
      [this]() { UWeatherLib::resetWeatherParams(unreal_world_); }, true);

  return true;
}

bool WorldSimApi::setWeatherParameter(WeatherParameter param, float val) {
  unsigned char param_n = static_cast<unsigned char>(
      UnrealHelpers::toNumeric<WeatherParameter>(param));
  EWeatherParamScalar param_e =
      UnrealHelpers::toEnum<EWeatherParamScalar>(param_n);

  bool status = UWeatherLib::setWeatherParamScalar(unreal_world_, param_e, val);
  return status;
}

std::unordered_map<std::string, float> WorldSimApi::getWeatherParameters() {
  std::unordered_map<std::string, float> result;

  for (int i = 1; i <= static_cast<int>(WeatherParameter::Fog); i++) {
    EWeatherParamScalar param_e = UnrealHelpers::toEnum<EWeatherParamScalar>(i);
    std::string strNumber = std::to_string(i);
    result[strNumber] =
        UWeatherLib::getWeatherParamScalar(unreal_world_, param_e);
  }
  return result;
}

std::string WorldSimApi::getUniqueName(const std::string& object_name) const {
  auto object_name_temp = object_name;
  std::vector<std::string> matching_names = UnrealHelpers::ListMatchingActors(
      unreal_world_, ".*" + object_name_temp + ".*");
  if (matching_names.size() > 0) {
    size_t greatest_num{0}, result{0};
    for (auto match : matching_names) {
      std::string number_extension =
          match.substr(match.find_last_not_of("0123456789") + 1);
      if (number_extension != "") {
        result = std::stoi(number_extension);
        greatest_num = greatest_num > result ? greatest_num : result;
      }
    }
    object_name_temp += std::to_string(greatest_num + 1);
  }

  return object_name_temp;
}

WorldSimApi::Pose WorldSimApi::GetPoseFromGeoPoint(
    const double lat, const double lon, const float alt,
    const std::vector<float>& rotation) {
  auto geo_converter = sim_scene_->GetGeodeticConverter();

  // Convert lat/lon to Pose
  double n, e, d;
  geo_converter->geodetic2Ned(lat, lon, alt, &n, &e, &d);
  auto translation = Vector3(n, e, d);
  auto pose =
      Pose(translation, projectairsim::Quaternion(rotation[0], rotation[1],
                                                  rotation[2], rotation[3]));

  return pose;
}

template <typename T>
T* WorldSimApi::createNewActor(const FActorSpawnParameters& spawn_params,
                               const FTransform& actor_transform,
                               const std::vector<float>& scale,
                               UStaticMesh* static_mesh) {
  T* NewActor = unreal_world_->SpawnActor<T>(
      T::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator,
      spawn_params);  // new

  if (NewActor) {
    UStaticMeshComponent* ObjectComponent =
        NewObject<UStaticMeshComponent>(NewActor);
    if (static_mesh) {
      ObjectComponent->SetStaticMesh(static_mesh);
      ObjectComponent->SetRelativeLocation(FVector(0, 0, 0));
      ObjectComponent->SetWorldScale3D(
          FVector(scale.at(0), scale.at(1), scale.at(2)));
      ObjectComponent->SetHiddenInGame(false, true);
      ObjectComponent->RegisterComponent();

      ObjectComponent->SetMobility(EComponentMobility::Movable);

      NewActor->SetRootComponent(ObjectComponent);
    }

    NewActor->SetActorLocationAndRotation(
        actor_transform.GetLocation(), actor_transform.GetRotation(), false,
        nullptr, ETeleportType::TeleportPhysics);
  }
  return NewActor;
}

template <typename T>
T* WorldSimApi::createNewSkeletalActor(
    const FActorSpawnParameters& spawn_params,
    const FTransform& actor_transform, const std::vector<float>& scale,
    USkeletalMesh* skeletal_mesh) {
  T* NewActor = unreal_world_->SpawnActor<T>(
      T::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator,
      spawn_params);  // new

  if (NewActor) {
    USkeletalMeshComponent* ObjectComponent =
        NewObject<USkeletalMeshComponent>(NewActor);
    if (skeletal_mesh) {
      ObjectComponent->SetSkeletalMesh(skeletal_mesh);
      ObjectComponent->SetRelativeLocation(FVector(0, 0, 0));
      ObjectComponent->SetWorldScale3D(
          FVector(scale.at(0), scale.at(1), scale.at(2)));
      ObjectComponent->SetHiddenInGame(false, true);
      ObjectComponent->RegisterComponent();

      ObjectComponent->SetMobility(EComponentMobility::Movable);

      NewActor->SetRootComponent(ObjectComponent);
    }

    NewActor->SetActorLocationAndRotation(
        actor_transform.GetLocation(), actor_transform.GetRotation(), false,
        nullptr, ETeleportType::TeleportPhysics);
  }
  return NewActor;
}

template <typename T>
T* WorldSimApi::createNewBlueprintActor(
    const FActorSpawnParameters& spawn_params,
    const FTransform& actor_transform, const std::vector<float>& scale,
    UClass* bp_generated_class) {
  if (!bp_generated_class) {
    return nullptr;
  }

  T* NewActor =
      unreal_world_->SpawnActor<T>(bp_generated_class, FVector::ZeroVector,
                                   FRotator::ZeroRotator, spawn_params);

  if (NewActor) {
    NewActor->SetActorLocationAndRotation(
        actor_transform.GetLocation(), actor_transform.GetRotation(), false,
        nullptr, ETeleportType::TeleportPhysics);
  }

  return NewActor;
}

bool WorldSimApi::setMaterial(const std::string& object_name,
                              const std::string& material_path) {
  bool found = false;
  int pos = material_path.find_last_of('/');
  const std::string material_unreal_path =
      material_path + "." + material_path.substr(pos + 1);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &material_unreal_path, &found]() {
        AActor* actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()));

        UMaterial* material = static_cast<UMaterial*>(
            StaticLoadObject(UMaterial::StaticClass(), unreal_world_,
                             *FString(material_unreal_path.c_str())));

        if (!material) {
          UnrealLogger::Log(projectairsim::LogLevel::kError,
                            TEXT("[WorldSimApi::setMaterial] Cannot find "
                                 "specified material."));
        } else {
          if (actor) {
            TArray<UStaticMeshComponent*> components;
            actor->GetComponents<UStaticMeshComponent>(components);
            for (UStaticMeshComponent* staticMeshComponent : components) {
              staticMeshComponent->SetMaterial(0, material);
            }
            found = true;
          } else {
            UnrealLogger::Log(projectairsim::LogLevel::kError,
                              TEXT("[WorldSimApi::setMaterial] Cannot find "
                                   "specified object."));
          }
        }
      },
      true);
  return found;
}

bool WorldSimApi::setTextureFromUrl(const std::string& object_name,
                                    const std::string& url) {
  bool found = false;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &url, &found]() {
        ATextureShuffleActor* actor =
            UnrealHelpers::FindActor<ATextureShuffleActor>(
                unreal_world_, FString(object_name.c_str()));
        if (actor != nullptr) {
          actor->SetTextureFromUrl(FString(url.c_str()));
          found = true;
        }
      },
      true);
  return found;
}

bool WorldSimApi::setTextureFromFile(const std::string& object_name,
                                     const std::vector<uint8_t>& raw_img) {
  bool found = false;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &raw_img, &found]() {
        UMaterial* textureSwapMat = getTextureSwapMaterial();

        if (textureSwapMat) {
          AActor* actor = UnrealHelpers::FindActor<AActor>(
              unreal_world_, FString(object_name.c_str()));
          if (actor) {
            TArray<uint8> RawData;
            auto ImgDataPtr = reinterpret_cast<const uint8*>(raw_img.data());
            for (int Idx = 0; Idx < raw_img.size(); ++Idx) {
              RawData.Add(*ImgDataPtr);
              ImgDataPtr++;
            }
            UTexture2D* texture = FImageUtils::ImportBufferAsTexture2D(RawData);

            if (!texture) {
              UnrealLogger::Log(
                  projectairsim::LogLevel::kError,
                  TEXT("[WorldSimApi::setTextureFromFile] Cannot import "
                       "specified texture."));
            } else {
              setObjectTexture(actor, texture, textureSwapMat);
              found = true;
            }
          } else {
            UnrealLogger::Log(
                projectairsim::LogLevel::kError,
                TEXT("[WorldSimApi::setTextureFromFile] Cannot find "
                     "specified object."));
          }
        }
      },
      true);
  return found;
}

bool WorldSimApi::setTextureFromFileServiceMethod(
    const std::string& object_name, const nlohmann::json::binary_t& raw_img) {
  // Service methods needs explicit nlohmann::json::binary_t type for binary
  // byte array for it to be parsed properly from the JSON binary-type data,
  // but its underlying C++ data type is a vector<uint8_t> so it can be passed
  // through directly.
  return setTextureFromFile(object_name, raw_img);
}

bool WorldSimApi::setTextureFromPackagedAsset(const std::string& object_name,
                                              const std::string& texture_path) {
  bool found = false;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &texture_path, &found]() {
        UMaterial* textureSwapMat = getTextureSwapMaterial();

        if (textureSwapMat) {
          AActor* actor = UnrealHelpers::FindActor<AActor>(
              unreal_world_, FString(object_name.c_str()));
          if (actor) {
            UTexture2D* texture = static_cast<UTexture2D*>(
                StaticLoadObject(UTexture2D::StaticClass(), unreal_world_,
                                 *FString(texture_path.c_str())));

            if (!texture) {
              UnrealLogger::Log(
                  projectairsim::LogLevel::kError,
                  TEXT("[WorldSimApi::setMaterialFromTexture] Cannot find "
                       "specified texture."));
            } else {
              setObjectTexture(actor, texture, textureSwapMat);
              found = true;
            }
          } else {
            UnrealLogger::Log(
                projectairsim::LogLevel::kError,
                TEXT("[WorldSimApi::setMaterialFromTexture] Cannot find "
                     "specified object."));
          }
        }
      },
      true);
  return found;
}

void WorldSimApi::setObjectTexture(AActor* actor, UTexture2D* texture,
                                   UMaterial* textureSwapMat) {
  TArray<UStaticMeshComponent*> components;
  actor->GetComponents<UStaticMeshComponent>(components);

  for (UStaticMeshComponent* staticMeshComponent : components) {
    UMaterialInstanceDynamic* dynamicMaterial =
        UMaterialInstanceDynamic::Create(textureSwapMat, staticMeshComponent);
    setMaterialProperties(dynamicMaterial, texture);
    staticMeshComponent->SetMaterial(0, dynamicMaterial);
  }
}

UMaterial* WorldSimApi::getTextureSwapMaterial() const {
  UMaterial* textureSwapMat = Cast<UMaterial>(
      StaticLoadObject(UMaterial::StaticClass(), unreal_world_,
                       TEXT("/ProjectAirSim/Runtime/"
                            "RuntimeMeshMaterial.RuntimeMeshMaterial")));

  if (!textureSwapMat) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[WorldSimApi::setMaterialFromTexture] Cannot load "
                           "RuntimeMeshMaterial."));
  }
  return textureSwapMat;
}

void WorldSimApi::setMaterialProperties(
    UMaterialInstanceDynamic* dynamic_material, UTexture2D* texture) {
  // Set color values for using base color textures and no emissive color
  const FLinearColor BaseColor(1.0f, 1.0f, 1.0f, 1.0f);
  const FLinearColor EmissiveColor(0.0f, 0.0f, 0.0f, 0.0f);
  const float MetallicFactor = 0.0f;
  const float RoughnessFactor = 1.0f;
  const float SpecularFactor = 0.5f;

  dynamic_material->SetTextureParameterValue("BaseColorTex", texture);
  dynamic_material->SetScalarParameterValue("BaseColorTexRatio", 1.0f);

  dynamic_material->SetVectorParameterValue("EmissiveColor", EmissiveColor);
  dynamic_material->SetScalarParameterValue("EmissiveTexRatio", 0.0f);

  dynamic_material->SetScalarParameterValue("MetallicFactor", MetallicFactor);
  dynamic_material->SetScalarParameterValue("MetallicTexRatio", 0.0f);

  dynamic_material->SetScalarParameterValue("RoughnessFactor", RoughnessFactor);
  dynamic_material->SetScalarParameterValue("RoughnessTexRatio", 0.0f);

  dynamic_material->SetScalarParameterValue("SpecularFactor", SpecularFactor);
  dynamic_material->SetScalarParameterValue("SpecularTexRatio", 0.0f);
}

std::vector<std::string> WorldSimApi::swapTextures(const std::string& tag,
                                                   int tex_id, int component_id,
                                                   int material_id) {
  std::vector<std::string> swappedObjectNames;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &tag, tex_id, component_id, material_id, &swappedObjectNames]() {
        // Split the tag string into individual tags.
        TArray<FString> splitTags;
        FString notSplit = FString(tag.c_str());
        FString next = "";
        while (notSplit.Split(",", &next, &notSplit)) {
          next.TrimStartInline();
          splitTags.Add(next);
        }
        notSplit.TrimStartInline();
        splitTags.Add(notSplit);

        // Texture swap on actors that have all of those tags.
        TArray<AActor*> shuffleables;
        UnrealHelpers::FindAllActors<ATextureShuffleActor>(unreal_world_,
                                                           shuffleables);
        for (auto* shuffler : shuffleables) {
          bool invalidChoice = false;
          for (auto required_tag : splitTags) {
            invalidChoice |= !shuffler->ActorHasTag(FName(*required_tag));
            if (invalidChoice) break;
          }

          if (invalidChoice) continue;
          dynamic_cast<ATextureShuffleActor*>(shuffler)->SwapTexture(
              tex_id, component_id, material_id);
          swappedObjectNames.push_back(TCHAR_TO_UTF8(*shuffler->GetName()));
        }
      },
      true);
  return swappedObjectNames;
}

bool WorldSimApi::setLightIntensity(const std::string& object_name,
                                    float new_intensity) {
  bool result = false;

  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &new_intensity, &result]() {
        ALightActorBase* lightActor = UnrealHelpers::FindActor<ALightActorBase>(
            unreal_world_, FString(object_name.c_str()));

        if (lightActor) {
          result = lightActor->SetIntensity(new_intensity);
        } else {
          UnrealLogger::Log(projectairsim::LogLevel::kError,
                            TEXT("[WorldSimApi::setLightIntensity] Cannot find "
                                 "specified object."));
        }
      },
      true);
  return result;
}

bool WorldSimApi::setLightColor(const std::string& object_name,
                                const std::vector<float>& color_rgb) {
  if (color_rgb.size() != 3) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[WorldSimApi::setLightColor] Invalid color_rgb."));
    return false;
  }

  FColor color =
      FLinearColor{color_rgb[0], color_rgb[1], color_rgb[2], 1.0f}.ToFColor(
          true);

  bool result = false;

  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &color, &result]() {
        ALightActorBase* lightActor = UnrealHelpers::FindActor<ALightActorBase>(
            unreal_world_, FString(object_name.c_str()));

        if (lightActor) {
          result = lightActor->SetLightFColor(color);
        } else {
          UnrealLogger::Log(projectairsim::LogLevel::kError,
                            TEXT("[WorldSimApi::setLightColor] Cannot find "
                                 "specified object."));
        }
      },
      true);
  return result;
}

bool WorldSimApi::setLightRadius(const std::string& object_name,
                                 float new_radius) {
  bool result = false;

  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &new_radius, &result]() {
        ALightActorBase* lightActor = UnrealHelpers::FindActor<ALightActorBase>(
            unreal_world_, FString(object_name.c_str()));

        if (lightActor) {
          result = lightActor->SetRadius(new_radius);
        } else {
          UnrealLogger::Log(projectairsim::LogLevel::kError,
                            TEXT("[WorldSimApi::setLightRadius] Cannot find "
                                 "specified object."));
        }
      },
      true);
  return result;
}

void WorldSimApi::InitSegmentationIDs() {
  const auto& seg_settings = sim_scene_->GetSegmentationSettings();
  if (seg_settings.initialize_ids == false) return;

  for (TObjectIterator<UStaticMeshComponent> comp; comp; ++comp) {
    UnrealHelpers::InitSegmentationID(*comp, seg_settings.ignore_existing,
                                      seg_settings.use_owner_name, seg_map_);
  }
  for (TObjectIterator<USkinnedMeshComponent> comp; comp; ++comp) {
    UnrealHelpers::InitSegmentationID(*comp, seg_settings.ignore_existing,
                                      seg_settings.use_owner_name, seg_map_);
  }
  for (TObjectIterator<ALandscapeProxy> comp; comp; ++comp) {
    UnrealHelpers::InitSegmentationID(*comp, seg_settings.ignore_existing,
                                      seg_settings.use_owner_name, seg_map_);
  }
}

bool WorldSimApi::SetSegmentationIDByName(const std::string& mesh_name,
                                          int segmentation_id,
                                          bool is_name_regex,
                                          bool use_owner_name) {
  bool success = false;
  UnrealHelpers::RunCommandOnGameThread(
      [&seg_map_ = seg_map_, &mesh_name, &segmentation_id, &is_name_regex,
       &use_owner_name, &success]() {
        success = UnrealHelpers::SetSegmentationIDByName(
            mesh_name, segmentation_id, is_name_regex, use_owner_name,
            seg_map_);
      },
      true);
  return success;
}

int WorldSimApi::GetSegmentationIDByName(const std::string& mesh_name,
                                         bool use_owner_name) {
  int id = -1;
  UnrealHelpers::RunCommandOnGameThread(
      [&seg_map_ = seg_map_, &mesh_name, &use_owner_name, &id]() {
        auto id_ptr = seg_map_.Find(FString(mesh_name.c_str()));
        if (id_ptr) {
          id = *id_ptr;
        }
      },
      true);
  return id;
}

nlohmann::json WorldSimApi::GetSegmentationIDMap() {
  nlohmann::json seg_json = nlohmann::json({});
  UnrealHelpers::RunCommandOnGameThread(
      [&seg_map_ = seg_map_, &seg_json]() {
        for (const auto& [name, id] : seg_map_) {
          seg_json[TCHAR_TO_UTF8(*name)] = id;
        }
      },
      true);
  return seg_json;
}

AActor* WorldSimApi::FindActor(const std::string& object_name) {
  AActor* actor;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &object_name, &actor]() {
        actor = UnrealHelpers::FindActor<AActor>(
            unreal_world_, FString(object_name.c_str()), &scene_object_map_);
      },
      true);
  return actor;
}

std::vector<int> WorldSimApi::createVoxelGrid(
    const WorldSimApi::Pose& position, const float x_size, const float y_size,
    const float z_size, const float res, const int n_z_resolution,
    std::vector<std::string> actors_to_ignore, bool use_segmentation) {
  int ncells_x = x_size / res;
  int ncells_y = y_size / res;
  int ncells_z = z_size / (res * n_z_resolution);

  voxel_grid_.resize(ncells_x * ncells_y * ncells_z);
  float scale_cm = res * 100;
  FCollisionQueryParams params;
  params.bFindInitialOverlaps = true;
  params.bTraceComplex = false;
  params.TraceTag = "";

  for (auto& actor_name : actors_to_ignore) {
    auto actor = FindActor(actor_name);

    if (actor != nullptr) params.AddIgnoredActor(actor);
  }

  FTransform origin_transform = UnrealTransform::FromGlobalNed(position);
  auto position_in_UE = origin_transform.GetLocation();
  UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                    TEXT("Generating Voxel Grid with %d voxels"),
                    ncells_x * ncells_y * ncells_z);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &ncells_x, &ncells_z, &ncells_y, &n_z_resolution, &params, &scale_cm,
       &position_in_UE, &use_segmentation]() {
        ParallelFor(ncells_x, [&](int32 i) {
          ParallelFor(ncells_y, [&](int32 j) {
            ParallelFor(ncells_z, [&](int32 k) {
              TArray<FOverlapResult> overlapping_objects;
              int idx = i + ncells_x * (k + ncells_z * j);  // first index is x, then z, then y
              FVector vposition = FVector((i - ncells_x / 2) * scale_cm,
                                          (j - ncells_y / 2) * scale_cm,
                                          (k - ncells_z / 2) * (scale_cm * n_z_resolution)) +
                                  position_in_UE;
              bool overlapping = unreal_world_->OverlapMultiByChannel(
                  overlapping_objects, vposition, FQuat::Identity,
                  ECollisionChannel::ECC_Visibility,
                  FCollisionShape::MakeBox(FVector(scale_cm / 2, scale_cm / 2, n_z_resolution * scale_cm / 2)), params);
              if (overlapping) {
                if (use_segmentation) {
                  int max_segmentationID = 0;
                  FString max_object_name = TEXT("None");

                  for (const auto& overlap : overlapping_objects) {
                      UPrimitiveComponent* component = overlap.GetComponent();
                      int segID = component->CustomDepthStencilValue;
                      FString object_name = component->GetOwner()->GetName();

                      // Save the object name with the highest segmentation ID
                      if (segID > max_segmentationID) {
                          max_segmentationID = segID;
                          max_object_name = object_name;
                      }
                  }
                  voxel_grid_[idx] = max_segmentationID;
                } else {
                  voxel_grid_[idx] = 1;
                }
              } else {
                voxel_grid_[idx] = 0;
              }
            });
          });
        });
      },
      true);
  UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                    TEXT("Voxel Grid generated with %d voxels"),
                    voxel_grid_.size());

  return voxel_grid_;
}

bool WorldSimApi::debugFlushPersistentMarkers() {
  UnrealHelpers::RunCommandOnGameThread(
      [this]() { FlushPersistentDebugLines(unreal_world_); }, true);
  UnrealHelpers::RunCommandOnGameThread(
      [this]() { FlushDebugStrings(unreal_world_); }, true);
  return true;
}

bool WorldSimApi::debugPlotPoints(const std::vector<std::vector<float>>& points,
                                  const std::vector<float>& color_rgba,
                                  float size, float duration,
                                  bool is_persistent) {
  if (color_rgba.size() != 4) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotPoints] Invalid color_rgba."));
    return false;
  }

  FColor color =
      FLinearColor{color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]}
          .ToFColor(true);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &points, &color, size, duration, is_persistent]() {
        for (const auto& point : points) {
          Vector3 vec_point(point[0], point[1], point[2]);
          DrawDebugPoint(unreal_world_,
                         UnrealTransform::NedToUnrealLinear(vec_point), size,
                         color, is_persistent, duration);
        }
      },
      true);
  return true;
}

bool WorldSimApi::debugPlotSolidLine(
    const std::vector<std::vector<float>>& points,
    const std::vector<float>& color_rgba, float thickness, float duration,
    bool is_persistent) {
  if (color_rgba.size() != 4) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotSolidLine] Invalid color_rgba."));
    return false;
  }

  FColor color =
      FLinearColor{color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]}
          .ToFColor(true);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &points, &color, thickness, duration, is_persistent]() {
        for (size_t idx = 0; idx < points.size() - 1; ++idx) {
          Vector3 start_point(points[idx][0], points[idx][1], points[idx][2]);
          Vector3 end_point(points[idx + 1][0], points[idx + 1][1],
                            points[idx + 1][2]);
          DrawDebugLine(unreal_world_,
                        UnrealTransform::NedToUnrealLinear(start_point),
                        UnrealTransform::NedToUnrealLinear(end_point), color,
                        is_persistent, duration, 0, thickness);
        }
      },
      true);
  return true;
}

bool WorldSimApi::debugPlotDashedLine(
    const std::vector<std::vector<float>>& points,
    const std::vector<float>& color_rgba, float thickness, float duration,
    bool is_persistent) {
  if (color_rgba.size() != 4) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotDashedLine] Invalid color_rgba."));
    return false;
  }

  FColor color =
      FLinearColor{color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]}
          .ToFColor(true);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &points, &color, thickness, duration, is_persistent]() {
        for (size_t idx = 0; idx < points.size() - 1; idx += 2) {
          Vector3 start_point(points[idx][0], points[idx][1], points[idx][2]);
          Vector3 end_point(points[idx + 1][0], points[idx + 1][1],
                            points[idx + 1][2]);
          DrawDebugLine(unreal_world_,
                        UnrealTransform::NedToUnrealLinear(start_point),
                        UnrealTransform::NedToUnrealLinear(end_point), color,
                        is_persistent, duration, 0, thickness);
        }
      },
      true);
  return true;
}

bool WorldSimApi::debugPlotArrows(
    const std::vector<std::vector<float>>& points_start,
    const std::vector<std::vector<float>>& points_end,
    const std::vector<float>& color_rgba, float thickness, float arrow_size,
    float duration, bool is_persistent) {
  if (points_start.size() != points_end.size()) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotArrows] Number of points_start "
             "and points_end do not match."));
    return false;
  }

  if (color_rgba.size() != 4) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotArrows] Invalid color_rgba."));
    return false;
  }

  FColor color =
      FLinearColor{color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]}
          .ToFColor(true);
  UnrealHelpers::RunCommandOnGameThread(
      [this, &points_start, &points_end, &color, thickness, arrow_size,
       duration, is_persistent]() {
        for (int idx = 0; idx < points_start.size(); ++idx) {
          Vector3 start(points_start[idx][0], points_start[idx][1],
                        points_start[idx][2]);
          Vector3 end(points_end[idx][0], points_end[idx][1],
                      points_end[idx][2]);
          DrawDebugDirectionalArrow(
              unreal_world_, UnrealTransform::NedToUnrealLinear(start),
              UnrealTransform::NedToUnrealLinear(end), arrow_size, color,
              is_persistent, duration, 0, thickness);
        }
      },
      true);
  return true;
}

bool WorldSimApi::debugPlotStrings(
    const std::vector<std::string>& strings,
    const std::vector<std::vector<float>>& positions, float scale,
    const std::vector<float>& color_rgba, float duration) {
  if (strings.size() != positions.size()) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[WorldSimApi::debugPlotStrings] Number of strings "
                           "and positions do not match."));
    return false;
  }

  if (color_rgba.size() != 4) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotStrings] Invalid color_rgba."));
    return false;
  }

  FColor color =
      FLinearColor{color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]}
          .ToFColor(true);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &strings, &positions, &color, scale, duration]() {
        for (size_t idx = 0; idx < positions.size(); ++idx) {
          Vector3 position(positions[idx][0], positions[idx][1],
                           positions[idx][2]);
          DrawDebugString(unreal_world_,
                          UnrealTransform::NedToUnrealLinear(position),
                          FString(strings[idx].c_str()), NULL, color, duration,
                          false, scale);
        }
      },
      true);
  return true;
}

bool WorldSimApi::debugPlotTransforms(const std::vector<Pose>& poses,
                                      float scale, float thickness,
                                      float duration, bool is_persistent) {
  UnrealHelpers::RunCommandOnGameThread(
      [this, &poses, scale, thickness, duration, is_persistent]() {
        for (const auto& pose : poses) {
          FTransform transform = UnrealTransform::FromGlobalNed(pose);
          DrawDebugCoordinateSystem(unreal_world_, transform.GetLocation(),
                                    transform.Rotator(), scale, is_persistent,
                                    duration, 0, thickness);
        }
      },
      true);
  return true;
}

bool WorldSimApi::debugPlotTransformsWithNames(
    const std::vector<Pose>& poses, const std::vector<std::string>& names,
    float tf_scale, float tf_thickness, float text_scale,
    const std::vector<float>& text_color_rgba, float duration) {
  if (poses.size() != names.size()) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[WorldSimApi::debugPlotTransformsWithNames] Number of poses "
             "and names do not match."));
    return false;
  }

  if (text_color_rgba.size() != 4) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[WorldSimApi::debugPlotTransformsWithNames] "
                           "Invalid text_color_rgba."));
    return false;
  }

  FColor color = FLinearColor{text_color_rgba[0], text_color_rgba[1],
                              text_color_rgba[2], text_color_rgba[3]}
                     .ToFColor(true);

  UnrealHelpers::RunCommandOnGameThread(
      [this, &poses, &names, &color, tf_scale, tf_thickness, text_scale,
       duration]() {
        for (size_t idx = 0; idx < poses.size(); ++idx) {
          FTransform transform = UnrealTransform::FromGlobalNed(poses[idx]);
          DrawDebugCoordinateSystem(unreal_world_, transform.GetLocation(),
                                    transform.Rotator(), tf_scale, false,
                                    duration, 0, tf_thickness);

          DrawDebugString(unreal_world_, transform.GetLocation(),
                          FString(names[idx].c_str()), NULL, color, duration,
                          false, text_scale);
        }
      },
      true);
  return true;
}

bool WorldSimApi::ImportNEDTrajectory(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& pose_x, const std::vector<float>& pose_y,
    const std::vector<float>& pose_z, const std::vector<float>& pose_roll,
    const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
    const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
    const std::vector<float>& vel_z_lin) {
  // we cannot have duplicate trajectory names
  if (sim_scene_->GetTrajectoryPtrByName(traj_name) == nullptr) {
    sim_scene_->AddNEDTrajectory(traj_name, time, pose_x, pose_y, pose_z,
                                 pose_roll, pose_pitch, pose_yaw, vel_x_lin,
                                 vel_y_lin, vel_z_lin);
    return true;
  }

  UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                    TEXT("'%hs' was not imported. A duplicate trajectory name "
                         "already exists."),
                    traj_name.c_str());
  return false;
}

bool WorldSimApi::ImportGeoTrajectory(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& latitudes, const std::vector<float>& longitudes,
    const std::vector<float>& altitudes, const std::vector<float>& pose_roll,
    const std::vector<float>& pose_pitch, const std::vector<float>& pose_yaw,
    const std::vector<float>& vel_x_lin, const std::vector<float>& vel_y_lin,
    const std::vector<float>& vel_z_lin) {
  std::vector<float> x(time.size(), 0);
  std::vector<float> y(time.size(), 0);
  std::vector<float> z(time.size(), 0);

  GetNEDFromGeoCoords(latitudes, longitudes, altitudes, x, y, z);

  return ImportNEDTrajectory(traj_name, time, x, y, z, pose_roll, pose_pitch,
                             pose_yaw, vel_x_lin, vel_y_lin, vel_z_lin);
}

bool WorldSimApi::ImportGeoCoordinates(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& latitudes, const std::vector<float>& longitudes,
    const std::vector<float>& altitudes, const std::vector<float>& vel_x_lin,
    const std::vector<float>& vel_y_lin, const std::vector<float>& vel_z_lin) {
  std::vector<float> x(time.size(), 0);
  std::vector<float> y(time.size(), 0);
  std::vector<float> z(time.size(), 0);

  GetNEDFromGeoCoords(latitudes, longitudes, altitudes, x, y, z);

  // // auto rotate actor to the next waypoint
  std::vector<float> roll(time.size(), 0);
  std::vector<float> pitch(time.size(), 0);
  std::vector<float> yaw(time.size(), 0);

  GetRotations(x, y, z, roll, pitch, yaw);

  return ImportNEDTrajectory(traj_name, time, x, y, z, roll, pitch, yaw,
                             vel_x_lin, vel_y_lin, vel_z_lin);
}

// helper function to get x, y, z vectors from lat, lon, alt vectors
void WorldSimApi::GetNEDFromGeoCoords(const std::vector<float>& latitudes,
                                      const std::vector<float>& longitudes,
                                      const std::vector<float>& altitudes,
                                      std::vector<float>& x,
                                      std::vector<float>& y,
                                      std::vector<float>& z) {
  std::vector<Pose> poses;
  poses.reserve(latitudes.size());

  // use std::vector<float>(4, 0) as dummy holder for quaternion as a vector
  for (size_t idx = 0; idx < latitudes.size(); ++idx) {
    poses.push_back(GetPoseFromGeoPoint(latitudes[idx], longitudes[idx],
                                        altitudes[idx],
                                        std::vector<float>(4, 0)));
  }

  for (size_t idx = 0; idx < poses.size(); ++idx) {
    x[idx] = poses[idx].translation_.x();
    y[idx] = poses[idx].translation_.y();
    z[idx] = poses[idx].translation_.z();
  }
}

void WorldSimApi::GetRotations(const std::vector<float>& x,
                               const std::vector<float>& y,
                               const std::vector<float>& z,
                               std::vector<float>& roll,
                               std::vector<float>& pitch,
                               std::vector<float>& yaw) {
  // computes angles between projection of (x2, y2, z2) onto each plane with
  // (x1, y1, z1) translated to origin
  for (size_t idx = 1; idx < roll.size(); ++idx) {
    // need to shift curr point to origin
    float dx = x[idx] - x[idx - 1];
    float dy = y[idx] - y[idx - 1];
    float dz = z[idx] - z[idx - 1];

    yaw[idx - 1] = atan2f(dy, dx);
    pitch[idx - 1] = atan2f(-dz, sqrt(dx * dx + dy * dy));
  }
  pitch.back() = pitch.rbegin()[1];
  yaw.back() = yaw.rbegin()[1];
}

bool WorldSimApi::SetEnvActorTrajectory(
    const std::string& env_actor_name, const std::string& traj_name,
    const float time_offset, const float x_offset, const float y_offset,
    const float z_offset, const float roll_offset, const float pitch_offset,
    const float yaw_offset, const bool to_loop) {
  auto trajectory_ptr = sim_scene_->GetTrajectoryPtrByName(traj_name);

  if (trajectory_ptr) {
    auto& env_actors = sim_scene_->GetEnvActors();
    int env_actor_idx = -1;
    if (!env_actors.empty()) {
      env_actor_idx = sim_scene_->GetEnvActorIndex(env_actor_name);
    }
    if (env_actor_idx != -1) {
      auto& env_actor_ref = env_actors[env_actor_idx];
      auto& env_actor =
          static_cast<projectairsim::EnvActor&>(env_actor_ref.get());

      env_actor.SetTrajectory(trajectory_ptr, to_loop, time_offset, x_offset,
                              y_offset, z_offset, roll_offset, pitch_offset,
                              yaw_offset);
      return true;
    }
  }

  UnrealLogger::Log(
      projectairsim::LogLevel::kWarning,
      TEXT("SetTrajectory failed for EnvActor '%hs'. The trajectory '%hs'"
           " was not found. The previous trajectory will be used."),
      env_actor_name.c_str(), traj_name.c_str());
  return false;
}

bool WorldSimApi::SetEnvActorLinkRotAngle(const std::string& env_actor_name,
                                          const std::string& link_name,
                                          const float angle_deg) {
  auto& env_actors = sim_scene_->GetEnvActors();
  int env_actor_idx = sim_scene_->GetEnvActorIndex(env_actor_name);

  bool success = false;
  if (env_actor_idx != -1) {
    auto& env_actor_ref = env_actors[env_actor_idx];
    auto& env_actor =
        static_cast<projectairsim::EnvActor&>(env_actor_ref.get());
    success = env_actor.SetLinkRotationAngle(link_name, angle_deg);

    if (success) {
      UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                        TEXT("Rotation angle for link '%hs' set to %f deg for "
                             "EnvActor '%hs'."),
                        link_name.c_str(), angle_deg, env_actor_name.c_str());
    } else {
      UnrealLogger::Log(
          projectairsim::LogLevel::kWarning,
          TEXT("The link '%hs' for EnvActor '%hs' was not found."),
          link_name.c_str(), env_actor_name.c_str());
    }
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                      TEXT("EnvActor '%hs' was not found."),
                      env_actor_name.c_str());
  }
  return success;
}

bool WorldSimApi::SetEnvActorLinkRotRate(const std::string& env_actor_name,
                                         const std::string& link_name,
                                         const float rotation_deg_per_sec) {
  auto& env_actors = sim_scene_->GetEnvActors();
  int env_actor_idx = sim_scene_->GetEnvActorIndex(env_actor_name);

  bool success = false;
  if (env_actor_idx != -1) {
    auto& env_actor_ref = env_actors[env_actor_idx];
    auto& env_actor =
        static_cast<projectairsim::EnvActor&>(env_actor_ref.get());
    success = env_actor.SetLinkRotationRate(link_name, rotation_deg_per_sec);

    if (success) {
      UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                        TEXT("Rotation rate for link '%hs' set to %f deg/s for "
                             "EnvActor '%hs'."),
                        link_name.c_str(), rotation_deg_per_sec,
                        env_actor_name.c_str());
    } else {
      UnrealLogger::Log(
          projectairsim::LogLevel::kWarning,
          TEXT("The link '%hs' for EnvActor '%hs' was not found."),
          link_name.c_str(), env_actor_name.c_str());
    }
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                      TEXT("EnvActor '%hs' was not found."),
                      env_actor_name.c_str());
  }
  return success;
}

float WorldSimApi::GetZAtPoint(const float x, const float y) {
  // x and y in m
  float distance;
  static const float pointZ = 1000;  // m. Temp Z value to place the point

  // Create an FVector using the provided X, Y, and Z coordinates.
  const FVector LocVector = FVector(x, y, pointZ);
  // convert vector to cm
  const FVector LocVecCm = LocVector * 100.0f;
  // Rotation of the component is fixed to downward facing
  const FRotator DownwardRotation = FRotator(-90.0f, 0.0f, 0.0f);

  const FVector RayDirectionVector =
      UKismetMathLibrary::GetForwardVector(DownwardRotation);

  // Calculate "EndTrace": point corresponding to end of ray trace.
  const FVector EndTrace =
      LocVecCm + (projectairsim::TransformUtils::ToCentimeters(pointZ) *
                  RayDirectionVector);

  // Shoot ray via LineTraceSingleByChannel, result is saved in HitInfo
  FCollisionQueryParams TraceParams;
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

  FHitResult HitInfo(ForceInit);

  HitInfo = GetHitResult(LocVecCm, EndTrace, TraceParams);

  // float distance = HitInfo.Distance - pointZ;
  distance = pointZ - HitInfo.Distance / 100;

  return distance;
}

FHitResult WorldSimApi::GetHitResult(const FVector& start, const FVector& end,
                                     FCollisionQueryParams& TraceParams) {
  bool status = false;
  FHitResult HitInfo(ForceInit);
  UnrealHelpers::RunCommandOnGameThread(
      [this, &start, &end, &TraceParams, &HitInfo]() {
        unreal_world_->LineTraceSingleByChannel(
            HitInfo, start, end, ECC_Visibility, TraceParams,
            FCollisionResponseParams::DefaultResponseParam);
      },
      true);
  return HitInfo;
}
