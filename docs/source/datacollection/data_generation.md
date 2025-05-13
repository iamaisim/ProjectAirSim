# Data Generation

This documents serves to provide a detailed overview on the components that make up a data generation run and how to configure it. The folder [`example_user_scripts/datacollection`](../../client/python/example_user_scripts/datacollection/) provides a set of example scripts and configs for the developer's reference.

A data generation run consists of these main components:

* [Trajectory Generation](#trajectory-generation)
* [Data Collection](#data-collection)
* [Data Aggregation](#data-aggregation)
* [Data Validation](#data-validation)
* [Data Augmentation](augmentations.md#image-level-augmentations)

All these components are configured through [`datacollector_config.jsonc`](../../client/python/example_user_scripts/datacollection/configs/datacollector_config.jsonc) as well as a set of API's that allow for additional fine tuning on top of the config. For more information refer to [`config.md`](./config.md) and [`api.md`](./api.md)

### Note
There is an extra installation step that you need to follow in order to use the `datacollection` module inside `projectairsim` client. Once you have installed the `projectairsim` module, please run the following command in your python env

````bash
python -m pip install -U albumentations --no-binary qudida,albumentations
````

This manual step is required since the `albumentation` module installs `opencv-pyhton-headless` by default. Since this install interferes with the `opencv-python` install within the `projectairsim` module, we have not make it a dependancy within the wheel and are recommending users to manually install it with the above command, if using the `datacollection` module.

## Trajectory Generation
During the data generation run, the drone moves along the `WayPoint` objects defined during trajectory generation which makes this step crucial in terms of the quantity, quality and variety of data generated through the module. 

For more information on the types of trajectories available out-of-the-box and how to configure them, checkout [`trajectory.md`](./trajectory.md)

## Data Collection

The drone is moved along the generated trajectory. At each pose, ground-truth data is requested from the sim and saved along with other relevant meta data about that pose (weather/time conditions etc). What data is requested from the sim depends on the data-spec defined in the [`config`](./config.md#collection-spec)/through the [`API`](./api.md#pre-collection-apis)

Here, the sim scene and drone are configured through the `scene` and `robot` configs. 

## Data Validation

Once the data is collected and stored in memory, the data generation module allows the develop to run a set of validation checks on the data. These validation checks clean the dataset of data points that do not pass these tests. It also generate a report on the remaining data - proving quantitative measurements of the collected dataset.

These checks can be initiated through API calls to the data generator module. For more information, check out [`api.md`](./api.md#base-apis). [`datacollection_validation_apis.py`](../../client\python\example_user_scripts\datacollection/datacollection_validation_apis.py) is also included in the `example_user_scripts/datacollection` folder to provide an example usage of these API's
## Data Aggregation

Once the data is collected and validated, the data can be aggregated into specific spec formats that can later be used for tasks such as ingestion to data curation tools or fine-tuning of a pre-trained model.

For more information on the specific formats the data is saved to, check out [`data_aggregation.md`](./data_aggregation.md)

## Data Augmentation

Once the data is collected and aggregated, the module offers the option to augment this dataset with variations of itself. Its purpose is to add variety to your dataset and generate potential edge-case data points without going back to the sim. 

If enabled, this sub-module reads the image and metadata off of your directory, manipulates and saves it back to that directory. For images, the augmented images follow the same naming convention as those from the sim ({index}.png) and their indexes start from where the sim images end. As for the metadata, currently the 2D bbox data for the augmented images is appended to the `csv` generated through the data augmentation step

For more information about the types of available augmentations and configuring this sub-module through the config, checkout [config.md](./config.md#augmentation-spec)

For the available augmentation API's, checkout [`api.md`](./api.md). [`datacollection_augmentation_apis.py`](../../client\python\example_user_scripts\datacollection\datacollection_augmentation_apis.py#L61) is also included in the `example_user_scripts/datacollection` folder to provide an example usage of these API's