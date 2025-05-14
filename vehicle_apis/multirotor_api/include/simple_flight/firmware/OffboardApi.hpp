// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_OFFBOARDAPI_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_OFFBOARDAPI_HPP_

#include <cstdint>

#include "Params.hpp"
#include "RemoteControl.hpp"
#include "interfaces/CommonStructs.hpp"
#include "interfaces/ICommLink.hpp"
#include "interfaces/IGoal.hpp"
#include "interfaces/IOffboardApi.hpp"
#include "interfaces/IStateEstimator.hpp"
#include "interfaces/IUpdatable.hpp"

namespace simple_flight {

class OffboardApi : public IUpdatable, public IOffboardApi {
 public:
  OffboardApi(const Params* params, const IBoardClock* clock,
              const IBoardInputPins* board_inputs,
              IStateEstimator* state_estimator,
              IStateEstimator* state_estimator_fw, ICommLink* comm_link)
      : params_(params),
        rc_(params, clock, board_inputs, &vehicle_state_, state_estimator,
            comm_link),
        state_estimator_(state_estimator),
        state_estimator_fw_(state_estimator_fw),
        comm_link_(comm_link),
        clock_(clock),
        landed_(true) {}

  void Reset() override {
    IUpdatable::Reset();

    vehicle_state_.SetState(params_->default_vehicle_state);
    rc_.Reset();
    has_api_control_ = false;
    landed_ = true;
    takenoff_ = false;
    goal_timestamp_ = clock_->Millis();
    UpdateGoalFromRc();
  }

  void Update() override {
    IUpdatable::Update();

    rc_.Update();
    if (!has_api_control_)
      UpdateGoalFromRc();
    else {
      if (takenoff_ &&
          (clock_->Millis() - goal_timestamp_ > params_->api_goal_timeout)) {
        if (!is_api_timedout_) {
          comm_link_->Log(
              "API call was not received, entering hover mode for safety");
          goal_mode_ = GoalMode::GetPositionMode();
          goal_ = Axis4r::XyzToAxis4(state_estimator_->GetPosition(), true);
          is_api_timedout_ = true;
        }
        // do not update goal_timestamp_
      }
    }
    // else leave the goal set by IOffboardApi API

    DetectLanding();
    DetectTakingOff();
  }

  /**************** IOffboardApi ********************/

  const Axis4r& GetGoalValue() const override { return goal_; }

  const GoalMode& GetGoalMode() const override { return goal_mode_; }

  bool CanRequestApiControl(std::string& message) override {
    if (rc_.AllowApiControl())
      return true;
    else {
      message = "Remote Control switch position disallows API control";
      comm_link_->Log(message, ICommLink::kLogLevelError);
      return false;
    }
  }
  bool HasApiControl() override { return has_api_control_; }
  bool RequestApiControl(std::string& message) override {
    if (CanRequestApiControl(message)) {
      has_api_control_ = true;

      // initial value from RC for smooth transition
      UpdateGoalFromRc();

      comm_link_->Log("requestApiControl was successful",
                      ICommLink::kLogLevelInfo);

      return true;
    } else {
      comm_link_->Log("requestApiControl failed", ICommLink::kLogLevelError);
      return false;
    }
  }
  void ReleaseApiControl() override {
    has_api_control_ = false;
    comm_link_->Log("releaseApiControl was successful",
                    ICommLink::kLogLevelInfo);
  }
  bool SetGoalAndMode(const Axis4r* goal, const GoalMode* goal_mode,
                      std::string& message) override {
    if (has_api_control_) {
      if (goal != nullptr) goal_ = *goal;
      if (goal_mode != nullptr) goal_mode_ = *goal_mode;
      goal_timestamp_ = clock_->Millis();
      is_api_timedout_ = false;
      return true;
    } else {
      message = "requestApiControl() must be called before using API control";
      comm_link_->Log(message, ICommLink::kLogLevelError);
      return false;
    }
  }

