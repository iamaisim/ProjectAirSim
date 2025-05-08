// Copyright (C) Microsoft Corporation. All rights reserved.

#include "rover_api_base.hpp"

namespace microsoft {
namespace projectairsim {

RoverApiBase::RoverApiBase() : psim_transformtree_(nullptr), sim_robot_() {}

RoverApiBase::RoverApiBase(const Robot& robot, TransformTree* ptransformtree)
    : psim_transformtree_(ptransformtree), sim_robot_(robot) {}

bool RoverApiBase::CanArmServiceMethod(void) { return CanArm(); }

bool RoverApiBase::CancelLastTask(void) {
  try {
    cancel_token_.Cancel();
  } catch (std::exception e) {
    GetLogger().LogError(GetControllerName(),
                         "Error while cancelling last task: %s", e.what());
    return false;
  } catch (...) {
    GetLogger().LogError(GetControllerName(),
                         "Error while cancelling last task.");
    return false;
  }
  return true;  // Have to return something to be able to expose ServiceMethod
}

// executes a given function until it returns true. Each execution is spaced
// apart at command period. return value is true if exit was due to given
// function returning true, otherwise false (due to timeout)
vehicle_apis::FunctionCaller RoverApiBase::RunTimedCommand(
    WaitFunction flight_controller_function, float flight_command_timeout_sec,
    int64_t command_start_time_nanos) {
  vehicle_apis::FunctionCaller function_caller(
      GetCommandPeriod(), flight_command_timeout_sec, GetCancelToken(),
      command_start_time_nanos);

  if (flight_command_timeout_sec <= 0) {
    return function_caller;
  }

  do {
    if (flight_controller_function()) {
      function_caller.Complete();
      break;
    }
  } while (function_caller.WaitForInterval());
  return function_caller;
}

void RoverApiBase::RegisterServiceMethods() {
  // Register EnableApiControl
  auto method = ServiceMethod("EnableApiControl", {""});
  auto method_handler =
      method.CreateMethodHandler(&RoverApiBase::EnableApiControl, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register DisableApiControl
  method = ServiceMethod("DisableApiControl", {""});
  method_handler =
      method.CreateMethodHandler(&RoverApiBase::DisableApiControl, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register IsApiControlEnabled
  method = ServiceMethod("IsApiControlEnabled", {""});
  method_handler =
      method.CreateMethodHandler(&RoverApiBase::IsApiControlEnabled, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register Arm
  method = ServiceMethod("Arm", {"_service_method_start_time"});
  method_handler = method.CreateMethodHandler(&RoverApiBase::Arm, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register Disarm
  method = ServiceMethod("Disarm", {""});
  method_handler = method.CreateMethodHandler(&RoverApiBase::Disarm, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register CanArm
  method = ServiceMethod("CanArm", {""});
  method_handler =
      method.CreateMethodHandler(&RoverApiBase::CanArmServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register CancelLastTask
  method = ServiceMethod("CancelLastTask", {""});
  method_handler =
      method.CreateMethodHandler(&RoverApiBase::CancelLastTask, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveToPosition
  method = ServiceMethod(
      "MoveToPosition",
      {"x", "y", "velocity", "timeout_sec", "yaw_rate_max", "lookahead",
       "adaptive_lookahead", "_service_method_start_time"});
  method_handler =
      method.CreateMethodHandler(&RoverApiBase::MoveToPosition, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);

  // Register MoveByHeading
  method = ServiceMethod(
      "MoveByHeading",
      {"heading", "speed", "duration", "heading_margin",
        "yaw_rate", "timeout_sec", "_service_method_start_time"});
  method_handler =
      method.CreateMethodHandler(&RoverApiBase::MoveByHeading, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

}  // namespace projectairsim
}  // namespace microsoft
