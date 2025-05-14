"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

This script serves to showcase the trajectory generation API's available to the user through the datacollection module

"""

import argparse
from datetime import datetime

from projectairsim.types import WeatherParameter
from projectairsim.utils import projectairsim_log
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.trajectory.conical_trajectory import (
    ConicalTrajectory,
)
from projectairsim.datacollection.trajectory.cylindrical_trajectory import (
    CylindricalTrajectory,
)
from projectairsim.datacollection.trajectory.takeoff_landing_trajectory import (
    TakeoffLandingTrajectory,
)
from projectairsim.datacollection.types import CollectionType, GeoCoordinates

parser = argparse.ArgumentParser("Trajectory Gen API Example Script")
parser.add_argument("--config-dir", help="Path to config directory", required=True)

parser.add_argument(
    "--save-path", help="Path to directory for saving images", default="./"
)

parser.add_argument(
    "--server-ip", help="IP Address of the Sim Server", default="127.0.0.1"
)

args = parser.parse_args()

config_dir = args.config_dir
save_path = args.save_path
server_ip = args.server_ip

if __name__ == "__main__":

    projectairsim_log().info(f"Running and Storing data locally")

    data_generator = DataGenerator(
        config_dir=config_dir, server_ip=server_ip, save_path=save_path
    )
    # Add weather variation to config
    data_generator.add_weather_variation(WeatherParameter.FOG, [0.1, 0.6, 0.1])

    # Update time overlayed on trajectory
    today_date = datetime.strptime("2023/02/27", "%Y/%m/%d")
    start_time = datetime.strptime("13:00:00", "%H:%M:%S")
    end_time = datetime.strptime("19:00:00", "%H:%M:%S")
    step = datetime.strptime("1:00:00", "%H:%M:%S")
    data_generator.update_time_variation(today_date, start_time, end_time, step)

    # Add trajectory preset to config
    takeoff_landing = TakeoffLandingTrajectory(
        trajectory_len=300, altitude_change=20, radius=5
    )
    conic = ConicalTrajectory(trajectory_len=450, altitude_change=40, field_of_view=45)

    data_generator.add_trajectory_preset(
        name="TestPresetDownward", trajectories=[takeoff_landing, conic]
    )

    # Update trajectory preset for a location
    data_generator.update_location_trajectory_preset(
        location_name="DFW-downward", preset_name="TestPresetDownward"
    )

    # Add a cylindrical trajectory preset to the config
    cylindrical = CylindricalTrajectory(
        trajectory_len=250,
        altitude_change=5,
        outer_radius=20,
        inner_radius=10,
        angular_variation=6,
    )
    data_generator.add_trajectory_preset(
        name="TestPresetForward", trajectories=[cylindrical]
    )

    # Add a GeoLocation at Blocks
    data_generator.add_geo_location(
        location_name="Test-Blocks",
        trajectory_name="TestPresetForward",  # Defined in config/via API
        scene_name="Block",
        scene_config="Blocks",  # Defined in the scene-configs section of the config file
        coordinates=GeoCoordinates(33.047, -97.2919, 250),  # lat-lon-alt
        asset="BasicLandingPad",  # Defined in the asset section of the config file
        object_id="TestLandingPad",  # Unique ID
    )

    # Add Env Actor at new location
    # NOTE: The env actor has to be already defined in the corresponding scene config
    # This API merely allows you to specify a given trajectory for the Env Actor
    data_generator.add_env_actor_to_location(
        location_name="Test-Blocks",
        actor_name="TestEnvActor",
        trajectory_preset_name="EnvActor-trajectory",
        duration=15.5,
    )

    # Trajectory Generation based on params in the config
    geo_locations = data_generator.generate_trajectory()

    # Collect data on generated trajectory with sim in the loop
    dataset = data_generator.collect_data(CollectionType.SYNC, server_ip)

    projectairsim_log().info(f"Data Collected. {len(dataset.images)} Images Collected")

    # Data Validation
    # Runs the collected data through a variety of validation checks
    dataset = data_generator.validate_data(dataset)

    # Data Aggregation
    # Aggregates and writes the collected data/metadata to spec files (csv/json/jsonl/coco)
    data_generator.aggregate_data(dataset)

    # Data Augmentation
    # Augment's collected data to add variety to to the dataset [post-processing step]
    data_generator.augment_data()
