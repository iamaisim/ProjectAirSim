// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef ROVER_API_INCLUDE_IROVER_API_HPP_
#define ROVER_API_INCLUDE_IROVER_API_HPP_

#include <stdint.h>

#include <string>

#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

// Represents public Rover APIs intended for client, but other wheeled ground
// robot APIs can be added as well.
struct IRoverApi {
  //---------------------------------------------------------------------------
  // API control management
  virtual bool DisableApiControl(void) = 0;
  virtual bool EnableApiControl(void) = 0;
  virtual bool IsApiControlEnabled(void) = 0;

  //---------------------------------------------------------------------------
  // Vehicle arming management
  virtual bool Arm(int64_t command_start_time_nanos) = 0;
  virtual bool Disarm(void) = 0;
  virtual bool CanArm(void) const = 0;

  //---------------------------------------------------------------------------
  // Movement commands

  // Move to location (NED coordinates)
  virtual bool MoveToPosition(float x, float y, float velocity,
                              float timeout_sec, float yaw_rate_max,
                              float lookahead, float adaptive_lookahead,
                              int64_t command_start_time_nanos) = 0;

  virtual bool MoveByHeading(float heading, float speed,
                             float duration, float heading_margin,
                             float yaw_rate, float timeout_sec,
                             int64_t command_start_time_nanos) = 0;
};  // interface IRoverApi

}  // namespace projectairsim
}  // namespace microsoft

#endif