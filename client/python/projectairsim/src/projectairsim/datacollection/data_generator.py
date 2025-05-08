"""
Copyright (C) Microsoft Corporation. All rights reserved.

This script implements the DataGenerator class. This class acts as
the tool develops will use to interact with the datacollection module. This class implements basics
API's that allow for trajectory generation, data collection, aggregation and validation. It also
implements API's that allow users to get finer access to the workings of the module and acts as another layer
the developer can modify along with the config 
"""

import pathlib
from datetime import datetime
from typing import Dict, List
from azure.storage.blob import BlobServiceClient, ContainerClient

from projectairsim.types import WeatherParameter
from projectairsim.utils import projectairsim_log
import projectairsim.datacollection.randomization.randomization as randomization
from projectairsim.datacollection.augmentation.dataaug import augment_data
from projectairsim.datacollection.augmentation.position_aug import DataAugmentation
from projectairsim.datacollection.collection import datacollector_sync
from projectairsim.datacollection.trajectory.conical_trajectory import (
    ConicalTrajectory,
)
from projectairsim.datacollection.trajectory.cylindrical_trajectory import (
    CylindricalTrajectory,
)
from projectairsim.datacollection.trajectory.port2port_trajectory import (
    Port2PortTrajectory,
)
from projectairsim.datacollection.trajectory.takeoff_landing_trajectory import (
    TakeoffLandingTrajectory,
)
from projectairsim.datacollection.trajectory.trajectory_generator import (
    Trajectory,
    TrajectoryGenerator,
)
from projectairsim.datacollection.specs import spec_main
from projectairsim.datacollection.types import (
    Annotation,
    CollectionType,
    Config,
    Dataset,
    GeoCoordinates,
    GeoLocation,
    Modality,
)
from projectairsim.datacollection.validation.data_quality import DataQualityValidator
from projectairsim.datacollection.validation.qualitative import VideoGenerator


