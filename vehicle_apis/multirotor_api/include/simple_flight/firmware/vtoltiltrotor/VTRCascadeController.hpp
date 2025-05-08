// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VTRCASCADECONTROLLER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VTRCASCADECONTROLLER_HPP_

// #define LVMON_REPORTING

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>

#include "../../AirSimSimpleFlightCommon.hpp"
#endif  // LVMON_REPORTING

#include <exception>
#include <string>

#include "../../AirSimSimpleFlightEstimatorFW.hpp"
#include "../interfaces/ICommLink.hpp"
#include "../interfaces/IController.hpp"
#include "../interfaces/IGoal.hpp"
#include "../interfaces/IStateEstimator.hpp"
#include "../multirotor/CascadeController.hpp"
#include "TRController.hpp"

namespace simple_flight {

// Main controller for VTOL Tilt-Rotor quadplane
class VTRCascadeController : public IController {
 public:
  VTRCascadeController(Params* params, const IBoardClock* clock,
                       ICommLink* comm_link,
                       microsoft::projectairsim::AirSimSimpleFlightEstimatorFW*
                           airsim_simple_flight_estimator_fw)
      : boardclock_(clock),
        cascadecontroller_(params, clock, comm_link),
        cfixedwing_mode_ok_(0),
        foutput_is_dirty_(true),
        trcontroller_(&params_fixed_wing_, clock, comm_link),
        flight_mode_cur_(FlightMode::Multirotor),
        goal_(nullptr),
        micros_transition_last_(0),
        output_(kCChannelOutputAdditional),
        params_(params),
        params_fixed_wing_(*params),
        proportion_fixed_wing_(0),
        state_estimator_(nullptr) {
    // TODO: Move this into Param class initialization
  }

  void Initialize(const IGoal* goal,
                  const IStateEstimator* state_estimator) override {
    goal_ = goal;
    state_estimator_ = state_estimator;

    cascadecontroller_.Initialize(goal, state_estimator);
    trcontroller_.Initialize(goal, state_estimator);

#ifdef LVMON_REPORTING
    LVMon::Set("VFT/propFW",
               (flight_mode_cur_ == FlightMode::FixedWing) ? 1.0f : 0.0f);
    LVMon::Set("VFT/flightmode", (flight_mode_cur_ == FlightMode::FixedWing)
                                     ? "Fixed Wing"
                                     : "Multirotor");
#endif  // LVMON_REPORTING
  }

  void Reset() override {
    IController::Reset();
    cascadecontroller_.Reset();
    trcontroller_.Reset();

    cfixedwing_mode_ok_ = 0;
    flight_mode_cur_ = FlightMode::Multirotor;
    proportion_fixed_wing_ = 0.0f;

#ifdef LVMON_REPORTING
    LVMon::Set("VFT/propFW", 0.0f);
    LVMon::Set("VFT/flightmode", "Multirotor");
#endif  // LVMON_REPORTING
  }

