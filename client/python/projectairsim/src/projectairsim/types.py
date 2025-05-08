"""
Copyright (C) Microsoft Corporation. All rights reserved.
ProjectAirSim Type definitions
"""

from enum import IntEnum
from typing import Dict, List
import projectairsim.utils as utils


class FrameType(IntEnum):
    """Message frame type enum values:
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE,
    UNSUBSCRIBEALL
    """

    SUBSCRIBE = 0
    UNSUBSCRIBE = 1
    MESSAGE = 2
    UNSUBSCRIBEALL = 3


class SerializationType(IntEnum):
    """Message serialization type enum values:
    MSGPACK_JSON,
    PURE_JSON
    """

    MSGPACK_JSON = 0
    PURE_JSON = 1


class WeatherParameter(IntEnum):
    """Weather parameter enum values:
    ENABLED,
    RAIN,
    ROAD_WETNESS,
    SNOW,
    ROAD_SNOW,
    MAPLE_LEAF,
    ROAD_LEAF,
    DUST,
    FOG
    """

    ENABLED = 0
    RAIN = 1
    ROAD_WETNESS = 2
    SNOW = 3
    ROAD_SNOW = 4
    MAPLE_LEAF = 5
    ROAD_LEAF = 6
    DUST = 7
    FOG = 8


class ImageType(IntEnum):
    """Image type enum values:
    SCENE,
    DEPTH_PLANAR,
    DEPTH_PERSPECTIVE,
    SEGMENTATION,
    DEPTH_VIS,
    DISPARITY_NORMALIZED,
    SURFACE_NORMALS
    """

    SCENE = 0
    DEPTH_PLANAR = 1
    DEPTH_PERSPECTIVE = 2
    SEGMENTATION = 3
    DEPTH_VIS = 4
    DISPARITY_NORMALIZED = 5
    SURFACE_NORMALS = 6


class BoxAlignment(IntEnum):
    """Bounding box alignment enum values:
    WORLD_AXIS,
    OBJECT_ORIENTED
    """

    WORLD_AXIS = 0
    OBJECT_ORIENTED = 1

class LandedState(IntEnum):
    LANDED = 0
    FLYING = 1


class ProjectAirSimTopic:
    """Pub-sub topic info data.

    Args:
        path (str): topic path
        topic_type (str): 'published' or 'subscribed' by the sim server
        message_type (str): message type
        frequency (int): topic frequency in Hz
    """

    def __init__(self, path: str, topic_type: str, message_type: str, frequency: int):
        self.path = path
        self.topic_type = topic_type
        self.message_type = message_type
        self.frequency = frequency


class AttrDict(dict):
    """Base attribute dictionary type to set other types to use Dict as the
    underlying data type."""

    def __getattr__(self, attr):
        try:
            return self[attr]
        except KeyError:
            raise AttributeError(attr)

    def __setattr__(self, attr, value):
        self[attr] = value


class Transform(AttrDict):
    """Class to represent a Transform as a dict with the following form:
        { "frame_id": "DEFAULT_FRAME", "translation": Vector3 dict, "rotation": Quaternion dict }

    Args:
        transform (Dict): Initial dict to construct from.
    """

    def __init__(self, transform: Dict):
        self.frame_id = transform.get("frame_id", "DEFAULT_FRAME")
        translation = transform.get("translation")
        self.translation = utils.decode(translation)
        self.translation = Vector3(self.translation)

        rotation = transform.get("rotation")
        self.rotation = utils.decode(rotation)
        self.rotation = Quaternion(self.rotation)


Pose = Transform  # Just an alias


class Vector3(AttrDict):
    """Class to represent a 3-element vector as a dict with the following form:
        { "x": 0.0, "y": 0.0, "z": 0.0 }

    Args:
        vec (Dict): Initial dict to construct from.
    """

    def __init__(self, vec: Dict):
        self.x = vec["x"]
        self.y = vec["y"]
        self.z = vec["z"]

    def to_list(self):
        return [self.x, self.y, self.z]


class Quaternion(AttrDict):
    """Class to represent a 4-element quaternion as a dict with the following form:
        { "w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0 }

    Args:
        quat (Dict): Initial dict to construct from.
    """

    def __init__(self, quat: Dict):
        self.w = quat["w"]
        self.x = quat["x"]
        self.y = quat["y"]
        self.z = quat["z"]


class Color:
    """Class to represent a single RGB color value

    Args:
        rgb (List): List of initial color values in RGB order as [r, g, b]
    """

    def __init__(self, rgb: List):
        self.r = rgb[0]
        self.g = rgb[1]
        self.b = rgb[2]

    def __hash__(self):
        return hash((self.r, self.g, self.b))

    def __eq__(self, other):
        return self.r == other.r and self.b == other.b and self.g == other.g
