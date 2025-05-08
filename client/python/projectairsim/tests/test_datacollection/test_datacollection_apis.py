"""
Copyright (C) Microsoft Corporation. All rights reserved.

Tests for API's exposed by the DataGenerator module
"""
from datetime import datetime

from projectairsim.types import WeatherParameter
from projectairsim.datacollection.augmentation.position_aug import RandomHorizontalFlip
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.trajectory.cylindrical_trajectory import (
    CylindricalTrajectory,
)
from projectairsim.datacollection.trajectory.takeoff_landing_trajectory import (
    TakeoffLandingTrajectory,
)
from projectairsim.datacollection.types import (
    Annotation,
    Config,
    GeoCoordinates,
    GeoLocation,
    Modality,
)


def test_add_weather_variation_correct_input(data_generator: DataGenerator):

    assert type(data_generator.config.env_spec[Config.WEATHER_KEY]) == list
    assert len(data_generator.config.env_spec[Config.WEATHER_KEY]) == 3
    success = data_generator.add_weather_variation(
        WeatherParameter.MAPLE_LEAF, [0.1, 0.7, 0.2]
    )
    assert success
    assert len(data_generator.config.env_spec[Config.WEATHER_KEY]) == 4


def test_add_weather_variation_incorrect_input(data_generator: DataGenerator):

    assert type(data_generator.config.env_spec[Config.WEATHER_KEY]) == list
    assert len(data_generator.config.env_spec[Config.WEATHER_KEY]) == 4

    # Test with bad input
    success = data_generator.add_weather_variation(WeatherParameter.DUST, [0.1])
    assert not success
    assert len(data_generator.config.env_spec[Config.WEATHER_KEY]) == 4


def test_update_time_variation_correct_input(data_generator: DataGenerator):

    assert data_generator.config.env_spec[Config.TOD_KEY] == [
        "2022-06-20",
        "7:15 AM",
        "3:15 PM",
        "4:00:00",
    ]

    today_date = datetime.strptime("2023/02/27", "%Y/%m/%d")
    start_time = datetime.strptime("13:00:00", "%H:%M:%S")
    end_time = datetime.strptime("19:00:00", "%H:%M:%S")
    step = datetime.strptime("1:00:00", "%H:%M:%S")
    success = data_generator.update_time_variation(
        today_date, start_time, end_time, step
    )

    assert success
    assert data_generator.config.env_spec[Config.TOD_KEY] == [
        "2023-02-27",
        "01:00 PM",
        "07:00 PM",
        "01:00:00",
    ]

    # Test bad input
    today_date = "2323/02/27"
    success = data_generator.update_time_variation(
        today_date, start_time, end_time, step
    )
    assert not success


def test_update_time_variation_bad_input(data_generator: DataGenerator):

    today_date = "2323/02/27"  # incorrect format
    start_time = datetime.strptime("13:00:00", "%H:%M:%S")
    end_time = datetime.strptime("19:00:00", "%H:%M:%S")
    step = datetime.strptime("1:00:00", "%H:%M:%S")
    success = data_generator.update_time_variation(
        today_date, start_time, end_time, step
    )

    success = data_generator.update_time_variation(
        today_date, start_time, end_time, step
    )
    assert not success


def test_add_trajectory_preset_correct_input(data_generator: DataGenerator):

    takeoff_landing = TakeoffLandingTrajectory(
        trajectory_len=200, altitude_change=5, radius=10
    )
    cylindrical = CylindricalTrajectory(
        trajectory_len=50,
        altitude_change=20,
        outer_radius=20,
        inner_radius=15,
        angular_variation=7,
    )

    success = data_generator.add_trajectory_preset(
        name="test", trajectories=[takeoff_landing, cylindrical]
    )
    assert success


def test_add_trajectory_preset_incorrect_input(data_generator: DataGenerator):

    takeoff_landing = TakeoffLandingTrajectory(
        trajectory_len=200, altitude_change=5, radius=10
    )

    success = data_generator.add_trajectory_preset(
        name="TestTraj", trajectories=takeoff_landing
    )
    assert not success


def test_update_loaction_trajectory_preset_valid_inputs(data_generator: DataGenerator):

    location_name = "Blocks-planned"  # from config
    traj_1_name = "A2B-planned"  # from config
    traj_2_name = "A2B-random"  # from config

    assert data_generator.geo_locations[location_name].trajectory_preset == traj_1_name
    success = data_generator.update_location_trajectory_preset(
        location_name=location_name, preset_name=traj_2_name
    )
    assert success
    assert data_generator.geo_locations[location_name].trajectory_preset == traj_2_name

    # Test with location not in config
    success = data_generator.update_location_trajectory_preset(
        "testlocation", traj_1_name
    )
    assert not success

    # Test with preset not in config
    success = data_generator.update_location_trajectory_preset(
        location_name, "testtraj"
    )
    assert not success
    assert data_generator.geo_locations[location_name].trajectory_preset == traj_2_name


