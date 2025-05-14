// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_COMMON_HPP_
#define ROVER_API_INCLUDE_COMMON_HPP_

#include <common/common.hpp>
#include <vector>

#include "ilogger.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

using rover_api::ILogger;

typedef std::vector<float> VecR;

// Vehicle controller target goal for a coordinate axis or vehicle control
// parameter
struct Goal {
  // How to interpret the goal's value.  If the mode does not apply apply to the
  // target (such as specifying kAngleLevel for the throttle) the mode is
  // ignored or treated as kUnknown.
  enum class Mode {
    kNone,         // No goal (controller may control automatically)
    kPassthrough,  // Goal value is passed through directly to affected
                   // actuators as control values
    kAngleLevel,   // Goal value is absolute angle (radians)
    kAngleRate,    // Goal value is rate of change of angle (radians per second)
    kVelocityWorld,  // Goal value is velocity along associated world coordinate
                     // axis
    kVelocityBody,   // Goal value is velocity along associated coordinate axis
                     // of world X-Y plane aligned to vehicle's yaw
    kPositionWorld,  // Goal value is an absolute position along associated
  };                 // enum class Mode

  Mode mode;    // Goal's mode
  float value;  // Goal's value

  Goal(float value_in = 0.0f) : mode(Mode::kNone), value(value_in) {}
  void Set(Mode mode_in, float value_in) {
    mode = mode_in;
    value = value_in;
  }

  operator float(void) const { return value; }
};  // struct Goal

// Vehicle controller indexes of various control targets within the list of
// goals
enum GoalTargetIndex {
  kX = 0,  // X-axis goal (kPassthrough and kAngle* modes are ignored)
  kY = 1,  // Y-axis goal (kPassthrough and kAngle* modes are ignored)
  kZ = 2,  // Z-axis goal; used for heading (kAngleLevel or  kAngleRate modes)
           // or steering (kPassthrough) only, not vertical movement or position
  kThrottle = 3,  // Throttle goal (kNone, kPassthrough, kVelocityWorld, and
                  // kVelocityBody modes only)
  kBrake = 4,     // Brake goal (kNone or kPassthrough modes only)

  // THE FOLLOWING MUST BE LAST
  MAX
};  // enum GoalTargetIndex

// The modes of the goal targets
struct GoalModes : public vehicle_apis::AxisN<Goal::Mode> {
  GoalModes(void)
      : AxisN{
            Goal::Mode::kNone,  // kX
            Goal::Mode::kNone,  // kY
            Goal::Mode::kNone,  // kZ
            Goal::Mode::kNone,  // kThrottle
            Goal::Mode::kNone   // kBrake
        } {}
};  // namespace simple_drive

// A set of goals
class Goals : public vehicle_apis::AxisN<Goal> {
 public:
  Goals(void)
      : AxisN{
            Goal(),  // kX
            Goal(),  // kY
            Goal(),  // kZ
            Goal(),  // kThrottle
            Goal(),  // kBrake
        } {}

    bool CheckChangedModes(const Goals &goals_other);
};  // class Goals

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_COMMON_HPP_
