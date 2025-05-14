// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include <simple_drive/channel_controller_base.hpp>

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

ChannelControllerBase::ChannelControllerBase(
    std::shared_ptr<const Params>& pparams, ClockBase* clock, Logger& logger)
    : logger_(logger),
      output_(0),
      pclock_(clock),
      pgoals_(),
      pparams_(pparams),
      pistate_estimator_() {}

ChannelControllerBase::~ChannelControllerBase() {}

void ChannelControllerBase::Initialize(
    std::shared_ptr<const Goals>& goals,
    std::shared_ptr<const vehicle_apis::IStateEstimator>& state_estimator) {
  pgoals_ = goals;
  pistate_estimator_ = state_estimator;
}

void ChannelControllerBase::Reset(void) {}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