def test_update_loaction_trajectory_preset_invalid_location(
    data_generator: DataGenerator,
):

    traj_1_name = "A2B-planned"  # from config

    # Test with location not in config
    success = data_generator.update_location_trajectory_preset(
        "testlocation", traj_1_name
    )
    assert not success


def test_update_loaction_trajectory_preset_invalid_preset(
    data_generator: DataGenerator,
):

    location_name = "Blocks-planned"  # from config
    traj_2_name = "A2B-random"  # from config

    # Test with preset not in config
    success = data_generator.update_location_trajectory_preset(
        location_name, "testtraj"
    )
    assert not success
    assert data_generator.geo_locations[location_name].trajectory_preset == traj_2_name


def test_add_env_actor_to_location_valid_inputs(data_generator: DataGenerator):

    location_name = "Blocks-planned"  # from config
    assert data_generator.geo_locations[location_name].env_actor_name == ""

    success = data_generator.add_env_actor_to_location(
        location_name=location_name,
        actor_name="TestEnvActor",
        trajectory_preset_name="downward-facing-cam",  # from config
        duration=15.5,
        to_loop=False,
        time_offset=10,
    )
    assert success
    assert data_generator.geo_locations[location_name].env_actor_name == "TestEnvActor"


def test_add_env_actor_to_location_invalid_location(data_generator: DataGenerator):

    # Test with location not in config
    success = data_generator.add_env_actor_to_location(
        location_name="testlocation",
        actor_name="TestEnvActor",
        trajectory_preset_name="downward-facing-cam",  # from config
        duration=15.5,
    )
    assert not success


def test_add_env_actor_to_location_invalid_preset(data_generator: DataGenerator):

    location_name = "Blocks-planned"  # from config
    # Test with trajectory preset not in config
    success = data_generator.add_env_actor_to_location(
        location_name=location_name,
        actor_name="TestEnvActor",
        trajectory_preset_name="testtraj",
        duration=15.5,
    )
    assert not success


def test_add_geo_location(data_generator: DataGenerator):

    assert len(data_generator.geo_locations.keys()) == 4

    success = data_generator.add_geo_location(
        location_name="Test-Seattle",
        trajectory_name="downward-facing-cam",  # Defined in config/via API
        scene_name="Seattle",
        scene_config="Seattle",  # Defined in the scene-configs section of the config file
        coordinates=GeoCoordinates(33.047, -97.2919, 250),  # lat-lon-alt
        asset="BasicLandingPad",  # Defined in the asset section of the config file
        object_id="TestLandingPad",  # Unique ID
    )
    assert success
    assert len(data_generator.geo_locations.keys()) == 5

    assert type(data_generator.geo_locations["Test-Seattle"]) == GeoLocation
    location = data_generator.geo_locations["Test-Seattle"]
    assert location.name == "Test-Seattle"

    data_generator.generate_trajectory()
    assert len(data_generator.geo_locations["Test-Seattle"].trajectory) != 0


def test_update_data_spec(data_generator: DataGenerator):

    assert data_generator.config.data_spec["modalities"] == [Modality.RGB]
    assert data_generator.config.data_spec["annotations"] == [
        Annotation.TWO_D_BBOX,
        Annotation.THREE_D_BBOX,
        Annotation.SEGMENTATION,
    ]

    success = data_generator.update_data_spec(
        enabled_modalities=[], enabled_annotations=[Annotation.TWO_D_BBOX]
    )

    assert success
    assert data_generator.config.data_spec["modalities"] == []
    assert data_generator.config.data_spec["annotations"] == [Annotation.TWO_D_BBOX]


def test_update_augmentation_spec(data_generator: DataGenerator):
    data_generator.config.augmentation_spec[0]: RandomHorizontalFlip

    assert len(data_generator.config.augmentation_spec) == 9
    assert data_generator.config.augmentation_spec[0].p == 1

    horizontal_flip = RandomHorizontalFlip({"p": 0.5})
    data_generator.update_augmentation_spec(augmentations=[horizontal_flip])

    assert data_generator.config.augmentation_spec[0].p == 0.5
