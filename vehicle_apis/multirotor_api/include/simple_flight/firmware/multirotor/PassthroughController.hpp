// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PASSTHROUGHCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PASSTHROUGHCONTROLLER_HPP_

#include <memory>

#include "../Params.hpp"
#include "../SimpleFlightCommonUtils.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IBoardClock.hpp"

namespace simple_flight {

class PassthroughController : public IAxisController {
 public:
  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    axis_ = axis;
    goal_ = goal;
    unused_simpleflight(state_estimator);
  }

  void Reset() override {
    IAxisController::Reset();
    output_ = TReal();
  }

  void Update() override {
    IAxisController::Update();
    output_ = goal_->GetGoalValue()[axis_];
  }

  TReal GetOutput() override { return output_; }

 private:
  unsigned int axis_;
  const IGoal* goal_;
  TReal output_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PASSTHROUGHCONTROLLER_HPP_