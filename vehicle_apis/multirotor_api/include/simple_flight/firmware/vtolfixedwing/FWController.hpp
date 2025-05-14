// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWCONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWCONTROLLER_HPP_

#include <exception>
#include <string>

#include "../interfaces/ICommLink.hpp"
#include "../interfaces/IController.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IStateEstimator.hpp"
#include "FWThrottleController.hpp"
#include "FWXTorqueController.hpp"
#include "FWYTorqueController.hpp"
#include "FWZTorqueController.hpp"

namespace simple_flight {

// 4-axis controller for fixed-wing flight.  For orientation, the vehicle is
// treated like a multicopter sitting on its side.
class FWController : public IController {
 public:
  FWController(Params* params, const IBoardClock* clock, ICommLink* comm_link)
      : clock_(clock), comm_link_(comm_link), params_(params) {}

  void Initialize(const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    goal_ = goal;
    state_estimator_ = state_estimator;

    // Create axis controllers
    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (axis_controllers_[axis] == nullptr) {
        IAxisController* piaxiscontroller = nullptr;

        switch (axis) {
          case IAxisController::kXRoll:
            piaxiscontroller = new FWXTorqueController(params_, clock_);
            break;

          case IAxisController::kYPitch:
            piaxiscontroller = new FWYTorqueController(params_, clock_);
            break;

          case IAxisController::kZYaw:
            piaxiscontroller = new FWZTorqueController(params_, clock_);
            break;

          case IAxisController::kZHeight:
            piaxiscontroller = new FWThrottleController(params_, clock_);
        }

        // initialize axis controller
        if (piaxiscontroller != nullptr) {
          axis_controllers_[axis].reset(piaxiscontroller);
          axis_controllers_[axis]->Initialize(axis, goal_, state_estimator_);
          axis_controllers_[axis]->Reset();
        }
      }
    }
  }

  void Reset() override {
    IController::Reset();

    last_goal_mode_ = GoalMode::GetUnknown();
    output_ = Axis4r();

    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis)
      if (axis_controllers_[axis] != nullptr) axis_controllers_[axis]->Reset();
  }

  void Update() override {
    IController::Update();

    // Update axis controllers
    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      if (axis_controllers_[axis] != nullptr) {
        axis_controllers_[axis]->Update();
        output_[axis] = axis_controllers_[axis]->GetOutput();
      } else {
        comm_link_->Log(std::string("Axis controller type is not set for axis ")
                            .append(std::to_string(axis)),
                        ICommLink::kLogLevelInfo);
      }
    }
  }

  const AxisNr& GetOutput() override { return output_; }

 private:
  std::unique_ptr<IAxisController>
      axis_controllers_[Axis4r::AxisCount()];  // Controllers for each "axis"
                                               // (X-roll, Y-pitch, Z-yaw, and
                                               // Z-height)
  const IBoardClock* clock_;
  ICommLink* comm_link_;
  const IGoal* goal_;
  GoalMode last_goal_mode_;
  Axis4r last_goal_val_;
  AxisNr output_;
  Params* params_;
  const IStateEstimator* state_estimator_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FWCONTROLLER_HPP_