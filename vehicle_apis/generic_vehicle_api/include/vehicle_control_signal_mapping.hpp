// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef GENERIC_VEHICLE_API_INCLUDE_VEHICLE_CONTROL_SIGNAL_MAPPING_HPP_
#define GENERIC_VEHICLE_API_INCLUDE_VEHICLE_CONTROL_SIGNAL_MAPPING_HPP_

#include <string>
#include <vector>

namespace microsoft {
namespace projectairsim {

// Controller type string constants used by scene/robot config and SimServer.
inline constexpr const char* kControllerTypeUnrealVehicleApi =
    "unreal-vehicle-api";
inline constexpr const char* kControllerTypeSimpleDriveApi = "simple-drive-api";

// Maps controller-native control signals into the Blueprint / force-application
// order used by AUnrealRobot: throttle, brake, steering.
//
// Input layouts:
//   unreal-vehicle-api: [throttle, brake, steering]
//   simple-drive-api:   [throttle, steering, brake]
//
// Missing channels are zero-filled.
inline void MapVehicleControlSignals(const std::string& controller_type,
                                     const std::vector<float>& signals,
                                     float& throttle, float& brake,
                                     float& steering) {
  throttle = 0.f;
  brake = 0.f;
  steering = 0.f;

  if (controller_type == kControllerTypeSimpleDriveApi) {
    if (signals.size() > 0) throttle = signals[0];
    if (signals.size() > 1) steering = signals[1];
    if (signals.size() > 2) brake = signals[2];
    return;
  }

  // Default / unreal-vehicle-api: [throttle, brake, steering]
  if (signals.size() > 0) throttle = signals[0];
  if (signals.size() > 1) brake = signals[1];
  if (signals.size() > 2) steering = signals[2];
}

}  // namespace projectairsim
}  // namespace microsoft

#endif  // GENERIC_VEHICLE_API_INCLUDE_VEHICLE_CONTROL_SIGNAL_MAPPING_HPP_
