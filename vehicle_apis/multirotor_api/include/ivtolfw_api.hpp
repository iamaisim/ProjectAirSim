// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_IVTOLFW_API_HPP_
#define MULTIROTOR_API_INCLUDE_IVTOLFW_API_HPP_

#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

enum class VTOLMode { Multirotor = 0, FixedWing = 1 };

// Represents public VTOL fixed-wing APIs intended for client
class IVTOLFWApi {
 public:
  // return value of these functions is true if command was completed without
  // interruption or timeouts

  // VTOL
  virtual bool SetVTOLMode(VTOLMode vtolmode) = 0;  // Set the VTOL modes

  // Fixed wing
  virtual bool MoveByHeading(float heading, float speed, float vz,
                             float duration, float heading_margin,
                             float yaw_rate, float timeout_sec,
                             int64_t command_start_time_nanos) = 0;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_IVTOLFW_API_HPP_