  bool Arm(std::string& message) override {
    if (has_api_control_) {
      if (vehicle_state_.GetState() == VehicleStateType::kArmed) {
        message = "Vehicle is already armed";
        comm_link_->Log(message, ICommLink::kLogLevelInfo);
        return true;
      } else if ((vehicle_state_.GetState() == VehicleStateType::kInactive ||
                  vehicle_state_.GetState() == VehicleStateType::kDisarmed ||
                  vehicle_state_.GetState() ==
                      VehicleStateType::kBeingDisarmed)) {
        vehicle_state_.SetState(VehicleStateType::kArmed);
        goal_ = Axis4r(0, 0, 0, params_->rc.min_angling_throttle);
        goal_mode_ = GoalMode::GetAllRateMode();

        message = "Vehicle is armed";
        comm_link_->Log(message, ICommLink::kLogLevelInfo);
        return true;
      } else {
        message =
            "Vehicle cannot be armed because it is not in Inactive, Disarmed "
            "or BeingDisarmed state";
        comm_link_->Log(message, ICommLink::kLogLevelError);
        return false;
      }
    } else {
      message =
          "Vehicle cannot be armed via API because API has not been given "
          "control";
      comm_link_->Log(message, ICommLink::kLogLevelError);
      return false;
    }
  }

  bool Disarm(std::string& message) override {
    if (has_api_control_ &&
        (vehicle_state_.GetState() == VehicleStateType::kActive ||
         vehicle_state_.GetState() == VehicleStateType::kArmed ||
         vehicle_state_.GetState() == VehicleStateType::kBeingArmed)) {
      vehicle_state_.SetState(VehicleStateType::kDisarmed);
      goal_ = Axis4r(0, 0, 0, 0);
      goal_mode_ = GoalMode::GetAllRateMode();

      message = "Vehicle is disarmed";
      comm_link_->Log(message, ICommLink::kLogLevelInfo);
      return true;
    } else {
      message =
          "Vehicle cannot be disarmed because it is not in Active, Armed or "
          "BeingArmed state";
      comm_link_->Log(message, ICommLink::kLogLevelError);
      return false;
    }
  }

  VehicleStateType GetVehicleState() const override {
    return vehicle_state_.GetState();
  }

  const IStateEstimator& GetStateEstimator() override {
    return (params_->vtol.in_fixed_wing_mode ? *state_estimator_fw_
                                             : *state_estimator_);
  }

  bool GetLandedState() const override { return landed_; }

 private:
  void UpdateGoalFromRc() {
    goal_ = rc_.GetGoalValue();
    goal_mode_ = rc_.GetGoalMode();
  }

  void DetectLanding() {
    // if we are not trying to move by setting motor outputs
    if (takenoff_) {
      // if (!isGreaterThanArmedThrottle(goal_.throttle())) {
      float checkThrottle = rc_.GetMotorOutput();
      if (!IsGreaterThanArmedThrottle(checkThrottle)) {
        // and we are not currently moving (based on current velocities)
        auto angular = state_estimator_->GetAngularVelocity();
        auto velocity = state_estimator_->GetLinearVelocity();
        if (IsAlmostZero(angular.Roll()) && IsAlmostZero(angular.Pitch()) &&
            IsAlmostZero(angular.Yaw()) && IsAlmostZero(velocity.X()) &&
            IsAlmostZero(velocity.Y()) && IsAlmostZero(velocity.Z())) {
          // then we must be landed...
          landed_ = true;
          takenoff_ = false;
        }
      }
    }
  }

  void DetectTakingOff() {
    // if we are not trying to move by setting motor outputs
    if (!takenoff_) {
      float checkThrottle = rc_.GetMotorOutput();
      // TODO: better handling of landed & takenoff states
      if (IsGreaterThanArmedThrottle(checkThrottle) &&
          std::abs(state_estimator_->GetLinearVelocity().Z()) > 0.01f) {
        takenoff_ = true;
        landed_ = false;
      }
    }
  }

  bool IsAlmostZero(float v) { return std::abs(v) < kMovementTolerance; }
  bool IsGreaterThanArmedThrottle(float throttle) {
    return throttle > params_->MinArmedThrottle();
  }

 private:
  const TReal kMovementTolerance = (TReal)0.08;
  const Params* params_;
  RemoteControl rc_;
  IStateEstimator* state_estimator_;
  IStateEstimator* state_estimator_fw_;
  ICommLink* comm_link_;
  const IBoardClock* clock_;

  VehicleState vehicle_state_;

  Axis4r goal_;
  GoalMode goal_mode_;
  uint64_t goal_timestamp_;

  bool has_api_control_;
  bool is_api_timedout_;
  bool landed_, takenoff_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_OFFBOARDAPI_HPP_