"""
Copyright (C) Microsoft Corporation. All rights reserved.

This script defines the following classes: Trajectory, TrajectoryGenerator

The `Trajectory` class acts as a parent class that all individual types of trajectories inherit from
and defines the high level methods each of those classes hold

The `TrajectoryGenerator` class is the class that helps generate trajectories for a give
GeoLocation object. It holds methods to process the input config, generate the appropriate
list of WayPoint objects, assign them appropriate metadata (time, weather, index) and 
assign that list to the GeoLocation
"""
import datetime
from abc import ABC, abstractmethod
from copy import deepcopy
from typing import Dict, List

import numpy as np

from projectairsim.datacollection.types import Config, GeoLocation, WayPoint
from projectairsim.types import WeatherParameter
from projectairsim.utils import projectairsim_log


class Trajectory(ABC):
    TRAJECTORY_LEN_KEY = "trajectory-len"

    @abstractmethod
    def __init__(self, trajectory_len: int) -> None:
        self.trajectory_len = trajectory_len

    @abstractmethod
    def initialize_from_dict(self, param_dict: dict) -> None:
        """Takes in a dict that holds the class params"""
        self.trajectory_len = param_dict.get(Trajectory.TRAJECTORY_LEN_KEY)

    @abstractmethod
    def generate_trajectory(
        self, location: GeoLocation, agent_type: str = "simulation"
    ) -> GeoLocation:
        """Generate trajectory based on type and params"""

    # @abstractmethod
    # def validate(self) -> bool:
    #     """Validates input params"""


