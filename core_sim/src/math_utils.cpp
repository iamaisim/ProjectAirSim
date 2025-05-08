// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/math_utils.hpp"

#include "core_sim/transforms/transform_utils.hpp"

namespace microsoft {
namespace projectairsim {

double MathUtils::eps(int number) { return 1.0 / (pow(10, number)); }

double MathUtils::rad2Deg(const double radians) {
  return TransformUtils::ToDegrees(radians);
}

double MathUtils::deg2Rad(const double degrees) {
  return TransformUtils::ToRadians(degrees);
}

}  // namespace projectairsim
}  // namespace microsoft
