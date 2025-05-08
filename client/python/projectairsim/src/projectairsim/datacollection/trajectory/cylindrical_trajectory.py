"""
Copyright (C) Microsoft Corporation. All rights reserved.

Script for generating a cylindrical trajectory around a point of interest. 

This pattern allows the user to move the Non-Physics drone along the surface of a 
hypothetical cylinder in the world-space. Port-2-port trajectories are generated 
from the points on the outer surface to corresponding points on the inner surface.
The cylinder is centered around the `geo-locations` defined in the config. 
This pattern can be used to collect data around an object of interest.
E.g. Get images from all angles of an AirTaxi in the scene

The `generate_trajectory` method returns a modified version of the input GeoLocation object

"""

import math
from copy import deepcopy
from typing import List
import numpy as np

import projectairsim.datacollection.utils as utils
from projectairsim.datacollection.trajectory.trajectory_generator import (
    Trajectory,
)
from projectairsim.datacollection.types import GeoLocation, WayPoint


class CylindricalTrajectory(Trajectory):
    ALTITUDE_CHANGE_KEY = "altitude-change"
    OUTER_RADIUS_KEY = "outer-radius"
    INNER_RADIUS_KEY = "inner-radius"
    ANGULAR_VAR_KEY = "angular-variation"

    def __init__(
        self,
        trajectory_len: int = None,
        altitude_change: float = None,
        outer_radius: float = None,
        inner_radius: float = None,
        angular_variation: int = None,
    ) -> None:
        super().__init__(trajectory_len)
        self.altitude_change = altitude_change
        self.outer_radius = outer_radius
        self.inner_radius = inner_radius
        self.angular_variation = angular_variation

    def initialize_from_dict(self, param_dict: dict) -> None:
        """Sets class var values from input dict"""
        super().initialize_from_dict(param_dict)
        self.altitude_change = param_dict.get(CylindricalTrajectory.ALTITUDE_CHANGE_KEY)
        self.outer_radius = param_dict.get(CylindricalTrajectory.OUTER_RADIUS_KEY)
        self.inner_radius = param_dict.get(CylindricalTrajectory.INNER_RADIUS_KEY)
        self.angular_variation = param_dict.get(CylindricalTrajectory.ANGULAR_VAR_KEY)

    def generate_trajectory(
        self, location: GeoLocation, agent_type: str = "simulation"
    ) -> GeoLocation:
        """
        Our goal is to determine the path spacing between points on the same horizontal plane
        and, spacing between points on the same vertical plane
        Let,
        n = number of vertical variations per point on the circular plane around the object
        x = number of points on the trajectory between outer and inner radius

        Then, looking at the function `generate_points()`:
        Trajectory length = (# geo-locations) * (# angular variations) * n * x
        Here, (# geo-locations) and (# angular variations) are constant (from config) so, n * x = constant
        Lets say we put an arbitrary relationship between n and x - 100*n = x, then we can solve for n and x

        Once we know the value of x, path_spacing = (Outer/Inner radius diff)/ x
        And, n = x/100
        """

        x_n_ratio = 100  # See comment block for context

        len_per_cylinder_point = self.trajectory_len / (self.angular_variation)

        port2port_len = math.sqrt(len_per_cylinder_point * x_n_ratio)

        port2port_dist = self.outer_radius - self.inner_radius

        path_spacing = port2port_dist / port2port_len
        altitude_variations = math.ceil(port2port_len / x_n_ratio)

        cylindrical_trajectory = self.generate_points(
            location,
            path_spacing,
            altitude_variations,
        )

        if agent_type == "simulation":
            location.trajectory.extend(cylindrical_trajectory)
        elif agent_type == "env":
            location.env_actor_trajectory.extend(cylindrical_trajectory)

        return location

    def generate_points(
        self,
        geo_location: GeoLocation,
        path_spacing: float,
        altitude_variations: int,
    ) -> List[WayPoint]:
        """Generate a list of waypoints based on input/computed params"""

        sample_points = []

        origin_port = WayPoint(geo_location.scene_coordinates)
        origin_port.add_rpy(orientation=[0, 0, 0])

        circle_points = []

        for theta in (
            math.pi * 2 * i / self.angular_variation
            for i in range(self.angular_variation)
        ):

            dest_port = deepcopy(origin_port)
            dest_port.pose_x += self.outer_radius * math.cos(theta)
            dest_port.pose_y += self.outer_radius * math.sin(theta)
            circle_points.append(dest_port)

        for destination_port in circle_points:
            destination_port: WayPoint
            for rel_z in np.linspace(
                -self.altitude_change, self.altitude_change, altitude_variations
            ):

                destination_port.pose_z -= rel_z  # point on cylinder outer radius

                trajectory = utils.generate_p2p_path(
                    destination_port, origin_port, path_spacing, self.inner_radius
                )
                sample_points.extend(trajectory)

        return sample_points
