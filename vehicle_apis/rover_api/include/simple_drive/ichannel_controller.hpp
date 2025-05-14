// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_IChannelController_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_IChannelController_HPP_

#include <memory>
#include <string>

#include "common/IStateEstimator.hpp"
#include "simple_drive/common.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Interface for a controller used by an ISimpleDriveController responsible
// for generating one of the time-aware high-level actuator control signals to
// execute low-level vehicle drive commands.
class IChannelController {
 public:
  virtual ~IChannelController() {}

  //---------------------------------------------------------------------------
  // Management Methods

  // Initialize with pointers to the goals and the vehicle state estimator
  virtual void Initialize(std::shared_ptr<const Goals>& goals,
                          std::shared_ptr<const vehicle_apis::IStateEstimator>&
                              state_estimator) = 0;

  // Reset the controller to the initial state after initialization
  virtual void Reset(void) = 0;

  // Update the control outputs
  virtual void Update(void) = 0;

  //---------------------------------------------------------------------------
  // Output Methods

  // Get the control output
  virtual float GetOutput(void) = 0;
};  // class IChannelController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_IChannelController_HPP_
