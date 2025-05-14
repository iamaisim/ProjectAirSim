// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_REMOTECONTROL_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_REMOTECONTROL_HPP_

#include <cstdint>
#include <vector>

#include "interfaces/CommonStructs.hpp"
#include "interfaces/IBoardClock.hpp"
#include "interfaces/IBoardInputPins.hpp"
#include "interfaces/IGoal.hpp"

namespace simple_flight {

class RemoteControl : public IGoal, public IUpdatable {
 public:
  RemoteControl(const Params* params, const IBoardClock* clock,
                const IBoardInputPins* board_inputs,
                VehicleState* vehicle_state, IStateEstimator* state_estimator,
                ICommLink* comm_link)
      : params_(params),
        clock_(clock),
        board_inputs_(board_inputs),
        vehicle_state_(vehicle_state),
        state_estimator_(state_estimator),
        comm_link_(comm_link) {}

  void Reset() override {
    IUpdatable::Reset();

    goal_ = Axis4r::Zero();
    goal_mode_ = params_->default_goal_mode;
    allow_api_control_ = params_->rc.allow_api_always;
    last_rec_read_ = 0;
    last_angle_mode_ = std::numeric_limits<TReal>::min();
    request_duration_ = 0;
  }

  void Update() override {
    IUpdatable::Update();

    uint64_t time = clock_->Millis();

    // don't keep reading if not updated
    uint64_t dt = time - last_rec_read_;
    if (dt <= params_->rc.read_interval_ms) return;
    last_rec_read_ = time;

    // read channel values
    Axis4r channels;
    for (unsigned int axis = 0; axis < Axis4r::AxisCount(); ++axis) {
      channels[axis] =
          board_inputs_->IsRcConnected()
              ? board_inputs_->ReadChannel(params_->rc.channels[axis])
              : 0;
    }

    // set goal mode as per the switch position on RC
    UpdateGoalMode();
    UpdateAllowApiControl();

    // get any special action being requested by user such as arm/disarm
    RcRequestType rc_action = GetActionRequest(channels);

    // state machine
    switch (vehicle_state_->GetState()) {
      case VehicleStateType::kInactive:
        // comm_link_->Log(std::string("State:\t ").append("Inactive state"));

        if (rc_action == RcRequestType::kArmRequest) {
          comm_link_->Log(std::string("State:\t ")
                              .append("Inactive state, Arm request received"));
          request_duration_ += dt;

          if (request_duration_ > params_->rc.arm_duration) {
            vehicle_state_->SetState(VehicleStateType::kBeingArmed);
            request_duration_ = 0;
          }
        }
        // else ignore
        break;
      case VehicleStateType::kBeingArmed:
        comm_link_->Log(std::string("State:\t ").append("Being armed"));

        // start the motors
        goal_ = Axis4r::Zero();  // neural activation while still being armed
        goal_.Throttle() = params_->Params::MinArmedThrottle();

        // we must wait until sticks are at neutral or we will have random
        // behavior
        if (rc_action == RcRequestType::kNeutralRequest) {
          request_duration_ += dt;

          if (request_duration_ > params_->rc.neutral_duration) {
            // TODO: this code should be reused in OffboardApi
            vehicle_state_->SetState(VehicleStateType::kArmed);
            comm_link_->Log(std::string("State:\t ").append("Armed"));
            request_duration_ = 0;
          }
        }
        // else ignore
        break;
      case VehicleStateType::kArmed:
        // unless disarm is being requested, set goal from stick position
        if (rc_action == RcRequestType::kDisarmRequest) {
          comm_link_->Log(std::string("State:\t ")
                              .append("Armed state, disarm request received"));
          request_duration_ += dt;

          if (request_duration_ > params_->rc.disarm_duration) {
            vehicle_state_->SetState(VehicleStateType::kBeingDisarmed);
            request_duration_ = 0;
          }
        } else {
          request_duration_ = 0;  // if there was spurious disarm request
          UpdateGoal(channels);
        }
        break;
      case VehicleStateType::kBeingDisarmed:
        comm_link_->Log(std::string("State:\t ").append("Being state"));

        // TODO: this code should be reused in OffboardApi
        goal_.SetAxis3(
            Axis3r::Zero());  // neutral activation while being disarmed
        vehicle_state_->SetState(VehicleStateType::kDisarmed);
        request_duration_ = 0;

        break;
      case VehicleStateType::kDisarmed:
        comm_link_->Log(std::string("State:\t ").append("Disarmed"));

        goal_ = Axis4r::Zero();  // neutral activation while being disarmed
        vehicle_state_->SetState(VehicleStateType::kInactive);
        request_duration_ = 0;

        break;
      default:
        throw std::runtime_error(
            "VehicleStateType has unknown value for RemoteControl::Update()");
    }
  }

