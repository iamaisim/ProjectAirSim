"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
Classes that define data randomization operations.

Each operation is a subclass of the abstract `Randomization` class. All
randomization operations must implement the `Randomization` interface.
"""
import argparse
import pathlib
import warnings
from abc import ABC, abstractmethod
from enum import Enum
from typing import Dict, List, Optional

import projectairsim
import projectairsim.datacollection.utils as utils
import projectairsim.geodetic_converter as geo_converter
import projectairsim.types
from projectairsim.datacollection.types import WayPoint


class RandomizationWarning(Warning):
    """Raise for any randomization warnings."""


class RandomizationException(Exception):
    """Raise for any randomization exceptions."""


class Randomization(ABC):
    @abstractmethod
    def __init__(self, config: dict) -> None:
        """Initializes Randomization object with appropriate variables."""

    @abstractmethod
    def create_poses(self, entity_origin_xyz: List[float]) -> List[WayPoint]:
        """Uses class variables to create randomized poses."""


class RandomizationWithBounds(Randomization):
    AMOUNT_KEY: str
    UPPER_BOUND_KEY: str
    LOWER_BOUND_KEY: str
    LOWER_LIMIT: float
    UPPER_LIMIT: float

    def __init__(self, config: dict) -> None:
        self.amount = config[self.AMOUNT_KEY]
        if config[self.UPPER_BOUND_KEY] < config[self.LOWER_BOUND_KEY]:
            raise RandomizationException(
                "upper_bound must be greater than lower_bound."
            )
        if config[self.UPPER_BOUND_KEY] > self.UPPER_LIMIT:
            raise RandomizationException(
                f"upper_bound {config[self.UPPER_BOUND_KEY]} is greater than the maximum {self.UPPER_LIMIT}."
            )
        if config[self.LOWER_BOUND_KEY] < self.LOWER_LIMIT:
            raise RandomizationException(
                f"lower_bound {config[self.LOWER_BOUND_KEY]} is less than the minimum {self.LOWER_LIMIT}."
            )
        self.upper_bound = config[self.UPPER_BOUND_KEY]
        self.lower_bound = config[self.LOWER_BOUND_KEY]


class Scaler(RandomizationWithBounds):
    """Scales an object using a unitless multiple."""

    AMOUNT_KEY = "amount"
    UPPER_BOUND_KEY = "upper_bound"
    LOWER_BOUND_KEY = "lower_bound"
    LOWER_LIMIT = 0.01
    UPPER_LIMIT = (
        99999  # Arbitrary - is there an upper limit to the scale multiple?
    )
    SCALE_KEY = "scale"

    def __init__(self, config: dict) -> None:
        super().__init__(config)

    def create_poses(self, entity_origin_xyz: List[float]) -> List[WayPoint]:
        """Creates poses for an entity based on initial configuration specs."""
        absolute_range = self.upper_bound + abs(self.lower_bound)
        # Handle case where amount is 0 or 1
        if self.amount < 2:
            step = 0
        else:
            step = int(absolute_range / (self.amount - 1))

        rpy = [0, 0, 0]
        xyzrpy = [*entity_origin_xyz, *rpy]
        scale_xyz = [self.lower_bound] * 3
        poses = []
        for i in range(1, self.amount + 1):
            pose = WayPoint(xyzrpy, scale_xyz=scale_xyz, altered=self.SCALE_KEY)
            poses.append(pose)
            scale_xyz = [step * i] * 3
        return poses


class Rotator(RandomizationWithBounds):
    """Rotates an object."""

    AXIS_KEY = "axis"
    AXIS_OPTIONS_MAP = {"roll": 3, "pitch": 4, "yaw": 5}
    AMOUNT_KEY = "amount"
    UPPER_BOUND_KEY = "upper_bound"
    LOWER_BOUND_KEY = "lower_bound"
    LOWER_LIMIT = -180
    UPPER_LIMIT = 180
    ROTATE_KEY = "rotate"

    def __init__(self, config: dict) -> None:
        """Initializes a Rotator based on randomization configuration."""
        super().__init__(config)
        if config[self.AXIS_KEY] not in self.AXIS_OPTIONS_MAP.keys():
            raise RandomizationException(
                f"Unrecognized axis {config[self.AXIS_KEY]}. Options are: {self.AXIS_OPTIONS_MAP.keys()}"
            )
        self.axis = config[self.AXIS_KEY]

    def create_poses(self, entity_origin_xyz: List[float]) -> List[WayPoint]:
        """Creates rotated poses for an entity based on initial configuration specs."""
        absolute_range = self.upper_bound + abs(self.lower_bound)
        # Handle case where amount is 0 or 1
        if self.amount < 2:
            step = 0
        else:
            step = absolute_range / (self.amount - 1)

        index_of_axis = self.AXIS_OPTIONS_MAP[self.axis]
        rpy = [0, 0, 0]
        xyzrpy = [*entity_origin_xyz, *rpy]
        xyzrpy[index_of_axis] = self.lower_bound
        poses = []
        for i in range(self.amount):
            pose = WayPoint(xyzrpy, altered=self.ROTATE_KEY)
            poses.append(pose)
            xyzrpy[index_of_axis] += step
        return poses


class Translater(RandomizationWithBounds):
    """Translates an object."""

    AXIS_KEY = "axis"
    AXIS_OPTIONS_MAP = {"x": 0, "y": 1, "z": 2}
    AMOUNT_KEY = "amount"
    UPPER_BOUND_KEY = "upper_bound"
    LOWER_BOUND_KEY = "lower_bound"
    UPPER_LIMIT = 100
    LOWER_LIMIT = -100
    TRANSLATE_KEY = "translate"

    def __init__(self, config: dict) -> None:
        super().__init__(config)
        if config[self.AXIS_KEY] not in self.AXIS_OPTIONS_MAP.keys():
            raise RandomizationException(
                f"Unrecognized axis {config[self.AXIS_KEY]}. Options are: {self.AXIS_OPTIONS_MAP.keys()}"
            )
        self.axis = config[self.AXIS_KEY]

    def create_poses(self, entity_origin_xyz: List[float]) -> List[WayPoint]:
        """Creates translated poses for an entity based on initial configuration specs."""
        absolute_range = self.upper_bound + abs(self.lower_bound)
        # Handle case where amount is 0 or 1
        if self.amount < 2:
            step = 0
        else:
            step = absolute_range / (self.amount - 1)

        index_of_axis = self.AXIS_OPTIONS_MAP[self.axis]
        rpy = [0, 0, 0]
        xyzrpy = [*entity_origin_xyz, *rpy]
        xyzrpy[index_of_axis] = self.lower_bound
        poses = []
        for i in range(self.amount):
            pose = WayPoint(xyzrpy, altered=self.TRANSLATE_KEY)
            poses.append(pose)
            xyzrpy[index_of_axis] += step
        return poses


class Flipper(Randomization):
    """Flips an object about a specific axis."""

    AXIS_KEY = "axis"
    AXIS_OPTIONS_MAP = {"roll": 3, "pitch": 4, "yaw": 5}
    INITIAL_RPY_KEY = "initial_rpy"
    FLIP_KEY = "flip"

    def __init__(self, config: dict) -> None:
        if config[self.AXIS_KEY] not in self.AXIS_OPTIONS_MAP.keys():
            raise RandomizationException(
                f"Unrecognized axis {config[self.AXIS_KEY]}. Options are: {self.AXIS_OPTIONS_MAP.keys()}"
            )
        self.axis = config[self.AXIS_KEY]
        self.initial_rpy = config[self.INITIAL_RPY_KEY]

    def create_poses(self, entity_origin_xyz: List[float]) -> List[WayPoint]:
        """Creates rotated poses for an entity based on initial configuration specs."""
        index_of_axis = self.AXIS_OPTIONS_MAP[self.axis]
        rpy = self.initial_rpy
        xyzrpy = [*entity_origin_xyz, *rpy]
        poses = []
        for _ in range(2):
            pose = WayPoint(xyzrpy, altered=self.FLIP_KEY)
            poses.append(pose)
            xyzrpy[index_of_axis] *= -1
        return poses


class CoordinateFrame(Enum):
    GEO = 0
    UE = 1


class Entity:
    def __init__(
        self,
        entity_type: str,
        origin_xyz: List[float],
        randomizations: Optional[List[Randomization]] = None,
    ) -> None:
        self.type = entity_type
        self.origin_xyz = origin_xyz
        self.poses: List[WayPoint] = []
        self.randomizations = randomizations
        self._coordinate_frame: CoordinateFrame = CoordinateFrame.GEO

    @property
    def coordinate_frame(self):
        return self._coordinate_frame

    def set_poses(self, poses: List[WayPoint]):
        self.poses = poses

    def convert_coordinates_from_geo_to_UE(
        self, converter: geo_converter.GeodeticConverter
    ) -> None:
        """Converts coordinates from geodedic to UE. No-op if already in UE coordinates."""
        if self._coordinate_frame == CoordinateFrame.UE:
            return None

        converted_poses: List[WayPoint] = []
        # Convert poses to UE.
        for pose in self.poses:
            xyzrpy = pose.pose_list
            converted_coords = utils.convert_geo_to_UE(
                geo_coord=xyzrpy[0:3], converter=converter
            )
            pose.update_pose_xyz(converted_coords)
            converted_poses.append(pose)
        self.poses = converted_poses
        self.origin_xyz = utils.convert_geo_to_UE(
            geo_coord=self.origin_xyz, converter=converter
        )
        self._coordinate_frame = CoordinateFrame.UE


class Config:
    ENV_SPEC_KEY = "env-spec"
    ASSETS_KEY = "assets"
    ENTITY_TYPE_KEY = "type"
    ENTITY_ORIGIN_KEY = "origin_xyz"
    RANDOMIZATION_KEY = "randomizations"
    TEXTURE_KEY = "texture"
    SCALING_KEY = "scale"
    ROTATION_KEY = "rotation"
    TRANSLATON_KEY = "translation"
    FLIP_KEY = "flip"

    def __init__(self, config_file_path: pathlib.Path):
        config = utils.read_cjson(config_file_path)
        config = config[self.ENV_SPEC_KEY][self.ASSETS_KEY]
        self.config = [asset_config for _, asset_config in config.items()]

    def get_static_entity_configs(self) -> List[Dict]:
        return self.config


class RandomizationBuilder:
    def build(self, config: List[Dict]) -> Optional[List[Randomization]]:
        """Builds randomizations using given configuration dictionary."""
        randomization_list: List[Randomization] = []
        if Config.TEXTURE_KEY in config:
            """TODO: add Texture."""
        if Config.SCALING_KEY in config:
            randomization_list.append(Scaler(config[Config.SCALING_KEY]))
        if Config.ROTATION_KEY in config:
            randomization_list.append(Rotator(config[Config.ROTATION_KEY]))
        if Config.TRANSLATON_KEY in config:
            randomization_list.append(Translater(config[Config.TRANSLATON_KEY]))
        if Config.FLIP_KEY in config:
            randomization_list.append(Flipper(config[Config.FLIP_KEY]))
        return None if not randomization_list else randomization_list


class EntityBuilder:
    def build(self, config: Dict) -> List[Entity]:
        """Builds entities using given configuration dictionary."""
        entity_list = []
        for entity_config in config:
            randomization_config = entity_config[Config.RANDOMIZATION_KEY]
            randomizations = RandomizationBuilder().build(randomization_config)
            entity = Entity(
                entity_config[Config.ENTITY_TYPE_KEY],
                entity_config[Config.ENTITY_ORIGIN_KEY],
                randomizations,
            )
            entity_list.append(entity)
        return entity_list


class PoseGenerator:
    def run(self, entities: List[Entity]) -> None:
        for entity in entities:
            entity_poses = []
            if not entity.randomizations:
                warnings.warn(
                    RandomizationWarning(
                        f"Cannot generate poses; entity {entity} has no randomizations."
                    )
                )
                continue
            for randomization in entity.randomizations:
                subset_of_poses = randomization.create_poses(entity.origin_xyz)
                entity_poses.extend(subset_of_poses)
            entity.set_poses(entity_poses)
