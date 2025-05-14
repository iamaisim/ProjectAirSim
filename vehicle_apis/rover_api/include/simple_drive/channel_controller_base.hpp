// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_CHANNEL_CONTROLLER_BASE_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_CHANNEL_CONTROLLER_BASE_HPP_

#include "core_sim/clock.hpp"
#include "core_sim/logger.hpp"
#include "simple_drive/ichannel_controller.hpp"
#include "simple_drive/params.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Base class for a SimpleDrive channel controller
class ChannelControllerBase : public IChannelController {
 public:
  ChannelControllerBase(std::shared_ptr<const Params>& pparams,
                        ClockBase* clock, Logger& logger);
  virtual ~ChannelControllerBase();

  //---------------------------------------------------------------------------
  // Management Methods

  // Initialize with pointers to the goals and the vehicle state estimator
  virtual void Initialize(
      std::shared_ptr<const Goals>& pgoals,
      std::shared_ptr<const vehicle_apis::IStateEstimator>& pstate_estimator);

  // Reset the controller to the initial state after initialization
  virtual void Reset(void);

  // Update the control outputs
  virtual void Update(void) = 0;

  //---------------------------------------------------------------------------
  // Output Methods

  // Get the control output
  virtual float GetOutput(void) { return output_; }

 protected:
  Logger logger_;                        // Logger object
  float output_;                         // Channel output value
  ClockBase* pclock_;                    // Simulation clock
  std::shared_ptr<const Goals> pgoals_;  // Control goals
  std::shared_ptr<const Params> pparams_;  // Control parameters
  std::shared_ptr<const vehicle_apis::IStateEstimator>
      pistate_estimator_;  // Vehicle state estimator
};                         // class IController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_CHANNEL_CONTROLLER_BASE_HPP_
