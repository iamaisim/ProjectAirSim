// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_CONSTANTOUTPUTCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_CONSTANTOUTPUTCONTROLLER_HPP_

#include <memory>

#include "../Params.hpp"
#include "../SimpleFlightCommonUtils.hpp"
#include "../interfaces/CommonStructs.hpp"
#include "../interfaces/IAxisController.hpp"
#include "../interfaces/IBoardClock.hpp"

namespace simple_flight {

class ConstantOutputController : public IAxisController {
 public:
  ConstantOutputController(TReal update_output = TReal(),
                           TReal reset_output = TReal())
      : update_output_(update_output), reset_output_(reset_output) {}

  void Initialize(unsigned int axis, const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    axis_ = axis;
    unused_simpleflight(goal);
    unused_simpleflight(state_estimator);
  }

  void Reset() override {
    IAxisController::Reset();
    output_ = reset_output_;
  }

  void Update() override {
    IAxisController::Update();
    output_ = update_output_;
  }

  TReal GetOutput() override { return output_; }

 private:
  unsigned int axis_;
  TReal update_output_;
  TReal reset_output_;
  TReal output_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_CONSTANTOUTPUTCONTROLLER_HPP_