class TrajectoryGenerator:
    # Define trajectory type names
    CYLINDRICAL_KEY = "cylindrical"
    TAKEOFF_LANDING_KEY = "takeoff-landing"
    CONIC_KEY = "conic"
    PORT2PORT_KEY = "port2port"

    def __init__(
        self,
        config: Config,
        server_ip: str,
        enable_time_sweep=True,
        enable_weather_sweep=True,
        num_sims: int = 1,
        sim_id: int = 0,
    ) -> None:
        self.config: Config = config
        self.server_ip = server_ip
        self.enable_time_sweep = enable_time_sweep
        self.enable_weather_sweep = enable_weather_sweep
        self.num_sims = num_sims
        self.sim_id = sim_id

    def run(self, location: GeoLocation) -> GeoLocation:
        # Get desired waypoint list for location
        location = self.generate_location_path(location)

        # Add time, weather, index data to waypoint list
        location.trajectory = self.add_time_weather_data(location.trajectory)

        projectairsim_log().info(
            f"Total trajectory length for location {location.name}: {len(location.trajectory)}"
        )
        return location

    def add_time_weather_data(
        self, waypoint_list: List[WayPoint]
    ) -> List[WayPoint]:
        if self.enable_time_sweep:
            waypoint_list = self.overlay_tod_metadata(waypoint_list)

        if self.enable_weather_sweep:
            weather_list = self.config.env_spec.get("weather")
            waypoint_list = self.weather_sweep(waypoint_list, weather_list)
            # Sort the trajectory based on weather to minimize weather changes based API calls
            waypoint_list.sort(key=lambda x: x.weather["value"])

        return waypoint_list

    def generate_location_path(self, location: GeoLocation) -> GeoLocation:
        trajectory_preset = location.trajectory_preset

        # Validate if desired trajectory preset exists in config
        if trajectory_preset not in self.config.trajectory_spec.keys():
            projectairsim_log().warning(
                f"Specify valid trajectory preset for location - {location.name}. Moving On"
            )
            return location

        # Generate Sim Agent Trajectories
        projectairsim_log().info(
            f"Generating sim agent trajectory at {location.name}"
        )
        desired_trajectories: List[
            Trajectory
        ] = self.config.trajectory_spec.get(trajectory_preset)
        for trajectory_type in desired_trajectories:
            location = trajectory_type.generate_trajectory(location)

        # Return if no env actor in scene
        if not location.can_generate_env_actor_trajectory:
            return location

        # Validate if desired trajectory preset for env actor exists in config
        env_actor_trajectory_preset = (
            location.env_actor_trajectory_settings.get("name")
        )
        if (
            env_actor_trajectory_preset
            not in self.config.trajectory_spec.keys()
        ):
            projectairsim_log().warning(
                f"Specify valid trajectory preset for {location.env_actor_name} at {location.name}"
            )
            return location

        # Generate Env Actor Trajectories
        projectairsim_log().info(
            f"Generating env actor trajectory at {location.name}"
        )
        desired_trajectories_env: List[
            Trajectory
        ] = self.config.trajectory_spec.get(env_actor_trajectory_preset)
        for trajectory_type in desired_trajectories_env:
            location = trajectory_type.generate_trajectory(location, "env")

        return location

    # Add time proportionally to the poses (effectively keeping size of dataset same)
    def overlay_tod_metadata(
        self, waypoint_list: List[WayPoint]
    ) -> List[WayPoint]:
        # Get list of desired time's
        time_list: list = self.config.env_spec.get("time-of-day")
        tod_list = self.generate_time_list(
            time_list[1],
            time_list[2],
            time_list[3],
        )

        # Add date object to each time of day object
        date = datetime.datetime.strptime(time_list[0], "%Y-%m-%d")
        date_time_list = [
            datetime.datetime.combine(date, time_obj) for time_obj in tod_list
        ]

        tod_overlayed_poselist = []
        # Add time of day by dividing dataset into equal proportions
        time_split = np.array_split(waypoint_list, len(date_time_list))
        for i, time_split in enumerate(time_split):
            for pose in time_split:
                pose: WayPoint
                pose.time = date_time_list[i]
                tod_overlayed_poselist.append(pose)

        return tod_overlayed_poselist

    # generate list of tod values from the user given range
    def generate_time_list(self, start_time_str, end_time_str, step_size_str):
        try:
            # Try to convert the input strings to time values in seconds using the '%I:%M %p' format
            start_time = datetime.datetime.strptime(
                start_time_str, "%I:%M %p"
            ).time()
            end_time = datetime.datetime.strptime(
                end_time_str, "%I:%M %p"
            ).time()
            step_size = datetime.datetime.strptime(
                step_size_str, "%H:%M:%S"
            ).time()
        except ValueError:
            # If the '%I:%M %p' format fails, try the '%H:%M:%S' format
            start_time = datetime.datetime.strptime(
                start_time_str, "%H:%M:%S"
            ).time()
            end_time = datetime.datetime.strptime(
                end_time_str, "%H:%M:%S"
            ).time()
            step_size = datetime.datetime.strptime(
                step_size_str, "%H:%M:%S"
            ).time()
            # Note: the step_size format is the same for both the cases above

        # Calculate the start time and end time in seconds
        start_time_sec = (
            start_time.hour * 3600 + start_time.minute * 60 + start_time.second
        )
        end_time_sec = (
            end_time.hour * 3600 + end_time.minute * 60 + end_time.second
        )
        step_size_sec = (
            step_size.hour * 3600 + step_size.minute * 60 + step_size.second
        )

        # Generate the time list
        time_list = []
        for i in range(start_time_sec, end_time_sec, step_size_sec):
            # Convert the time value in seconds to a time object
            time_obj = datetime.time(
                hour=i // 3600, minute=(i % 3600) // 60, second=i % 60
            )
            time_list.append(time_obj)
        return time_list

    # Multiply the poses with weather list (effectively increasing size of dataset)
    def weather_sweep(
        self, waypoint_list: List[WayPoint], weather_list: list
    ) -> List[WayPoint]:
        weather_type_strings = []
        weather_type_values = []
        weather_intensity_values = []
        for i in range(len(weather_list)):
            # Generate the intensity list
            start_value = weather_list[i].get("intensity")[0]
            end_value = weather_list[i].get("intensity")[1]
            step_size = weather_list[i].get("intensity")[2]

            if end_value <= start_value:
                raise ValueError(
                    "weather intensity ranges' end_value should be greater than start_value"
                )
            if step_size > end_value - start_value:
                raise ValueError(
                    "weather intensity ranges' step_size should be less than end_value - start_value"
                )
            intensity_list = np.arange(
                start_value, end_value, step_size
            ).tolist()

            for j in range(len(intensity_list)):
                weather_type_str = weather_list[i].get("type")
                weather_type_strings.append(weather_type_str)
                try:
                    weather_type_val = getattr(
                        WeatherParameter, weather_type_str
                    ).value
                except KeyError:
                    weather_type_val = 0
                weather_type_values.append(weather_type_val)
                weather_intensity = intensity_list[j]
                weather_intensity_values.append(weather_intensity)

        weather_multiplied_poselist = []
        for i in range(len(weather_type_values)):
            for pose in waypoint_list:
                pose: WayPoint
                new_pose = deepcopy(pose)
                new_pose.weather = {
                    "value": weather_type_values[i],
                    "type": weather_type_strings[i],
                    "intensity": weather_intensity_values[i],
                }
                weather_multiplied_poselist.append(new_pose)

        return weather_multiplied_poselist

    def get_sim_id_start_index(
        self, location_dict: Dict[str, GeoLocation]
    ) -> int:
        traj_len_list = []

        # Emulate the trajectory split for each sim_id
        for location_name in location_dict.keys():
            location = location_dict[location_name]
            num_randomizations = 1  # Base params
            for randomized_entity in location.randomized_entities:
                if randomized_entity.type == location.asset:
                    # Count the number of unique poses.
                    num_randomizations += len(randomized_entity.poses)
                    break
            location_split = [
                len(list(split)) * num_randomizations
                for split in np.array_split(location.trajectory, self.num_sims)
            ]
            traj_len_list.append((location_split))

        start_index = 0

        # Calculate num_poses each preceding sim instance would have been assigned
        for location_len in traj_len_list:
            start_index += sum(location_len[: self.sim_id])

        return start_index

    def split_trajectory(
        self, location_dict: Dict[str, GeoLocation]
    ) -> Dict[str, GeoLocation]:
        # Split for parallelization
        for location_name in location_dict.keys():
            location = location_dict[location_name]
            location_trajectory_split = np.array_split(
                location.trajectory, self.num_sims
            )
            location_dict[location_name].trajectory = list(
                location_trajectory_split[self.sim_id]
            )

        return location_dict
