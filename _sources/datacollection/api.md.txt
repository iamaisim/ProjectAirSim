# Data Generation API Overview

The Data Generation [module](../../client\python/projectairsim/src/projectairsim/datacollection/data_generator.py) API's provide a develop interface to interact, update, add to and launch data collection runs and its underlying parameters/functionalities. These API's combined with the [config](../../client\python\example_user_scripts\datacollection\configs\datacollector_config.jsonc) act as a way for the developer to accomplish their goals as easily as possible. 

These API's can be divided into `4` major categories:
* [Base API's](#base-apis)
* [Pre Trajectory Generation API's](#pre-trajectory-generation-apis)
* [Pre Collection API's](#pre-collection-apis) 
* [Validation API's](#validation-apis)
* [Augmentation API's](#augmentation-apis)

## DataGenerator Object

- [`DataGenerator(config_dir, server_ip, save_path)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L51)

## Base APIs

These API's are the fundamental tasks you can achieve via the module. [basic_datacollection.py](../../client\python\example_user_scripts\datacollection\basic_datacollection.py) acts an example script for these API's

* [`generate_trajectory(enable_weather_sweep, enable_time_sweep)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L122) - Generate trajectory for all GeoLocation's and return them in a list
* [`collect_data(collection_type, server_ip)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L160) - Collect desired data over the generated trajectories
* [`validate_data(dataset)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L182) - Validate collected data and return a clean dataset
* [`aggregate_data(dataset)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L200) - Aggregate all metadata into csv/json/json/coco formats
* [`augment_data()`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L219) - Augment collected data based on config

## Pre Trajectory Generation APIs

These API's provides the developer with finer control over trajectory generation. Through these API's, the developer can programmatically update and add to the parameters defined in the config adn thus adjust the generated trajectory to their desire. These API's only have their intended effect if called before the trajectory generation API. [datacollection_trajectory_apis.py](../../client\python\example_user_scripts\datacollection/datacollection_trajectory_apis.py) acts as an example script for these API's

* [`add_weather_variation(type, intensity)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L239) - Adds a new weather variation
* [`update_time_variation(state, start_time, end_time, step)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L263) - Update the time variation applied to trajectory
* [`add_trajectory_preset(name, trajectories)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L284) - Add a trajectory preset to config
* [`update_location_trajectory_preset(location_name, preset_name)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L300) - Update a location with new trajectory preset
* [`add_env_actor_to_location(location_name, actor_name, trajectory_preset_name, duration, to_loop, time_offset, overwrite)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L324) - Add/Update env actor settings for given location
* [`add_geo_location(location_name, trajectory_name, scene_name, scene_config, coordinates, asset, object_id)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L367) - Add a location to the data collection config (does not support env actors)
## Pre Collection APIs

These API's provide control over data collection parameters. These API's only have their intended effect if called before the data collection API. [datacollection_collection_apis.py](../../client\python\example_user_scripts\datacollection/datacollection_collection_apis.py) acts as an example script for these API's

* [`update_data_spec(enabled_modalities, enabled_annotations)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L411) - Update the modalities and annotations to be collected

## Validation APIs
These API's expose the various sub-functions within the validation API and allow the develop to access that data. These API's only have their intended effect if called after the collection API. [datacollection_validation_apis.py](../../client\python\example_user_scripts\datacollection/datacollection_validation_apis.py) acts as an example script for these API's

* [`get_data_card_dict(dataset)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L420) - Returns the validated data card dictionary
* [`generate_video(annotation, num_images)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L433) - Generate a visual representation of the collected data with 2d/3d Annotations

## Augmentation APIs
These API's provide the developer with finer control over the augmentation settings that will be applied over the alrealy-collected dataset. [datacollection_augmentation_apis.py](../../client\python\example_user_scripts\datacollection/datacollection_augmentation_apis.py) acts as an example script for these API's

* [`update_augmentation_spec(augmentations)`](../../client\python\projectairsim\src\projectairsim\datacollection\data_generator.py#L460) - Change the augmentations and their params that will be applied to the collected data