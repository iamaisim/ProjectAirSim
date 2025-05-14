"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Script for generating a conical trajectory around a point of interest

This pattern allows the user to move the Non-Physics drone along the surface of a 
hypothetical cone in the world-space.
The Cone is centered around the `geo-location` the trajectory is assigned to. 
This pattern can be used to collect data of an object of interest from various top-down angles.
E.g. Get images of varying angles relative to a landing pad below the drone

The `generate_trajectory` method returns a modified version of the input GeoLocation object
"""
import math
from typing import List
import numpy as np

from projectairsim.datacollection.trajectory.trajectory_generator import (
    Trajectory,
)
from projectairsim.datacollection.types import GeoLocation, WayPoint
from projectairsim.datacollection.utils import norm_0_to_2pi


class ConicalTrajectory(Trajectory):
    ALTITUDE_CHANGE_KEY = "altitude-change"
    FOV_KEY = "FOV"

    def __init__(
        self,
        trajectory_len: int = None,
        altitude_change: float = None,
        field_of_view: int = None,
    ) -> None:
        super().__init__(trajectory_len)
        self.altitude_change = altitude_change
        self.fov = field_of_view

    def initialize_from_dict(self, param_dict: dict) -> None:
        """Sets class var values from input dict"""
        super().initialize_from_dict(param_dict)
        self.altitude_change = param_dict.get(ConicalTrajectory.ALTITUDE_CHANGE_KEY)
        self.fov = param_dict.get(ConicalTrajectory.FOV_KEY)

    def generate_trajectory(
        self, location: GeoLocation, agent_type: str = "simulation"
    ) -> GeoLocation:
        """
        Our goal is to determine the number of vertical points (height_variation), number of
        yaw values to sample (yaw_pts) and the number of points on each horizontal place (circumference_pts) -
        image cutting a cone horizontally at each height point. the resulting circle is on which the drone
        will move

        Let,
        n = number of height variations
        x = number of circumference points
        y = number of yaw's to sample

        Then, looking at the method `generate_conic_path()`:
        Total Trajectory length = (# geo-locations) * y * x * n
        Here, (# geo-locations) is constant (from config) so, y * x * n = constant
        Lets say we put an arbitrary relationship between k, n and x - (5n = x); (10y = x), then we can solve for y, n and x
        """

        xn_ratio = 5  # see comment block for context
        yn_ratio = 10

        circumference_pts = (self.trajectory_len * xn_ratio * yn_ratio) ** (1.0 / 3.0)
        height_variation = math.ceil(circumference_pts / xn_ratio)
        yaw_pts = math.ceil(circumference_pts / yn_ratio)
        circumference_pts = math.ceil(circumference_pts)

        points = self.generate_conic_path(
            location,
            circumference_pts,
            height_variation,
            yaw_pts,
        )

        if agent_type == "simulation":
            location.trajectory.extend(points)
        elif agent_type == "env":
            location.env_actor_trajectory.extend(points)

        return location

    def generate_conic_path(
        self,
        location: GeoLocation,
        circumference_pts: int,
        height_variation: int,
        yaw_pts: float,
    ) -> List[WayPoint]:

        min_yaw = 0 if yaw_pts <= 2 else -math.pi

        coordinates = location.scene_coordinates
        sampled_points = []  # list of points in global coordinate
        for rel_z in np.linspace(
            self.altitude_change / height_variation,
            self.altitude_change,
            height_variation,
        ):
            radius = rel_z * math.tan(self.fov * math.pi / 180 / 2)

            for theta in np.linspace(-math.pi, math.pi, circumference_pts):
                rel_x = radius * math.cos(theta)
                rel_y = radius * math.sin(theta)
                x = rel_x + coordinates[0]
                y = rel_y + coordinates[1]
                z = coordinates[2] - rel_z  # in NED

                for yaw in np.linspace(min_yaw, math.pi, yaw_pts):
                    yaw = norm_0_to_2pi(yaw)
                    point = WayPoint([x, y, z])
                    point.add_rpy(orientation=[0, 0, yaw])
                    sampled_points.append(point)

        return sampled_points
