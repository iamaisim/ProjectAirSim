// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/geodetic_converter.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;

// TODO: at some point we should revisit the accuracy of the
// functions with larger error thresholds.
TEST(GeodeticConverter, geodetic2Ecef) {
  auto geoConverter = projectairsim::GeodeticConverter();
  auto error = projectairsim::MathUtils::eps(3);

  // test default home location
  {
    auto lat = 0.0;
    auto lon = 0.0;
    auto alt = 0.0;

    double x, y, z;
    geoConverter.geodetic2Ecef(lat, lon, alt, &x, &y, &z);

    ASSERT_NEAR(x, projectairsim::GeodeticConverter::kSemimajorAxis, error);
    ASSERT_NEAR(y, 0., error);
    ASSERT_NEAR(z, 0., error);
  }

  // random point
  {
    auto lat = 25.6811373357;
    auto lon = -100.3271484375;
    auto alt = 225.000;

    auto expectedX = -1031137.876;
    auto expectedY = -5658742.520;
    auto expectedZ = 2747366.287;

    double x, y, z;
    geoConverter.geodetic2Ecef(lat, lon, alt, &x, &y, &z);

    ASSERT_NEAR(x, expectedX, error);
    ASSERT_NEAR(y, expectedY, error);
    ASSERT_NEAR(z, expectedZ, error);
  }

  // changing home shouldn't impact anything
  {
    auto lat = 25.6811373357;
    auto lon = -100.3271484375;
    auto alt = 225.000;
    geoConverter.setHome(lat, lon, alt);

    auto expectedX = -1031137.876;
    auto expectedY = -5658742.520;
    auto expectedZ = 2747366.287;

    double x, y, z;
    geoConverter.geodetic2Ecef(lat, lon, alt, &x, &y, &z);

    ASSERT_NEAR(x, expectedX, error);
    ASSERT_NEAR(y, expectedY, error);
    ASSERT_NEAR(z, expectedZ, error);
  }

  // test axis
  {
    auto lat = 0.;
    auto lon = 90.;
    auto alt = 1.f;

    double x, y, z;
    geoConverter.geodetic2Ecef(lat, lon, alt, &x, &y, &z);

    ASSERT_NEAR(x, 0., error);
    ASSERT_NEAR(y, projectairsim::GeodeticConverter::kSemimajorAxis + 1, error);
    ASSERT_NEAR(z, 0., error);
  }
}

TEST(GeodeticConverter, ecef2Geodetic) {
  auto geoConverter = projectairsim::GeodeticConverter();
  auto error = projectairsim::MathUtils::eps(3);

  // test default home location
  {
    auto x = projectairsim::GeodeticConverter::kSemimajorAxis;
    auto y = 0.;
    auto z = 0.;

    double lat, lon;
    float alt;

    geoConverter.ecef2Geodetic(x, y, z, &lat, &lon, &alt);

    ASSERT_NEAR(lat, 0., error);
    ASSERT_NEAR(lon, 0., error);
    ASSERT_NEAR(alt, 0., error);
  }

  // random point
  {
    auto x = 871411.1278;
    auto y = -5863295.2438;
    auto z = 3181232.062;

    auto expectedLat = 28.3734;
    auto expectedLon = -81.5465;
    auto expectedAlt = 354056.0f;

    double lat, lon;
    float alt;

    geoConverter.ecef2Geodetic(x, y, z, &lat, &lon, &alt);

    ASSERT_NEAR(lat, expectedLat, error);
    ASSERT_NEAR(lon, expectedLon, error);
    ASSERT_NEAR(alt, expectedAlt, error);
  }

  // changing home shouldnt affect results
  {
    auto x = 871411.1278;
    auto y = -5863295.2438;
    auto z = 3181232.062;

    auto expectedLat = 28.3734;
    auto expectedLon = -81.5465;
    auto expectedAlt = 354056.0f;

    double lat, lon;
    float alt;

    geoConverter.setHome(1., 2., 3.f);
    geoConverter.ecef2Geodetic(x, y, z, &lat, &lon, &alt);

    ASSERT_NEAR(lat, expectedLat, error);
    ASSERT_NEAR(lon, expectedLon, error);
    ASSERT_NEAR(alt, expectedAlt, error);
  }

  // test axis
  {
    auto delta_alt = 12.3;
    auto x = 0.;
    auto y = 0.;
    auto z = delta_alt - projectairsim::GeodeticConverter::kSemiminorAxis;

    auto expectedLat = -90.;
    auto expectedLon = 0.;
    auto expectedAlt = -delta_alt;

    double lat, lon;
    float alt;

    geoConverter.ecef2Geodetic(x, y, z, &lat, &lon, &alt);

    ASSERT_NEAR(lat, expectedLat, error);
    ASSERT_NEAR(lon, expectedLon, error);
    ASSERT_NEAR(alt, expectedAlt, error);
  }
}