class DataGenerator:
    """This class holds methods that define all actions that can be taken through the data generation module"""

    def __init__(
        self,
        config_dir: str,
        server_ip: str = "127.0.0.1",
        save_path: pathlib.Path = pathlib.Path("./"),
        compute_location: str = "local",
        sas_url: str = "",
    ) -> None:
        self.server_ip = server_ip
        self.save_path = save_path
        self.compute_location = compute_location
        self.start_index = 0
        self.sas_url = sas_url

        self.config = Config(config_dir)
        self.randomization_config = randomization.Config(self.config.config_path)
        self.randomized_entities = None

        self.process_trajectory_spec()
        self.geo_locations: Dict[str, GeoLocation] = self.process_geo_locations()

        self.container_client = self.get_container_client(sas_url)

    def get_container_client(self, sas_url) -> ContainerClient:
        try:
            container_url, sas_token = sas_url.split("?")
            # Find the index of the last forward slash
            last_slash_index = container_url.rfind("/")
            account_url = container_url[:last_slash_index]
            container_name = container_url.split("/")[-1]
            blob_service_client = BlobServiceClient(
                account_url=account_url, credential=sas_token
            )
            container_client = blob_service_client.get_container_client(container_name)
            return container_client
        except Exception as e:
            projectairsim_log().warning(
                f"Getting container client failed with error: {e}"
            )
            return None

    def process_trajectory_spec(self) -> dict:
        """Converts trajectory spec dicts to Trajectory objects"""
        for trajectory_name in self.config.trajectory_spec.keys():
            trajectory_dict: dict = self.config.trajectory_spec[trajectory_name]
            location_trajectory_spec = []
            for trajectory_type in trajectory_dict.keys():
                type_dict = trajectory_dict[trajectory_type]
                if trajectory_type == TrajectoryGenerator.CYLINDRICAL_KEY:
                    trajectory = CylindricalTrajectory()
                elif trajectory_type == TrajectoryGenerator.TAKEOFF_LANDING_KEY:
                    trajectory = TakeoffLandingTrajectory()
                elif trajectory_type == TrajectoryGenerator.CONIC_KEY:
                    trajectory = ConicalTrajectory()
                elif trajectory_type == TrajectoryGenerator.PORT2PORT_KEY:
                    trajectory = Port2PortTrajectory(self.config.dir, self.server_ip)
                trajectory.initialize_from_dict(type_dict)
                location_trajectory_spec.append(trajectory)

            self.config.trajectory_spec[trajectory_name] = location_trajectory_spec

        return self.config.trajectory_spec

    def process_geo_locations(self) -> Dict[str, GeoLocation]:
        """Aggregates config data into GeoLocation objects that will be iterated over when collecting data"""

        geo_locations = {}
        randomized_entity_builder = randomization.EntityBuilder()
        randomized_entities = randomized_entity_builder.build(
            self.randomization_config.get_static_entity_configs()
        )
        randomized_pose_generator = randomization.PoseGenerator()
        randomized_pose_generator.run(randomized_entities)
        self.randomized_entities = randomized_entities

        for location_name in self.config.geo_locations.keys():
            location = GeoLocation(
                location_name,
                self.config.geo_locations[location_name],
                self.config.env_spec.get("scene-configs", {}),
                self.randomized_entities,
                self.config.dir,
            )
            geo_locations[location_name] = location

        return geo_locations

    def generate_trajectory(
        self,
        num_sim: int = 1,
        sim_id: int = 0,
        enable_weather_sweep: bool = True,
        enable_time_sweep: bool = True,
    ) -> Dict[str, GeoLocation]:
        """Generate trajectory for all GeoLocation's and return them in a list"""

        trajectory_generator = TrajectoryGenerator(
            self.config,
            self.server_ip,
            enable_time_sweep,
            enable_weather_sweep,
            num_sim,
            sim_id,
        )

        for location_name in self.geo_locations.keys():
            # Add trajectory to each location
            location = self.geo_locations[location_name]
            updated_location = trajectory_generator.run(location)
            self.geo_locations[location_name] = updated_location

        # Avoid extra compute if not necessary
        if num_sim == 1:
            return self.geo_locations

        # GEt start index for current sim_id
        self.start_index = trajectory_generator.get_sim_id_start_index(
            self.geo_locations
        )

        # Split trajectories for parallelization
        self.geo_locations = trajectory_generator.split_trajectory(self.geo_locations)

        return self.geo_locations

    def collect_data(
        self, collection_type: CollectionType, server_ip="127.0.0.1"
    ) -> Dataset:
        """Collect desired data over the generated trajectories"""

        # TODO: Add a proper async implementation
        if collection_type == CollectionType.ASYNC:
            projectairsim_log().info(
                f"Async data collection is currently not supported. Stay Tuned!"
            )
            return Dataset()

        if collection_type == CollectionType.SYNC:
            return datacollector_sync.collect_data(
                self.geo_locations,
                self.config,
                server_ip,
                self.save_path,
                self.sas_url,
                self.start_index,
            )

    def validate_data(self, dataset: Dataset, sim_id: int = 0) -> Dataset:
        """Validate collected data and return a clean dataset"""

        projectairsim_log().info(f"Validating and Cleaning Collected Data")
        validator = DataQualityValidator(
            dataset,
            self.save_path,
            self.config.data_spec,
            self.config.input_config_dict,
            sim_id,
        )
        validator.validate_data()
        validator.generate_data_card()
        validator.write_data_card()
        projectairsim_log().info(f"Validation complete. Data card generated")

        return validator.dataset  # Update dataset to validated version

    def aggregate_data(self, dataset: Dataset) -> bool:
        """Aggregate all metadata into csv/json/json/coco formats"""
        # If no images collected, exit
        if len(dataset.images) == 0:
            projectairsim_log().info("No data to populate specs with")
            return False

        # Populate the specs
        spec_main.populate_specs(
            dataset=dataset,
            output_spec=self.config.output_spec,
            save_path=self.save_path,
            data_spec=self.config.data_spec,
            compute_location=self.compute_location,
            container_client=self.container_client,
        )
        return True

    def augment_data(self) -> bool:
        """Augment collected data based on config"""

        if self.compute_location == "aks":
            projectairsim_log().info(
                f"Augmentation is currently not supported when running on Azure. Please run it as a separate pipeline script"
            )
            return False

        projectairsim_log().info(f"Augmenting Data, after collection")
        augmented_imageids, bbox_2d_coords = augment_data(
            self.config.augmentation_spec,
            self.save_path,
            self.connection_string,
            self.compute_location,
        )
        projectairsim_log().info(f"Images Augmented and Stored.")

        return True

    def add_weather_variation(
        self, type: WeatherParameter, intensity: List[float]
    ) -> bool:
        """[Pre-Trajectory Generation] Adds a new weather variation

        Args:
            type (WeatherParameter): type of desired weather
            intensity (List[float]): intensity param sweep - [start, stop, step]
        """
        # Validation
        if len(intensity) != 3:
            projectairsim_log().warning(
                f"Please provide valid intensity list. Cannot add these weather params to config"
            )
            return False

        weather_dict = {"type": type.name, "intensity": intensity}

        weather_list = self.config.env_spec.get(Config.WEATHER_KEY, [])
        weather_list.append(weather_dict)
        self.config.env_spec[Config.WEATHER_KEY] = weather_list
        projectairsim_log().info(f"{type.name} added to config")
        return True

    def update_time_variation(
        self, date: datetime, start_time: datetime, end_time: datetime, step: datetime
    ) -> bool:
        """[Pre-Trajectory Generation] Update the time variation applied to trajectory"""

        try:
            time_list = [
                str(date.strftime("%Y-%m-%d")),
                start_time.strftime("%I:%M %p"),
                end_time.strftime("%I:%M %p"),
                step.strftime("%I:%M:%S"),
            ]

            self.config.env_spec[Config.TOD_KEY] = time_list
            projectairsim_log().info(f"Time-Of-Day settings updated")
        except Exception as e:
            projectairsim_log().warning(f"Error updating Time-Of-Day settings - {e}")
            return False

        return True

    def add_trajectory_preset(self, name: str, trajectories: List[Trajectory]) -> bool:
        """[Pre-Trajectory Generation] Add a trajectory preset to config"""

        if name in self.config.trajectory_spec.keys():
            projectairsim_log().warning(
                f"Trajectory preset {name} already exists in the config. Overwriting it"
            )

        if type(trajectories) != list:
            projectairsim_log().info(f"Please input trajectories as a list")
            return False

        self.config.trajectory_spec[name] = trajectories

        return True

    def update_location_trajectory_preset(
        self, location_name: str, preset_name: str
    ) -> bool:
        """[Pre-Trajectory Generation] Update a location with new trajectory preset"""

        if location_name not in self.config.geo_locations.keys():
            projectairsim_log().warning(
                f"Could not find location {location_name} in config"
            )
            return False

        if preset_name not in self.config.trajectory_spec.keys():
            projectairsim_log().warning(
                f"Could not find preset {preset_name} in config"
            )
            return False

        self.geo_locations[location_name].trajectory_preset = preset_name
        projectairsim_log().info(
            f"Trajectory preset for {location_name} updated to {preset_name}"
        )

        return True

    def add_env_actor_to_location(
        self,
        location_name: str,
        actor_name: str,
        trajectory_preset_name: str,
        duration: float,
        to_loop: bool = True,
        time_offset: float = 0,
        overwrite=False,
    ) -> bool:
        """[Pre-Trajectory Generation] Add/Update env actor settings for given location"""

        if location_name not in self.geo_locations.keys():
            projectairsim_log().warning(
                f"Location {location_name} not defined in the config"
            )
            return False

        if trajectory_preset_name not in self.config.trajectory_spec.keys():
            projectairsim_log().warning(f"trajectory not defined in config")
            return False

        location = self.geo_locations.get(location_name)

        if location.env_actor_name != "" and (not overwrite):
            projectairsim_log().info(
                f"No permission to overwrite existing env actor settings for location {location_name}"
            )
            return False

        location.env_actor_name = actor_name
        env_actor_trajectory_dict = {
            "name": trajectory_preset_name,
            "loop": to_loop,
            "time-offset": time_offset,
            "duration": duration,
        }
        location.env_actor_trajectory_settings = env_actor_trajectory_dict
        self.geo_locations[location_name] = location
        projectairsim_log().info(f"Env Actor {actor_name} added to {location_name}")

        return True

    def add_geo_location(
        self,
        location_name: str,
        trajectory_name: str,
        scene_name: str,
        scene_config: str,
        coordinates: GeoCoordinates = GeoCoordinates(0, 0, 0),
        asset: str = "",
        object_id: str = "",
    ) -> bool:
        """[Pre-Trajectory Generation] Add a location to the data collection config (does not support env actors)"""

        if location_name in self.geo_locations.keys():
            projectairsim_log().warning()(
                f"Location {location_name} already exists in the config and will be overwritten"
            )
        try:
            location_dict = {
                GeoLocation.LAT_KEY: coordinates.latitude,
                GeoLocation.LONG_KEY: coordinates.longitude,
                GeoLocation.ALT_KEY: coordinates.altitude,
                GeoLocation.SCENE_NAME_KEY: scene_name,
                GeoLocation.SCENE_CONFIG_KEY: scene_config,
                GeoLocation.OBJECT_ID_KEY: object_id,
                GeoLocation.ASSET_KEY: asset,
                GeoLocation.TRAJ_KEY: trajectory_name,
            }

            location = GeoLocation(
                location_name,
                location_dict,
                self.config.env_spec.get("scene-configs", {}),
                self.randomized_entities,
                self.config.dir,
            )

            self.geo_locations[location_name] = location
            projectairsim_log().info(f"Location {location_name} added to config")
        except Exception as e:
            projectairsim_log().info(f"Failed to add {location_name} to config - {e}")
            return False

        return True

    def update_data_spec(
        self, enabled_modalities: List[Modality], enabled_annotations: List[Annotation]
    ) -> bool:
        """[Pre-Collection] Update the modalities and annotations to be collected"""

        self.config.data_spec["modalities"] = enabled_modalities
        self.config.data_spec["annotations"] = enabled_annotations
        return True

    def get_data_card_dict(self, dataset: Dataset) -> Dict:
        """[Pose-Collection] Returns the validated data card dictionary"""

        validator = DataQualityValidator(
            dataset,
            self.save_path,
            self.config.data_spec,
            self.config.input_config_dict,
        )
        validator.validate_data()

        return validator.generate_data_card()

    def generate_video(self, annotation: Annotation, num_images: int = 0) -> bool:
        """[Post-Aggregation] Generate a visual representation of the collected data with 2d/3d Annotations"""

        if annotation == Annotation.TWO_D_BBOX:
            pose_dim = 4
        elif annotation == Annotation.THREE_D_BBOX:
            pose_dim = 16
        else:
            projectairsim_log().warning(
                f"Only 2d/3d annotations supported for video generation"
            )
            return False

        try:
            video_generator = VideoGenerator(
                config_dir=self.config.dir,
                num_images=num_images,
                save_path=self.save_path,
                pose_dim=pose_dim,
            )
            video_generator.generate_video()
            projectairsim_log().info(f"Video saved to {self.save_path}")
        except Exception as e:
            projectairsim_log().info(f"Video generation task failed with {e}")
            return False
        return True

    def update_augmentation_spec(self, augmentations: List[DataAugmentation]) -> bool:
        """[Pre-Augmentation] Change the augmentations and their params that will be applied to the collected data"""

        self.config.augmentation_spec = augmentations

        return True
