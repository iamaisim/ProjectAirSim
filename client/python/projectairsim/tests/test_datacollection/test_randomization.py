"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Testing suite for Randomization module.
"""

import pathlib

import pytest

import projectairsim.datacollection.randomization.randomization as randomization
import projectairsim.datacollection.utils as utils
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.types import WayPoint
from projectairsim.geodetic_converter import GeodeticConverter

CONFIG_FILE_PATH = pathlib.Path(
    "./test_datacollection/configs/datacollector_config.jsonc"
)


@pytest.fixture
def full_config():
    config = utils.read_cjson(CONFIG_FILE_PATH)
    config = config[randomization.Config.ENV_SPEC_KEY][
        randomization.Config.ASSETS_KEY
    ]
    return [asset_config for _, asset_config in config.items()]


# --- Rotator Fixtures ---
@pytest.fixture
def landingpad_with_rotator():
    landing_pad_randomizations = [
        randomization.Rotator(
            {
                "amount": 3,
                "axis": "roll",
                "upper_bound": 75.0,
                "lower_bound": -75.0,
            }
        ),
    ]
    return randomization.Entity(
        "LandingPad", [1.0, 4.5, 1.3], landing_pad_randomizations
    )


@pytest.fixture
def rolled_poses():
    poses = []
    xyz = [1.0, 4.5, 1.3]
    rpy = [-75, 0, 0]
    for i in range(3):
        poses.append(WayPoint([*xyz, *rpy], altered="rotate"))
        rpy[0] += 75
    return poses


# End Rotator Fixtures


# --- Scaler Fixtures ---
@pytest.fixture
def landingpad_with_scaler():
    landing_pad_randomizations = [
        randomization.Scaler(
            {
                "amount": 3,
                "upper_bound": 100,
                "lower_bound": 1,
            }
        ),
    ]
    return randomization.Entity(
        "LandingPad", [1.0, 4.5, 1.3], landing_pad_randomizations
    )


@pytest.fixture
def scaled_poses():
    poses = []
    xyz = [1.0, 4.5, 1.3]
    rpy = [0, 0, 0]
    scale_xyz = [[1, 1, 1], [50, 50, 50], [100, 100, 100]]
    for i in range(3):
        poses.append(WayPoint([*xyz, *rpy], scale_xyz[i], altered="scale"))
    return poses


# End Scaler Fixtures


# --- Translater Fixtures ---
@pytest.fixture
def landingpad_with_translater():
    landing_pad_randomizations = [
        randomization.Translater(
            {"amount": 3, "upper_bound": 5, "lower_bound": 0, "axis": "z"}
        ),
    ]
    return randomization.Entity(
        "LandingPad", [1.0, 4.5, 0], landing_pad_randomizations
    )


@pytest.fixture
def translated_poses():
    poses = []
    xyz = [1.0, 4.5, 0]
    rpy = [0, 0, 0]
    for i in range(3):
        poses.append(WayPoint([*xyz, *rpy], altered="translate"))
        xyz[2] += 2.5
    return poses


# End Translater Fixtures


# --- Flipper Fixtures ---
@pytest.fixture
def landingpad_with_flipper():
    landing_pad_randomizations = [
        randomization.Flipper({"axis": "roll", "initial_rpy": [-20, 0, 0]}),
    ]
    return randomization.Entity(
        "LandingPad", [1.0, 4.5, 0], landing_pad_randomizations
    )


@pytest.fixture
def flipped_poses():
    poses = []
    xyz = [1.0, 4.5, 0]
    rpy = [-20, 0, 0]
    for i in range(3):
        poses.append(WayPoint([*xyz, *rpy], altered="flip"))
        rpy[0] *= -1
    return poses


# End Flipper Fixtures


@pytest.fixture
def randomization_config():
    return {
        "texture": ["default"],
        "scale": {"amount": 3, "upper_bound": 100, "lower_bound": 1},
        "rotation": {
            "amount": 3,
            "upper_bound": 90.0,
            "lower_bound": -90.0,
            "axis": "roll",
        },
        "translation": {
            "amount": 3,
            "upper_bound": 5.0,
            "lower_bound": 0,
            "axis": "z",
        },
        "flip": {"axis": "roll", "initial_rpy": [0, 0, 0]},
    }


@pytest.fixture
def entities_without_randomizations_config():
    return [
        {
            "type": "LandingPad",
            "origin_xyz": [
                1.0,
                4.5,
                1.3,
            ],
            "randomizations": {},
        },
        {
            "type": "AirTaxi",
            "origin_xyz": [
                5.0,
                0.5,
                -25.0,
            ],
            "randomizations": {},
        },
    ]


@pytest.fixture
def entities_with_randomizations_config(randomization_config):
    return [
        {
            "type": "LandingPad",
            "origin_xyz": [
                1.0,
                4.5,
                1.3,
            ],
            "randomizations": randomization_config,
        },
        {
            "type": "AirTaxi",
            "origin_xyz": [
                5.0,
                0.5,
                -25.0,
            ],
            "randomizations": randomization_config,
        },
    ]


@pytest.fixture
def entities_without_randomizations():
    return [
        randomization.Entity(
            "LandingPad",
            [
                1.0,
                4.5,
                1.3,
            ],
        ),
        randomization.Entity(
            "AirTaxi",
            [
                5.0,
                0.5,
                -25.0,
            ],
        ),
    ]


@pytest.fixture
def entities_with_randomizations(randomization_config):
    SCALING_KEY = randomization.Config.SCALING_KEY
    ROTATION_KEY = randomization.Config.ROTATION_KEY
    TRANSLATION_KEY = randomization.Config.TRANSLATON_KEY
    FLIP_KEY = randomization.Config.FLIP_KEY
    randomizations = [
        randomization.Scaler(randomization_config[SCALING_KEY]),
        randomization.Rotator(randomization_config[ROTATION_KEY]),
        randomization.Translater(randomization_config[TRANSLATION_KEY]),
        randomization.Flipper(randomization_config[FLIP_KEY]),
    ]
    return [
        randomization.Entity(
            "LandingPad",
            [
                1.0,
                4.5,
                1.3,
            ],
            randomizations,
        ),
        randomization.Entity(
            "AirTaxi",
            [
                5.0,
                0.5,
                -25.0,
            ],
            randomizations,
        ),
    ]


def test_config_initializes_config(full_config):
    # * Arrange
    config = randomization.Config(CONFIG_FILE_PATH)

    # * Act / Assert
    assert config.config == full_config


def test_config_returns_static_entity_configs(
    data_generator: DataGenerator, full_config
):
    # * Act / Assert
    assert (
        data_generator.randomization_config.get_static_entity_configs()
        == full_config
    )


def test_entity_builder_builds_entities_without_randomizations(
    entities_without_randomizations_config,
    entities_without_randomizations,
):
    # * Arrange
    builder = randomization.EntityBuilder()

    # * Act
    test_entities = builder.build(entities_without_randomizations_config)

    # * Assert
    for test, actual in zip(test_entities, entities_without_randomizations):
        assert vars(test) == vars(actual)


def test_entity_builder_builds_entities_with_randomizations(
    entities_with_randomizations_config,
    entities_with_randomizations,
):
    # * Arrange
    builder = randomization.EntityBuilder()

    # * Act
    test_entities = builder.build(entities_with_randomizations_config)

    # * Assert
    for test, actual in zip(test_entities, entities_with_randomizations):
        for test_randomization, actual_randomization in zip(
            test.randomizations, actual.randomizations
        ):
            assert vars(test_randomization) == vars(actual_randomization)


def test_flipper_initializes_values_from_config(randomization_config):
    # * Arrange
    flip_config = randomization_config[randomization.Config.FLIP_KEY]
    flipper = randomization.Flipper(flip_config)

    # * Act / Assert
    for test, actual in zip(vars(flipper).items(), flip_config.items()):
        assert test == actual


def test_translater_initializes_values_from_config(randomization_config):
    # * Arrange
    translate_config = randomization_config[randomization.Config.TRANSLATON_KEY]
    translater = randomization.Translater(translate_config)

    # * Act / Assert
    for test, actual in zip(vars(translater).items(), translate_config.items()):
        assert test == actual


def test_scaler_initializes_values_from_config(randomization_config):
    # * Arrange
    scaling_config = randomization_config[randomization.Config.SCALING_KEY]
    scaler = randomization.Scaler(scaling_config)

    # * Act / Assert
    for test, actual in zip(vars(scaler).items(), scaling_config.items()):
        assert test == actual


def test_rotator_initializes_values_from_config(randomization_config):
    # * Arrange
    rotator_config = randomization_config[randomization.Config.ROTATION_KEY]
    rotator = randomization.Rotator(rotator_config)

    # * Act / Assert
    for test, actual in zip(vars(rotator).items(), rotator_config.items()):
        assert test == actual


def test_translater_raises_exception_for_upper_bound_less_than_lower_bound(
    randomization_config,
):
    # * Arrange
    translater_config = randomization_config[
        randomization.Config.TRANSLATON_KEY
    ]
    translater_config[randomization.Translater.UPPER_BOUND_KEY] = -50

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Translater(translater_config)


def test_translater_raises_exception_for_upper_bound_limit(
    randomization_config,
):
    # * Arrange
    translater_config = randomization_config[
        randomization.Config.TRANSLATON_KEY
    ]
    translater_config[randomization.Translater.UPPER_BOUND_KEY] = 1001

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Translater(translater_config)


def test_translater_raises_exception_for_lower_bound_limit(
    randomization_config,
):
    # * Arrange
    translater_config = randomization_config[
        randomization.Config.TRANSLATON_KEY
    ]
    translater_config[randomization.Translater.LOWER_BOUND_KEY] = -200

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Translater(translater_config)


def test_scaler_raises_exception_for_upper_bound_less_than_lower_bound(
    randomization_config,
):
    # * Arrange
    scaler_config = randomization_config[randomization.Config.SCALING_KEY]
    scaler_config[randomization.Scaler.UPPER_BOUND_KEY] = 0

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Scaler(scaler_config)


def test_scaler_raises_exception_for_upper_bound_limit(
    randomization_config,
):
    # * Arrange
    scaler_config = randomization_config[randomization.Config.SCALING_KEY]
    scaler_config[randomization.Scaler.UPPER_BOUND_KEY] = 9999999

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Scaler(scaler_config)


def test_scaler_raises_exception_for_lower_bound_limit(
    randomization_config,
):
    # * Arrange
    scaler_config = randomization_config[randomization.Config.SCALING_KEY]
    scaler_config[randomization.Scaler.LOWER_BOUND_KEY] = -1

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Scaler(scaler_config)


def test_rotator_raises_exception_for_upper_bound_less_than_lower_bound(
    randomization_config,
):
    # * Arrange
    rotator_config = randomization_config[randomization.Config.ROTATION_KEY]
    rotator_config[randomization.Rotator.UPPER_BOUND_KEY] = -120.0

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Rotator(rotator_config)


def test_rotator_raises_exception_for_upper_bounds_limit(randomization_config):
    # * Arrange
    rotator_config = randomization_config[randomization.Config.ROTATION_KEY]
    rotator_config[randomization.Rotator.UPPER_BOUND_KEY] = 181.0

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Rotator(rotator_config)


def test_rotator_raises_exception_for_lower_bounds_limit(randomization_config):
    # * Arrange
    rotator_config = randomization_config[randomization.Config.ROTATION_KEY]
    rotator_config[randomization.Rotator.LOWER_BOUND_KEY] = -181.0

    # * Act / Assert
    with pytest.raises(randomization.RandomizationException) as exception:
        _ = randomization.Rotator(rotator_config)


def test_pose_generator_creates_flipped_poses(
    landingpad_with_flipper, flipped_poses
):
    # * Arrange
    pose_generator = randomization.PoseGenerator()

    # * Act
    pose_generator.run([landingpad_with_flipper])

    # * Assert
    for test, actual in zip(landingpad_with_flipper.poses, flipped_poses):
        assert vars(test) == vars(actual)


def test_pose_generator_creates_translated_poses(
    landingpad_with_translater, translated_poses
):
    # * Arrange
    pose_generator = randomization.PoseGenerator()

    # * Act
    pose_generator.run([landingpad_with_translater])

    # * Assert
    for test, actual in zip(landingpad_with_translater.poses, translated_poses):
        assert vars(test) == vars(actual)


def test_pose_generator_creates_scale_poses(
    landingpad_with_scaler, scaled_poses
):
    # * Arrange
    pose_generator = randomization.PoseGenerator()

    # * Act
    pose_generator.run([landingpad_with_scaler])

    # * Assert
    for test, actual in zip(landingpad_with_scaler.poses, scaled_poses):
        assert vars(test) == vars(actual)


def test_pose_generator_creates_rotation_poses(
    landingpad_with_rotator, rolled_poses
):
    # * Arrange
    pose_generator = randomization.PoseGenerator()

    # * Act
    pose_generator.run([landingpad_with_rotator])

    # * Assert
    for test, actual in zip(landingpad_with_rotator.poses, rolled_poses):
        assert vars(test) == vars(actual)


def test_pose_generator_raises_exception_if_no_randomizations(
    entities_without_randomizations,
):
    # * Arrange
    pose_generator = randomization.PoseGenerator()

    # * Act / Assert
    with pytest.warns(randomization.RandomizationWarning) as warning:
        poses = pose_generator.run(entities_without_randomizations)


def test_entity_converts_coordinates_from_geo_to_ue(landingpad_with_translater):
    # * Arrange
    # Initialize poses.
    pose_generator = randomization.PoseGenerator()
    pose_generator.run([landingpad_with_translater])
    converter = GeodeticConverter(45, 61, 2)  # Arbitrary home lat/lon/altitude.
    actual_converted_poses = [
        [-2402054, -5317833, 3792354, 0, 0, 0],
        [-2402055, -5317835, 3792353, 0, 0, 0],
        [-2402056, -5317838, 3792352, 0, 0, 0],
    ]

    # * Act
    landingpad_with_translater.convert_coordinates_from_geo_to_UE(converter)

    # * Assert
    for waypoint, actual in zip(
        landingpad_with_translater.poses, actual_converted_poses
    ):
        assert waypoint.pose_list == actual


# TODO: Add test for axis options exception thrown