TEST(GeodeticConverter, ecef2Ned) {
  auto geoConverter = projectairsim::GeodeticConverter();

  // ecef and home, both at default home location LLA=0,0,0
  {
    double n, e, d;
    geoConverter.ecef2Ned(projectairsim::GeodeticConverter::kSemimajorAxis, 0.,
                          0., &n, &e, &d);

    ASSERT_EQ(n, 0.);
    ASSERT_EQ(e, 0.);
    ASSERT_EQ(d, 0.);
  }

  // test at ecef origin, home at default
  {
    double n, e, d;
    geoConverter.ecef2Ned(0., 0., 0., &n, &e, &d);

    ASSERT_EQ(n, 0.);
    ASSERT_EQ(e, 0.);
    ASSERT_EQ(d, projectairsim::GeodeticConverter::kSemimajorAxis);
  }

  // pure Z,Y => N,E
  {
    auto x = 0.0;
    auto y = 456.654;
    auto z = 234.0;

    double n, e, d;
    geoConverter.ecef2Ned(x, y, z, &n, &e, &d);

    ASSERT_EQ(n, z);
    ASSERT_EQ(e, y);
    ASSERT_EQ(d, projectairsim::GeodeticConverter::kSemimajorAxis);
  }

  // pure X => -D (+ home loc offset)
  {
    auto x = 234.0;
    auto y = 0.0;
    auto z = 0.0;

    double n, e, d;
    geoConverter.ecef2Ned(x, y, z, &n, &e, &d);

    ASSERT_EQ(n, z);
    ASSERT_EQ(e, y);
    ASSERT_EQ(d, projectairsim::GeodeticConverter::kSemimajorAxis - x);
  }

  // home at random location, ecef right above it
  {
    auto lat = 25.6811373357;
    auto lon = -100.3271484375;
    auto alt = 225.000;
    geoConverter.setHome(lat, lon, alt);

    auto delta_alt = 10.0;
    double x, y, z;
    geoConverter.geodetic2Ecef(lat, lon, alt + delta_alt, &x, &y, &z);

    double n, e, d;
    geoConverter.ecef2Ned(x, y, z, &n, &e, &d);

    auto error = projectairsim::MathUtils::eps(1);
    ASSERT_NEAR(n, 0., error);
    ASSERT_NEAR(e, 0., error);
    ASSERT_NEAR(d, -delta_alt, error);
  }
}