  const Axis4r& GetGoalValue() const override { return goal_; }

  const GoalMode& GetGoalMode() const override { return goal_mode_; }

  bool AllowApiControl() { return allow_api_control_; }

  float GetMotorOutput() { return board_inputs_->GetAvgMotorOutput(); }

 private:
  enum class RcRequestType {
    kNone,
    kArmRequest,
    kDisarmRequest,
    kNeutralRequest
  };

  void UpdateGoalMode() {
    if (!board_inputs_->IsRcConnected()) {
      // TODO: is it good idea to keep the last mode?
      // if (!goal_mode_.equals4(GoalMode::getUnknown()))
      //    goal_mode_ = GoalMode::getUnknown();

      // For angle as well as rate mode, keep only throttle
      goal_.SetAxis3(Axis3r());

      return;
    }

    // set up RC mode as level or rate
    angle_mode_ =
        board_inputs_->ReadChannel(params_->rc.rate_level_mode_channel);
    if (last_angle_mode_ != angle_mode_) {
      // for 3 way switch, 1/3 value for each position
      if (angle_mode_ < params_->rc.max_angle_level_switch)
        goal_mode_ = GoalMode::GetStandardAngleMode();
      else
        goal_mode_ = GoalMode::GetAllRateMode();

      last_angle_mode_ = angle_mode_;
    }
  }

  void UpdateAllowApiControl() {
    bool allow = params_->rc.allow_api_always;
    allow |= board_inputs_->IsRcConnected()
                 ? board_inputs_->ReadChannel(
                       params_->rc.allow_api_control_channel) > 0.1f
                 : params_->rc.allow_api_when_disconnected;

    if (allow_api_control_ != allow)
      comm_link_->Log(std::string("API control enabled:\t")
                          .append(std::to_string(allow_api_control_)));

    allow_api_control_ = allow;
  }

  void UpdateGoal(const Axis4r& channels) {
    // for 3 way switch, 1/3 value for each position
    if (angle_mode_ < params_->rc.max_angle_level_switch) {
      // we are in control-by-level mode
      goal_ = channels.ColWiseMultiply4(params_->angle_level_pid.max_limit);
    } else {  // we are in control-by-rate mode
      goal_ = channels.ColWiseMultiply4(params_->angle_level_pid.max_limit);
    }

    // if throttle is too low then set all motors to same value as throttle
    // because otherwise values in pitch/roll/yaw would get clipped randomly and
    // can produce random results in other words: we can't do angling if
    // throttle is too low
    if (channels.Throttle() < params_->rc.min_angling_throttle)
      goal_.Throttle() = params_->rc.min_angling_throttle;
  }

  static bool IsInTolerance(TReal val, TReal tolerance,
                            TReal center = TReal()) {
    return val <= center + tolerance && val >= center - tolerance;
  }

  RcRequestType GetActionRequest(const Axis4r& channels) {
    TReal tolerance = params_->rc.action_request_tolerance;
    TReal stick_min = 1 - tolerance;

    bool yaw_action_positive = channels.Yaw() >= stick_min;
    bool yaw_action_negative = channels.Yaw() <= -stick_min;
    bool throttle_action = channels.Throttle() <= tolerance;

    bool roll_action_positive = channels.Roll() >= stick_min;
    bool roll_action_negative = channels.Roll() <= -stick_min;
    TReal normalized_pitch = (channels.Pitch() + 1) / 2;  //-1 to 1 --> 0 to 1
    bool pitch_action = normalized_pitch >= stick_min;

    if (yaw_action_positive && throttle_action && roll_action_negative &&
        pitch_action)
      return RcRequestType::kArmRequest;
    else if (yaw_action_negative && throttle_action && roll_action_positive &&
             pitch_action)
      return RcRequestType::kDisarmRequest;
    else if (IsInTolerance(channels.Roll(), tolerance) &&
             IsInTolerance(channels.Pitch(), tolerance) &&
             IsInTolerance(channels.Yaw(), tolerance))
      return RcRequestType::kNeutralRequest;
    else
      return RcRequestType::kNone;
  }

 private:
  const Params* params_;
  const IBoardClock* clock_;
  const IBoardInputPins* board_inputs_;
  VehicleState* vehicle_state_;
  IStateEstimator* state_estimator_;
  ICommLink* comm_link_;

  Axis4r goal_;
  GoalMode goal_mode_;

  uint64_t last_rec_read_;
  TReal angle_mode_, last_angle_mode_;
  bool allow_api_control_;

  uint64_t request_duration_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_REMOTECONTROL_HPP_