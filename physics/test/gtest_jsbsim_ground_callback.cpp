// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <cmath>

#include "core_sim/earth_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "gtest/gtest.h"
#include "jsbsim_ground_callback.hpp"
#include "math/FGColumnVector3.h"
#include "math/FGLocation.h"

namespace projectairsim = microsoft::projectairsim;

namespace {

constexpr double kWgs84SemiMajorFt = 20925646.32546;
constexpr double kWgs84SemiMinorFt = 20855486.5951;

JSBSim::FGLocation MakeLocation(double latitude_deg, double longitude_deg,
                                double altitude_asl_m) {
  JSBSim::FGLocation location;
  location.SetEllipse(kWgs84SemiMajorFt, kWgs84SemiMinorFt);
  location.SetPositionGeodetic(
      projectairsim::MathUtils::deg2Rad(longitude_deg),
      projectairsim::MathUtils::deg2Rad(latitude_deg),
      altitude_asl_m * projectairsim::MathUtils::meters_to_feets);
  return location;
}

double GetAltitudeAboveGroundFt(
    const JSBSim::FGGroundCallback& ground_callback,
    const JSBSim::FGLocation& location) {
  JSBSim::FGLocation contact;
  JSBSim::FGColumnVector3 normal;
  JSBSim::FGColumnVector3 v;
  JSBSim::FGColumnVector3 w;

  return ground_callback.GetAGLevel(0.0, location, contact, normal, v, w);
}

}  // namespace

TEST(JSBSimGroundCallback, ConstantModeUsesConfiguredElevation) {
  constexpr double kHomeAltitudeAslM = 100.0;
  constexpr double kAircraftAltitudeAslM = 150.0;
  projectairsim::JSBSimConstantGroundCallback ground_callback(
      kHomeAltitudeAslM);

  const auto location = MakeLocation(47.642101, -122.137001,
                                     kAircraftAltitudeAslM);

  const auto agl_ft = GetAltitudeAboveGroundFt(ground_callback, location);

  EXPECT_NEAR(agl_ft,
              (kAircraftAltitudeAslM - kHomeAltitudeAslM) *
                  projectairsim::MathUtils::meters_to_feets,
              1e-3);
}

TEST(JSBSimGroundCallback, TerrainModeSamplesTerrainCallback) {
  constexpr double kHomeAltitudeAslM = 100.0;
  constexpr double kTerrainAltitudeAslM = 120.0;
  constexpr double kAircraftAltitudeAslM = 170.0;
  int callback_calls = 0;
  projectairsim::GeoPoint home_geo_point(47.642101, -122.137001,
                                         kHomeAltitudeAslM);
  projectairsim::JSBSimTerrainGroundCallback ground_callback(
      [&](double, double) {
        ++callback_calls;
        return kTerrainAltitudeAslM;
      },
      home_geo_point);

  const auto location = MakeLocation(home_geo_point.latitude,
                                     home_geo_point.longitude,
                                     kAircraftAltitudeAslM);

  const auto agl_ft = GetAltitudeAboveGroundFt(ground_callback, location);

  EXPECT_EQ(callback_calls, 1);
  EXPECT_NEAR(agl_ft,
              (kAircraftAltitudeAslM - kTerrainAltitudeAslM) *
                  projectairsim::MathUtils::meters_to_feets,
              1e-3);
}
