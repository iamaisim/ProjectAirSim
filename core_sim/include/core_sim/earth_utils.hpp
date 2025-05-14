// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_EARTH_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_EARTH_UTILS_HPP_

#include <cmath>

#include "core_sim/error.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Geo Points

struct GeoPoint {
  double latitude = 0.0;
  double longitude = 0.0;
  float altitude = 0.0f;

  GeoPoint() {}

  GeoPoint(double latitude_val, double longitude_val, float altitude_val) {
    Set(latitude_val, longitude_val, altitude_val);
  }

  void Set(double latitude_val, double longitude_val, float altitude_val) {
    if (std::abs(latitude_val) > 90.0 || std::abs(longitude_val) > 180.0) {
      throw Error(
          "Latitude must be between [-90,90], and longitude must be between "
          "[-180,180].");
    }

    latitude = latitude_val;
    longitude = longitude_val;
    altitude = altitude_val;
  }
};

struct HomeGeoPoint {
  GeoPoint geo_point;
  double lat_rad;
  double lon_rad;
  double cos_lat;
  double sin_lat;

  HomeGeoPoint() {}

  explicit HomeGeoPoint(const GeoPoint& home_geo_point_val) {
    Initialize(home_geo_point_val);
  }

  void Initialize(const GeoPoint& home_geo_point_val) {
    geo_point = home_geo_point_val;
    lat_rad = TransformUtils::ToRadians(home_geo_point_val.latitude);
    lon_rad = TransformUtils::ToRadians(home_geo_point_val.longitude);
    cos_lat = std::cos(lat_rad);
    sin_lat = std::sin(lat_rad);
  }
};

// -----------------------------------------------------------------------------
// Earth Utility Functions

class EarthUtils {
 public:
  static constexpr float kGravity =
      9.80665f;  // Standard acceleration due to gravity on Earth (m/s^2)
  static constexpr float kSpeedOfLight =
      299792458.0f;  // Speed of light in a vacuum (m/s)
  static constexpr float kEarthRadius =
      6378137.0f;  // IAU 2009 equatorial radius of Earth (m)
  static constexpr float kSeaLevelPressure =
      101325.0f;  // Atmospheric pressure at standard sea level (Pascal)
  static constexpr float kSeaLevelTemperature =
      288.15f;  // Ambient temperature at standard sea level (Kelvin)
  // SL air density = (SL pressure 101325.0) / 287.053 / (SL temp 288.15)
  static constexpr float kSeaLevelAirDensity =
      1.224999f;  // Density of air at standard sea level (kg/m^3)
  static constexpr float kSeaLevelSpeedOfSound =
      340.29f;  // Speed of sound at standard sea level (m/s)
  // ref: https://www.ngdc.noaa.gov/geomag/GeomagneticPoles.shtml
  static constexpr double MagPoleLat = TransformUtils::ToRadians(80.31f);
  static constexpr double MagPoleLon = TransformUtils::ToRadians(-72.62f);
  // https://en.wikipedia.org/wiki/Dipole_model_of_the_Earth's_magnetic_field
  static constexpr double MeanMagField = 3.12E-5;  // Tesla
  static constexpr float kObliquity = TransformUtils::ToRadians(23.4397f);
  static constexpr double kPerihelion =
      TransformUtils::ToRadians(102.9372);  // perihelion of the Earth (radians)
  static constexpr double kDistanceFromSun =
      149597870700.0;  // One AU, roughly corresponding to average distance of
                       // Earth from the Sun (meters)

  // TODO Make a non-NED version to fix coordinate systems?
  static GeoPoint NedToGeodetic(const Vector3& pos_ned,
                                const HomeGeoPoint& home_geo_point) {
    double x_rad = pos_ned.x() / kEarthRadius;
    double y_rad = pos_ned.y() / kEarthRadius;
    double c = std::sqrt(x_rad * x_rad + y_rad * y_rad);
    double sin_c = std::sin(c), cos_c = std::cos(c);
    double lat_rad, lon_rad;

    if (!MathUtils::IsApproximatelyZero(c)) {  // avoids large changes?
      lat_rad = std::asin(cos_c * home_geo_point.sin_lat +
                          (x_rad * sin_c * home_geo_point.cos_lat) / c);

      lon_rad = (home_geo_point.lon_rad +
                 std::atan2(y_rad * sin_c,
                            c * home_geo_point.cos_lat * cos_c -
                                x_rad * home_geo_point.sin_lat * sin_c));

      return GeoPoint(TransformUtils::ToDegrees<double>(lat_rad),
                      TransformUtils::ToDegrees<double>(lon_rad),
                      home_geo_point.geo_point.altitude - pos_ned.z());
    } else {
      return GeoPoint(home_geo_point.geo_point.latitude,
                      home_geo_point.geo_point.longitude,
                      home_geo_point.geo_point.altitude - pos_ned.z());
    }
  }

