// Copyright (C) IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "jsbsim_ground_callback.hpp"

#include <cmath>

#include "core_sim/math_utils.hpp"
#include "math/FGLocation.h"

namespace microsoft {
namespace projectairsim {

namespace {
constexpr double kWgs84SemiMajorFt = 20925646.32546;
constexpr double kWgs84SemiMinorFt = 20855486.5951;
}

JSBSimConstantGroundCallback::JSBSimConstantGroundCallback(
    double terrain_elevation_asl_m)
    : JSBSim::FGDefaultGroundCallback(kWgs84SemiMajorFt, kWgs84SemiMinorFt),
      terrain_elevation_asl_m_(terrain_elevation_asl_m) {
  SetTerrainElevation(terrain_elevation_asl_m_ * MathUtils::meters_to_feets);
}

double JSBSimConstantGroundCallback::GetAGLevel(
    double t, const JSBSim::FGLocation& location, JSBSim::FGLocation& contact,
    JSBSim::FGColumnVector3& normal, JSBSim::FGColumnVector3& v,
    JSBSim::FGColumnVector3& w) const {
  auto* mutable_this = const_cast<JSBSimConstantGroundCallback*>(this);
  mutable_this->SetTerrainElevation(terrain_elevation_asl_m_ *
                                    MathUtils::meters_to_feets);

  return JSBSim::FGDefaultGroundCallback::GetAGLevel(t, location, contact,
                                                     normal, v, w);
}

JSBSimTerrainGroundCallback::JSBSimTerrainGroundCallback(
    const Robot::TerrainElevationCallback& terrain_elevation_callback,
    const GeoPoint& home_geo_point)
    : JSBSim::FGDefaultGroundCallback(kWgs84SemiMajorFt, kWgs84SemiMinorFt),
      terrain_elevation_callback_(terrain_elevation_callback),
      geodetic_converter_(std::make_unique<GeodeticConverter>(
          home_geo_point.latitude, home_geo_point.longitude,
          home_geo_point.altitude)),
      fallback_terrain_elevation_asl_m_(home_geo_point.altitude),
      last_valid_terrain_elevation_asl_m_(home_geo_point.altitude) {}

double JSBSimTerrainGroundCallback::GetAGLevel(
    double t, const JSBSim::FGLocation& location, JSBSim::FGLocation& contact,
    JSBSim::FGColumnVector3& normal, JSBSim::FGColumnVector3& v,
    JSBSim::FGColumnVector3& w) const {
  double terrain_elevation_asl_m = last_valid_terrain_elevation_asl_m_;

  if (terrain_elevation_callback_ != nullptr && geodetic_converter_ != nullptr) {
    JSBSim::FGLocation loc_with_ellipse(location);
    loc_with_ellipse.SetEllipse(kWgs84SemiMajorFt, kWgs84SemiMinorFt);

    const auto latitude_deg = loc_with_ellipse.GetGeodLatitudeDeg();
    const auto longitude_deg = loc_with_ellipse.GetLongitudeDeg();
    const auto altitude_asl_m =
        loc_with_ellipse.GetGeodAltitude() * MathUtils::feets_to_meters;
    double n = 0.0;
    double e = 0.0;
    double d = 0.0;
    geodetic_converter_->geodetic2Ned(latitude_deg, longitude_deg,
                                      altitude_asl_m, &n, &e, &d);

    const auto terrain_cb_val = terrain_elevation_callback_(n, e);
    if (std::isfinite(terrain_cb_val)) {
      terrain_elevation_asl_m = terrain_cb_val;
      last_valid_terrain_elevation_asl_m_ = terrain_cb_val;
    }
  }

  if (!std::isfinite(terrain_elevation_asl_m)) {
    terrain_elevation_asl_m = fallback_terrain_elevation_asl_m_;
  }

  auto* mutable_this = const_cast<JSBSimTerrainGroundCallback*>(this);
  mutable_this->SetTerrainElevation(terrain_elevation_asl_m *
                                    MathUtils::meters_to_feets);

  return JSBSim::FGDefaultGroundCallback::GetAGLevel(t, location, contact,
                                                     normal, v, w);
}

}  // namespace projectairsim
}  // namespace microsoft
