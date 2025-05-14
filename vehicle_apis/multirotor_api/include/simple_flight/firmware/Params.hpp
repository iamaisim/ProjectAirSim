// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PARAMS_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PARAMS_HPP_

#include "interfaces/CommonStructs.hpp"

namespace simple_flight {

struct Params {
 public:
  static float MinArmedThrottle() {
    static float val = 0.1f;
    return val;
  }

  void LoadParams(const std::unordered_map<std::string, float>& params_map) {
    if (params_map.find("MC_TKO_Z") != params_map.end())
      takeoff.takeoff_z = params_map.at("MC_TKO_Z");

    // Multirotor angle rate PID parameters
    if (params_map.find("MC_ROLLRATE_MAX") != params_map.end())
      angle_rate_pid.max_limit.Roll() = params_map.at("MC_ROLLRATE_MAX");

    if (params_map.find("MC_PITCHRATE_MAX") != params_map.end())
      angle_rate_pid.max_limit.Pitch() = params_map.at("MC_PITCHRATE_MAX");

    if (params_map.find("MC_YAWRATE_MAX") != params_map.end())
      angle_rate_pid.max_limit.Yaw() = params_map.at("MC_YAWRATE_MAX");

    if (params_map.find("MC_ROLLRATE_P") != params_map.end())
      angle_rate_pid.p.Roll() = params_map.at("MC_ROLLRATE_P");

    if (params_map.find("MC_ROLLRATE_I") != params_map.end())
      angle_rate_pid.i.Roll() = params_map.at("MC_ROLLRATE_I");

    if (params_map.find("MC_ROLLRATE_D") != params_map.end())
      angle_rate_pid.d.Roll() = params_map.at("MC_ROLLRATE_D");

    if (params_map.find("MC_PITCHRATE_P") != params_map.end())
      angle_rate_pid.p.Pitch() = params_map.at("MC_PITCHRATE_P");

    if (params_map.find("MC_PITCHRATE_I") != params_map.end())
      angle_rate_pid.i.Pitch() = params_map.at("MC_PITCHRATE_I");

    if (params_map.find("MC_PITCHRATE_D") != params_map.end())
      angle_rate_pid.d.Pitch() = params_map.at("MC_PITCHRATE_D");

    if (params_map.find("MC_YAWRATE_P") != params_map.end())
      angle_rate_pid.p.Yaw() = params_map.at("MC_YAWRATE_P");

    if (params_map.find("MC_YAWRATE_I") != params_map.end())
      angle_rate_pid.i.Yaw() = params_map.at("MC_YAWRATE_I");

    if (params_map.find("MC_YAWRATE_D") != params_map.end())
      angle_rate_pid.d.Yaw() = params_map.at("MC_YAWRATE_D");

    // Multirotor angle level PID parameters
    if (params_map.find("MC_ROLL_MAX") != params_map.end())
      angle_level_pid.max_limit.Roll() = params_map.at("MC_ROLL_MAX");

    if (params_map.find("MC_PITCH_MAX") != params_map.end())
      angle_level_pid.max_limit.Pitch() = params_map.at("MC_PITCH_MAX");

    if (params_map.find("MC_YAW_MAX") != params_map.end())
      angle_level_pid.max_limit.Yaw() = params_map.at("MC_YAW_MAX");

    if (params_map.find("MC_ROLL_P") != params_map.end())
      angle_level_pid.p.Roll() = params_map.at("MC_ROLL_P");

    if (params_map.find("MC_ROLL_I") != params_map.end())
      angle_level_pid.i.Roll() = params_map.at("MC_ROLL_I");

    if (params_map.find("MC_ROLL_D") != params_map.end())
      angle_level_pid.d.Roll() = params_map.at("MC_ROLL_D");

    if (params_map.find("MC_PITCH_P") != params_map.end())
      angle_level_pid.p.Pitch() = params_map.at("MC_PITCH_P");

    if (params_map.find("MC_PITCH_I") != params_map.end())
      angle_level_pid.i.Pitch() = params_map.at("MC_PITCH_I");

    if (params_map.find("MC_PITCH_D") != params_map.end())
      angle_level_pid.d.Pitch() = params_map.at("MC_PITCH_D");

    if (params_map.find("MC_YAW_P") != params_map.end())
      angle_level_pid.p.Yaw() = params_map.at("MC_YAW_P");

    if (params_map.find("MC_YAW_I") != params_map.end())
      angle_level_pid.i.Yaw() = params_map.at("MC_YAW_I");

    if (params_map.find("MC_YAW_D") != params_map.end())
      angle_level_pid.d.Yaw() = params_map.at("MC_YAW_D");

    // Multirotor position PID parameters
    if (params_map.find("MPC_XY_P") != params_map.end()) {
      float val = params_map.at("MPC_XY_P");
      position_pid.p.X() = val;
      position_pid.p.Y() = val;
    }

    if (params_map.find("MPC_XY_I") != params_map.end()) {
      float val = params_map.at("MPC_XY_I");
      position_pid.i.X() = val;
      position_pid.i.Y() = val;
    }

    if (params_map.find("MPC_XY_D") != params_map.end()) {
      float val = params_map.at("MPC_XY_D");
      position_pid.d.X() = val;
      position_pid.d.Y() = val;
    }

    if (params_map.find("MPC_Z_P") != params_map.end())
      position_pid.p.Throttle() = params_map.at("MPC_Z_P");

    if (params_map.find("MPC_Z_I") != params_map.end())
      position_pid.i.Throttle() = params_map.at("MPC_Z_I");

    if (params_map.find("MPC_Z_D") != params_map.end())
      position_pid.d.Throttle() = params_map.at("MPC_Z_D");

    // Multirotor velocity PID parameters
    if (params_map.find("MPC_MIN_THR") != params_map.end())
      velocity_pid.min_throttle = params_map.at("MPC_MIN_THR");

    if (params_map.find("MPC_XY_VEL_MAX") != params_map.end()) {
      float val = params_map.at("MPC_XY_VEL_MAX");
      velocity_pid.max_limit.X() = val;
      velocity_pid.max_limit.Y() = val;
    }

    if (params_map.find("MPC_Z_VEL_MAX") != params_map.end())
      velocity_pid.max_limit.Throttle() = params_map.at("MPC_Z_VEL_MAX");

    if (params_map.find("MPC_XY_VEL_P") != params_map.end()) {
      float val = params_map.at("MPC_XY_VEL_P");
      velocity_pid.p.X() = val;
      velocity_pid.p.Y() = val;
    }

    if (params_map.find("MPC_XY_VEL_I") != params_map.end()) {
      float val = params_map.at("MPC_XY_VEL_I");
      velocity_pid.i.X() = val;
      velocity_pid.i.Y() = val;
    }

    if (params_map.find("MPC_XY_VEL_D") != params_map.end()) {
      float val = params_map.at("MPC_XY_VEL_D");
      velocity_pid.d.X() = val;
      velocity_pid.d.Y() = val;
    }

    if (params_map.find("MPC_Z_VEL_P") != params_map.end())
      velocity_pid.p.Throttle() = params_map.at("MPC_Z_VEL_P");

    if (params_map.find("MPC_Z_VEL_I") != params_map.end())
      velocity_pid.i.Throttle() = params_map.at("MPC_Z_VEL_I");

    if (params_map.find("MPC_Z_VEL_D") != params_map.end())
      velocity_pid.d.Throttle() = params_map.at("MPC_Z_VEL_D");

    // Fixed-wing vehicle parameters
    if (params_map.find("FW_AIRSPD_STALL") != params_map.end())
      fixed_wing.speed_stall = params_map.at("FW_AIRSPD_STALL");

    if (params_map.find("FW_AIRSPD_FWD") != params_map.end())
      fixed_wing.speed_forward_default = params_map.at("FW_AIRSPD_FWD");

    // Fixed-wing angle rate PID parameters
    if (params_map.find("FW_R_RMAX") != params_map.end())
      fixed_wing.angle_rate_pid.max_limit.Roll() = params_map.at("FW_R_RMAX");

    if (params_map.find("FW_P_RMAX") != params_map.end())
      fixed_wing.angle_rate_pid.max_limit.Pitch() = params_map.at("FW_P_RMAX");

    if (params_map.find("FW_Y_RMAX") != params_map.end())
      fixed_wing.angle_rate_pid.max_limit.Yaw() = params_map.at("FW_Y_RMAX");

    if (params_map.find("FW_RR_P") != params_map.end())
      fixed_wing.angle_rate_pid.p.Roll() = params_map.at("FW_RR_P");

    if (params_map.find("FW_RR_I") != params_map.end())
      fixed_wing.angle_rate_pid.i.Roll() = params_map.at("FW_RR_I");

    if (params_map.find("FW_RR_D") != params_map.end())
      fixed_wing.angle_rate_pid.d.Roll() = params_map.at("FW_RR_D");

    if (params_map.find("FW_PR_P") != params_map.end())
      fixed_wing.angle_rate_pid.p.Pitch() = params_map.at("FW_PR_P");

    if (params_map.find("FW_PR_I") != params_map.end())
      fixed_wing.angle_rate_pid.i.Pitch() = params_map.at("FW_PR_I");

    if (params_map.find("FW_PR_D") != params_map.end())
      fixed_wing.angle_rate_pid.d.Pitch() = params_map.at("FW_PR_D");

    if (params_map.find("FW_YR_P") != params_map.end())
      fixed_wing.angle_rate_pid.p.Yaw() = params_map.at("FW_YR_P");

    if (params_map.find("FW_YR_I") != params_map.end())
      fixed_wing.angle_rate_pid.i.Yaw() = params_map.at("FW_YR_I");

    if (params_map.find("FW_YR_D") != params_map.end())
      fixed_wing.angle_rate_pid.d.Yaw() = params_map.at("FW_YR_D");

    // Fixed-wing angle level PID parameters
    if (params_map.find("FW_ROLL_MAX") != params_map.end())
      fixed_wing.angle_level_pid.max_limit.Roll() =
          params_map.at("FW_ROLL_MAX");

    if (params_map.find("FW_PITCH_MAX") != params_map.end())
      fixed_wing.angle_level_pid.max_limit.Pitch() =
          params_map.at("FW_PITCH_MAX");

    if (params_map.find("FW_YAW_MAX") != params_map.end())
      fixed_wing.angle_level_pid.max_limit.Yaw() = params_map.at("FW_YAW_MAX");

    if (params_map.find("FW_ROLL_P") != params_map.end())
      fixed_wing.angle_level_pid.p.Roll() = params_map.at("FW_ROLL_P");

    if (params_map.find("FW_ROLL_I") != params_map.end())
      fixed_wing.angle_level_pid.i.Roll() = params_map.at("FW_ROLL_I");

    if (params_map.find("FW_ROLL_D") != params_map.end())
      fixed_wing.angle_level_pid.d.Roll() = params_map.at("FW_ROLL_D");

    if (params_map.find("FW_PITCH_P") != params_map.end())
      fixed_wing.angle_level_pid.p.Pitch() = params_map.at("FW_PITCH_P");

    if (params_map.find("FW_PITCH_I") != params_map.end())
      fixed_wing.angle_level_pid.i.Pitch() = params_map.at("FW_PITCH_I");

    if (params_map.find("FW_PITCH_D") != params_map.end())
      fixed_wing.angle_level_pid.d.Pitch() = params_map.at("FW_PITCH_D");

    if (params_map.find("FW_YAW_P") != params_map.end())
      fixed_wing.angle_level_pid.p.Yaw() = params_map.at("FW_YAW_P");

    if (params_map.find("FW_YAW_I") != params_map.end())
      fixed_wing.angle_level_pid.i.Yaw() = params_map.at("FW_YAW_I");

    if (params_map.find("FW_YAW_D") != params_map.end())
      fixed_wing.angle_level_pid.d.Yaw() = params_map.at("FW_YAW_D");

    // Fixed-wing position PID parameters
    if (params_map.find("FW_XY_P") != params_map.end()) {
      float val = params_map.at("FW_XY_P");
      fixed_wing.position_pid.p.X() = val;
      fixed_wing.position_pid.p.Y() = val;
    }

    if (params_map.find("FW_XY_I") != params_map.end()) {
      float val = params_map.at("FW_XY_I");
      fixed_wing.position_pid.i.X() = val;
      fixed_wing.position_pid.i.Y() = val;
    }

    if (params_map.find("FW_XY_D") != params_map.end()) {
      float val = params_map.at("FW_XY_D");
      fixed_wing.position_pid.d.X() = val;
      fixed_wing.position_pid.d.Y() = val;
    }

    if (params_map.find("FW_Z_P") != params_map.end())
      fixed_wing.position_pid.p.Throttle() = params_map.at("FW_Z_P");

    if (params_map.find("FW_Z_I") != params_map.end())
      fixed_wing.position_pid.i.Throttle() = params_map.at("FW_Z_I");

    if (params_map.find("FW_Z_D") != params_map.end())
      fixed_wing.position_pid.d.Throttle() = params_map.at("FW_Z_D");

    // Fixed-wing velocity PID parameters
    if (params_map.find("FW_MIN_THR") != params_map.end())
      fixed_wing.velocity_pid.min_throttle = params_map.at("FW_MIN_THR");

    if (params_map.find("FW_XY_VEL_MAX") != params_map.end()) {
      float val = params_map.at("FW_XY_VEL_MAX");
      fixed_wing.velocity_pid.max_limit.X() = val;
      fixed_wing.velocity_pid.max_limit.Y() = val;
    }

    if (params_map.find("FW_Z_VEL_MAX") != params_map.end())
      fixed_wing.velocity_pid.max_limit.Throttle() =
          params_map.at("FW_Z_VEL_MAX");

    if (params_map.find("FW_XY_VEL_P") != params_map.end()) {
      float val = params_map.at("FW_XY_VEL_P");
      fixed_wing.velocity_pid.p.X() = val;
      fixed_wing.velocity_pid.p.Y() = val;
    }

    if (params_map.find("FW_XY_VEL_I") != params_map.end()) {
      float val = params_map.at("FW_XY_VEL_I");
      fixed_wing.velocity_pid.i.X() = val;
      fixed_wing.velocity_pid.i.Y() = val;
    }

    if (params_map.find("FW_XY_VEL_D") != params_map.end()) {
      float val = params_map.at("FW_XY_VEL_D");
      fixed_wing.velocity_pid.d.X() = val;
      fixed_wing.velocity_pid.d.Y() = val;
    }

    if (params_map.find("FW_Z_VEL_P") != params_map.end())
      fixed_wing.velocity_pid.p.Throttle() = params_map.at("FW_Z_VEL_P");

    if (params_map.find("FW_Z_VEL_I") != params_map.end())
      fixed_wing.velocity_pid.i.Throttle() = params_map.at("FW_Z_VEL_I");

    if (params_map.find("FW_Z_VEL_D") != params_map.end())
      fixed_wing.velocity_pid.d.Throttle() = params_map.at("FW_Z_VEL_D");

    fixed_wing.f_pid_changed = true;

    // VTOL vehicle parameters
    if (params_map.find("FW_PITCH_MIN") != params_map.end())
      vtol.pitch_min_fixed_wing = params_map.at("FW_PITCH_MIN");
  }

