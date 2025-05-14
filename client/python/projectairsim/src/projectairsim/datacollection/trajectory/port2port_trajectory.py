"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Script for generating a random/planned trajectory between two points of interest

This pattern allows the user to move the Non-Physics drone along random/planned trajectories
between the two given points in the scene.
The planned trajectories are generated through an A* planner. 
The random trajectories are generated through an RRT algorithm that only optimizes for distance to goal location

The `generate_trajectory` method returns a modified version of the input GeoLocation object
"""
import copy
import random
from typing import List

import projectairsim.datacollection.collection.helper as helper
import projectairsim.datacollection.utils as utils
from projectairsim.datacollection.trajectory.planner import planner
from projectairsim.datacollection.trajectory.trajectory_generator import (
    Trajectory,
)
from projectairsim.datacollection.types import (
    GeoLocation,
    OccupancyMap,
    WayPoint,
)
from projectairsim.utils import projectairsim_log


class Port2PortTrajectory(Trajectory):
    TRAJECTORY_TYPE_KEY = "trajectory-type"
    NUM_TRAJ_KEY = "num-trajectories"
    START_KEY = "start-geo"
    END_KEY = "end-geo"

    def __init__(
        self,
        config_dir: str,
        server_ip: str = "127.0.0.1",
        trajectory_len: int = None,
        trajectory_type: str = None,
        num_trajectories: int = None,
        start_geo: List[float] = None,
        end_geo: List[float] = None,
    ) -> None:
        super().__init__(trajectory_len)
        self.config_dir = config_dir
        self.server_ip = server_ip
        self.trajectory_type = trajectory_type
        self.num_trajectories = num_trajectories
        self.start_geo = start_geo
        self.end_geo = end_geo

    def initialize_from_dict(self, param_dict: dict) -> None:
        """Sets class var values from input dict"""
        super().initialize_from_dict(param_dict)
        self.trajectory_type = param_dict.get(Port2PortTrajectory.TRAJECTORY_TYPE_KEY)
        self.num_trajectories = param_dict.get(Port2PortTrajectory.NUM_TRAJ_KEY)
        self.start_geo = param_dict.get(Port2PortTrajectory.START_KEY)
        self.end_geo = param_dict.get(Port2PortTrajectory.END_KEY)

    def generate_trajectory(
        self, location: GeoLocation, agent_type: str = "simulation"
    ) -> GeoLocation:
        """Generate port-port flight paths for each pair of geo locations"""

        start = WayPoint(
            utils.convert_geo_to_UE(self.start_geo, location.geodedic_converter)
        )
        start.add_rpy([0, 0, 0])

        end = WayPoint(
            utils.convert_geo_to_UE(self.end_geo, location.geodedic_converter)
        )

        edge_len = utils.get_edge_len(start, end)
        resolution = 1  # m

        projectairsim_log().info(f"Generating Voxel Grid")
        voxel_grid = helper.generate_voxel_grid(
            start,
            edge_len,
            resolution,
            location.scene_config,
            self.server_ip,
            self.config_dir,
        )
        projectairsim_log().info(f"Voxel Grid Generated")

        occupancy_map = OccupancyMap(voxel_grid, edge_len, resolution, start)
        trajectory = []

        if self.trajectory_type == "random":
            projectairsim_log().info("Generating Random Paths")
            for i in range(self.num_trajectories):
                trajectory.extend(
                    self.generate_random_trajectory(start, end, occupancy_map)
                )
                projectairsim_log().info(f"Generated {i} Random Path/s")
        elif self.trajectory_type == "optimal":
            projectairsim_log().info(f"Generating Planned Path")
            trajectory.extend(planner(occupancy_map, start, end, self.trajectory_len))

        if agent_type == "simulation":
            location.trajectory.extend(trajectory)
            location.occupancy_map = occupancy_map
        elif agent_type == "env":
            location.env_actor_trajectory.extend(trajectory)

        return location

    def generate_random_trajectory(
        self,
        start: WayPoint,
        end: WayPoint,
        occupancy_map: OccupancyMap,
        min_distance=5,
        smoothness_factor=0.15,
    ):
        start_neu = copy.deepcopy(start)
        goal_neu = copy.deepcopy(end)

        start_neu.pose_z *= -1
        goal_neu.pose_z *= -1

        trajectory_neu = [start_neu]
        cur_pose = copy.deepcopy(start_neu)
        cur_distance = utils.dist_xyz(start_neu, goal_neu)
        step_len = (cur_distance) / self.trajectory_len

        while utils.dist_xyz(cur_pose, goal_neu) > min_distance:
            temp = copy.deepcopy(cur_pose)

            temp.pose_x += round(random.uniform(-step_len, step_len))
            temp.pose_y += round(random.uniform(-step_len, step_len))
            temp.pose_z += round(random.uniform(-step_len, step_len))

            if utils.dist_xyz(temp, goal_neu) < abs(
                cur_distance - smoothness_factor * step_len
            ):  # We want the new distance to be not just smaller than the prev but smaller by at least (smoothness_factor*step_len). This should avoid generating very jagged trajectories
                if occupancy_map.check_validity(temp):
                    cur_distance = utils.dist_xyz(temp, goal_neu)
                    cur_pose = copy.deepcopy(temp)
                    trajectory_neu.append(copy.deepcopy(cur_pose))

        # Convert trajectory_neu to NED
        trajectory_ned = []
        for point in trajectory_neu:
            point.pose_z *= -1
            trajectory_ned.append(point)

        return trajectory_ned
