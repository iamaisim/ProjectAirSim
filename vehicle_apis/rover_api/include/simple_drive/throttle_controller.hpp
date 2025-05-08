// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_THROTTLE_CONTROLLER_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_THROTTLE_CONTROLLER_HPP_

#include "common/pid_controller.hpp"
#include "simple_drive/channel_controller_base.hpp"
#include "simple_drive/params.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Interface for a controller used by an ISimpleDriveController responsible
// for generating one of the time-aware high-level actuator control signals to
// execute low-level vehicle drive commands.
class ThrottleController : public ChannelControllerBase {
 public:
  ThrottleController(std::shared_ptr<const Params>& pparams, ClockBase* clock,
                     Logger& logger);
  virtual ~ThrottleController() {}

  //---------------------------------------------------------------------------
  // IChannelController Method Overrides

  // Update the control outputs
  virtual void Update(void);

 protected:
  Goals goals_last_;  // Goals during the last update
  PidController
      pid_controller_throttle_;  // PID controller to generate throttle signal
};                              // class ThrottleController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_THROTTLE_CONTROLLER_HPP_