  void Update() override {
    uint64_t microsCur;
    float velocityFW;

    if (params_->fixed_wing.f_pid_changed) {
      params_fixed_wing_.angle_level_pid = params_->fixed_wing.angle_level_pid;
      params_fixed_wing_.angle_rate_pid = params_->fixed_wing.angle_rate_pid;
      params_fixed_wing_.position_pid = params_->fixed_wing.position_pid;
      params_fixed_wing_.velocity_pid = params_->fixed_wing.velocity_pid;
      params_->fixed_wing.f_pid_changed = false;

      trcontroller_.Reset();
    }

    IController::Update();
    cascadecontroller_.Update();
    trcontroller_.Update();
    foutput_is_dirty_ = true;

    // Since we don't care about the absolute value of the clock, convert to
    // uint64_t so we can easily handle wrap-around
    {
      int64_t microsT = boardclock_->Micros();

      microsCur = *(uint64_t*)(&microsT);
    }

    // Determine our "fixed-wing velocity" or the velocity that can be used for
    // fixed-wing flight
    {
      auto measured_velocity_world = state_estimator_->GetLinearVelocity();
      auto velocityX = measured_velocity_world[0];
      auto velocityY = measured_velocity_world[1];

      velocityFW = sqrt(velocityX * velocityX + velocityY * velocityY);
    }

    // Determine whether we should be in multirotor or fixed-wing mode based
    // on the vehicle speed
    if (velocityFW <= params_->fixed_wing.speed_stall) {
      if (cfixedwing_mode_ok_ > 0) --cfixedwing_mode_ok_;
    } else if (velocityFW >
               (params_->fixed_wing.speed_stall + kDSpeedTransition)) {
      if (cfixedwing_mode_ok_ < kCFixedWingModeOKMax) ++cfixedwing_mode_ok_;
    }

    if (flight_mode_cur_ == FlightMode::FixedWing) {
      if (!params_->vtol.enable_fixed_wing_mode || (cfixedwing_mode_ok_ <= 1)) {
        // Want to switch to multirotor mode
        flight_mode_cur_ = FlightMode::Multirotor;
        micros_transition_last_ = microsCur;
        params_->vtol.in_fixed_wing_mode = false;
      }
    } else if (params_->vtol.enable_fixed_wing_mode &&
               (cfixedwing_mode_ok_ >= (kCFixedWingModeOKMax - 1))) {
      // Want to switch to fixed-wing mode
      flight_mode_cur_ = FlightMode::FixedWing;
      micros_transition_last_ = microsCur;
      params_->vtol.in_fixed_wing_mode = true;
    }

    if (((flight_mode_cur_ == FlightMode::FixedWing) &&
         (proportion_fixed_wing_ < 1.0f)) ||
        ((flight_mode_cur_ == FlightMode::Multirotor) &&
         (proportion_fixed_wing_ > 0.0f))) {
      uint64_t dmicros = microsCur - micros_transition_last_;
      float dprop = (float)dmicros / kDMicrosTransition;
      float propFW =
          proportion_fixed_wing_ +
          ((flight_mode_cur_ == FlightMode::FixedWing) ? dprop : -dprop);

      if (propFW <= 0.0f)
        proportion_fixed_wing_ = 0.0f;
      else if (propFW > 1.0f)
        proportion_fixed_wing_ = 1.0f;
      else
        proportion_fixed_wing_ = propFW;

      micros_transition_last_ = microsCur;
    }

#ifdef LVMON_REPORTING
    {
      static const char* c_mpgoalmodesz[] = {
          "kUnknown",       "kPassthrough",   "kAngleLevel",    "kAngleRate",
          "kVelocityWorld", "kPositionWorld", "kConstantOutput"};

      auto& goalmode = goal_->GetGoalMode();
      auto& goalvalue = goal_->GetGoalValue();
      LVMon::Set("Goal/0/mode", c_mpgoalmodesz[(int)goalmode[0]]);
      LVMon::Set("Goal/0/value", goalvalue[0]);
      LVMon::Set("Goal/1/mode", c_mpgoalmodesz[(int)goalmode[1]]);
      LVMon::Set("Goal/1/value", goalvalue[1]);
      LVMon::Set("Goal/2/mode", c_mpgoalmodesz[(int)goalmode[2]]);
      LVMon::Set("Goal/2/value", goalvalue[2]);
      LVMon::Set("Goal/3/mode", c_mpgoalmodesz[(int)goalmode[3]]);
      LVMon::Set("Goal/3/value", goalvalue[3]);

      auto axis3rPosition = state_estimator_->GetPosition();
      LVMon::Set("Vehicle/Pos/x", axis3rPosition.X());
      LVMon::Set("Vehicle/Pos/y", axis3rPosition.Y());
      LVMon::Set("Vehicle/Pos/z", axis3rPosition.Z());

      auto axis3rLinearVel = state_estimator_->GetLinearVelocity();
      LVMon::Set("Vehicle/LVel/x", axis3rLinearVel.X());
      LVMon::Set("Vehicle/LVel/y", axis3rLinearVel.Y());
      LVMon::Set("Vehicle/LVel/z", axis3rLinearVel.Z());

      // auto axis3rAngles = state_estimator_->GetAngles();
      auto axis3rAngles = state_estimator_->GetAngles();
      LVMon::Set("Vehicle/Orient/roll", axis3rAngles.Roll());
      LVMon::Set("Vehicle/Orient/pitch", axis3rAngles.Pitch());
      LVMon::Set("Vehicle/Orient/yaw", axis3rAngles.Yaw());

      auto axis3rAngularVel = state_estimator_->GetAngularVelocity();
      LVMon::Set("Vehicle/AVel/roll", axis3rAngularVel.Roll());
      LVMon::Set("Vehicle/AVel/pitch", axis3rAngularVel.Pitch());
      LVMon::Set("Vehicle/AVel/yaw", axis3rAngularVel.Yaw());

      LVMon::Set("VFT/flightmode", (flight_mode_cur_ == FlightMode::FixedWing)
                                       ? "Fixed-wing"
                                       : "Multirotor");
      LVMon::Set("VFT/propFW", proportion_fixed_wing_);
      LVMon::Set("VFT/fwspeed", velocityFW);
    }
#endif  // LVMON_REPORTING
  }

