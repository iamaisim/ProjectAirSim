// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_EARTH_CELESTIAL_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_EARTH_CELESTIAL_HPP_

#include <chrono>
#include <ctime>

#include "core_sim/earth_utils.hpp"
#include "core_sim/physics_common_utils.hpp"

namespace microsoft {
namespace projectairsim {

class EarthCelestial {
 public:
  struct CelestialGlobalCoord {
    double declination;
    double rightAscension;
    double distance = std::numeric_limits<double>::quiet_NaN();
    double parallacticAngle = std::numeric_limits<double>::quiet_NaN();
  };

  struct CelestialLocalCoord {
    double azimuth;
    double altitude;
    double distance = std::numeric_limits<double>::quiet_NaN();
    double parallacticAngle = std::numeric_limits<double>::quiet_NaN();
  };

  struct CelestialPhase {
    double fraction;
    double phase;
    double angle;
  };

 public:
  static CelestialLocalCoord getSunCoordinates(uint64_t date, double lat,
                                               double lng) {
    double lw = TransformUtils::ToRadians(-lng);
    double phi = TransformUtils::ToRadians(lat);
    double d = toDays(date);

    CelestialGlobalCoord c = getGlobalSunCoords(d);
    double H = siderealTime(d, lw) - c.rightAscension;

    CelestialLocalCoord coord;
    coord.azimuth =
        TransformUtils::ToDegrees(azimuth(H, phi, c.declination)) + 180.0;
    coord.altitude = TransformUtils::ToDegrees(altitude(H, phi, c.declination));

    return coord;
  }

  static CelestialLocalCoord getMoonCoordinates(uint64_t date, double lat,
                                                double lng) {
    double lw = TransformUtils::ToRadians(-lng);
    double phi = TransformUtils::ToRadians(lat);
    double d = toDays(date);

    CelestialGlobalCoord c = getGlobalMoonCoords(d);
    double H = siderealTime(d, lw) - c.rightAscension;

    // formula 14.1 of "Astronomical Algorithms" 2nd edition by Jean Meeus
    // (Willmann-Bell, Richmond) 1998.
    double pa =
        std::atan2(std::sin(H), std::tan(phi) * std::cos(c.declination) -
                                    std::sin(c.declination) * std::cos(H));

    double h = altitude(H, phi, c.declination);
    h = h + astroRefraction(h);  // altitude correction for refraction

    CelestialLocalCoord coord;
    coord.azimuth = TransformUtils::ToDegrees(azimuth(H, phi, c.declination));
    coord.altitude = TransformUtils::ToDegrees(h);
    coord.distance = c.distance;
    coord.parallacticAngle = TransformUtils::ToDegrees(pa);
    return coord;
  };

  // calculations for illumination parameters of the moon,
  // based on http://idlastro.gsfc.nasa.gov/ftp/pro/astro/mphase.pro formulas
  // and Chapter 48 of "Astronomical Algorithms" 2nd edition by Jean Meeus
  // (Willmann-Bell, Richmond) 1998.
  static CelestialPhase getMoonPhase(uint64_t date) {
    double d = toDays(date);
    CelestialGlobalCoord s = getGlobalSunCoords(d);
    CelestialGlobalCoord m = getGlobalMoonCoords(d);

    double sdist = EarthUtils::kDistanceFromSun /
                   1000;  // distance from Earth to Sun in km

    double phi = std::acos(std::sin(s.declination) * std::sin(m.declination) +
                           std::cos(s.declination) * std::cos(m.declination) *
                               std::cos(s.rightAscension - m.rightAscension));
    double inc =
        std::atan2(sdist * std::sin(phi), m.distance - sdist * std::cos(phi));
    double angle = std::atan2(
        std::cos(s.declination) * std::sin(s.rightAscension - m.rightAscension),
        std::sin(s.declination) * std::cos(m.declination) -
            std::cos(s.declination) * std::sin(m.declination) *
                std::cos(s.rightAscension - m.rightAscension));

    CelestialPhase moonPhase;
    moonPhase.fraction = (1 + cos(inc)) / 2;
    moonPhase.phase = 0.5 + 0.5 * inc * (angle < 0 ? -1 : 1) / M_PI;
    moonPhase.angle = angle;
    return moonPhase;
  };

