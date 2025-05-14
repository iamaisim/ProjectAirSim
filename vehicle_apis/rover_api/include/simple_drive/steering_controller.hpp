// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_STEERING_CONTROLLER_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_STEERING_CONTROLLER_HPP_

#include "common/pid_controller.hpp"
#include "simple_drive/channel_controller_base.hpp"
#include "simple_drive/params.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Interface for a controller used by an ISimpleDriveController responsible
// for generating one of the time-aware high-level actuator control signals to
// execute low-level vehicle drive commands.
class SteeringController : public ChannelControllerBase {
 public:
  SteeringController(std::shared_ptr<const Params>& pparams, ClockBase* clock,
                     Logger& logger);
  virtual ~SteeringController() {}

  //---------------------------------------------------------------------------
  // IChannelController Method Overrides

  // Update the control outputs
  virtual void Update(void);

 protected:
  Goals goals_last_;  // Goals during the last update
  PidController
      pid_controller_steering_;  // PID controller to generate steering signal
  PidController
      pid_controller_yaw_rate_;  // PID controller for limiting yaw rate
};                               // class SteeringController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_STEERING_CONTROLLER_HPP_
