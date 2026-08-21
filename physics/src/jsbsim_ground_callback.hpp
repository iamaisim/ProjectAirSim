// Copyright (C) IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef PHYSICS_SRC_JSBSIM_GROUND_CALLBACK_HPP_
#define PHYSICS_SRC_JSBSIM_GROUND_CALLBACK_HPP_

#include <memory>

#include "core_sim/actor/robot.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/geodetic_converter.hpp"
#include "FGJSBBase.h"
#include "input_output/FGGroundCallback.h"

namespace microsoft {
namespace projectairsim {

class JSBSimConstantGroundCallback final
    : public JSBSim::FGDefaultGroundCallback {
 public:
  explicit JSBSimConstantGroundCallback(double terrain_elevation_asl_m);

  double GetAGLevel(double t, const JSBSim::FGLocation& location,
                    JSBSim::FGLocation& contact,
                    JSBSim::FGColumnVector3& normal,
                    JSBSim::FGColumnVector3& v,
                    JSBSim::FGColumnVector3& w) const override;

 private:
  double terrain_elevation_asl_m_;
};

class JSBSimTerrainGroundCallback final : public JSBSim::FGDefaultGroundCallback {
 public:
  JSBSimTerrainGroundCallback(
      const Robot::TerrainElevationCallback& terrain_elevation_callback,
      const GeoPoint& home_geo_point);

  double GetAGLevel(double t, const JSBSim::FGLocation& location,
                    JSBSim::FGLocation& contact,
                    JSBSim::FGColumnVector3& normal,
                    JSBSim::FGColumnVector3& v,
                    JSBSim::FGColumnVector3& w) const override;

 private:
  Robot::TerrainElevationCallback terrain_elevation_callback_;
  std::unique_ptr<GeodeticConverter> geodetic_converter_;
  double fallback_terrain_elevation_asl_m_;
  mutable double last_valid_terrain_elevation_asl_m_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_SRC_JSBSIM_GROUND_CALLBACK_HPP_
