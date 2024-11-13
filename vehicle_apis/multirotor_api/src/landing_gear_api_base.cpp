// Copyright (C) Microsoft Corporation. All rights reserved.

#include "landing_gear_api_base.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace microsoft {
namespace projectairsim {

LandingGearApiBase::LandingGearApiBase(const Robot& robot,
                                       TransformTree* ptransformtree)
    : VTOLFWApiBase(robot, ptransformtree) {}

bool LandingGearApiBase::SetBrakesServiceMethod(float value) {
  return SetBrakes(value);
}

void LandingGearApiBase::RegisterServiceMethods(void) {
  VTOLFWApiBase::RegisterServiceMethods();

  // Register SetBrakes
  auto method = ServiceMethod("SetBrakes", {"_brakes_value"});
  auto method_handler = method.CreateMethodHandler(&LandingGearApiBase::SetBrakesServiceMethod, *this);
  sim_robot_.RegisterServiceMethod(method, method_handler);
}

}  // namespace projectairsim
}  // namespace microsoft