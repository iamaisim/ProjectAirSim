// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_ENVIRONMENT_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_ENVIRONMENT_HPP_

#include "core_sim/earth_utils.hpp"
#include "core_sim/math_utils.hpp"

namespace microsoft {
namespace projectairsim {

struct Environment {
  struct EnvInfo {
    GeoPoint geo_point;                 // Geographic position
    Vector3 gravity = Vector3::Zero();  // Acceleration due to gravity (m/s^2)
    float temperature = 0.0f;   // Air temperature (e.g., by altitude) (Kelvin)
    float air_pressure = 0.0f;  // Atmospheric pressure (Pascal)
    float air_density = 0.0f;   // Air density (kg/m^3)
  };

  inline static Vector3 wind_velocity = Vector3::Zero();
  Vector3 position_ned = Vector3::Zero();
  HomeGeoPoint home_geo_point;
  EnvInfo env_info;  // Current environmental condition that can vary (e.g., by
                     // current location, time of day, etc.)

  Environment() {}

  Environment(const Vector3& position_val,
              const HomeGeoPoint& home_geo_point_val) {
    home_geo_point = home_geo_point_val;
    SetPosition(position_val);
  }

  void SetPosition(const Vector3& position_ned_val) {
    position_ned = position_ned_val;

    env_info.geo_point =
        EarthUtils::NedToGeodetic(position_ned, home_geo_point);

    env_info.gravity =
        Vector3(0, 0, EarthUtils::GetGravity(env_info.geo_point.altitude));

    float geo_potential =
        EarthUtils::GetGeopotential(env_info.geo_point.altitude / 1000.0f);

    env_info.temperature = EarthUtils::GetStandardTemperature(geo_potential);

    env_info.air_pressure =
        EarthUtils::GetStandardPressure(geo_potential, env_info.temperature);

    env_info.air_density =
        EarthUtils::GetAirDensity(env_info.air_pressure, env_info.temperature);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_ENVIRONMENT_HPP_