  const AxisNr& GetOutput() override {  // AxisNr vs Axis4r!!!!!
    if (!foutput_is_dirty_) return (output_);

    Axis4r outputMultirotor = cascadecontroller_.GetOutput();
    Axis4r outputFW = trcontroller_.GetOutput();

#ifdef LVMON_REPORTING
    LVMon::Set("Multir/output/x-roll", outputMultirotor.X());
    LVMon::Set("Multir/output/y-pitch", outputMultirotor.Y());
    LVMon::Set("Multir/output/z-yaw", outputMultirotor.Z());
    LVMon::Set("Multir/output/throttle", outputMultirotor.Val4());

    LVMon::Set("FW/output/x-roll", outputFW.X());
    LVMon::Set("FW/output/y-pitch", outputFW.Y());
    LVMon::Set("FW/output/z-yaw", outputFW.Z());
    LVMon::Set("FW/output/throttle", outputFW.Val4());
#endif  // LVMON_REPORTING

    // Mix the multirotor and fixed-wing outputs to create our final output
    if (proportion_fixed_wing_ >= 1.0f)  // should be clamped to 1
      output_ =
          AxisNr(outputFW, {proportion_fixed_wing_}, kCChannelOutputAdditional);
    else if (proportion_fixed_wing_ <= 0.0f)  // should be clamped to 0
      output_ = AxisNr(outputMultirotor, {proportion_fixed_wing_},
                       kCChannelOutputAdditional);
    else {
      auto proportion_multirotor = 1.0f - proportion_fixed_wing_;

      for (unsigned int i = 0; i < outputFW.AxisCount(); ++i)
        output_[i] = outputMultirotor[i] * proportion_multirotor +
                     outputFW[i] * proportion_fixed_wing_;
      output_[rotor_angle_index_] = proportion_fixed_wing_;
    }

    return (output_);
  }

 private:  // Private types
  // The flight mode specifies how we're flying the aircraft
  enum class FlightMode {
    Multirotor,  // Flying with default orientation, rotors and wing facing up
    FixedWing,   // Flying with vehicle rotate 90 degrees around Y axis so that
                 // rotors are sideway and usually wing parallel to ground
  };             // enum class FlightMode

 private:
  static constexpr unsigned int kCChannelOutputAdditional =
      1;  // Number of additional output channels beyond those in Axix4r
  static const int kCFixedWingModeOKMax =
      10;  // Number of times speed exceeds the fixed-wing mode threshold (stall
           // + kDSpeedTransition) before starting transition to fixed-wing mode
  static constexpr float kDSpeedTransition =
      2.5;  // Speed above stall to completely transition to fixed-wing mode
            // (meters per second)
  static constexpr uint64_t kDMicrosTransition =
      5000 * 1000;  // How long to transition between multirotor and fixed-wing
                    // modes (microseconds)

 private:
  const IBoardClock* boardclock_;        // Event clock
  CascadeController cascadecontroller_;  // multirotor flight mode controller
  int cfixedwing_mode_ok_;  // Used to filter when speed exceeds mode transition
                            // thresholds--counter advances towards
                            // kCFixedWingModeOKMax when the speed has equaled
                            // or exceeded the fixed-wing mode threshold,
                            // retreats towards zero when the speed is below the
                            // stall speed
  bool foutput_is_dirty_;   // If true, output_ needs to be updated
  TRController trcontroller_;  // Fixed-wing flight mode controller
  FlightMode
      flight_mode_cur_;  // Current flight mode or flight mode we're
                         // transitioning to (determines interpretation of goal)
  const IGoal* goal_;    // Goal provider
  uint64_t
      micros_transition_last_;   // Timestamp when the flight mode transition
                                 // was last updated (microseconds)
  AxisNr output_;                // Our last output (axis torque & throttle)
  Params* params_;               // Source and multirotor controller parameters
  Params params_fixed_wing_;     // Fixed-wing controller parameters
  float proportion_fixed_wing_;  // How to mix the multirotor and fixed-wing
                                 // controller outputs (0 = all multirotor, 1 =
                                 // all fixed-wing)
  const IStateEstimator* state_estimator_;  // Vehicle state estimator

  const unsigned int rotor_angle_index_ = 4;
};  // class VTRCascadeController

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_VTRCASCADECONTROLLER_HPP_