  // this should match up with target board
  // simulation board should respect possible values
  struct Motor {
    uint16_t motor_count = 4;
    float min_motor_output = 0;
    float max_motor_output = 1;
    float min_control_output = -1;  // Minimum non-rotor control value
    float max_control_output = 1;   // Maximum non-rotor control value
    // if min_armed_output too low then noise in pitch/roll can destabilize quad
    // copter when throttle is zero
    float min_angling_throttle = Params::MinArmedThrottle() / 2;
  } motor;

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

    Axis4<int16_t> channels = Axis4<int16_t>(0, 3, 1, 2);

    TReal max_angle_level_switch = 0.3f;

    // should be >= motor.min_angling_throttle
    float min_angling_throttle = Params::MinArmedThrottle() / 1.5f;

    bool allow_api_when_disconnected = true;
    bool allow_api_always = true;
  } rc;

  struct AngleRatePid {
    // max_xxx_rate > 5 would introduce wobble/oscillations
    static constexpr float kMaxLimit = 2.5f;
    Axis3r max_limit = Axis3r(kMaxLimit, kMaxLimit,
                              kMaxLimit);  // roll, pitch, yaw (radians/sec)

    // p_xxx_rate params are sensitive to gyro noise. Values higher than 0.5
    // would require noise filtration
    static constexpr const float kP = 0.25f, kI = 0.0f, kD = 0.0f;
    Axis4r p = Axis4r(kP, kP, kP, 1.0f);
    Axis4r i = Axis4r(kI, kI, kI, 0.0f);
    Axis4r d = Axis4r(kD, kD, kD, 0.0f);
  } angle_rate_pid;

