"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Script for generating a takeoff-landing trajectory about a point of interest

This pattern allows the user to move the Non-Physics drone along vertical lines
originating from the point of interest.
This pattern can be used to collect data of an object of interest from various top-down angles.
E.g. Get images of varying angles relative to a landing pad below the drone

The `generate_trajectory` method returns a modified version of the input GeoLocation object
"""
import copy
import math
from typing import List
import numpy as np

from projectairsim.datacollection.trajectory.trajectory_generator import (
    Trajectory,
)
from projectairsim.datacollection.types import GeoLocation, WayPoint
from projectairsim.datacollection.utils import norm_0_to_2pi


class TakeoffLandingTrajectory(Trajectory):
    """Generate takeoff-landing flight paths for given location"""

    ALTITUDE_CHANGE_KEY = "altitude-change"
    RADIUS_KEY = "radius"

    def __init__(
        self,
        trajectory_len: int = None,
        altitude_change: float = None,
        radius: float = None,
    ) -> None:
        super().__init__(trajectory_len)
        self.altitude_change = altitude_change
        self.radius = radius

    def initialize_from_dict(self, param_dict: dict) -> None:
        """Sets class var values from input dict"""
        super().initialize_from_dict(param_dict)
        self.altitude_change = param_dict.get(
            TakeoffLandingTrajectory.ALTITUDE_CHANGE_KEY
        )
        self.radius = param_dict.get(TakeoffLandingTrajectory.RADIUS_KEY)

    def generate_trajectory(
        self, location: GeoLocation, agent_type: str = "simulation"
    ) -> GeoLocation:
        """
        Our goal is to determine the path spacing between points on the same vertical trajectory,
        the number of points in the x and y directions to form a grid over the landing pad (vertical trajectories will start from each grid point)
        and, the number of yaw variations

        Let,
        n = length of each vertical trajectory
        x = number of yaw variations
        k = number of grid points (kx=ky)

        Then, looking at the method `generate_takeoff_trajectory()`:
        Total Trajectory length = (# geo-locations) * k_x * k_y * x * n
        Here, (# geo-locations) is constant (from config) and kx=ky=k so, k**2 * x * n = constant
        Lets say we put an arbitrary relationship between k, n and x - (50k = n); (100x = n), then we can solve for k, n and x

        Once we know the value of n, path_spacing = (Alt Change)/ n
        """

        nk_ratio = 100  # See comment block for context
        nx_ratio = 100  # See comment block for context

        port2port_traj_len = (self.trajectory_len * nx_ratio * nk_ratio ** 2) ** (
            1.0 / 4.0
        )
        path_spacing = self.altitude_change / port2port_traj_len

        grid_pts = math.ceil(port2port_traj_len / nk_ratio)  # x and y
        yaw_pts = math.ceil(port2port_traj_len / nx_ratio)

        takeoff_path_coords = self.generate_takeoff_trajectory(
            location,
            path_spacing,
            grid_pts,
            yaw_pts,
        )

        if agent_type == "simulation":
            location.trajectory.extend(takeoff_path_coords)
        elif agent_type == "env":
            location.env_actor_trajectory.extend(takeoff_path_coords)

        return location

    def generate_takeoff_trajectory(
        self,
        geo_location: GeoLocation,
        path_spacing: float,
        grid_pts: int,
        yaw_pts: int,
    ) -> List[WayPoint]:

        min_yaw = 0 if yaw_pts <= 2 else -math.pi

        # define line
        kx = np.linspace(-self.radius, self.radius, grid_pts) if grid_pts > 2 else [0]
        ky = kx

        takeoff_trajectory = []

        curr_port = geo_location.scene_coordinates
        curr_grid_coords = []

        # Generate grid coords for current vertiport
        for del_x in kx:
            for del_y in ky:
                new_coord_row = [
                    (curr_port[0] + del_x),
                    (curr_port[1] + del_y),
                    curr_port[2],
                ]
                curr_grid_coords.append(new_coord_row)

        # Generate takeoff paths at each yaw variation for each grid coord
        for i in curr_grid_coords:
            # Set start/end coords from vertiport JSON UE coords
            start_pose = WayPoint(i)

            # add roll, pitch, yaw, gis-location, lat-lon coordinates, object ID
            start_pose.add_rpy(orientation=[0, 0, 0])

            end_pose = copy.deepcopy(start_pose)
            end_pose.pose_z -= self.altitude_change

            for yaw in np.linspace(min_yaw, math.pi, yaw_pts):

                yaw = norm_0_to_2pi(yaw)
                cur_start_pose = copy.deepcopy(start_pose)
                cur_end_pose = copy.deepcopy(end_pose)

                cur_start_pose.yaw = cur_end_pose.yaw = yaw
                path = self.generate_path(cur_start_pose, cur_end_pose, path_spacing)
                takeoff_trajectory.extend(path)

        return takeoff_trajectory

    def generate_path(
        self, start_coord: WayPoint, end_coord: WayPoint, spacing
    ) -> List[WayPoint]:
        """Generate a flight path from a start/end coord"""

        path_coords: List[WayPoint] = []
        # Add start_coord as first coord
        path_coords.append(copy.deepcopy(start_coord))
        # Add vertical points going up (more negative) until reaching ending z
        cur_coord = copy.deepcopy(start_coord)
        while (cur_coord.pose_z - end_coord.pose_z) >= spacing:
            cur_coord.pose_z = cur_coord.pose_z - spacing
            path_coords.append(copy.deepcopy((cur_coord)))

        # Add end_coord as final coord
        path_coords.append(copy.deepcopy(end_coord))

        return path_coords
