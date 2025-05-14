"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
End-to-end tests for ProjectAirSim Services, request-response APIs used by the data collection module
"""
import random

import pytest
from projectairsim import ProjectAirSimClient, World
from projectairsim.types import WeatherParameter
from pynng import NNGException

from projectairsim.datacollection.collection import helper
from projectairsim.datacollection.collection.helper import set_time, set_weather
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.types import GeoLocation, WayPoint


@pytest.fixture()
def client(request) -> ProjectAirSimClient:
    client = ProjectAirSimClient()
    try:
        client.connect()
    except NNGException as err:
        err_msg = (
            f"ProjectAirSim client connection failed with reason:{str(err)}\n"
            f"Is the ProjectAirSim server running?"
        )
        raise Exception(err_msg)

    def disconnect():
        client.disconnect()

    request.addfinalizer(disconnect)
    return client


@pytest.fixture()
def world(client):
    world = World(
        client,
        "scene_basic_drone.jsonc",
        1,
        sim_config_path=r"./test_datacollection/configs/sim_config",
    )

    return world


def test_weather_api(data_generator: DataGenerator, world: World):

    geo_locations = data_generator.geo_locations
    # Test setting of weather for 3 random waypoints from each location
    for location_name in geo_locations.keys():
        location = geo_locations.get(location_name)
        for i in random.sample(range(0, len(location.trajectory)), 3):
            pose: WayPoint = location.trajectory[i]
            weather_value = WeatherParameter[pose.weather["type"]]
            success, world = set_weather(
                world, weather_value, pose.weather["intensity"]
            )
            assert success is True


def test_time_api(data_generator: DataGenerator, world: World):

    geo_locations = data_generator.geo_locations
    # Test setting of time for 3 random waypoints from each location
    for location_name in geo_locations.keys():
        location = geo_locations.get(location_name)
        for i in random.sample(range(0, len(location.trajectory)), 3):
            pose: WayPoint = location.trajectory[i]
            success, world = set_time(world, pose.time)
            assert success is True


def test_env_actor_spawning_correct_locations(
    data_generator: DataGenerator, world: World, client: ProjectAirSimClient
):

    geo_locations = data_generator.geo_locations
    # Test wether env actors are being properly setup
    for location_name in geo_locations.keys():
        location = geo_locations.get(location_name)
        world, env_actor = helper.setup_env_actor(client, world, location)

        if (
            location.name == "Blocks-random" or location.name == "Blocks-planned"
        ):  # random from config/ planned from API
            assert env_actor is not None


def test_env_actor_spawning_incorrect_locations(
    data_generator: DataGenerator, world: World, client: ProjectAirSimClient
):

    geo_locations = data_generator.geo_locations
    # Test wether env actors are being properly setup
    for location_name in geo_locations.keys():
        location = geo_locations.get(location_name)
        world, env_actor = helper.setup_env_actor(client, world, location)

        if not (
            location.name == "Blocks-random" or location.name == "Blocks-planned"
        ):  # random from config/ planned from API
            assert env_actor is None