TEST(GeodeticConverter, ned2Ecef) {
  auto geoConverter = projectairsim::GeodeticConverter();

  // ned at default home LLA=0,0,0
  {
    double x, y, z;
    geoConverter.ned2Ecef(0., 0., 0., &x, &y, &z);

    ASSERT_EQ(x, projectairsim::GeodeticConverter::kSemimajorAxis);
    ASSERT_EQ(y, 0.);
    ASSERT_EQ(z, 0.);
  }

  // N,E => Z,Y
  {
    auto n = 345.664;
    auto e = 123.440;
    auto d = 0.0;

    double x, y, z;
    geoConverter.ned2Ecef(n, e, d, &x, &y, &z);

    ASSERT_EQ(x, projectairsim::GeodeticConverter::kSemimajorAxis);
    ASSERT_EQ(y, e);
    ASSERT_EQ(z, n);
  }

  // X => -D (+ home loc offset)
  {
    auto n = 0.0;
    auto e = 0.0;
    auto d = -8904.5;

    double x, y, z;
    geoConverter.ned2Ecef(n, e, d, &x, &y, &z);

    ASSERT_EQ(x, -d + projectairsim::GeodeticConverter::kSemimajorAxis);
    ASSERT_EQ(y, e);
    ASSERT_EQ(z, n);
  }

  // test random point
  // TODO: we should look into the accuracy issues here and lower the
  // error threshold These sample values came from:
  // https://www.mathworks.com/help/map/ref/ned2geodetic.html
  {
    auto home_lat = 44.532;
    auto home_lon = -72.782;
    auto home_alt = 1699.f;
    geoConverter.setHome(home_lat, home_lon, home_alt);

    auto n = 1334.3;
    auto e = -2544.4;
    auto d = 360.0;

    double x, y, z;
    geoConverter.ned2Ecef(n, e, d, &x, &y, &z);

    auto error = 50.;
    ASSERT_NEAR(x, 1.3457e+06, error);
    ASSERT_NEAR(y, -4.3509e+06, error);
    ASSERT_NEAR(z, 4.4523e+06, error);
  }
}

TEST(GeodeticConverter, geodetic2Ned) {
  auto geoConverter = projectairsim::GeodeticConverter();

  // test default home location
  {
    auto lat = 0.0;
    auto lon = 0.0;
    auto alt = 0.f;
    geoConverter.setHome(lat, lon, alt);

    double n, e, d;
    geoConverter.geodetic2Ned(lat, lon, alt, &n, &e, &d);

    ASSERT_EQ(n, 0.);
    ASSERT_EQ(e, 0.);
    ASSERT_EQ(d, 0.);
  }

  // test pure alt change
  {
    auto lat = 0.0;
    auto lon = 0.0;
    auto alt = 0.f;

    double n, e, d;
    geoConverter.geodetic2Ned(lat, lon, alt, &n, &e, &d);

    ASSERT_EQ(n, 0.);
    ASSERT_EQ(e, 0.);
    ASSERT_EQ(d, -alt);
  }

  // test random point
  // TODO: we should look into the accuracy issues here and lower the
  // error threshold These sample values came from:
  // https://www.mathworks.com/help/map/ref/ned2geodetic.html
  {
    auto home_lat = 44.532;
    auto home_lon = -72.782;
    auto home_alt = 1699;
    geoConverter.setHome(home_lat, home_lon, home_alt);

    auto lat = 44.544;
    auto lon = -72.814;
    auto alt = 1340;

    double n, e, d;
    geoConverter.geodetic2Ned(lat, lon, alt, &n, &e, &d);

    auto error = 5.0;
    ASSERT_NEAR(n, 1334.3, error);
    ASSERT_NEAR(e, -2543.6, error);
    ASSERT_NEAR(d, 359.65, error);
  }
}