  struct AngleLevelPid {
    // max_pitch/roll_angle > (PI / 5.5) would produce vertical thrust that is
    // not enough to keep vehicle in air at extremities of controls
    Axis4r max_limit = Axis4r(kPI / 5.5f, kPI / 5.5f, kPI,
                              1.0f);  // roll, pitch, yaw (in radians)

    static constexpr float kP = 2.5f, kI = 0.0f, kD = 0.0f;
    Axis4r p = Axis4r(kP, kP, kP, 1.0f);
    Axis4r i = Axis4r(kI, kI, kI, 0.0f);
    Axis4r d = Axis4r(kD, kD, kD, 0.0f);
  } angle_level_pid;

  struct PositionPid {
    static constexpr float kMaxLimit =
        8.8E26f;  // some big number like size of known universe
    Axis4r max_limit =
        Axis4r(kMaxLimit, kMaxLimit, kMaxLimit, 1.0f);  // x, y, z in meters

    static constexpr float kP = 0.25f, kI = 0.0f, kD = 0.0f;
    Axis4r p = Axis4r(kP, kP, 0, kP);
    Axis4r i = Axis4r(kI, kI, kI, kI);
    Axis4r d = Axis4r(kD, kD, kD, kD);
  } position_pid;

  struct VelocityPid {
    const float kMinThrottle =
        std::min(1.0f, Params::MinArmedThrottle() * 3.0f);
    static constexpr float kMaxLimit = 6.0f;  // m/s
    Axis4r max_limit =
        Axis4r(kMaxLimit, kMaxLimit, 0, kMaxLimit);  // x, y, yaw, z in meters

