// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once
#include <exception>
#include <memory>

#include "ASCDecl.h"
#include "Client.h"
#include "World.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class Drone {
 public:
  // Camera sensor image types
  enum class ImageType : int {
    Scene = 0,
    DepthPlanar = 1,
    DepthPerspective = 2,
    Segmentation = 3,
    DepthVis = 4,
    DisparityNormalized = 5,
    SurfaceNormals = 6,
    // kInfrared = 7,  // this type not implemented yet

    // The following must be last
    MAX
  };

  // Yaw control modes
  enum class YawControlMode {
    MaxDegreeOfFreedom =
        0,  // Yaw angle, if specified, is relative to initial yaw
    ForwardOnly =
        1,  // Yaw angle, if specified, is relative to direction of travel
  };        // enum class YawControlMode

  // VTOL flight modes
  enum class VTOLMode {
    Multirotor = 0,  // Multirotor (helicopter) mode
    FixedWing = 1,   // Fixed-wing mode when possible, multirotor otherwise
  };                 // enum class VTOLMode

 public:
  // Timeout value meaning no timeout
  ASC_DECL static const float kNoTimeout;

 public:
  ASC_DECL Drone(void) noexcept;
  ASC_DECL ~Drone();

  // Initialize the drone object. Connects to named drone in
  // simulation connected to the client and in the specified world.
  //
  // Arguments:
  //   pclient     Pointer to client object
  //   pworld      Pointer to world object
  //   drone_name  Name of the drone to which the object attaches
  //
  // Returns:
  //   (Return)    Initialization status
  ASC_DECL Status Initialize(std::shared_ptr<Client>& pclient,
                             std::shared_ptr<World>& pworld,
                             const std::string& drone_name);

  // API control
  ASC_DECL Status CancelLastTask(bool* pf_task_is_canceled_out);
  ASC_DECL Status DisableAPIControl(bool* pf_is_disabled_out);
  ASC_DECL Status EnableAPIControl(bool* pf_is_enabled_out);
  ASC_DECL Status IsAPIControlEnabled(bool* pf_is_enabled_out) const;

  // Arming control
  ASC_DECL Status Arm(bool* pf_is_armed_out);
  ASC_DECL Status CanArm(bool* pf_can_be_armed_out) const;
  ASC_DECL Status Disarm(bool* pf_is_disarmed_out);
  ASC_DECL Status GetReadyState(ReadyState* pready_state_out) const;
  ASC_DECL Status GetLandedState(LandedState* planded_state_out) const;

  // Topic names
  ASC_DECL size_t GetRobotInfoCount(void) const;
  ASC_DECL const char* GetRobotInfoName(int iname) const;
  ASC_DECL const char* GetRobotTopic(const std::string& str_info_name) const;
  ASC_DECL size_t GetSensorCount(void) const;
  ASC_DECL size_t GetSensorInfoCount(const std::string& str_sensor_name) const;
  ASC_DECL const char* GetSensorInfoName(const std::string& str_sensor_name,
                                         int iinfo) const;
  ASC_DECL const char* GetSensorName(int iname) const;
  ASC_DECL const char* GetSensorTopic(const std::string& str_sensor_name,
                                      const std::string& str_info_name) const;

  // Sensors
  ASC_DECL Status GetCameraRay(const std::string& str_camera_name,
                               ImageType image_type, int x, int y,
                               Pose* ppose_out);
  ASC_DECL Status GetIMUData(const std::string& str_sensor_name,
                             json* pjson_out) const;
  ASC_DECL Status GetGPSData(const std::string& str_sensor_name,
                             json* pjson_out) const;
  ASC_DECL Status GetAirspeedData(const std::string& str_sensor_name,
                                  json* pjson_out) const;
  ASC_DECL Status GetBarometerData(const std::string& str_sensor_name,
                                   json* pjson_out) const;
  ASC_DECL Status GetMagnetometerData(const std::string& str_sensor_name,
                                      json* pjson_out) const;
  ASC_DECL Status GetLidarData(const std::string& str_sensor_name,
                               json* pjson_out) const;
  ASC_DECL Status GetRadarDetections(const std::string& str_sensor_name,
                                     json* pjson_out) const;
  ASC_DECL Status GetRadarTracks(const std::string& str_sensor_name,
                                 json* pjson_out) const;
  ASC_DECL Status GetBatteryState(json* pjson_out) const;
  ASC_DECL Status SetBatteryRemaining(float desired_battery_remaining,
                                      bool* pf_is_set_out);
  ASC_DECL Status GetBatteryDrainRate(float* pf_drain_rate_out) const;
  ASC_DECL Status SetBatteryDrainRate(float desired_drain_rate,
                                      bool* pf_is_set_out);
  ASC_DECL Status SetBatteryHealthStatus(bool is_desired_state_healthy,
                                         bool* pf_is_set_out);
  ASC_DECL Status GetImages(const std::string& str_camera_id,
                            const std::vector<int>& veci_image_type_id,
                            json* pjson_out) const;
  ASC_DECL Status CameraLookAtObject(const std::string& str_camera_id,
                                     const std::string& str_object_name,
                                     bool wait_for_pose_update,
                                     bool* pf_success_out);
  ASC_DECL Status CameraDrawFrustum(const std::string& str_camera_id,
                                    bool to_enable, ImageType image_type,
                                    bool* pf_success_out);
  ASC_DECL Status SetCameraPose(const std::string& str_camera_id,
                                const json& json_pose,
                                bool wait_for_pose_update,
                                bool* pf_success_out);
  ASC_DECL Status ResetCameraPose(const std::string& str_camera_id,
                                  bool wait_for_pose_update,
                                  bool* pf_success_out);
  ASC_DECL Status SetFocalLength(const std::string& str_camera_id,
                                 int image_type_id, float focal_length,
                                 bool* pf_success_out);
  ASC_DECL Status SetDepthOfFieldTransitionThreshold(
      const std::string& str_camera_id, int image_type_id,
      float transition_threshold, bool* pf_success_out);
  ASC_DECL Status SetDepthOfFieldFocalRegion(const std::string& str_camera_id,
                                             int image_type_id,
                                             float max_focal_distance,
                                             bool* pf_success_out);
  ASC_DECL Status SetChromaticAberrationIntensity(
      const std::string& str_camera_id, int image_type_id, float intensity,
      bool* pf_success_out);
  ASC_DECL Status SetFieldOfView(const std::string& str_camera_id,
                                 int image_type_id, float field_of_view,
                                 bool* pf_success_out);

  // Drone state
  ASC_DECL Status GetGroundTruthPose(Transform* ptransform_out);
  ASC_DECL Status GetGroundTruthGeoLocation(GeoPosition* pgeo_position_out);
  ASC_DECL Status GetEstimatedGeoLocation(GeoPosition* pgeo_position_out);
  ASC_DECL Status GetGroundTruthKinematics(json* pjson_out) const;
  ASC_DECL Status GetEstimatedKinematics(json* pjson_out) const;
  ASC_DECL Status SetGroundTruthKinematics(const json& json_kinematics,
                                           bool* pf_success_out);
  ASC_DECL Status SetPose(const Transform& transform, bool reset_kinematics);
  ASC_DECL Status SetControlSignals(const json& json_control_signal_map,
                                    bool* pf_success_out);
  ASC_DECL Status UpdateActuatorFaultState(const std::string& actuator_id,
                                           bool fault_configured,
                                           bool* pf_success_out);
  ASC_DECL Status SetExternalForce(const std::vector<float>& ext_force,
                                   bool* pf_success_out);

  // Take-off and landing
  ASC_DECL AsyncResult
  LandAsync(float sec_timeout = kNoTimeout,
            FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult
  TakeoffAsync(float sec_timeout = kNoTimeout,
               FnResponseCallback fnresponse_callback = nullptr);

  // Movement
  ASC_DECL AsyncResult
  GoHomeAsync(float sec_timeout = kNoTimeout, float velocity = 0.5f,
              FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult
  HoverAsync(FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult
  RequestControlAsync(FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult
  SetMissionModeAsync(FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult
  SetVTOLModeAsync(VTOLMode vtol_mode,
                   FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult MoveByVelocityAsync(
      float v_north, float v_east, float v_down, float sec_duration,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f,
      FnResponseCallback fnresponse_callback = nullptr);
    ASC_DECL AsyncResult MoveByVelocityZAsync(
      float v_north, float v_east, float z, float sec_duration,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f,
      FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult MoveByVelocityBodyFrameAsync(
      float v_forward, float v_right, float v_down, float sec_duration,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f,
      FnResponseCallback fnresponse_callback = nullptr);
    ASC_DECL AsyncResult MoveByVelocityBodyFrameZAsync(
      float v_forward, float v_right, float z, float sec_duration,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f,
      FnResponseCallback fnresponse_callback = nullptr);
    ASC_DECL AsyncResult MoveByHeadingAsync(
      float heading, float speed, float v_down, float sec_duration,
      float heading_margin = 0.08726646259971647f, float yaw_rate = 0.0f,
      float timeout_sec = kNoTimeout,
      FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult MoveOnPathAsync(
      const VecVector3& path, float velocity, float timeout_sec = kNoTimeout,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f, float lookahead = -1,
      float adaptive_lookahead = 1.0f,
      FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult MoveToPositionAsync(
      float north, float east, float down, float velocity,
      float timeout_sec = kNoTimeout,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0, float lookahead = -1.0,
      float adaptive_lookahead = 1.0,
      FnResponseCallback fnresponse_callback = nullptr);
    ASC_DECL AsyncResult RotateToYawAsync(
      float yaw, float timeout_sec = kNoTimeout,
      float margin = 0.08726646259971647f, float yaw_rate = 0.0f,
      FnResponseCallback fnresponse_callback = nullptr);
    ASC_DECL AsyncResult RotateByYawRateAsync(
      float yaw_rate, float sec_duration,
      FnResponseCallback fnresponse_callback = nullptr);

 protected:
  class Impl;  // Drone implementation

 protected:
  std::shared_ptr<Impl> pimpl_;  // Pointer to implementation
};                               // class Drone

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
