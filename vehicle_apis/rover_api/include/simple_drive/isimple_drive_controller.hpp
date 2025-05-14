// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_ISIMPLE_DRIVE_CONTROLLER_HPP_
#define ROVER_API_INCLUDE_ISIMPLE_DRIVE_CONTROLLER_HPP_

#include <string>

#include "common/IStateEstimator.hpp"
#include "simple_drive/common.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Interface for a controller wrapped by SimpleDrive responsible for generating
// the time-aware high-level actuator control signals to execute low-level
// vehicle drive commands.
class ISimpleDriveController {
 public:
  // The index of the control values in the output vector
  enum OutputIndices {
    kThrottle = 0,  // Throttle amount (backwards -1.0 - 1.0 forwards)
    kSteering = 1,  // Steering angle (left -1.0 - 1.0 right)
    kBrakes = 2,    // Braking amount (none 0.0 - 1.0 maximum)
  };                // enum OutputIndices

 public:
  virtual ~ISimpleDriveController() {}

  //---------------------------------------------------------------------------
  // Management Methods

  // Initialize with pointers to the goals and the vehicle state estimator
  virtual void Initialize(
      std::shared_ptr<const Goals> goals,
      std::shared_ptr<const vehicle_apis::IStateEstimator> state_estimator) = 0;

  // Reset the controller to the initial state after initialization
  virtual void Reset(void) = 0;

  // Update the control outputs
  virtual void Update(void) = 0;

  //---------------------------------------------------------------------------
  // Output Methods

  // Get the control outputs
  virtual const std::vector<float>& GetOutput(void) = 0;
};  // class ISimpleDriveController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_ISIMPLE_DRIVE_CONTROLLER_HPP_
