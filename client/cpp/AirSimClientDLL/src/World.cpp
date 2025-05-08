// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "World.h"

#include <stdio.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>

#include "AirSimClient.h"
#include "AirSimMessage/jsonc.hpp"
#include "AirSimMessage/response_message.hpp"
#include "Common.h"
#include "JSONUtils.h"
#include "RefCounted.h"
#include "Status.h"
#include "Utils.h"
#include "core_sim/math_utils.hpp"
#include "pch.h"

namespace mp = microsoft::projectairsim;
namespace mpc = microsoft::projectairsim::client;

namespace microsoft {
namespace projectairsim {
namespace client {

class JSONBoolArrayBuffer : public internal::TRefCounted<BoolArray::IBuffer> {
 public:
  JSONBoolArrayBuffer(const json& json_in)
      : cf_(json_in.size()), json_bool_array_(json_in) {}

 public:
  virtual size_t Cf(void) const override { return (cf_); }

  virtual bool operator[](size_t ibool) const override {
    return (((ibool < 0) || (ibool >= cf_))
                ? false
                : static_cast<bool>(json_bool_array_[ibool]));
  }

 protected:
  size_t cf_;                   // Number of bools in json_bool_array_
  const json json_bool_array_;  // JSON object containing the array of bools
};                              // class JSONBoolArrayBuffer

class World::Impl {
 public:
  // File paths used when loading the scene
  struct FilePaths {
    std::string str_path_scene;  // File path to scene file
    VecStr vec_str_env_actor;    // Array of file paths to environment actor
                                 // configuration files
    VecStr vec_str_robot_actor;  // Array of file paths to robot actor
                                 // configuration files
    std::string str_path_tiles;  // File path to GLTF tiles
  };                             // struct FilePaths

 public:
  Impl() noexcept;
  ~Impl();

  Status Initialize(std::shared_ptr<Client>& pclient,
                    const std::string& scene_config_name,
                    const std::string& sim_config_path = "sim_config/",
                    float delay_after_load_sec = 0, int sim_instance_idx = -1);

 public:
  Status CreateVoxelGrid(const mpc::Pose& position, int x_size, int y_size,
                         int z_size, float resolution,
                         BoolArray* pbool_array_ret,
                         const std::vector<std::string>& actors_to_ignore,
                         bool write_file, std::string file_path) const;
  Status FlushPersistentMarkers(void);
  const json& GetConfiguration(void) const;
  const VecStr& GetDrones(void) const;
  const char* GetParentTopic(void) const;
  Status HitTest(const Pose& pose, Vector3* pvec3_out);
  Status LoadScene(const json& json_scene_config,
                   float delay_after_load_sec = 0);
  Status PlotDebugArrows(const VecVector3& points_start,
                         const VecVector3& points_end,
                         const ColorRGBA& color_rgba, float thickness,
                         float arrow_size, float sec_duration,
                         bool is_persistent);
  Status PlotDebugDashedLine(const VecVector3& points,
                             const ColorRGBA& color_rgba, float thickness,
                             float sec_duration, bool is_persistent);
  Status PlotDebugPoints(const VecVector3& points, const ColorRGBA& color_rgba,
                         float size, float sec_duration, bool is_persistent);
  Status PlotDebugSolidLine(const VecVector3& points,
                            const ColorRGBA& color_rgba, float thickness,
                            float sec_duration, bool is_persistent);
  Status PlotDebugStrings(const VecStr& strings, const VecVector3& positions,
                          float scale, const ColorRGBA& color_rgba,
                          float sec_duration);
  Status PlotDebugTransforms(const VecPose& poses, float scale, float thickness,
                             float sec_duration, bool is_persistent);
  Status Resume(Message* pmessage_response_out);

 protected:
  static Status MakeAbsoluteSimConfigPath(
      const std::filesystem::path& path_in,
      const std::filesystem::path& path_sim_config,
      std::filesystem::path* ppath_out) noexcept;
  static Status LoadSceneConfig(const std::string& scene_config_name,
                                const std::string& sim_config_path,
                                int sim_instance_idx, json* pjson_config_out,
                                FilePaths* pfile_paths_out);

 protected:
  Status LoadObjectsViaFileFromConfig(const json& json_config,
                                      VecStr* pvec_str_objects_loaded_ret);
  Status LoadPackagedObjectsFromConfig(const json& json_config,
                                       VecStr* pvec_str_objects_loaded_ret);
  Status LoadSceneObjectsFromConfig(const json& json_config,
                                    VecStr* pvec_str_objects_loaded_ret);
  Status SpawnObject(const std::string& str_name,
                     const std::string& str_asset_path, const json& json_pose,
                     std::vector<float> vec_float_scale, bool fenable_physics);
  Status SpawnObjectAtGeo(const std::string& str_name,
                          const std::string& str_asset_path, float latitude,
                          float longitude, float altitude,
                          std::vector<float> rotation,
                          std::vector<float> vec_float_scale,
                          bool fenable_physics);
  Status SpawnObjectFromFile(const std::string& str_name,
                             const std::string& str_file_format,
                             const VecB& vecb_asset, bool fis_binary,
                             const json& json_pose,
                             std::vector<float> vec_float_scale,
                             bool fenable_physics);
  Status SpawnObjectFromFileAtGeo(const std::string& str_name,
                                  const std::string& str_file_format,
                                  const VecB& vecb_asset, bool fis_binary,
                                  float latitude, float longitude,
                                  float altitude, std::vector<float> rotation,
                                  std::vector<float> vec_float_scale,
                                  bool fenable_physics);