    static constexpr float kP = 0.2f, kI = 2.0f, kD = 0.0f;
    Axis4r p = Axis4r(kP, kP, 0.0f, 2.0f);
    Axis4r i = Axis4r(0.0f, 0.0f, 0.0f, kI);
    Axis4r d = Axis4r(kD, kD, kD, kD);

    Axis4r iterm_discount = Axis4r(1, 1, 1, 0.9999f);
    Axis4r output_bias = Axis4r(0, 0, 0, 0);

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

  struct Takeoff {
    float takeoff_z = -2.0f;
    // float velocity = -1.0f;
  } takeoff;

  // Fixed wing (airplane) vehicle parameters
  struct FixedWing {
    AngleLevelPid
        angle_level_pid;  // Angle level PID parameters for fixed-wing mode
    AngleRatePid
        angle_rate_pid;        // Angle rate PID parameters for fixed-wing mode
    PositionPid position_pid;  // Position PID parameters for fixed-wing mode
    VelocityPid velocity_pid;  // Velocity PID parameters for fixed-wing mode
    bool f_pid_changed =
        false;  // If true, one or more of angle_rate_pid, angle_level_pid,
                // position_pid or velocity_pid has changed
    float speed_forward_default = 20.0f;  // Default speed when a velocity is
                                          // not specified (meters per second)
    float speed_stall = 7.5f;             // Stall speed (meter per second)
  } fixed_wing;

  // Vertical Take-Off and Landing vehicle parameters
  struct VTOL {
    bool enable_fixed_wing_mode =
        false;  // If true, automatically transition between multirotor and
                // fixed-wing flight modes; if false, stay in multirotor mode
    bool in_fixed_wing_mode =
        false;  // If true, the controller running the fixed-wing flight mode
    float pitch_min_fixed_wing =
        float(kPI / 10.0f);  // Minimum pitch angle for fixed-wing flight mode
                             // where 0 == straight up (radians)
  } vtol;

  enum class ControllerType {
    kCascade,      // Multirotor controller
    kVFWTCascade,  // VTOL Fixed-Wing Tailsitter controller
    kVTRCascade    // VTOL Tilt-Rotor controller
  };

  GoalMode default_goal_mode = GoalMode::GetStandardAngleMode();
  VehicleStateType default_vehicle_state = VehicleStateType::kInactive;
  uint64_t api_goal_timeout = 60;  // milliseconds
  ControllerType controller_type = ControllerType::kCascade;
  bool gains_changed = false;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_PARAMS_HPP_