// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_GEODETIC_CONVERTER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_GEODETIC_CONVERTER_HPP_

#include "math_utils.hpp"

namespace microsoft {
namespace projectairsim {

class GeodeticConverter {
 public:
  GeodeticConverter(double home_latitude = 0, double home_longitude = 0,
                    float home_altitude = 0);

  void setHome(double home_latitude, double home_longitude,
               float home_altitude);

  void getHome(double* latitude, double* longitude, float* altitude) const;

  void geodetic2Ecef(const double latitude, const double longitude,
                     const double altitude, double* x, double* y,
                     double* z) const;

  void ecef2Geodetic(const double x, const double y, const double z,
                     double* latitude, double* longitude,
                     float* altitude) const;

  void ecef2Ned(const double x, const double y, const double z, double* north,
                double* east, double* down) const;

  void ned2Ecef(const double north, const double east, const float down,
                double* x, double* y, double* z) const;

  void geodetic2Ned(const double latitude, const double longitude,
                    const float altitude, double* north, double* east,
                    double* down) const;

  void ned2Geodetic(const double north, const double east, const float down,
                    double* latitude, double* longitude, float* altitude) const;

  void geodetic2Enu(const double latitude, const double longitude,
                    const double altitude, double* east, double* north,
                    double* up) const;

  void enu2Geodetic(const double east, const double north, const float up,
                    double* latitude, double* longitude, float* altitude) const;

  Matrix3x3d getEcefToNeuRotationMatrix() const;

  static constexpr double kSemimajorAxis = 6378137;
  static constexpr double kSemiminorAxis = 6356752.3142;

 private:
  inline Matrix3x3d nRe(const double lat_radians,
                        const double lon_radians) const;

  double home_latitude_rad_, home_latitude_;
  double home_longitude_rad_, home_longitude_;
  float home_altitude_;

  double home_ecef_x_;
  double home_ecef_y_;
  double home_ecef_z_;

  Matrix3x3d ecef_to_ned_matrix_;
  Matrix3x3d ned_to_ecef_matrix_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_GEODETIC_CONVERTER_HPP_