  static float GetGeopotential(float altitude_km) {
    static constexpr float radius_km = kEarthRadius / 1000.0f;
    return radius_km * altitude_km / (radius_km + altitude_km);
  }

  static float GetStandardTemperature(float geopot_height) {
    // geopot_height = Earth_radius * altitude / (Earth_radius + altitude)
    // all in kilometers temperature is in Kelvin = 273.15 + celcius

    // Calculate standard atmospheric pressure
    // Below 51km: Practical Meteorology by Roland Stull, pg 12
    // Above 51km: http://www.braeunig.us/space/atmmodel.htm
    if (geopot_height <= 11) {  // troposphere
      return kSeaLevelTemperature - (6.5f * geopot_height);
    } else if (geopot_height <= 20) {  // Staroshere starts
      return 216.65f;
    } else if (geopot_height <= 32) {
      return 196.65f + geopot_height;
    } else if (geopot_height <= 47) {
      return 228.65f + 2.8f * (geopot_height - 32);
    } else if (geopot_height <= 51) {  // Mesosphere starts
      return 270.65f;
    } else if (geopot_height <= 71) {
      return 270.65f - 2.8f * (geopot_height - 51);
    } else if (geopot_height <= 84.85f) {
      return 214.65f - 2 * (geopot_height - 71);
    } else {
      return 3;
      // Thermospehere has high kinetic temperature (500c to 2000c) but
      // temperature as measured by thermometer would be very low because of
      // almost vacuum throw std::out_of_range("geopot_height must be less than
      // 85km. Space domain is not supported yet!");
    }
  }

  static float GetStandardPressure(float geopot_height,
                                   float std_temperature) {  // return Pa
    // Below 51km: Practical Meteorology by Roland Stull, pg 12
    // Above 51km: http://www.braeunig.us/space/atmmodel.htm
    // Validation data:
    // https://www.avs.org/AVS/files/c7/c7edaedb-95b2-438f-adfb-36de54f87b9e.pdf

    // TODO: handle -ve altitude better (shouldn't grow indefinitely!)

    if (geopot_height <= 11) {
      // at alt 0, return sea level pressure
      return kSeaLevelPressure *
             std::pow(kSeaLevelTemperature / std_temperature, -5.255877f);
    } else if (geopot_height <= 20) {
      return 22632.06f * std::exp(-0.1577f * (geopot_height - 11));
    } else if (geopot_height <= 32) {
      return 5474.889f * std::pow(216.65f / std_temperature, 34.16319f);
    } else if (geopot_height <= 47) {
      return 868.0187f * std::pow(228.65f / std_temperature, 12.2011f);
    } else if (geopot_height <= 51) {
      return 110.9063f * std::exp(-0.1262f * (geopot_height - 47));
    } else if (geopot_height <= 71) {
      return 66.93887f * std::pow(270.65f / std_temperature, -12.2011f);
    } else if (geopot_height <= 84.85f) {
      return 3.956420f * std::pow(214.65f / std_temperature, -17.0816f);
    } else {
      return 1E-3f;
      // throw std::out_of_range("altitude must be less than 86km. Space domain
      // is not supported yet!");
    }
  }

  static float GetStandardPressure(float altitude) {
    // Takes `altitude` in meters as arg and returns pressure in `Pa`
    float geopotential_height = GetGeopotential(altitude / 1000.0f);
    float temperature = GetStandardTemperature(geopotential_height);
    return GetStandardPressure(geopotential_height, temperature);
  }

  static float GetAirDensity(float std_pressure, float std_temperature) {
    // http://www.braeunig.us/space/atmmodel.htm
    return std_pressure / 287.053f / std_temperature;  // kg/m^3
  }

  static float GetGravity(float altitude) {
    // derivation: http://www.citycollegiate.com/gravitation_XId.htm
    if (altitude < 10000 && altitude > -10000) {
      // up to 10 km, difference is too small
      return kGravity;
    } else if (altitude < 100000 && altitude > -100000) {
      // use first exproximation using binomial expansion
      return kGravity * (1.0f - 2.0f * altitude / kEarthRadius);
    } else {
      float factor = 1.0f + altitude / kEarthRadius;
      return kGravity / factor / factor;
    }
  }

  static float GetPressureAltitude(float pressure) {
    // apply altimeter formula
    // https://en.wikipedia.org/wiki/Pressure_altitude
    float pressure_altitude =
        (1 - pow(pressure / kSeaLevelPressure, 0.190284f)) * 145366.45f *
        0.3048f;  // Feet to Meters
    return pressure_altitude;
  }

