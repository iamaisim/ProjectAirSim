// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once
#include <map>
#include <memory>
#include <string>

#include "ASCDecl.h"
#include "Client.h"
#include "Status.h"
#include "Types.h"
#include "World.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class EnvActor {
 public:
  ASC_DECL EnvActor(void) noexcept;
  ASC_DECL ~EnvActor();

  // Initialize the env actor object.
  //
  // Arguments:
  //   pclient     Pointer to client object
  //   pworld      Pointer to world object
  //   actor_name  Name of the environment actor in the scene
  //
  // Returns:
  //   (Return)    Initialization status
  ASC_DECL Status Initialize(std::shared_ptr<Client>& pclient,
                             std::shared_ptr<World>& pworld,
                             const std::string& actor_name);

  // Assign a trajectory to this actor.
  //
  // Arguments:
  //   traj_name     Name of the trajectory asset
  //   to_loop       Whether to loop the trajectory
  //   time_offset   Time offset into the trajectory (seconds)
  //   x/y/z_offset  Position offset (meters)
  //   roll/pitch/yaw_offset  Orientation offset (radians)
  ASC_DECL Status SetTrajectory(const std::string& traj_name,
                                bool to_loop = false, float time_offset = 0.0f,
                                float x_offset = 0.0f, float y_offset = 0.0f,
                                float z_offset = 0.0f,
                                float roll_offset = 0.0f,
                                float pitch_offset = 0.0f,
                                float yaw_offset = 0.0f);

  // Rotate a single link to the specified angle.
  ASC_DECL Status SetLinkRotationAngle(const std::string& link_name,
                                       float angle_deg);

  // Rotate a single link at the specified rate.
  ASC_DECL Status SetLinkRotationRate(const std::string& link_name,
                                      float rotation_deg_per_sec);

  // Rotate multiple links to specified angles.
  // rotation_angle_map: map from link name to angle in degrees
  ASC_DECL Status SetLinkRotationAngles(
      const std::map<std::string, float>& rotation_angle_map);

  // Rotate multiple links at specified rates.
  // rotation_rate_map: map from link name to degrees/second
  ASC_DECL Status SetLinkRotationRates(
      const std::map<std::string, float>& rotation_rate_map);

 protected:
  class Impl;

 protected:
  std::unique_ptr<Impl> pimpl_;
};  // class EnvActor

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
