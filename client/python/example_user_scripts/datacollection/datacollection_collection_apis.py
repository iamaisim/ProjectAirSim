"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

This script serves to showcase the collection API's available to the user through the datacollection module

For more information, refer to the `docs/datacollection` folder

"""

import argparse

from projectairsim.utils import projectairsim_log
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.types import Annotation, CollectionType, Modality

parser = argparse.ArgumentParser("Collection API's Example Script")
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
    # Trajectory Generation based on params in the config
    geo_locations = data_generator.generate_trajectory()

    # Update what annotations and modalities are collected by the collection API
    data_generator.update_data_spec(
        enabled_modalities=[Modality.RGB],
        enabled_annotations=[Annotation.TWO_D_BBOX, Annotation.THREE_D_BBOX],
    )

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
