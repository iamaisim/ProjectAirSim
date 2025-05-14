// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include <memory>
#include <simple_drive/ackermann_controller.hpp>
#include <simple_drive/braking_controller.hpp>
#include <simple_drive/steering_controller.hpp>
#include <simple_drive/throttle_controller.hpp>

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

AckermannController::AckermannController(std::shared_ptr<const Params> pparams,
                                         ClockBase* clock, Logger logger)
    : channel_controllers_(),
      logger_(logger),
      output_(0),
      pclock_(clock),
      pgoals_(),
      pparams_(pparams),
      pistate_estimator_() {
  // Size output array to output channels
  output_.resize(3);

  // Create the channel controllers
  channel_controllers_.resize(3);
  channel_controllers_[kThrottle] =
      std::make_unique<ThrottleController>(pparams_, clock, logger);
  channel_controllers_[kSteering] =
      std::make_unique<SteeringController>(pparams_, clock, logger);
  channel_controllers_[kBrakes] =
      std::make_unique<BrakingController>(pparams_, clock, logger);
}

AckermannController::~AckermannController() {}

const std::vector<float>& AckermannController::GetOutput(void) {
  return output_;
}

void AckermannController::Initialize(
    std::shared_ptr<const Goals> goals,
    std::shared_ptr<const vehicle_apis::IStateEstimator> state_estimator) {
  pgoals_ = goals;
  pistate_estimator_ = state_estimator;

  for (auto& channel_controller : channel_controllers_)
    channel_controller->Initialize(goals, state_estimator);
}

void AckermannController::Reset(void) {}

void AckermannController::Update(void) {
  auto ioutput = 0;
  for (auto& channel_controller : channel_controllers_) {
    channel_controller->Update();
    output_[ioutput++] = channel_controller->GetOutput();
  }
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
