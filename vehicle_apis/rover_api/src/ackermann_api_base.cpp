// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "ackermann_api_base.hpp"

namespace microsoft {
namespace projectairsim {

AckermannApiBase::AckermannApiBase(const Robot& robot,
                                   TransformTree* ptransformtree)
    : RoverApiBase(robot, ptransformtree) {}

void AckermannApiBase::RegisterServiceMethods(void) {
  RoverApiBase::RegisterServiceMethods();

  // Register SetRoverControls
  auto method =
      ServiceMethod("SetRoverControls", {"engine", "steering_angle", "brake"});
  auto method_handler =
      method.CreateMethodHandler(&AckermannApiBase::SetRoverControls, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

}  // namespace projectairsim
}  // namespace microsoft
