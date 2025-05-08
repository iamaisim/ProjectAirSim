import math

import numpy as np

K_FIRST_ECCENTRICITY_SQUARED = 6.69437999014 * 0.001
K_SEMI_MAJOR_AXIS = 6378137


class GeodeticConverter:
    def __init__(self, home_latitude, home_longitude, home_altitude):
        assert abs(home_latitude) <= 90
        assert abs(home_longitude) <= 180

        self.home_ecef = self.geodetic_to_ecef(
            np.array([home_latitude, home_longitude, home_altitude])
        )

        home_latitude_rad = math.radians(home_latitude)
        home_longitude_rad = math.radians(home_longitude)

        phiP = math.atan2(
            self.home_ecef[2],
            math.sqrt(pow(self.home_ecef[0], 2) + pow(self.home_ecef[1], 2)),
        )

        self._ecef_to_ned_matrix = self._nRe(phiP, home_longitude_rad)
        self._ned_to_ecef_matrix = self._nRe(
            home_latitude_rad, home_longitude_rad
        ).T

    def _nRe(self, lat_radians, lon_radians):
        sLat = math.sin(lat_radians)
        sLon = math.sin(lon_radians)
        cLat = math.cos(lat_radians)
        cLon = math.cos(lon_radians)

        conv_matrix = np.array(
            [
                [-sLat * cLon, -sLat * sLon, cLat],
                [-sLon, cLon, 0.0],
                [cLat * cLon, cLat * sLon, sLat],
            ]
        )
        return conv_matrix

    def geodetic_to_ecef(self, lat_long_alt):
        """Converts geodetic coordinates to earth-center-earth-fixed."""
        lat_rad = math.radians(lat_long_alt[0])
        long_rad = math.radians(lat_long_alt[1])
        alt = lat_long_alt[2]

        xi = math.sqrt(
            1 - K_FIRST_ECCENTRICITY_SQUARED * math.sin(lat_rad) ** 2
        )

        x = (
            (K_SEMI_MAJOR_AXIS / xi + alt)
            * math.cos(lat_rad)
            * math.cos(long_rad)
        )
        y = (
            (K_SEMI_MAJOR_AXIS / xi + alt)
            * math.cos(lat_rad)
            * math.sin(long_rad)
        )
        z = (
            K_SEMI_MAJOR_AXIS / xi * (1 - K_FIRST_ECCENTRICITY_SQUARED) + alt
        ) * math.sin(lat_rad)
        return np.array([x, y, z])

    def ecef_to_ned(self, ecef):
        """Converts earth-center-earth-fixed coordinates to North, East, Down."""
        vec = ecef - self.home_ecef
        ned = self._ecef_to_ned_matrix @ vec
        ned[2] *= -1
        return ned

    def geodetic_to_ned(self, lat_long_alt):
        """Converts geodetic coordinates to North, East, Down."""
        ecef = self.geodetic_to_ecef(lat_long_alt)
        return self.ecef_to_ned(ecef)