  static Vector3 GetMagneticField(const GeoPoint& geo_point) {
    double declination, inclination;
    return GetMagneticField(geo_point, declination, inclination);
  }

  static Vector3 GetMagneticField(const GeoPoint& geo_point,
                                  double& declination, double& inclination) {
    /*
    We calculate magnetic field using simple dipol model of Earth, i.e., assume
    earth as perfect dipole sphere and ignoring all but first order terms.
    This obviously is inaccurate because of huge amount of irregularities,
    magnetic pole that is constantly moving, shape of Earth, higher order terms,
    dipole that is not perfectly aligned etc. For simulation we are not looking
    for actual values of magnetic field but rather if field changes correctly as
    vehicle moves in any direction and if field component signs are correct. For
    this purpose, simple diapole model is good enough. Keep in mind that actual
    field values may differ by as much as 10X in either direction although for
    many tests differences seems to be within 3X or sometime even to first
    decimal digit. Again what matters is how field changes wrt to movement as
    opposed to actual field values. To get better field strength one should use
    latest World Magnetic Model like WMM2015 from NOAA. However these recent
    model is fairly complex and very expensive to calculate. Other
    possibilities:
        - WMM2010 mocel, expensive to compute:
    http://williams.best.vwh.net/magvar/magfield.c
        - Android's mag field calculation (still uses WMM2010 and fails at North
    Pole): https://goo.gl/1CZB9x

    Performance:
        This function takes about 1 microsecond on (Intel Xeon E3-1505M v5 CPU)
        Basic trignometry functions runs at 30ns.

    Accuracy:
        Two points separated by sqrt(2 km)
        Dipole Model:   2.50394e-05     3.40771e-06     3.6567e-05 (dec: 7.7500,
    inc: 55.3530) WMM2015
    Model:  1.8350e-05		5.201e-06		5.0158e-05
    (dec: 15.8248, inc: 69.1805) geo:            47.637  -122.147    622

        Dipole Model:   2.5047e-05      3.41024e-06     3.65953e-05
    (dec: 7.7536, inc: 55.36532) WMM2015
    Model:  1.8353e-05		5.203e-06		5.0191e-05
    (dec: 15.8278, inc: 69.1897) geo:            47.646  -122.134    -378
    */

    // ref: The Earth's Magnetism: An Introduction for Geologists, Roberto
    // Lanza, Antonio Meloni Sec 1.2.5, pg 27-30 https://goo.gl/bRm7wt some
    // theory at http://www.tulane.edu/~sanelson/eens634/Hmwk6MagneticField.pdf

    double lat =
        TransformUtils::ToRadians(geo_point.latitude);  // geographic colatitude
    double lon = TransformUtils::ToRadians(geo_point.longitude);
    double altitude = geo_point.altitude + kEarthRadius;

    // cache value
    double sin_MagPoleLat = sin(MagPoleLat);
    double cos_MagPoleLat = cos(MagPoleLat);
    double cos_lat = cos(lat);
    double sin_lat = sin(lat);

    // find magnetic colatitude
    double mag_clat = acos(cos_lat * cos_MagPoleLat +
                           sin_lat * sin_MagPoleLat * cos(lon - MagPoleLon));

    // calculation of magnetic longitude is not needed but just in case if
    // someone wants it double mag_lon = asin(
    //    (sin(lon - MagPoleLon) * sin(lat)) /
    //    sin(mag_clat));

    // field strength only depends on magnetic colatitude
    // https://en.wikipedia.org/wiki/Dipole_model_of_the_Earth's_magnetic_field
    double cos_mag_clat = cos(mag_clat);
    double field_mag = MeanMagField * pow(kEarthRadius / altitude, 3) *
                       sqrt(1 + 3 * cos_mag_clat * cos_mag_clat);

    // find inclination and declination
    // equation of declination in above referenced book is only partial
    // full equation is (4a) at
    // http://www.tulane.edu/~sanelson/eens634/Hmwk6MagneticField.pdf
    double lat_test = sin_MagPoleLat * sin_lat;
    double dec_factor = cos_MagPoleLat / sin(mag_clat);
    if (cos_mag_clat > lat_test)
      declination = asin(sin(lon - MagPoleLon) * dec_factor);
    else
      declination = asin(cos(lon - MagPoleLon) * dec_factor);
    inclination = atan(2.0 / tan(mag_clat));  // do not use atan2 here

    // transform magnetic field vector to geographical coordinates
    // ref: http://www.geo.mtu.edu/~jdiehl/magnotes.html
    double field_xy = field_mag * cos(inclination);
    return Vector3(static_cast<float>(field_xy * cos(declination)),
                   static_cast<float>(field_xy * sin(declination)),
                   static_cast<float>(field_mag * sin(inclination)));
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_EARTH_UTILS_HPP_