 private:
  static double toDays(uint64_t date) {
    static constexpr double kJulianDaysOnY2000 = 2451545;
    static constexpr double kDaysToHours = 60 * 60 * 24;
    static constexpr double kJulianDaysOnEpoch = 2440588;

    double julian_days = date / kDaysToHours - 0.5 + kJulianDaysOnEpoch;
    ;
    return julian_days - kJulianDaysOnY2000;
  }

  static double rightAscension(double l, double b) {
    return std::atan2(std::sin(l) * std::cos(EarthUtils::kObliquity) -
                          std::tan(b) * std::sin(EarthUtils::kObliquity),
                      std::cos(l));
  }

  static double declination(double l, double b) {
    return std::asin(std::sin(b) * std::cos(EarthUtils::kObliquity) +
                     std::cos(b) * std::sin(EarthUtils::kObliquity) *
                         std::sin(l));
  }

  static double azimuth(double H, double phi, double declination) {
    return std::atan2(std::sin(H), std::cos(H) * std::sin(phi) -
                                       std::tan(declination) * std::cos(phi));
  }

  static double altitude(double H, double phi, double declination) {
    return std::asin(std::sin(phi) * std::sin(declination) +
                     std::cos(phi) * std::cos(declination) * std::cos(H));
  }

  static double siderealTime(double d, double lw) {
    return TransformUtils::ToRadians((280.16 + 360.9856235 * d)) - lw;
  }

  static double astroRefraction(double h) {
    if (h < 0)  // the following formula works for positive altitudes only.
      h = 0;    // if h = -0.08901179 a div/0 would occur.

    // formula 16.4 of "Astronomical Algorithms" 2nd edition by Jean Meeus
    // (Willmann-Bell, Richmond) 1998. 1.02 / tan(h + 10.26 / (h + 5.10)) h in
    // degrees, result in arc minutes -> converted to rad:
    return 0.0002967 / std::tan(h + 0.00312536 / (h + 0.08901179));
  }

  static double solarMeanAnomaly(double d) {
    return TransformUtils::ToRadians((357.5291 + 0.98560028 * d));
  }

  static double eclipticLongitude(double M) {
    double C = TransformUtils::ToRadians(
        (1.9148 * std::sin(M) + 0.02 * std::sin(2 * M) +
         0.0003 * std::sin(3 * M)));  // equation of center

    return M + C + EarthUtils::kPerihelion + M_PI;
  }

  static CelestialGlobalCoord getGlobalSunCoords(double d) {
    double M = solarMeanAnomaly(d);
    double L = eclipticLongitude(M);

    CelestialGlobalCoord sunCoords;
    sunCoords.declination = declination(L, 0);
    sunCoords.rightAscension = rightAscension(L, 0);

    return sunCoords;
  }

  // moon calculations, based on http://aa.quae.nl/en/reken/hemelpositie.html
  // formulas
  static CelestialGlobalCoord getGlobalMoonCoords(double d) {
    // geocentric ecliptic coordinates of the moon

    double L = TransformUtils::ToRadians(
        (218.316 + 13.176396 * d));  // ecliptic longitude
    double M =
        TransformUtils::ToRadians((134.963 + 13.064993 * d));  // mean anomaly
    double F =
        TransformUtils::ToRadians((93.272 + 13.229350 * d));  // mean distance

    double l = L + TransformUtils::ToRadians(6.289 * std::sin(M));  // longitude
    double b = TransformUtils::ToRadians(5.128 * std::sin(F));      // latitude
    double dt = 385001 - 20905 * std::cos(M);  // distance to the moon in km

    CelestialGlobalCoord moonCoords;
    moonCoords.rightAscension = rightAscension(l, b);
    moonCoords.declination = declination(l, b);
    moonCoords.distance = dt;

    return moonCoords;
  }
};

}  // namespace projectairsim
}  // namespace microsoft
#endif