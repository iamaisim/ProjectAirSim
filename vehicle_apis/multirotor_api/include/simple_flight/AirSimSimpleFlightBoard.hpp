// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTBOARD_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTBOARD_HPP_

#include <exception>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "firmware/Params.hpp"
#include "firmware/interfaces/IBoard.hpp"

namespace microsoft {
namespace projectairsim {

class AirSimSimpleFlightBoard : public simple_flight::IBoard {
 public:
  AirSimSimpleFlightBoard(const simple_flight::Params* params)
      : params_(params) {}

  // interface for simulator
  // --------------------------------------------------------------------------------
  // for now we don't do any state estimation and use ground truth (i.e. assume
  // perfect sensors)
  // called by multirotorpawnsimapi::initialize(), after creation of
  // simpleflightapi
  void SetGroundTruthKinematics(const Kinematics* kinematics) {
    kinematics_ = kinematics;
  }

  // called to get o/p motor signal as float value
  float GetMotorControlSignal(unsigned int index) const {
    // convert PWM to scaled 0 to 1 control signal
    return static_cast<float>(motor_output_[index]);
  }

  // set current RC stick status
  void SetInputChannel(unsigned int index, float val) {
    input_channels_[index] = static_cast<float>(val);
  }

  void SetIsRcConnected(bool is_connected) { is_connected_ = is_connected; }

 public:
  // Board interface implementation
  int64_t Micros() const override { return Clock()->NowSimMicros(); }

  int64_t Millis() const override { return Clock()->NowSimMillis(); }

  float ReadChannel(uint16_t index) const override {
    return input_channels_[index];
  }

  float GetAvgMotorOutput() const override {
    return ((GetMotorControlSignal(0) + GetMotorControlSignal(1) +
             GetMotorControlSignal(2) + GetMotorControlSignal(3)) /
            4);
  }

  bool IsRcConnected() const override { return is_connected_; }

  void WriteOutput(uint16_t index, float value) override {
    motor_output_[index] = value;
  }

  void ReadAccel(float accel[3]) const override {
    const auto& linear_accel = PhysicsUtils::TransformVectorToBodyFrame(
        kinematics_->accels.linear, kinematics_->pose.orientation);
    accel[0] = linear_accel.x();
    accel[1] = linear_accel.y();
    accel[2] = linear_accel.z();
  }

  void ReadGyro(float gyro[3]) const override {
    const auto angular_vel =
        kinematics_->twist
            .angular;  // angular velocity is already in body frame
    gyro[0] = angular_vel.x();
    gyro[1] = angular_vel.y();
    gyro[2] = angular_vel.z();
  }

  void Reset() override {
    IBoard::Reset();

    motor_output_.assign(params_->motor.motor_count, 0);
    input_channels_.assign(params_->rc.channel_count, 0);
    is_connected_ = false;
  }

  void Update() override {
    IBoard::Update();

    // no op for now
  }

 private:
  // todo ratnesh do I need both const and non-const clock getter?
  ClockBase* Clock() { return SimClock::Get(); }
  const ClockBase* Clock() const { return SimClock::Get(); }

 private:
  // motor outputs
  std::vector<float> motor_output_;
  std::vector<float> input_channels_;
  bool is_connected_;

  const simple_flight::Params* params_;
  const Kinematics* kinematics_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTBOARD_HPP_