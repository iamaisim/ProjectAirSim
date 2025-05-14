"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Thsi script implements several custom classes that are used throughout the datacollection module
"""

import copy
import pathlib
from collections import namedtuple
from datetime import datetime
from enum import Enum
from typing import Dict, List

import numpy as np
import projectairsim.datacollection.augmentation.position_aug as AugmentationTypes
import projectairsim.datacollection.utils as utils
import projectairsim.geodetic_converter as geodedic_converter
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import projectairsim_log, rpy_to_quaternion


class CollectionType(Enum):
    """Denotes the type of collection scripts available to the user"""

    SYNC = "sync"
    ASYNC = "async"


class Modality(Enum):
    """Denotes the modalities available to be collected"""

    RGB = "RGB"


class Annotation(Enum):
    """Denotes the annotations available to be collected"""

    TWO_D_BBOX = "2DBBox"
    THREE_D_BBOX = "3DBBox"
    SEGMENTATION = "segmentation"


GeoCoordinates = namedtuple("GeoCoordinates", ["latitude", "longitude", "altitude"])


class Config:
    """
    This class holds the data held in the jsonc config as well as has methods to parse and compile complex structs within the config
    """

    ENV_SPEC_KEY = "env-spec"
    COLLECTION_SPEC_KEY = "collection-spec"
    TRAJECTORY_SPEC_KEY = "trajectory-spec"
    GEO_LOCATIONS_KEY = "geo-locations"
    DATA_SPEC_KEY = "data-spec"
    OUTPUT_SPEC_KEY = "output-spec"
    AUGMENTATION_SPEC_KEY = "augmentation-spec"
    WEATHER_KEY = "weather"
    TOD_KEY = "time-of-day"

    def __init__(self, config_dir) -> None:
        self.dir = config_dir
        self.config_path = pathlib.Path(config_dir, "datacollector_config.jsonc")
        self.schema_path = pathlib.Path(
            config_dir, "schemas", "datacollector_config_schema.jsonc"
        )
        self.config_dict: dict = utils.read_cjson(self.config_path)
        self.validate()
        self.input_config_dict = copy.deepcopy(self.config_dict)

        self.env_spec: dict = self.config_dict.get(Config.ENV_SPEC_KEY, {})
        self.collection_spec: dict = self.config_dict.get(
            Config.COLLECTION_SPEC_KEY, {}
        )
        self.trajectory_spec: dict = self.collection_spec.get(
            Config.TRAJECTORY_SPEC_KEY, {}
        )
        self.geo_locations: dict = self.collection_spec.get(
            Config.GEO_LOCATIONS_KEY, {}
        )
        self.data_spec: dict = self.parse_data_spec()
        self.output_spec: dict = self.config_dict.get(Config.OUTPUT_SPEC_KEY, {})
        self.augmentation_spec: List[
            AugmentationTypes.DataAugmentation
        ] = self.parse_augmentation_spec()

    # Add validation logic
    def validate(self) -> None:
        projectairsim_log().info(f"Validating Config")
        utils.validate_config(config=self.config_dict, schema_path=self.schema_path)
        projectairsim_log().info(f"Config Validated")

    def parse_data_spec(self) -> Dict:
        data_spec_config = self.collection_spec.get(Config.DATA_SPEC_KEY, {})
        data_spec = {"modalities": [], "annotations": []}
        modalities: Dict = data_spec_config.get("modalities")
        annotations: Dict = data_spec_config.get("annotations")

        for modality in modalities.keys():
            if modalities[modality]:
                data_spec.get("modalities").append(Modality(modality))

        for annotation in annotations.keys():
            if annotations[annotation]:
                data_spec.get("annotations").append(Annotation(annotation))

        return data_spec

    def parse_augmentation_spec(
        self,
    ) -> List[AugmentationTypes.DataAugmentation]:
        enabled_augmentations = []
        available_augmentation_types = {
            "horizontal-flip": AugmentationTypes.RandomHorizontalFlip,
            "vertical-flip": AugmentationTypes.RandomVerticalFlip,
            "crop": AugmentationTypes.RandomCrop,
            "rotate": AugmentationTypes.RandomRotate,
            "affine-transform": AugmentationTypes.RandomAffine,
            "brightness-contrast": AugmentationTypes.RandomBrightnessContrast,
            "hue-saturation-value": AugmentationTypes.RandomHueSaturationValue,
            "motion-blur": AugmentationTypes.RandomMotionBlur,
            "gaussian-noise": AugmentationTypes.RandomGaussianNoise,
        }
        augmentation_config: dict = self.config_dict.get(Config.AUGMENTATION_SPEC_KEY)
        augmentations: dict = augmentation_config.get("augmentations", {})
        for augmentation_type in augmentations:
            if augmentation_type in available_augmentation_types and (
                augmentations[augmentation_type].get("enabled", False)
            ):
                augmentation: AugmentationTypes.DataAugmentation = (
                    available_augmentation_types[augmentation_type]
                )
                enabled_augmentations.append(
                    augmentation(augmentations.get(augmentation_type))
                )

        return enabled_augmentations


class BboxData:
    """This class holds 2d/3d bbox data and its category"""

    def __init__(self, category: str) -> None:
        self.category = category
        self.bbox_2d: List = []
        self.bbox_3d: List = []
        self.bbox_area = 0

    def add_2d_bbox(self, bbox_2d: List):
        self.bbox_2d = bbox_2d
        if self.bbox_2d != []:
            self.bbox_area = self.bbox_2d[2] * self.bbox_2d[3]

    def add_3d_bbox(self, bbox_3d: List):
        self.bbox_3d = bbox_3d


class WayPoint:
    """This class holds information about each point in the trajectory as well as metadata associated with that position"""

    def __init__(
        self, pose: List[float], scale_xyz: List[float] = None, altered=None
    ) -> None:
        self.pose_x: float = pose[0]
        self.pose_y: float = pose[1]
        self.pose_z: float = pose[2]
        if len(pose) > 3:
            self.roll: float = pose[3]
            self.pitch: float = pose[4]
            self.yaw: float = pose[5]
        else:
            self.roll: float = None
            self.pitch: float = None
            self.yaw: float = None
        self.index: int = None
        self.weather: Dict = {}
        self.time: datetime = None
        self.scale_xyz = scale_xyz
        self.altered = altered

    @property
    def pose_list(self):
        return [
            self.pose_x,
            self.pose_y,
            self.pose_z,
            self.roll,
            self.pitch,
            self.yaw,
        ]

    def add_rpy(self, orientation: List) -> None:
        self.roll = orientation[0]
        self.pitch = orientation[1]
        self.yaw = orientation[2]

    def update_pose_xyz(self, xyz: List[float]) -> None:
        self.pose_x = xyz[0]
        self.pose_y = xyz[1]
        self.pose_z = xyz[2]

    def convert_UE_to_NED(self):
        self.pose_z *= -1

    def get_transform(self):
        quat = rpy_to_quaternion(self.roll, self.pitch, self.yaw)
        translation = Vector3({"x": self.pose_x, "y": self.pose_y, "z": self.pose_z})
        rotation = Quaternion({"w": quat[0], "x": quat[1], "y": quat[2], "z": quat[3]})
        transform = {"translation": translation, "rotation": rotation}

        return Pose(transform)

    def __str__(self) -> str:
        return f"{self.pose_x, self.pose_y, self.pose_z}"


class GeoLocation:
    """This class holds information about each location data will be collected at including the trajectory the drone will follow at that location"""

    LAT_KEY = "latitude"
    LONG_KEY = "longitude"
    ALT_KEY = "altitude"
    SCENE_NAME_KEY = "scene-name"
    SCENE_CONFIG_KEY = "scene-config"
    OBJECT_ID_KEY = "object-id"
    ASSET_KEY = "asset"
    TRAJ_KEY = "trajectory"
    ENV_ACTOR_NAME_KEY = "env-actor-name"
    ENV_ACTOR_TRAJ_KEY = "env-actor-trajectory"

    def __init__(
        self,
        name: str,
        location_dict: Dict,
        scene_configs: Dict,
        randomized_entities: List,
        config_dir: str,
    ) -> None:
        self.name = name
        self.latitude: float = location_dict.get(self.LAT_KEY, 0)
        self.longitude: float = location_dict.get(self.LONG_KEY, 0)
        self.altitude: float = location_dict.get(self.ALT_KEY, 0)
        self.scene_name: str = location_dict.get(self.SCENE_NAME_KEY)
        self.scene_config: str = scene_configs.get(
            (location_dict.get(self.SCENE_CONFIG_KEY))
        )
        self.object_id: str = location_dict.get(self.OBJECT_ID_KEY, "")
        self.asset: str = location_dict.get(self.ASSET_KEY, "")
        self.trajectory_preset: str = location_dict.get(self.TRAJ_KEY)
        self.config_dir = config_dir
        self.randomized_entities = randomized_entities
        self.occupancy_map: OccupancyMap = None
        self.env_actor_name: str = location_dict.get(self.ENV_ACTOR_NAME_KEY, "")
        self.env_actor_trajectory_settings: Dict = location_dict.get(
            self.ENV_ACTOR_TRAJ_KEY, {}
        )

        self.trajectory: List[WayPoint] = []
        self.env_actor_trajectory: List[WayPoint] = []

        self.geodedic_converter: geodedic_converter.GeodeticConverter = (
            self._initialize_geoditic_converter()
        )

    def _initialize_geoditic_converter(
        self,
    ) -> geodedic_converter.GeodeticConverter:
        """Initializes a geodedic converter for converting coordinates to/from geo/UE."""
        path = pathlib.Path(self.config_dir, "sim_config", self.scene_config)
        config_data = utils.read_cjson(path)

        home_geo_point = config_data["home-geo-point"]
        converter = geodedic_converter.GeodeticConverter(
            home_geo_point["latitude"],
            home_geo_point["longitude"],
            home_geo_point["altitude"],
        )
        return converter

    @property
    def scene_coordinates(self):
        return utils.convert_geo_to_UE(
            geo_coord=[self.latitude, self.longitude, self.altitude],
            converter=self.geodedic_converter,
        )

    @property
    def can_generate_env_actor_trajectory(self) -> bool:
        """Returns whether or not environment actor trajectory can be generated."""
        if self.env_actor_name == "":
            return False
        trajectory_name = self.env_actor_trajectory_settings.get("name", "")

        if trajectory_name == "" or trajectory_name == "pre-defined":
            return False

        return True


class ImageData:
    """This class holds data for each data point collected through the data collection module"""

    def __init__(
        self,
        id: int,
        randomization_index: int,
        location: GeoLocation,
        weather: str,
        time_of_day: str,
        pose: List,
    ) -> None:
        self.id = id
        self.rand_id = randomization_index
        self.weather: str = weather
        self.time_of_day: str = time_of_day
        self.location: str = location.scene_name
        self.lat_lon: str = f"{location.latitude},{location.longitude}"
        self.pose: List = pose
        self.point_of_interest: str = location.name
        self.negative_sample = False
        self.name = f"{self.id}.png"

    def add_image_properties(self, height: int, width: int):
        self.height = height
        self.width = width

    def add_bbox_data(self, bbox: BboxData):
        self.bbox_data = bbox

    def add_segmentation_data(self, annotations):  # List of COCOAnnotations
        self.segmentation_data = annotations


class Dataset:
    """This class holds data about the dataset collected through the data collection module"""

    def __init__(self) -> None:
        self.images: List[ImageData] = []
        self.non_collected_images = []
        self.categories = {}
        self.configs = {}

    def get_max_area(self):
        max_area = 0
        for image in self.images:
            max_area = max(max_area, image.bbox_data.bbox_area)
        return max_area

    def clean_up(self, cutoff_ratio=0.05):
        max_area = self.get_max_area()
        cutoff_area = max_area * cutoff_ratio
        temp = self.images.copy()
        for i, image in enumerate(self.images):
            if image.bbox_data.bbox_area < cutoff_area:
                self.non_collected_images.append(image.id)
                temp.remove(image)
        self.images = temp

    def update_categories(self, categories: Dict):
        self.categories = categories

    def add_config(self, location: str, scene_config: str, robot_config: str):
        self.configs[location] = {"scene": scene_config, "robot": robot_config}


class COCOAnnotations:
    def __init__(self, category_id, polygon, segmentation) -> None:
        self.category_id: int = category_id
        self.polygon: List = polygon
        self.segmentation = segmentation


class OccupancyMap:
    """This class hold information about the voxel grid occupancy map that is generated at a location"""

    def __init__(
        self,
        occupancy_map: List[bool],
        edge_len: float,
        resolution: float,
        center: WayPoint,
    ) -> None:
        self.occupancy_map = occupancy_map
        self.edge_len = edge_len
        self.resolution = resolution
        self.center = center

        self.z_min = self.resolution  # Max z value in NED coordinates

    def get_grid_idx(self, coordinate: WayPoint):
        x = coordinate.pose_x - self.center.pose_x
        y = coordinate.pose_y - self.center.pose_y
        z = coordinate.pose_z - self.center.pose_z

        num_cells = self.edge_len // self.resolution

        x_idx = round(x / self.resolution + (num_cells / 2))
        y_idx = round(y / self.resolution + (num_cells / 2))

        z_idx = round(z / self.resolution + (num_cells / 2))

        grid_idx = x_idx + num_cells * (z_idx + num_cells * y_idx)

        return int(grid_idx)

    def check_validity(self, pose: WayPoint):
        # Takes in pose in NEU coordinates
        # Pose cannot be underground ( z < 0 in NED)
        if pose.pose_z < self.resolution:  # Needs to be greater than min res in z
            return False

        idx = self.get_grid_idx(pose)

        try:
            return not self.occupancy_map[idx]
        except:
            return False
