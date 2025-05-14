// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

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

  // Drone state
  ASC_DECL Status GetGroundTruthPose(Transform* ptransform_out);
  ASC_DECL Status SetPose(const Transform& transform, bool reset_kinematics);

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
  ASC_DECL AsyncResult MoveByVelocityAsync(
      float v_north, float v_east, float v_down, float sec_duration,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f,
      FnResponseCallback fnresponse_callback = nullptr);
  ASC_DECL AsyncResult MoveByVelocityBodyFrameAsync(
      float v_forward, float v_right, float v_down, float sec_duration,
      YawControlMode yaw_control_mode = YawControlMode::MaxDegreeOfFreedom,
      bool yaw_is_rate = true, float yaw = 0.0f,
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

 protected:
  class Impl;  // Drone implementation

 protected:
  std::shared_ptr<Impl> pimpl_;  // Pointer to implementation
};                               // class Drone

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
