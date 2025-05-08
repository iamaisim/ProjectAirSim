// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef ROVER_API_INCLUDE_IACKERMANN_API_HPP_
#define ROVER_API_INCLUDE_IACKERMANN_API_HPP_

#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

// Any APIs that pertain to Ackermann steering only can be placed here.
// For now, it only has the SetRoverControls() API.
class AckermannApi {
 public:
  virtual bool SetRoverControls(float engine, float steering_angle,
                                float brake) = 0;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_ACKERMANN_API_HPP_