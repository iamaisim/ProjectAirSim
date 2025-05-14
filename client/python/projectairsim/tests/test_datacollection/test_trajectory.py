"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Tests for the trajectory generated through the DataGenerator class
"""
import random
import pytest

from projectairsim.types import WeatherParameter
import projectairsim.datacollection.utils as utils
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.trajectory.trajectory_generator import (
    Trajectory,
)
from projectairsim.datacollection.types import GeoLocation, WayPoint


def test_config(data_generator: DataGenerator):
    assert type(data_generator.config.config_dict) == dict
    assert data_generator.config.collection_spec != {}
    assert data_generator.config.env_spec != {}


def test_trajectory_spec(data_generator: DataGenerator):
    trajectory_spec = data_generator.config.trajectory_spec

    assert type(trajectory_spec) == dict

    for trajectory_name in trajectory_spec.keys():
        location_trajectory_spec = trajectory_spec[trajectory_name]
        assert all(
            issubclass(type(trajectory), Trajectory)
            for trajectory in location_trajectory_spec
        )


def test_geo_locations(data_generator: DataGenerator):
    geo_locations = data_generator.geo_locations
    assert type(geo_locations) == dict
    assert (
        len(geo_locations.keys()) == 5
    )  # 4 from config. 1 from API (test_datacolletion_apis.py)

    for location_name in geo_locations.keys():
        location = geo_locations.get(location_name)
        assert type(location) == GeoLocation

        # Test lat-lon to UE coord conversion
        if location.name == "DFW-forward":
            assert location.scene_coordinates == pytest.approx(
                (-577, 5, -158), rel=1e-2
            )
        elif location.name == "DFW-downward":
            assert location.scene_coordinates == pytest.approx(
                (-481, 943, -83), rel=1e-2
            )


def test_trajectory_type(data_generator: DataGenerator):
    for location_name in data_generator.geo_locations.keys():
        location = data_generator.geo_locations.get(location_name)
        for i in random.sample(range(0, len(location.trajectory)), 3):
            pose: WayPoint = location.trajectory[i]
            assert type(pose) == WayPoint
            assert type(pose.weather) == dict
            assert list(pose.weather.keys()) == ["value", "type", "intensity"]
            assert type(pose.weather["type"]) == str
            assert (
                type(pose.weather["intensity"]) == float
                and (pose.weather["intensity"] >= 0)
                and (pose.weather["intensity"] <= 1)
            )
            assert pose.weather["type"] in WeatherParameter._member_names_


def test_planned_trajectory(data_generator: DataGenerator):
    for location_name in data_generator.geo_locations.keys():
        location = data_generator.geo_locations.get(location_name)
        if location.name == "Blocks-planned":
            path = location.trajectory
            optimal_distance = utils.dist_xyz(path[0], path[-1])
            planned_distance = 0

            for i in range(len(path) - 1):
                planned_distance += utils.dist_xyz(path[i], path[i + 1])

            assert optimal_distance < planned_distance


def test_trajectory_validity(data_generator: DataGenerator):
    for location_name in data_generator.geo_locations.keys():
        location = data_generator.geo_locations.get(location_name)
        if location.name in ["Blocks-random", "Blocks-planned"]:
            assert location.occupancy_map is not None

            for i in random.sample(range(0, len(location.trajectory)), 5):
                pose: WayPoint = location.trajectory[i]
                pose.pose_z *= -1  # Convert to NEU
                print(pose.pose_x, pose.pose_y, pose.pose_z)
                assert location.occupancy_map.check_validity(pose)


def test_env_actor_trajectory(data_generator: DataGenerator):
    for location_name in data_generator.geo_locations.keys():
        location = data_generator.geo_locations.get(location_name)
        if location.name == "Blocks-random":
            assert location.env_actor_trajectory != []
