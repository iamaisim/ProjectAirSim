// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once
#include "ASCDecl.h"
#include "Client.h"
#include "Common.h"
#include "Status.h"
#include "Types.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class World {
 public:
  ASC_DECL World(void) noexcept;
  ASC_DECL ~World();

  // Initializes the world.  If a scene config file name is given, the
  // scene is loaded into the simulation.  If a scene config file name is
  // not given, this object reflects scene currently loaded into the
  // simulator.  (Note that the scene configuration JSON will not be
  // available when using the currently loaded scene.)
  //
  // Arguments:
  //   pclient                  Pointer to client object
  //   scene_config_name        Path to scene config file
  //   sim_config_path          Path to directory containing files referenced by
  //   the scene config file delay_after_loading_sec  Time period to sleep after
  //   loading the scene (seconds) sim_instance_idx         Simulation instance
  //   number to append to scene ID (used for parallel cooperating simulations)
  //
  // Returns:
  //   (Return)  Initialization status
  ASC_DECL Status Initialize(std::shared_ptr<Client>& pclient,
                             const std::string& scene_config_name = "",
                             const std::string& sim_config_path = "sim_config/",
                             float delay_after_load_sec = 0,
                             int sim_instance_idx = -1);

  // Returns a voxel occupation grid for the specified volume.  The returned
  //  boolean array is the 3D voxel occupancy map returned as a linear array.
  //  A true value indicates the voxel is occupied while false means the voxel
  //  is empty.  The number of elements in the array is (nx * ny * nz) where:
  //       nx = floor(x_size / resolution)
  //       ny = floor(y_size / resolution)
  //       nz = floor(z_size / resolution)
  //
  //  The entries in the array are for voxels in the order:
  //       (x0, y0, z0), (x0, y0, z1), ..., (x0, y0, znz-1),
  //       (x0, y1, z0), (x0, y1, z1), ..., (x0, y1, znz-1),
  //       ...
  //       (x0, yny-1, z0), (x0, yny-1, z1), ..., (x0, yny-1, znz-1),
  //       (x1, y0, z0),    (x1, y0, z1),    ..., (x1, y0, znz-1),
  //       ...
  //       (xnx-1, yny-1, z0), (xnx-1, yny-1, z1),..., (xnx-1, yny-1, znz-1)
  //
  //  where (x0, y0, and z0) is the voxel at the minimum north, east, down
  //  position (far back, left, and up) while (xnx-1, ynx-1, znz-1) is at the
  //  maximum north, east, down position (far front, right, and down.)  Thus,
  //  the voxel at position (i, j, k) is at index (i * ny * nz + j * nz + k)
  //  of the returned boolean array.
  //
  //
  // Arguments:
  //   position         Center of volume to inspect (orientation ignored)
  //   x_size           Volume length along X-axis (meters)
  //   y_size           Volume length along Y-axis (meters)
  //   z_size           Volume length along Z-axis (meters)
  //   resolution       Length of each edge of a voxel (meters)
  //   pbool_array_ret  The buffer to get the boolean area indicating occupancy
  //   actors_to_ignore IDs of actors to ignore for occupancy
  //   write_file       If true, write the voxel grid to file_path in binvox
  //   format file_path        Path to output file if write_file is true
  //
  // Returns:
  //   (Return)  Call status
  //   pbool_array_ret  If call status is Status::OK, updated with the occupancy
  //   array
  ASC_DECL Status
  CreateVoxelGrid(const struct Pose& position, int x_size, int y_size,
                  int z_size, float resolution, BoolArray* pbool_array_ret,
                  const std::vector<std::string>& actors_to_ignore =
                      std::vector<std::string>(),
                  bool write_file = false,
                  std::string file_path = "./voxel_grid.binvox") const;

  // Return the scene configuration JSON loaded for this world (empty if
  // Initialize() was not called with a scene config file name.)
  ASC_DECL const json& GetConfiguration(void) const;

  // Return the names of the drones in the scene
  ASC_DECL const VecStr& GetDrones(void) const;

  // Return the prefix for topics associated with this scene
  ASC_DECL const char* GetParentTopic(void) const;

  // Simulation time and stepping
  ASC_DECL Status GetSimClockType(std::string* pstr_clock_type_out) const;
  ASC_DECL Status GetSimTime(std::int64_t* ptime_nanos_out) const;
  ASC_DECL Status Pause(std::string* pstr_state_out = nullptr);
  ASC_DECL Status Resume(std::string* pstr_state_out = nullptr);
  ASC_DECL Status IsPaused(bool* pf_is_paused_out) const;
  ASC_DECL Status ContinueForSimTime(std::int64_t delta_time_nanos,
                                     bool wait_until_complete,
                                     std::int64_t* ptime_nanos_out);
  ASC_DECL Status ContinueUntilSimTime(std::int64_t target_time_nanos,
                                       bool wait_until_complete,
                                       std::int64_t* ptime_nanos_out);
  ASC_DECL Status ContinueForNSteps(int n_steps, bool wait_until_complete,
                                    std::int64_t* ptime_nanos_out);
  ASC_DECL Status ContinueForSingleStep(bool wait_until_complete,
                                        std::int64_t* ptime_nanos_out);

  // Environment
  ASC_DECL Status SetSunlightIntensity(float intensity, bool* pf_success_out);
  ASC_DECL Status GetSunlightIntensity(float* pintensity_out) const;
  ASC_DECL Status SetCloudShadowStrength(float strength, bool* pf_success_out);
  ASC_DECL Status GetCloudShadowStrength(float* pstrength_out) const;
  ASC_DECL Status SetWindVelocity(float v_x, float v_y, float v_z,
                                  bool* pf_success_out);
  ASC_DECL Status GetWindVelocity(Vector3* pwind_velocity_out) const;

  // Scene objects and assets
  ASC_DECL Status ListActors(VecStr* pvecstr_actor_names_out) const;
  ASC_DECL Status ListObjects(const std::string& name_regex,
                              VecStr* pvecstr_object_names_out) const;
  ASC_DECL Status ListAssets(const std::string& name_regex,
                             VecStr* pvecstr_asset_names_out) const;
  ASC_DECL Status GetObjectPose(const std::string& object_name,
                                json* pjson_pose_out) const;
  ASC_DECL Status GetObjectPoses(const VecStr& object_names,
                                 json* pjson_poses_out) const;
  ASC_DECL Status SetObjectPose(const std::string& object_name,
                                const json& json_pose, bool teleport,
                                bool* pf_success_out);
  ASC_DECL Status GetObjectScale(const std::string& object_name,
                                 std::vector<float>* pvec_scale_out) const;
  ASC_DECL Status SetObjectScale(const std::string& object_name,
                                 const std::vector<float>& scale,
                                 bool* pf_success_out);
  ASC_DECL Status SpawnObject(const std::string& object_name,
                              const std::string& asset_path,
                              const json& json_pose,
                              const std::vector<float>& scale,
                              bool enable_physics,
                              std::string* pstr_spawned_name_out = nullptr);
  ASC_DECL Status SpawnObjectFromFile(
      const std::string& object_name, const std::string& file_format,
      const VecB& asset_bytes, bool is_binary, const json& json_pose,
      const std::vector<float>& scale, bool enable_physics,
      std::string* pstr_spawned_name_out = nullptr);
  ASC_DECL Status SpawnObjectAtGeo(const std::string& object_name,
                                   const std::string& asset_path,
                                   float latitude, float longitude,
                                   float altitude,
                                   const std::vector<float>& rotation,
                                   const std::vector<float>& scale,
                                   bool enable_physics,
                                   std::string* pstr_spawned_name_out = nullptr);
  ASC_DECL Status SpawnObjectFromFileAtGeo(
      const std::string& object_name, const std::string& file_format,
      const VecB& asset_bytes, bool is_binary, float latitude, float longitude,
      float altitude, const std::vector<float>& rotation,
      const std::vector<float>& scale, bool enable_physics,
      std::string* pstr_spawned_name_out = nullptr);
  ASC_DECL Status DestroyObject(const std::string& object_name,
                                bool* pf_success_out);
  ASC_DECL Status DestroyAllSpawnedObjects(bool* pf_success_out);

  // Weather, materials, lighting, segmentation
  ASC_DECL Status EnableWeatherVisualEffects(bool* pf_success_out);
  ASC_DECL Status DisableWeatherVisualEffects(bool* pf_success_out);
  ASC_DECL Status ResetWeatherEffects(bool* pf_success_out);
  ASC_DECL Status SetWeatherVisualEffectsParam(int param, float value,
                                               bool* pf_success_out);
  ASC_DECL Status GetWeatherVisualEffectsParam(json* pjson_params_out) const;
  ASC_DECL Status SetObjectMaterial(const std::string& object_name,
                                    const std::string& material_asset_path,
                                    bool* pf_success_out);
  ASC_DECL Status SetObjectTextureFromUrl(const std::string& object_name,
                                          const std::string& url,
                                          bool* pf_success_out);
  ASC_DECL Status SetObjectTextureFromFile(
      const std::string& object_name, const std::string& texture_file_path,
      bool* pf_success_out);
  ASC_DECL Status SetObjectTextureFromPackagedAsset(
      const std::string& object_name, const std::string& texture_asset_path,
      bool* pf_success_out);
  ASC_DECL Status SwapObjectTexture(const std::string& tag, int tex_id,
                                    bool* pf_success_out);
  ASC_DECL Status SetLightObjectIntensity(const std::string& object_name,
                                          float new_intensity,
                                          bool* pf_success_out);
  ASC_DECL Status SetLightObjectColor(const std::string& object_name,
                                      const std::vector<float>& color_rgb,
                                      bool* pf_success_out);
  ASC_DECL Status SetLightObjectRadius(const std::string& object_name,
                                       float new_radius, bool* pf_success_out);
  ASC_DECL Status SetTimeOfDay(bool enabled, const std::string& datetime,
                               bool is_dst, float clock_speed,
                               float update_interval, bool move_sun,
                               bool* pf_success_out);
  ASC_DECL Status GetTimeOfDay(json* pjson_time_of_day_out) const;
  ASC_DECL Status SetSunPositionFromDateTime(const std::string& date_time,
                                             bool is_dst,
                                             bool* pf_success_out);
  ASC_DECL Status SwitchStreamingView(bool* pf_success_out);
  ASC_DECL Status SetSegmentationIDByName(const std::string& mesh_name,
                                          int segmentation_id,
                                          bool is_name_regex,
                                          bool use_owner_name,
                                          bool* pf_success_out);
  ASC_DECL Status GetSegmentationIDByName(const std::string& mesh_name,
                                          bool use_owner_name,
                                          int* psegmentation_id_out) const;
  ASC_DECL Status GetSegmentationIDMap(json* pjson_segmentation_map_out) const;

  // Import a NED trajectory that can be assigned to robots or env actors.
  ASC_DECL Status ImportNEDTrajectory(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& pose_x, const std::vector<float>& pose_y,
      const std::vector<float>& pose_z,
      const std::vector<float>& pose_roll = std::vector<float>(),
      const std::vector<float>& pose_pitch = std::vector<float>(),
      const std::vector<float>& pose_yaw = std::vector<float>(),
      const std::vector<float>& vel_lin_x = std::vector<float>(),
      const std::vector<float>& vel_lin_y = std::vector<float>(),
      const std::vector<float>& vel_lin_z = std::vector<float>());

  //
  // Debug Plots
  //
  ASC_DECL Status FlushPersistentMarkers(void);
  ASC_DECL Status PlotDebugArrows(const VecVector3& points_start,
                                  const VecVector3& points_end,
                                  const ColorRGBA& color_rgba, float thickness,
                                  float arrow_size, float sec_duration,
                                  bool is_persistent);
  ASC_DECL Status PlotDebugDashedLine(const VecVector3& points,
                                      const ColorRGBA& color_rgba,
                                      float thickness, float sec_duration,
                                      bool is_persistent);
  ASC_DECL Status PlotDebugPoints(const VecVector3& points,
                                  const ColorRGBA& color_rgba, float size,
                                  float sec_duration, bool is_persistent);
  ASC_DECL Status PlotDebugSolidLine(const VecVector3& points,
                                     const ColorRGBA& color_rgba,
                                     float thickness, float sec_duration,
                                     bool is_persistent);
  ASC_DECL Status PlotDebugStrings(const VecStr& strings,
                                   const VecVector3& positions, float scale,
                                   const ColorRGBA& color_rgba,
                                   float sec_duration);
  ASC_DECL Status PlotDebugTransforms(const VecPose& poses, float scale,
                                      float thickness, float sec_duration,
                                      bool is_persistent);
  ASC_DECL Status PlotDebugTransformsWithNames(
      const VecPose& poses, const VecStr& names, float tf_scale,
      float tf_thickness, float text_scale, const ColorRGBA& text_color_rgba,
      float sec_duration);
  ASC_DECL Status SetTraceLine(const ColorRGBA& color_rgba, float thickness,
                               bool* pf_success_out);
  ASC_DECL Status ToggleTrace(bool* pf_success_out);
  ASC_DECL Status Get3DBoundingBox(const std::string& object_name,
                                   int box_alignment,
                                   json* pjson_bounding_box_out) const;
  ASC_DECL Status ImportGeoTrajectory(
      const std::string& traj_name, const std::vector<float>& time,
      const std::vector<float>& latitudes,
      const std::vector<float>& longitudes,
      const std::vector<float>& altitudes,
      const std::vector<float>& roll = std::vector<float>(),
      const std::vector<float>& pitch = std::vector<float>(),
      const std::vector<float>& yaw = std::vector<float>(),
      const std::vector<float>& vel_lin_x = std::vector<float>(),
      const std::vector<float>& vel_lin_y = std::vector<float>(),
      const std::vector<float>& vel_lin_z = std::vector<float>());
  ASC_DECL Status GetSurfaceElevationAtPoint(float x, float y,
                                             float* pelevation_out) const;

  //
  // Interactive UI Methods
  //

  // Return the 3D position of the first location hit by a ray projected
  // along the local X-axis of the specified pose.
  //
  //  Arguments:
  //    pose       Ray is projected from this point at this orientation
  //    pvec3_out  Buffer to get hit location
  //
  //  Returns:
  //    (Return)   Operation status
  //    pvec3_out  If return status is Status::OK, contains the location
  //                   where the ray first hit an object, or (NaN, NaN, Nan)
  //                   if nothing was hit (NED coordinates)
  ASC_DECL Status HitTest(const Pose& pose, Vector3* pvec3_out);

 protected:
  class Impl;  // World implementation

 protected:
  std::shared_ptr<Impl> pimpl_;  // Pointer to implementation
};                               // class World

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
