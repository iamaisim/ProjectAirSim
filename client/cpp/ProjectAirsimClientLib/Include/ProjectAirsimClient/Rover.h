// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once
#include <exception>
#include <memory>
#include <string>

#include "ASCDecl.h"
#include "AsyncResult.h"
#include "Client.h"
#include "Status.h"
#include "Types.h"
#include "World.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class Rover {
 public:
  // Timeout value meaning no timeout
  ASC_DECL static const float kNoTimeout;

 public:
  ASC_DECL Rover(void) noexcept;
  ASC_DECL ~Rover();

  // Initialize the rover object. Connects to named rover in
  // simulation connected to the client and in the specified world.
  //
  // Arguments:
  //   pclient     Pointer to client object
  //   pworld      Pointer to world object
  //   rover_name  Name of the rover to which the object attaches
  //
  // Returns:
  //   (Return)    Initialization status
  ASC_DECL Status Initialize(std::shared_ptr<Client>& pclient,
                             std::shared_ptr<World>& pworld,
                             const std::string& rover_name);

  // API control
  ASC_DECL Status CancelLastTask(bool* pf_task_is_canceled_out);
  ASC_DECL Status DisableAPIControl(bool* pf_is_disabled_out);
  ASC_DECL Status EnableAPIControl(bool* pf_is_enabled_out);
  ASC_DECL Status IsAPIControlEnabled(bool* pf_is_enabled_out) const;

  // Arming control
  ASC_DECL Status Arm(bool* pf_is_armed_out);
  ASC_DECL Status CanArm(bool* pf_can_be_armed_out) const;
  ASC_DECL Status Disarm(bool* pf_is_disarmed_out);

  // State
  ASC_DECL Status GetGroundTruthKinematics(json* pjson_out) const;
  ASC_DECL Status SetPose(const Transform& transform, bool reset_kinematics);

  // Movement
  ASC_DECL AsyncResult MoveToPositionAsync(
      float north, float east, float velocity,
      float timeout_sec = kNoTimeout, float yaw_rate_max = -1.0f,
      float lookahead = -1.0f, float adaptive_lookahead = 1.0f,
      FnResponseCallback fnresponse_callback = nullptr);

  ASC_DECL AsyncResult MoveByHeadingAsync(
      float heading, float speed, float sec_duration = 3.0f,
      float heading_margin = 0.08726646259971647f,  // 5 deg in radians
      float yaw_rate = 5.0f, float timeout_sec = kNoTimeout,
      FnResponseCallback fnresponse_callback = nullptr);

  ASC_DECL Status SetRoverControls(float engine, float steering_angle,
                                   float brake);

 protected:
  class Impl;

 protected:
  std::unique_ptr<Impl> pimpl_;
};  // class Rover

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
