// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_ACKERMANN_CONTROLLER_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_ACKERMANN_CONTROLLER_HPP_

#include <memory>

#include "core_sim/clock.hpp"
#include "core_sim/logger.hpp"
#include "simple_drive/common.hpp"
#include "simple_drive/isimple_drive_controller.hpp"
#include "simple_drive/params.hpp"
#include "steering_controller.hpp"
#include "throttle_controller.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// SimpleDrive controller class that controls a vehicle with an Ackermann
// steering geometry.
class AckermannController : public ISimpleDriveController {
//---------------------------------------------------------------------------
#pragma region Public Methods
 public:
  AckermannController(std::shared_ptr<const Params> params, ClockBase* clock,
                      Logger logger);
  virtual ~AckermannController();

#pragma endregion Public Methods

//---------------------------------------------------------------------------
#pragma region Public ISimpleDriveController Method Overrides

  virtual const std::vector<float>& GetOutput(void) override;
  virtual void Initialize(std::shared_ptr<const Goals> goals,
                          std::shared_ptr<const vehicle_apis::IStateEstimator>
                              state_estimator) override;
  virtual void Reset(void) override;
  virtual void Update(void) override;

#pragma endregion Protected Methods

  //---------------------------------------------------------------------------

#pragma region Protected Methods

  std::string GetControllerName(void) const { return "AckermannController"; }

#pragma endregion Protected Methods

//---------------------------------------------------------------------------
#pragma region Protected Data

 protected:
  std::vector<std::unique_ptr<IChannelController>>
      channel_controllers_;  // Throttle, steering, and braking controllers
  Logger logger_;            // Logger object
  VecR output_;              // Vehicle control outputs
  ClockBase* pclock_;        // Simulation clock
  std::vector<Goal::Mode>
      vec_goal_mode_last_;  // Goal modes from the previous Update()
  std::shared_ptr<const Goals> pgoals_;    // Control goals
  std::shared_ptr<const Params> pparams_;  // Controller parameters
  std::shared_ptr<const vehicle_apis::IStateEstimator>
      pistate_estimator_;  // Vehicle state estimator

#pragma endregion Protected Data
};  // class AckermannController

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_ACKERMANN_CONTROLLER_HPP_
