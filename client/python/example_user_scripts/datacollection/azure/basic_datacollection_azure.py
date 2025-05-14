"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

This script integrates all sub functions of datacollection
1. Reads datacollection config (datacollection_config.jsonc)
2. Generates poses based on trajectory specs
3. Launches data collection on that trajectory
4. Compiles collected data/metadata/ground-truth into spec files (csv, json etc)

For more information, refer to the `docs/datacollection` folder

"""

import argparse

from projectairsim.utils import projectairsim_log
from projectairsim.datacollection.data_generator import DataGenerator
from projectairsim.datacollection.types import CollectionType

parser = argparse.ArgumentParser("Integrated Data Collector")
parser.add_argument(
    "--config-dir",
    help="Path to config directory",
    default="./datagen_configs",
)

parser.add_argument(
    "--save-path", help="Path to directory for saving images", default="./"
)

parser.add_argument(
    "--server-ip", help="IP Address of the Sim Server", default="127.0.0.1"
)

parser.add_argument("--sim-id", help="i'th sim server", default=0, type=int)

parser.add_argument(
    "--num-sims",
    help="Number of sims data collection will be distributed over",
    default=1,
    type=int,
)

parser.add_argument(
    "--sas-url",
    help="Sas token for the azure blob container",
)

parser.add_argument(
    "--compute-location",
    help="Where the data collection will be run. OPTIONS: [local, aks]",
    default="aks",
)

args = parser.parse_args()

config_dir = args.config_dir
save_path = args.save_path
server_ip = args.server_ip
sim_id = args.sim_id
num_sims = args.num_sims
sas_url = args.sas_url
compute_location = str(args.compute_location)

if __name__ == "__main__":
    projectairsim_log().info(f"Running and Storing data on {compute_location}")

    data_generator = DataGenerator(
        config_dir=config_dir,
        server_ip=server_ip,
        save_path=save_path,
        compute_location=compute_location,
        sas_url=sas_url,
    )

    # Trajectory Generation based on params in the config
    geo_locations = data_generator.generate_trajectory(num_sims, sim_id)

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