 protected:
  FilePaths
      file_paths_;  // Files paths to configuration files in the loaded scene
  GeoPosition geo_position_home_;    // Home geographic position
  json json_scene_config_;           // Loaded scene configuration
  std::shared_ptr<Client> pclient_;  // Pointer to client
  std::string str_parent_topic_;     // Root topic name for the current scene
  VecStr vecstr_actor_names_;        // List of actors in the current scene
};                                   // class World

ASC_DECL World::World(void) noexcept : pimpl_(new Impl()) {}

ASC_DECL World::~World() {}

ASC_DECL Status
World::CreateVoxelGrid(const mpc::Pose& position, int x_size, int y_size,
                       int z_size, float resolution, BoolArray* pbool_array_ret,
                       const std::vector<std::string>& actors_to_ignore,
                       bool write_file, std::string file_path) const {
  return (pimpl_->CreateVoxelGrid(position, x_size, y_size, z_size, resolution,
                                  pbool_array_ret, actors_to_ignore, write_file,
                                  file_path));
}

ASC_DECL Status World::FlushPersistentMarkers(void) {
  RETURN_CATCH_STATUS(pimpl_->FlushPersistentMarkers());
}

ASC_DECL const json& World::GetConfiguration(void) const {
  return (pimpl_->GetConfiguration());
}

ASC_DECL const VecStr& World::GetDrones(void) const {
  return (pimpl_->GetDrones());
}

ASC_DECL const char* World::GetParentTopic(void) const {
  return (pimpl_->GetParentTopic());
}

ASC_DECL Status World::HitTest(const Pose& pose, Vector3* pvec3_out) {
  return (pimpl_->HitTest(pose, pvec3_out));
}

ASC_DECL Status World::Initialize(std::shared_ptr<Client>& pclient,
                                  const std::string& scene_config_name,
                                  const std::string& sim_config_path,
                                  float delay_after_load_sec,
                                  int sim_instance_idx) {
  RETURN_CATCH_STATUS(pimpl_->Initialize(pclient, scene_config_name,
                                         sim_config_path, delay_after_load_sec,
                                         sim_instance_idx));
}

ASC_DECL Status World::PlotDebugArrows(const VecVector3& points_start,
                                       const VecVector3& points_end,
                                       const ColorRGBA& color_rgba,
                                       float thickness, float arrow_size,
                                       float sec_duration, bool is_persistent) {
  RETURN_CATCH_STATUS(pimpl_->PlotDebugArrows(points_start, points_end,
                                              color_rgba, thickness, arrow_size,
                                              sec_duration, is_persistent));
}

ASC_DECL Status World::PlotDebugDashedLine(const VecVector3& points,
                                           const ColorRGBA& color_rgba,
                                           float thickness, float sec_duration,
                                           bool is_persistent) {
  RETURN_CATCH_STATUS(pimpl_->PlotDebugDashedLine(points, color_rgba, thickness,
                                                  sec_duration, is_persistent));
}

ASC_DECL Status World::PlotDebugPoints(const VecVector3& points,
                                       const ColorRGBA& color_rgba, float size,
                                       float sec_duration, bool is_persistent) {
  RETURN_CATCH_STATUS(pimpl_->PlotDebugPoints(points, color_rgba, size,
                                              sec_duration, is_persistent));
}

ASC_DECL Status World::PlotDebugSolidLine(const VecVector3& points,
                                          const ColorRGBA& color_rgba,
                                          float thickness, float sec_duration,
                                          bool is_persistent) {
  RETURN_CATCH_STATUS(pimpl_->PlotDebugSolidLine(points, color_rgba, thickness,
                                                 sec_duration, is_persistent));
}

ASC_DECL Status World::PlotDebugStrings(const VecStr& strings,
                                        const VecVector3& positions,
                                        float scale,
                                        const ColorRGBA& color_rgba,
                                        float sec_duration) {
  RETURN_CATCH_STATUS(pimpl_->PlotDebugStrings(strings, positions, scale,
                                               color_rgba, sec_duration));
}

ASC_DECL Status World::PlotDebugTransforms(const VecPose& poses, float scale,
                                           float thickness, float sec_duration,
                                           bool is_persistent) {
  RETURN_CATCH_STATUS(pimpl_->PlotDebugTransforms(poses, scale, thickness,
                                                  sec_duration, is_persistent));
}

World::Impl::Impl(void) noexcept
    : file_paths_(),
      geo_position_home_(),
      json_scene_config_(),
      pclient_(),
      str_parent_topic_(),
      vecstr_actor_names_() {}

World::Impl::~Impl() {}

Status World::Impl::CreateVoxelGrid(
    const Pose& position, int x_size, int y_size, int z_size, float resolution,
    BoolArray* pbool_array_ret,
    const std::vector<std::string>& actors_to_ignore, bool write_file,
    std::string file_path) const {
  Status status;
  json json_params;
  json json_pose;
  Message message_response;
  Transform transform;

  // Server is expecting a Transform even though the method is declared to
  // expect a Pose because of confusion between the Pose and Transform types
  // which needs to be cleaned up...eventually...
  transform.translation = position.position;

  json_params = {
      {"position", transform}, {"x_size", x_size},
      {"y_size", y_size},      {"z_size", z_size},
      {"res", resolution},     {"actors_to_ignore", actors_to_ignore},
  };

  if ((status = pclient_->Request(str_parent_topic_ + "/createVoxelGrid",
                                  json_params, &message_response)) ==
      Status::OK) {
    int err_code;
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    err_code = response_message.GetErrorCode();

    if (err_code == 0) {
      auto const& json_response = response_message.GetResult();

      if (json_response.is_array()) {
        pbool_array_ret->SetBuffer(new JSONBoolArrayBuffer(json_response));
      }
    } else {
      status = Status::RejectedByServer;

      log.ErrorF(
          "Server method \"%s\" failed: error %d: %s",
          (str_parent_topic_ + "/createVoxelGrid").c_str(),
          response_message.GetErrorCode(),
          static_cast<std::string>(response_message.GetResult()["message"])
              .c_str());
    }
  }

  if (write_file && (status == Status::OK)) {
    std::ofstream ofstream_out(file_path,
                               std::ios_base::out | std::ios_base::binary);

    if (ofstream_out.fail()) {
      int err = errno;

      log.ErrorF("Failed to open for write voxel grid file \"%s\": ",
                 file_path.c_str(), strerror(err));
      status = internal::ErrnoToStatus(errno);
    } else {
      int ctr;
      bool state;
      auto& voxel_grid = *pbool_array_ret;

      ofstream_out << "#binvox 1" << std::endl;
      ofstream_out << "dim " << (int)(x_size / resolution) << " "
                   << (int)(z_size / resolution) << " "
                   << (int)(y_size / resolution) << std::endl;
      ofstream_out << "translate " << (-x_size * 0.5f) << " "
                   << (-y_size * 0.5f) << " " << (-z_size * 0.5f) << std::endl;
      ofstream_out << "scale " << (1.0f / x_size) << std::endl;
      ofstream_out << "data" << std::endl;

      state = voxel_grid[0];
      ctr = 0;
      for (size_t ivoxel = 0, cvoxel = voxel_grid.Cf(); ivoxel < cvoxel;
           ++ivoxel) {
        bool c = voxel_grid[ivoxel];

        if (c == state) {
          ++ctr;

          // If ctr hits max, dump
          if (ctr == 255) {
            ofstream_out << (std::uint8_t)state;
            ofstream_out << (std::uint8_t)ctr;
            ctr = 0;
          }
        } else {
          // If switch state, dump
          ofstream_out << (std::uint8_t)state;
          ofstream_out << (std::uint8_t)ctr;
          state = c;
          ctr = 1;
        }
      }  // for

      // Flush out remainders
      if (ctr > 0) {
        ofstream_out << (std::uint8_t)state;
        ofstream_out << (std::uint8_t)ctr;
      }

      ofstream_out.close();
      log.InfoF("binvox file written to %s", file_path.c_str());
    }
  }

  return (status);
}

Status World::Impl::FlushPersistentMarkers(void) {
  json json_message;
  Message message;
  Status status;

  if ((status =
           pclient_->Request(str_parent_topic_ + "/debugFlushPersistentMarkers",
                             json_message, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot debug points: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot debug points: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

const json& World::Impl::GetConfiguration(void) const {
  return (json_scene_config_);
}

const char* World::Impl::GetParentTopic(void) const {
  return (str_parent_topic_.c_str());
}

const VecStr& World::Impl::GetDrones(void) const {
  return (vecstr_actor_names_);
}

Status World::Impl::HitTest(const Pose& pose, Vector3* pvec3_out) {
  json json_params;
  Message message;
  Status status;
  Transform transform;

  // Server is expecting a Transform even though the method is declared to
  // expect a Pose because of confusion between the Pose and Transform types
  // which needs to be cleaned up...eventually...
  transform.rotation = pose.orientation;
  transform.translation = pose.position;

  json_params["pose"] = transform;
  if ((status = pclient_->Request(str_parent_topic_ + "/HitTest", json_params,
                                  &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code == 0) {
      json json_vec3 = responsemessage.GetResult();

      if (!json_vec3.is_array() || (json_vec3.size() < 3))
        log.Error(
            "Failed to hit test location: result is not an array of three "
            "floats");
      else {
        pvec3_out->x = json_vec3[0];
        pvec3_out->y = json_vec3[1];
        pvec3_out->z = json_vec3[2];
      }
    } else {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to hit test location: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to hit test location: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::Initialize(std::shared_ptr<Client>& pclient,
                               const std::string& scene_config_name,
                               const std::string& sim_config_path,
                               float delay_after_load_sec,
                               int sim_instance_idx) {
  Status status = Status::OK;
  auto path_sim_config = std::filesystem::path(sim_config_path);

  // Save pointer to client
  pclient_ = pclient;

  // Clear any existing info
  file_paths_ = FilePaths();
  geo_position_home_ = GeoPosition();
  json_scene_config_ = json();
  str_parent_topic_.clear();
  vecstr_actor_names_.clear();

  // Load scene
  if (scene_config_name.empty()) {
    // No new scene config specified--get info from current config loaded on the
    // server
    auto vec_topic = pclient_->GetTopicInfo();

    // Get the scene topic root name
    {
      std::regex re("^(/Sim/[^/]+)/.*$");

      for (auto& str_topic : vec_topic) {
        std::smatch smatch_sim_scene;

        if (std::regex_match(str_topic, smatch_sim_scene, re)) {
          str_parent_topic_ = smatch_sim_scene[1].str();
          break;
        }
      }

      if (str_parent_topic_.empty()) {
        status = Status::NoScene;
        goto LError;
      }
    }

    // Create the list of drones
    {
      std::regex re("^/Sim/[^/]+/robots/([^/]+)/actual_pose$");
      std::unordered_set<std::string> us_drone;

      for (auto& str_topic : vec_topic) {
        std::smatch smatch_sim_scene;

        if (std::regex_match(str_topic, smatch_sim_scene, re))
          us_drone.insert(smatch_sim_scene[1].str());
      }

      for (auto& str : us_drone)
        vecstr_actor_names_.emplace_back(std::move(str));

      std::sort(vecstr_actor_names_.begin(), vecstr_actor_names_.end(),
                [](const std::string& str_left, const std::string& str_right) {
                  return (strcmp(str_left.c_str(), str_right.c_str()) <= 0);
                });
    }
  } else {
    // Load scene config file
    json json_scene;
    const char* sz_phase;

    sz_phase = "load scene config from file";
    status = LoadSceneConfig(scene_config_name, sim_config_path,
                             sim_instance_idx, &json_scene, &file_paths_);
    // std::cout << "scene config:\n" << json_scene << std::endl;

    if (status == Status::OK) {
      sz_phase = "load scene config to simulation";
      status = LoadScene(json_scene, delay_after_load_sec);
    }

    if ((status != Status::OK) && (status != Status::RejectedByServer)) {
      char sz_error[512];

      GetStatusString(status, sz_error);
      log.ErrorF("Failed to %s: %s", sz_phase, sz_error);
    }
  }

LError:
  return (status);
}

Status World::Impl::LoadScene(const json& json_scene_config,
                              float delay_after_load_sec) {
  Status status = Status::OK;
  bool fclock_is_steppable = false;
  bool fis_paused_on_start_by_user = false;
  json json_result;
  json json_scene_working = json_scene_config;

  // Reset scene-dependent info
  str_parent_topic_.clear();
  vecstr_actor_names_.clear();

  try {
    // Force simulation to pause on start if there are objects to spawn
    if (json_scene_working.contains("clock")) {
      json& json_clock = json_scene_working["clock"];

      fclock_is_steppable =
          json_clock.contains("type") &&
          (static_cast<std::string>(json_clock["type"]) == "steppable");
      fis_paused_on_start_by_user =
          json_clock.contains("pause-on-start") &&
          static_cast<bool>(json_clock["pause-on-start"]);

      if (json_scene_working.contains("spawn-objects")) {
        if (!fclock_is_steppable)
          log.Warning(
              "For real-time clock types, assets cannot be spawned before "
              "starting the sim clock.");
        else {
          json_clock["pause-on-start"] = true;
          log.Info(
              "Pausing sim clock on start until all objects have been "
              "spawned.");
        }
      }
    }

    // Load the new scene
    {
      int error_code;
      Message message_response;
      ResponseMessage response_message;

      if ((status = pclient_->RequestLoadScene(
               json_scene_working.dump(), &message_response)) != Status::OK)
        goto LError;

      // Process the response from the server
      response_message.Deserialize(message_response);
      json_result = response_message.GetResult();
      error_code = response_message.GetErrorCode();

      if (error_code != 0) {
        json json_message;

        if (json_result.contains("message"))
          log.ErrorF("Failed to load scene: error %d: %s", error_code,
                     static_cast<std::string>(json_result["message"]).c_str());
        else
          log.ErrorF("Failed to load scene: error %d", error_code);

        return (Status::RejectedByServer);
      }
    }

    // Success!  Save the new config.
    json_scene_config_ = json_scene_config;

    // Save the root topic name for the scene
    if (json_result.is_string())
      str_parent_topic_ =
          std::string("/Sim/") + static_cast<std::string>(json_result);
    else {
      log.WarningF(
          "Load scene request didn't return the scene topic root, instead "
          "returned: %s",
          json_result.dump().c_str());
    }

    // Save the home geographic location
    if (json_scene_config.contains("home-geo-point")) {
      const json& json_home_geo = json_scene_config["home-geo-point"];

      geo_position_home_.latitude = json_home_geo["latitude"];
      geo_position_home_.longitude = json_home_geo["longitude"];
      geo_position_home_.altitude = json_home_geo["altitude"];
    }

    // Wait for topic info to be published, which indicates that the scene has
    // been loaded and is running
    (void)pclient_->GetTopicInfo();

    // Sleep if delay was requested
    if (delay_after_load_sec > 0)
      std::this_thread::sleep_for(
          std::chrono::duration<float>(delay_after_load_sec));

    // Create auto-spawn objects
    if (json_scene_config.contains("spawn-objects")) {
      VecStr vec_str_objects_loaded;

      status = LoadSceneObjectsFromConfig(json_scene_config["spawn-objects"],
                                          &vec_str_objects_loaded);

      // Resume the clock if the clock type supports it and the user didn't set
      // pause-on-start
      if (fclock_is_steppable && !fis_paused_on_start_by_user) {
        Message message_response_ignored;

        Resume(&message_response_ignored);
        log.Info("Resuming sim clock since all objects have been spawned.");
      }

      {
        std::string str = "Loaded following objects successfully: ";

        if (!vec_str_objects_loaded.empty()) {
          str += vec_str_objects_loaded[0];
          for (auto it = vec_str_objects_loaded.begin() + 1,
                    itEnd = vec_str_objects_loaded.end();
               it != itEnd; ++it)
            str += ", " + *it;
        }

        str += ".";
        log.Info(str.c_str());
      }
    }

    // Create list of robot actors
    if (json_scene_config.contains("actors")) {
      auto& vec_actors = json_scene_config["actors"];

      if (vec_actors.is_array()) {
        for (auto& json_actor : vec_actors) {
          if ((json_actor["type"] == "robot") && json_actor.contains("name")) {
            auto& json_name = json_actor["name"];

            if (json_name.is_string()) vecstr_actor_names_.push_back(json_name);
          }
        }
      }
    }
  } catch (std::exception e) {
    log.ErrorF("Failed load scene: %s", e.what());
    status = Status::Failed;
  }

  // All done!
  if (status == Status::OK) {
    log.InfoF(
        "World object updated for the loaded scene '%s'. "
        "Note: Any previous Drone object instances may no longer be valid.",
        str_parent_topic_.c_str());
  }

LError:
  return (status);
}

Status World::Impl::LoadSceneConfig(const std::string& scene_config_name,
                                    const std::string& sim_config_path,
                                    int sim_instance_idx,
                                    json* pjson_config_out,
                                    FilePaths* pfile_paths_out) {
  Status status = Status::OK;
  int err;
  auto path_sim_config = std::filesystem::path(sim_config_path);

  // Load the scene file
  {
    std::filesystem::path path_scene;

    try {
      path_scene = std::filesystem::canonical(scene_config_name);
    } catch (std::filesystem::filesystem_error) {
      status = Status::NotFound;
      goto LError;
    }

    pfile_paths_out->str_path_scene = path_scene.string();
    log.InfoF("Loading scene config: %s",
              pfile_paths_out->str_path_scene.c_str());

    if ((err = LoadJSONCFromFile(path_scene.string(), pjson_config_out)) != 0) {
      status = internal::ErrnoToStatus(err);
      goto LError;
    }
  }

  // Append simulation instance index to scene ID (for distribution mapping)
  if (sim_instance_idx != -1)
    (*pjson_config_out)["id"] =
        static_cast<std::string>((*pjson_config_out)["id"]) + "-" +
        std::to_string(sim_instance_idx);

  // Load robot actor config files and replace reference in-line
  if (pjson_config_out->contains("actors")) {
    auto& vec_actors = (*pjson_config_out)["actors"];

    if (vec_actors.is_array()) {
      for (auto& json_actor : vec_actors) {
        if ((json_actor["type"] == "robot") &&
            json_actor.contains("robot-config")) {
          json json_actor_config = json_actor["robot-config"];

          if (json_actor_config.is_string()) {
            std::filesystem::path path_config;
            std::string str_path;

            if ((status = MakeAbsoluteSimConfigPath(
                     std::filesystem::path(
                         static_cast<std::string>(json_actor_config)),
                     path_sim_config, &path_config)) != Status::OK)
              goto LError;
            str_path = path_config.string();

            if ((err = LoadJSONCFromFile(str_path, &json_actor_config)) != 0) {
              status = internal::ErrnoToStatus(err);
              goto LError;
            }

            json_actor["robot-config"] = json_actor_config;
            pfile_paths_out->vec_str_robot_actor.emplace_back(
                std::move(str_path));
          }
        }
      }
    }
  }

  // Expand environment actor config in-line
  if (pjson_config_out->contains("environment-actors")) {
    auto& vec_actors = (*pjson_config_out)["environment-actors"];

    for (auto& json_actor : vec_actors) {
      if ((json_actor["type"] == "env_actor") &&
          json_actor.contains("env-actor-config")) {
        auto& json_actor_config = json_actor["env-actor-config"];

        if (json_actor_config.is_string()) {
          std::filesystem::path path_config;
          std::string str_path;

          if ((status = MakeAbsoluteSimConfigPath(
                   std::filesystem::path(
                       static_cast<std::string>(json_actor_config)),
                   path_sim_config, &path_config)) != Status::OK)
            goto LError;
          str_path = path_config.string();

          if ((err = LoadJSONCFromFile(str_path, &json_actor_config)) != 0) {
            status = internal::ErrnoToStatus(err);
            goto LError;
          }

          json_actor["env-actor-config"] = json_actor_config;
          pfile_paths_out->vec_str_env_actor.emplace_back(std::move(str_path));
        }
      }
    }
  }

  // Convert client-relative path into absolute path before sending to simulator
  if (pjson_config_out->contains("tiles-dir") &&
      static_cast<bool>((*pjson_config_out)["tiles-dir-is-client-relative"])) {
    std::filesystem::path path_absolute;

    try {
      path_absolute =
          std::filesystem::canonical((*pjson_config_out)["tiles-dir"]);
    } catch (std::filesystem::filesystem_error) {
      status = Status::NotFound;
      goto LError;
    }

    (*pjson_config_out)["tiles-dir"] = path_absolute.string();
    log.InfoF("Resolved client-relative tiles-dir path as: %s",
              path_absolute.string().c_str());
  }

LError:
  return (status);
}

Status World::Impl::LoadObjectsViaFileFromConfig(
    const json& json_config, VecStr* pvec_str_objects_loaded_ret) {
  Status status = Status::OK;

  for (auto& json_object : json_config) {
    std::string name = json_object["name"];
    std::string format = json_object["file-format"];
    bool fis_binary = json_object["is-binary"];
    std::vector<float> scale =
        internal::ParseStringWithSpacesToFloatArray(json_object["scale"]);
    bool fphysics_is_enabled = json_object["physics-enabled"];
    std::vector<float> rpy_quat =
        internal::GetObjectRPYQuatFromConfig(json_object);
    std::vector<uint8_t> vecb_asset;

    // Load the asset file
    {
      size_t cb;
      std::string str_asset_path = json_object["asset-path"];
      FILE* pfile = fopen(str_asset_path.c_str(), "rb");
      struct stat stat_asset = {0};

      fstat(_fileno(pfile), &stat_asset);
      vecb_asset.resize(stat_asset.st_size);
      cb = fread(vecb_asset.data(), 1, vecb_asset.size(), pfile);
      if (cb != stat_asset.st_size) vecb_asset.resize(cb);

      fclose(pfile);
    }

    // Differentiate between pose, lat/long, etc.
    if (json_object.contains("origin")) {
      json json_origin = json_object["origin"];

      if (json_origin.contains("xyz")) {
        std::vector<float> vec_float =
            internal::ParseStringWithSpacesToFloatArray(json_origin["xyz"]);
        Vector3 translation;
        json json_pose;

        if (vec_float.size() < 3) vec_float.resize(3);

        json_pose["translation"] =
            mp::Vector3(vec_float[0], vec_float[1], vec_float[2]);
        json_pose["rotation"] = rpy_quat;
        json_pose["frame_id"] = "DEFAULT_ID";

        SpawnObjectFromFile(name, format, vecb_asset, fis_binary, json_pose,
                            scale, fphysics_is_enabled);
      } else if (json_origin.contains("geo-point")) {
        std::vector<float> vec_float_geo =
            internal::ParseStringWithSpacesToFloatArray(
                json_origin["geo-point"]);

        if (vec_float_geo.size() < 3) vec_float_geo.resize(3);

        SpawnObjectFromFileAtGeo(name, format, vecb_asset, fis_binary,
                                 vec_float_geo[0], vec_float_geo[1],
                                 vec_float_geo[2], rpy_quat, scale,
                                 fphysics_is_enabled);
      }
    }

    // Add name of object to those loaded
    pvec_str_objects_loaded_ret->emplace_back(std::move(name));
  }

  return (status);
}

Status World::Impl::LoadPackagedObjectsFromConfig(
    const json& json_config, VecStr* pvec_str_objects_loaded_ret) {
  Status status = Status::OK;

  for (auto& json_object : json_config) {
    std::string str_name = json_object["name"];
    std::string str_asset_path = json_object["asset-path"];
    std::vector<float> scale =
        internal::ParseStringWithSpacesToFloatArray(json_object["scale"]);
    bool fphysics_is_enabled = json_object["physics-enabled"];
    std::vector<float> rpy_quat =
        internal::GetObjectRPYQuatFromConfig(json_object);

    // Differentiate between pose, lat/long, etc.
    if (json_object.contains("origin")) {
      json json_origin = json_object["origin"];

      if (json_origin.contains("xyz")) {
        std::vector<float> vec_float =
            internal::ParseStringWithSpacesToFloatArray(json_origin["xyz"]);
        json json_pose;

        if (vec_float.size() < 3) vec_float.resize(3);

        json_pose = {
            {"translation",
             mp::Vector3(vec_float[0], vec_float[1], vec_float[2])},
            {"rotation", rpy_quat},
            {"frame_id", "DEFAULT_ID"},
        };

        SpawnObject(str_name, str_asset_path, json_pose, scale,
                    fphysics_is_enabled);
      } else if (json_origin.contains("geo-point")) {
        std::vector<float> vec_float_geo =
            internal::ParseStringWithSpacesToFloatArray(
                json_origin["geo-point"]);

        if (vec_float_geo.size() < 3) vec_float_geo.resize(3);

        SpawnObjectAtGeo(str_name, str_asset_path, vec_float_geo[0],
                         vec_float_geo[1], vec_float_geo[2], rpy_quat, scale,
                         fphysics_is_enabled);
      }
    }

    // Add name of object to those loaded
    pvec_str_objects_loaded_ret->push_back(str_name);
  }

  return (status);
}

Status World::Impl::LoadSceneObjectsFromConfig(
    const json& json_config, VecStr* pvec_str_objects_loaded_ret) {
  Status status = Status::OK;

  if (json_config.contains("sim-packaged"))
    status = LoadPackagedObjectsFromConfig(json_config["sim-packaged"],
                                           pvec_str_objects_loaded_ret);

  if (json_config.contains("from-file"))
    status = LoadObjectsViaFileFromConfig(json_config["from-file"],
                                          pvec_str_objects_loaded_ret);

  return (status);
}

Status World::Impl::MakeAbsoluteSimConfigPath(
    const std::filesystem::path& path_in,
    const std::filesystem::path& path_sim_config,
    std::filesystem::path* ppath_out) noexcept {
  Status status = Status::OK;

  try {
    if (!path_in.is_absolute())
      *ppath_out = std::filesystem::canonical(path_sim_config / path_in);
    else
      *ppath_out = std::filesystem::canonical(path_in);
  } catch (std::filesystem::filesystem_error) {
    status = Status::NotFound;
  }

  return (status);
}

Status World::Impl::PlotDebugArrows(const VecVector3& points_start,
                                    const VecVector3& points_end,
                                    const ColorRGBA& color_rgba,
                                    float thickness, float arrow_size,
                                    float sec_duration, bool is_persistent) {
  json json_params = {
      {"points_start", points_start},   {"points_end", points_end},
      {"color_rgba", color_rgba},       {"thickness", thickness},
      {"arrow_size", arrow_size},       {"duration", sec_duration},
      {"is_persistent", is_persistent},
  };
  Message message;
  Status status;

  if ((status = pclient_->Request(str_parent_topic_ + "/debugPlotArrows",
                                  json_params, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot debug arrow: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot debug arrow: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::PlotDebugDashedLine(const VecVector3& points,
                                        const ColorRGBA& color_rgba,
                                        float thickness, float sec_duration,
                                        bool is_persistent) {
  json json_params = {
      {"points", points},
      {"color_rgba", color_rgba},
      {"thickness", thickness},
      {"duration", sec_duration},
      {"is_persistent", is_persistent},
  };
  Message message;
  Status status;

  if ((status = pclient_->Request(str_parent_topic_ + "/debugPlotDashedLine",
                                  json_params, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot solid line: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot solid line: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::PlotDebugPoints(const VecVector3& points,
                                    const ColorRGBA& color_rgba, float size,
                                    float sec_duration, bool is_persistent) {
  json json_params = {
      {"points", points},
      {"color_rgba", color_rgba},
      {"size", size},
      {"duration", sec_duration},
      {"is_persistent", is_persistent},
  };
  Message message;
  Status status;

  if ((status = pclient_->Request(str_parent_topic_ + "/debugPlotPoints",
                                  json_params, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot debug points: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot debug points: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::PlotDebugSolidLine(const VecVector3& points,
                                       const ColorRGBA& color_rgba,
                                       float thickness, float sec_duration,
                                       bool is_persistent) {
  json json_params = {
      {"points", points},
      {"color_rgba", color_rgba},
      {"thickness", thickness},
      {"duration", sec_duration},
      {"is_persistent", is_persistent},
  };
  Message message;
  Status status;

  if ((status = pclient_->Request(str_parent_topic_ + "/debugPlotSolidLine",
                                  json_params, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot solid line: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot solid line: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::PlotDebugStrings(const VecStr& strings,
                                     const VecVector3& positions, float scale,
                                     const ColorRGBA& color_rgba,
                                     float sec_duration) {
  json json_params = {
      {"strings", strings},       {"positions", positions},   {"scale", scale},
      {"color_rgba", color_rgba}, {"duration", sec_duration},
  };
  Message message;
  Status status;

  if ((status = pclient_->Request(str_parent_topic_ + "/debugPlotStrings",
                                  json_params, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot solid line: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot solid line: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::PlotDebugTransforms(const VecPose& poses, float scale,
                                        float thickness, float sec_duration,
                                        bool is_persistent) {
  json json_params = {
      {"poses", poses},
      {"scale", scale},
      {"thickness", thickness},
      {"duration", sec_duration},
      {"is_persistent", is_persistent},
  };
  Message message;
  Status status;

  if ((status = pclient_->Request(str_parent_topic_ + "/debugPlotTransforms",
                                  json_params, &message)) == Status::OK) {
    ResponseMessage responsemessage;
    int error_code;

    responsemessage.Deserialize(message);
    error_code = responsemessage.GetErrorCode();
    if (error_code != 0) {
      json json_result = responsemessage.GetResult();

      if (json_result.contains("message"))
        log.ErrorF("Failed to plot transforms: error %d: %s", error_code,
                   static_cast<std::string>(json_result["message"]).c_str());
      else
        log.ErrorF("Failed to plot transforms: error %d", error_code);

      return (Status::RejectedByServer);
    }
  }

  return (status);
}

Status World::Impl::Resume(Message* pmessage_response_out) {
  json json_params = {{"do_pause", false}};

  return (pclient_->Request(str_parent_topic_ + "/Pause", json_params,
                            pmessage_response_out));
}

Status World::Impl::SpawnObject(const std::string& str_name,
                                const std::string& str_asset_path,
                                const json& json_pose,
                                std::vector<float> vec_float_scale,
                                bool fenable_physics) {
  Status status;
  json json_params = {
      {"object_name", str_name},
      {"asset_path", str_asset_path},
      {"pose", json_pose},
      {"scale", vec_float_scale},
      {"enable_physics", fenable_physics},
  };
  Message message_response;

  if ((status = pclient_->Request(str_parent_topic_ + "/SpawnObject",
                                  json_params, &message_response)) ==
      Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() != 0) status = Status::RejectedByServer;
  }

  return (status);
}

Status World::Impl::SpawnObjectAtGeo(const std::string& str_name,
                                     const std::string& str_asset_path,
                                     float latitude, float longitude,
                                     float altitude,
                                     std::vector<float> rotation,
                                     std::vector<float> vec_float_scale,
                                     bool fenable_physics) {
  Status status;
  json json_params = {
      {"object_name", str_name},  {"asset_path", str_asset_path},
      {"latitude", latitude},     {"longitude", longitude},
      {"altitude", altitude},     {"rotation", rotation},
      {"scale", vec_float_scale}, {"enable_physics", fenable_physics},
  };
  Message message_response;

  if ((status = pclient_->Request(str_parent_topic_ + "/spawnObjectAtGeo",
                                  json_params, &message_response)) ==
      Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() != 0) status = Status::RejectedByServer;
  }

  return (status);
}

Status World::Impl::SpawnObjectFromFile(const std::string& str_name,
                                        const std::string& str_file_format,
                                        const VecB& vecb_asset, bool fis_binary,
                                        const json& json_pose,
                                        std::vector<float> vec_float_scale,
                                        bool fenable_physics) {
  Status status;
  json json_params = {
      {"object_name", str_name},
      {"file_format", str_file_format},
      {"binary_array", vecb_asset},
      {"is_binary", fis_binary},
      {"pose", json_pose},
      {"scale", vec_float_scale},
      {"enable_physics", fenable_physics},
  };
  Message message_response;

  if ((status = pclient_->Request(str_parent_topic_ + "/SpawnObjectFromFile",
                                  json_params, &message_response)) ==
      Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() != 0) status = Status::RejectedByServer;
  }

  return (status);
}

Status World::Impl::SpawnObjectFromFileAtGeo(
    const std::string& str_name, const std::string& str_file_format,
    const VecB& vecb_asset, bool fis_binary, float latitude, float longitude,
    float altitude, std::vector<float> rotation,
    std::vector<float> vec_float_scale, bool fenable_physics) {
  Status status;
  json json_params = {
      {"object_name", str_name},    {"file_format", str_file_format},
      {"binary_array", vecb_asset}, {"is_binary", fis_binary},
      {"latitude", latitude},       {"longitude", longitude},
      {"altitude", altitude},       {"rotation", rotation},
      {"scale", vec_float_scale},   {"enable_physics", fenable_physics},
  };
  Message message_response;

  if ((status =
           pclient_->Request(str_parent_topic_ + "/spawnObjectFromFileAtGeo",
                             json_params, &message_response)) == Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() != 0) status = Status::RejectedByServer;
  }

  return (status);
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
