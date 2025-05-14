// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "vtolfw_api_base.hpp"

#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>

#include "core_sim/math_utils.hpp"
#include "core_sim/message/flight_control_setpoint_message.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "imultirotor_api.hpp"

namespace microsoft {
namespace projectairsim {

VTOLFWApiBase::VTOLFWApiBase(const Robot& robot, TransformTree* ptransformtree)
    : MultirotorApiBase(robot, ptransformtree) {}

bool VTOLFWApiBase::MoveByHeading(float heading, float speed, float vz,
                                  float duration, float heading_margin,
                                  float yaw_rate, float timeout_sec,
                                  int64_t command_start_time_nanos) {
  SingleTaskCall lock(this);
  float yaw = MathUtils::NormalizeAngle<float>(heading);

  return (RunFlightCommand(
              [&]() {
                static const float kPI = (float)M_PI;
                static const float k2PI = (float)(2.0 * M_PI);
                static const float kPIHalf = (float)(M_PI / 2.0);

                // Yaw to the heading at the specified rate
                if ((yaw_rate > 0) && !IsYawWithinMargin(yaw, heading_margin)) {
                  auto vx = cos(heading) * speed;
                  auto vy = sin(heading) * speed;
                  float yaw_current = GetAngles()[2];
                  float dyaw = yaw - yaw_current;

                  // Pick the most direct rotation direction
                  if (dyaw > kPI)
                    dyaw -= k2PI;
                  else if (dyaw < -kPI)
                    dyaw += k2PI;
                  if (dyaw < 0) yaw_rate = -yaw_rate;

                  if (RunFlightCommand(
                          [&]() {
                            if (IsYawWithinMargin(yaw, heading_margin))
                              return (true);
                            else {
                              float dyaw = yaw - GetAngles()[2];

                              if (dyaw > kPI)
                                dyaw -= k2PI;
                              else if (dyaw < -kPI)
                                dyaw += k2PI;
                              if (fabs(dyaw) < kPIHalf) {
                                if ((dyaw > 0) != (yaw_rate > 0))
                                  return (
                                      true);  // Direction of yaw to target is
                                              // opposite of yaw rate--we've
                                              // turned past the target heading
                              }

                              CommandVelocity(vx, vy, vz, true, yaw_rate);
                            }

                            return (false);
                          },
                          timeout_sec, command_start_time_nanos)
                          .IsTimeout()) {
                    return (false);  // Let outer RunFlightCommand timeout
                  }
                }

                // Command the heading and wait until we've reached it
                if (RunFlightCommand(
                        [&]() {
                          if (IsYawWithinMargin(yaw, heading_margin))
                            return (true);
                          else
                            CommandHeading(yaw, speed, vz);

                          return (false);
                        },
                        timeout_sec, command_start_time_nanos)
                        .IsTimeout()) {
                  return (false);  // Let outer RunFlightCommand timeout
                }

                // Keep commanding the heading for the requested duration
                (void)RunFlightCommand(
                    [&]() {
                      CommandHeading(yaw, speed, vz);
                      return (false);
                    },
                    duration, SimClock::Get()->NowSimNanos());

                return (true);
              },
              timeout_sec, command_start_time_nanos)
              .IsComplete());
}

bool VTOLFWApiBase::MoveByHeadingServiceMethod(
    float heading, float speed, float vz, float duration, float heading_margin,
    float yaw_rate, float timeout_sec, TimeNano _service_method_start_time) {
  return MoveByHeading(heading, speed, vz, duration, heading_margin, yaw_rate,
                       timeout_sec, _service_method_start_time);
}

void VTOLFWApiBase::RegisterServiceMethods() {
  MultirotorApiBase::RegisterServiceMethods();

  // Register MoveByHeading
  auto method =
      ServiceMethod("MoveByHeading",
                    {"heading", "speed", "vz", "duration", "heading_margin",
                     "yaw_rate", "timeout_sec", "_service_method_start_time"});
  auto method_handler = method.CreateMethodHandler(
      &VTOLFWApiBase::MoveByHeadingServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register SetFlightMode
  method = ServiceMethod("SetVTOLMode", {"vtol_mode"});
  method_handler =
      method.CreateMethodHandler(&VTOLFWApiBase::SetVTOLMode, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

bool VTOLFWApiBase::SetVTOLMode(VTOLMode /*vtolmode*/) { return (false); }

}  // namespace projectairsim
}  // namespace microsoft
