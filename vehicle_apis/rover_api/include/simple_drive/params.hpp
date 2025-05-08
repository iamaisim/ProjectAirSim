// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_DRIVE_PARAMS_HPP_
#define MULTIROTOR_API_SIMPLE_DRIVE_PARAMS_HPP_

#include <unordered_map>

#include "common.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

struct Params {
 public:
  static float MinArmedThrottle() {
    static float val = 0.1f;
    return val;
  }

 public:
  void Load(const std::unordered_map<std::string, float>& params_map) {}

  struct Rc {
    uint16_t channel_count = 12;  // Total number of input channels supported
    uint64_t connection_update_timeout =
        500;  // Maximum period between input updates before controller is
              // marked as disconnected (milliseconds)
    uint16_t read_interval_ms =
        10;  // Period between reading input channels (milliseconds)
    int16_t rate_level_mode_channel = 4;  // corresponds to switch 0 in rc_data
    int16_t allow_api_control_channel = 5;  // corresponds to switch 1 in
                                            // rc_data

    // When actions such as arming/unarming, how much tolerance can be allowed
    // in stick positions from 0 to 1?
    float action_request_tolerance = 0.1f;

    // milliseconds while sticks should stay in position
    uint64_t arm_duration = 100;
    uint64_t disarm_duration = 100;
    uint64_t neutral_duration = 100;

    vehicle_apis::Axis4<int16_t> channels =
        vehicle_apis::Axis4<int16_t>(0, 3, 1, 2);

    vehicle_apis::TReal max_angle_level_switch = 0.3f;

    // should be >= motor.min_angling_throttle
    float min_angling_throttle = Params::MinArmedThrottle() / 1.5f;

    bool allow_api_when_disconnected = true;
    bool allow_api_always = true;
  } rc;

  struct AngleRatePid {
    // max_xxx_rate > 5 would introduce wobble/oscillations
    static constexpr float kMaxLimit = 2.5f;
    vehicle_apis::Axis3r max_limit =
        vehicle_apis::Axis3r(kMaxLimit, kMaxLimit,
                             kMaxLimit);  // roll, pitch, yaw (radians/sec)

    // p_xxx_rate params are sensitive to gyro noise. Values higher than 0.5
    // would require noise filtration
    static constexpr const float kP = 0.25f, kI = 0.0f, kD = 0.0f;
    vehicle_apis::Axis4r p = vehicle_apis::Axis4r(kP, kP, kP, 1.0f);
    vehicle_apis::Axis4r i = vehicle_apis::Axis4r(kI, kI, kI, 0.0f);
    vehicle_apis::Axis4r d = vehicle_apis::Axis4r(kD, kD, kD, 0.0f);
  } angle_rate_pid;

  struct AngleLevelPid {
    // max_pitch/roll_angle > (PI / 5.5) would produce vertical thrust that is
    // not enough to keep vehicle in air at extremities of controls
    vehicle_apis::Axis4r max_limit = vehicle_apis::Axis4r(
        vehicle_apis::kPI / 5.5f, vehicle_apis::kPI / 5.5f, vehicle_apis::kPI,
        1.0f);  // roll, pitch, yaw (in radians)

    static constexpr float kP = 0.5f, kI = 0.0f, kD = 0.0f;
    vehicle_apis::Axis4r p = vehicle_apis::Axis4r(kP, kP, kP, 1.0f);
    vehicle_apis::Axis4r i = vehicle_apis::Axis4r(kI, kI, kI, 0.0f);
    vehicle_apis::Axis4r d = vehicle_apis::Axis4r(kD, kD, kD, 0.0f);
  } angle_level_pid;

  struct PositionPid {
    static constexpr float kMaxLimit =
        8.8E26f;  // some big number like size of known universe
    vehicle_apis::Axis4r max_limit = vehicle_apis::Axis4r(
        kMaxLimit, kMaxLimit, kMaxLimit, 1.0f);  // x, y, z in meters

    static constexpr float kP = 0.25f, kI = 0.0f, kD = 0.0f;
    vehicle_apis::Axis4r p = vehicle_apis::Axis4r(kP, kP, 0, kP);
    vehicle_apis::Axis4r i = vehicle_apis::Axis4r(kI, kI, kI, kI);
    vehicle_apis::Axis4r d = vehicle_apis::Axis4r(kD, kD, kD, kD);
  } position_pid;

  struct VelocityPid {
    const float kMinThrottle =
        std::min(1.0f, Params::MinArmedThrottle() * 3.0f);
    static constexpr float kMaxLimit = 6.0f;  // m/s
    vehicle_apis::Axis4r max_limit = vehicle_apis::Axis4r(
        kMaxLimit, kMaxLimit, 0, kMaxLimit);  // x, y, yaw, z in meters

    static constexpr float kP = 0.2f, kI = 2.0f, kD = 0.0f;
    vehicle_apis::Axis4r p = vehicle_apis::Axis4r(kP, kP, 0.0f, 2.0f);
    vehicle_apis::Axis4r i = vehicle_apis::Axis4r(0.0f, 0.0f, 0.0f, kI);
    vehicle_apis::Axis4r d = vehicle_apis::Axis4r(kD, kD, kD, kD);

    vehicle_apis::Axis4r iterm_discount =
        vehicle_apis::Axis4r(1, 1, 1, 0.9999f);
    vehicle_apis::Axis4r output_bias = vehicle_apis::Axis4r(0, 0, 0, 0);

    // we keep min throttle higher so that if we are angling a lot, its still
    // supported
    float min_throttle = kMinThrottle;

    VelocityPid& operator=(const VelocityPid& other) {
      max_limit = other.max_limit;
      p = other.p;
      i = other.i;
      d = other.d;
      iterm_discount = other.iterm_discount;
      output_bias = other.output_bias;
      min_throttle = other.min_throttle;
      return (*this);
    }
  } velocity_pid;

  enum class ControllerType {
    kAckermannCascade,  // Ackermann steering geometry controller
  };

  vehicle_apis::VehicleStateType default_vehicle_state =
      vehicle_apis::VehicleStateType::kInactive;
  uint64_t api_goal_timeout = 60;  // milliseconds
  ControllerType controller_type = ControllerType::kAckermannCascade;
  bool gains_changed = false;
};

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_SIMPLE_DRIVE_PARAMS_HPP_