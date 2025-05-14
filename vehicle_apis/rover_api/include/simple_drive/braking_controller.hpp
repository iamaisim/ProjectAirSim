// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_BRAKING_CONTROLLER_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_BRAKING_CONTROLLER_HPP_

#include "core_sim/clock.hpp"
#include "core_sim/logger.hpp"
#include "simple_drive/channel_controller_base.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Interface for a controller used by an ISimpleDriveController responsible
// for generating one of the time-aware high-level actuator control signals to
// execute low-level vehicle drive commands.
class BrakingController : public ChannelControllerBase {
 public:
  using ChannelControllerBase::ChannelControllerBase;
  virtual ~BrakingController() {}

  //---------------------------------------------------------------------------
  // IChannelController Method Overrides

  // Update the control outputs
  virtual void Update(void);
};  // class BrakingController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_BRAKING_CONTROLLER_HPP_