TEST(GeodeticConverter, ned2Geodetic) {
  auto geoConverter = projectairsim::GeodeticConverter();

  // test default home location
  {
    auto n = 0.0;
    auto e = 0.0;
    auto d = 0.0;

    double lat, lon;
    float alt;
    geoConverter.ned2Geodetic(n, e, d, &lat, &lon, &alt);

    ASSERT_EQ(lat, 0.);
    ASSERT_EQ(lon, 0.);
    ASSERT_EQ(alt, 0.f);
  }

  // test pure D change
  {
    auto n = 0.0;
    auto e = 0.0;
    auto d = 3245.0;

    double lat, lon;
    float alt;
    geoConverter.ned2Geodetic(n, e, d, &lat, &lon, &alt);

    ASSERT_EQ(lat, 0.);
    ASSERT_EQ(lon, 0.);
    ASSERT_EQ(alt, -d);
  }

  // test random point
  {
    auto home_lat = 44.532;
    auto home_lon = -72.782;
    auto home_alt = 1699;
    geoConverter.setHome(home_lat, home_lon, home_alt);

    auto n = 1334.3;
    auto e = -2543.6;
    auto d = 359.65;

    double lat, lon;
    float alt;
    geoConverter.ned2Geodetic(n, e, d, &lat, &lon, &alt);

    auto error = 1;
    ASSERT_NEAR(lat, 44.544, error);
    ASSERT_NEAR(lon, -72.814, error);
    ASSERT_NEAR(alt, 1340, error);
  }
}

TEST(GeodeticConverter, geodetic2Enu) {
  auto geoConverter = projectairsim::GeodeticConverter();

  // test default home location
  {
    auto lat = 0.0;
    auto lon = 0.0;
    auto alt = 0.f;
    geoConverter.setHome(lat, lon, alt);

    double e, n, u;
    geoConverter.geodetic2Enu(lat, lon, alt, &e, &n, &u);

    ASSERT_EQ(e, 0.);
    ASSERT_EQ(n, 0.);
    ASSERT_EQ(u, 0.);
  }

  // test pure alt change
  {
    auto lat = 0.0;
    auto lon = 0.0;
    auto alt = 0.f;

    double e, n, u;
    geoConverter.geodetic2Enu(lat, lon, alt, &e, &n, &u);

    ASSERT_EQ(e, 0.);
    ASSERT_EQ(n, 0.);
    ASSERT_EQ(u, alt);
  }

  // test random home
  {
    auto home_lat = 46.017;
    auto home_lon = 7.750;
    auto home_alt = 1673;
    geoConverter.setHome(home_lat, home_lon, home_alt);

    auto lat = 45.976;
    auto lon = 7.658;
    auto alt = 4531;

    double e, n, u;
    geoConverter.geodetic2Enu(lat, lon, alt, &e, &n, &u);

    auto error = 16.;
    ASSERT_NEAR(e, -7134.8, error);
    ASSERT_NEAR(n, -4556.3, error);
    ASSERT_NEAR(u, 2852.4, error);
  }
}

TEST(GeodeticConverter, enu2Geodetic) {
  auto geoConverter = projectairsim::GeodeticConverter();
  auto error = projectairsim::MathUtils::eps(4);

  // test default home location
  {
    auto e = 0.0;
    auto n = 0.0;
    auto u = 0.0;

    double lat, lon;
    float alt;
    geoConverter.enu2Geodetic(e, n, u, &lat, &lon, &alt);

    ASSERT_EQ(lat, 0.);
    ASSERT_EQ(lon, 0.);
    ASSERT_NEAR(alt, 0., error);
  }

  // test pure U change
  {
    auto e = 0.0;
    auto n = 0.0;
    auto u = 554.2;

    double lat, lon;
    float alt;
    geoConverter.enu2Geodetic(e, n, u, &lat, &lon, &alt);

    ASSERT_EQ(lat, 0.);
    ASSERT_EQ(lon, 0.);
    ASSERT_NEAR(alt, u, error);
  }

  // test random home
  {
    auto home_lat = 46.017;
    auto home_lon = 7.750;
    auto home_alt = 1673;
    geoConverter.setHome(home_lat, home_lon, home_alt);

    auto e = -7134.8;
    auto n = -4556.3;
    auto u = 2852.4;

    double lat, lon;
    float alt;
    geoConverter.enu2Geodetic(e, n, u, &lat, &lon, &alt);

    auto error = projectairsim::MathUtils::eps(2);
    ASSERT_NEAR(lat, 45.976, error);
    ASSERT_NEAR(lon, 7.658, error);
    ASSERT_NEAR(alt, 4531., error);
  }
}