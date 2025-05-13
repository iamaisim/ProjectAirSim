# Data Collection Config

This document serves to provide context on the configuration file that defines the data generation run and what the different components and parameters within it mean. This configuration file is intended to be the single file that a user has to modify in order to collect the desired data from the sim.

A sample config - [datacollection_config.jsonc](../../client\python\example_user_scripts\datacollection\configs\datacollector_config.jsonc) is available for reference. Note - the datacollection API's only reads `this` file so please make all modifications to this file only.

The config file is broken into three main sections:
* [Environments Spec](#environment-spec)
* [Collection Spec](#collection-spec)
* [Augmentation Spec](#augmentation-spec)
* [Output Spec](#output-spec)

## Environment Spec

Referred to as `env-spec` in config, these set of params define the sim environment that the data collection will take place in. The following are the environment spec params:
* `assets` - defines the various assets that the user wants to spawn in the sim scene. Each individual `asset` is defined with the following params:
    - `source` - Choose between `GLTF` (runtime asset spawned from a local gltf/glb file), `UnrealProject` (asset baked into the project)
    - `file-name` - Name of the asset file (full path for local GLTF's)
    - `scale` - Array with asset scale in x,y and z (e.g. [10,2,5])
    - `rotation` - Array defining quaternion rotation of the asset
    - `seg-id` - Segmentation ID to be assigned to the asset (If not specified, a random ID will be assigned at spwan)
    - `origin_xyz` - The lat-lon spawn position of the asset
    - `randomizations` - A list of randomizations to be applied to the asset during data collection. For more information, please refer to [randomizations.md](randomizations.md)
* `weather`: Array containing desired weather variations in the data. The dataset is multiplied by the number of weather variations (for e.g. if the dataset length is 500 and the 5 weather variations are configured, the output dataset length will be 2500 data points). Each `dict` object in the list contains the following params:
    - `type`: The desired weather type (See `projectairsim.types.WeatherParameter` for available types)
    - `intensity`: A list of intensity values to sweep over - [start, end, step]. Weather intensity values are in the range (0 - 1.0)
* `time-of-day`: A list input is accepted.
    - The elements of the list give range for parametrization.
    - The generated date-time values are equally distributed over all poses.
    - There are two accepted input formats:
    - "time-of-day": ["2022-06-20","7:15 AM","3:15 PM", "0:15:00" (indicates 15 mins of step size)]
    - "time-of-day": ["2022-06-20","07:15:00", "15:15:00","0:15:00"]
* `scene-configs`: Reference to the sim-scene configs for different scenes

## Collection Spec

Referred to as `collection-spec` in the config, these params define the kind of data that will be collected in the above defined sim scene. The following are the collection spec params:
* `trajectory-spec`: This param contains trajectory presets that can be used later in the config. These presets can be composed of any combination of the supported trajectories. For more details on the supported trajectory types and their defining param check out [`trajectory.md`](./trajectory.md)
* `geo-locations`: defines the locations within the sim scene the drone trajectory will be centered around and where the above assets could be spawned. Each `geo-location` is defined by:
    - `latitude`: Global latitude for location 
    - `longitude`: Global longitude for location
    - `altitude`: Altitude at location in `m`
    - `trajectory`: Name of the trajectory preset to be used at this location
    - `scene-name`: Name of the scene at this location (e.g. DFW, Redmond etc)
    - `scene-config`: Sim scene config to be used at this location
    - `asset`: Name of the asset (from those defined in `env-spec`) to be spawned at this location
    - `object-id`: Sim name of asset to be spawned. `Note` - this `object-id` should also be specified in the robot config's `annotation-setting` section for the sensor you are collecting data from
    - `env-actor-name`: Name of env actor configured in corresponding scene config
    - `env-actor-trajectory`: Defines trajectory the env actor will follow
        - `name`: name of trajectory preset or `pre-defined` if defined in config
        - `loop`: Wether the env actor keeps looping above trajectory
        - `time-offset`: Delay for starting trajectory
        - `duration`: Time taken to complete each loop

* `data-spec`: Defines the modality/annotations to be collected. Currently, the following are supported:
    - `modalities`:
        - `RGB` - Saves RGB image from the scene camera for each pose

    - `annotations`:
        - `2DBBox` - Collects and saves 2D bbox data for spawned assets
        - `3DBBox` - Collects and saves 3D bbox data for spawned assets
        - `segmentation`: Collects and saves data from the segmentation camera

## Augmentation Spec

Referred to as `augmentation-spec` in the config, these params define the augmentations that will be applied to images collected through the module. 

Note: These augmentations can be applied as a part of the data collection pipeline by adding the corresponding API to your script or separately on a directory of data by running [dataaug.py](../../client\python\projectairsim\src\projectairsim\datacollection\augmentation\dataaug.py). In each case, the config file needs to configured with the desired augmentations.

The following augmentations are currently supported:
   
    * `horizontal-flip` - Flip horizontally around the y-axis
    - `vertical-flip` - Flip horizontally around the x-axis

Each augmentation type has the following configurable params:
    
    - `enabled` - If given augmentation type is applied to the data
    - `p` - Probability of applying given augmentation to a data point. Range: (0-1)
## Output Spec

Referred to as `output-spec` in the config, these params define how and where the data from the pipeline is stored. THese params also define some of the static data associated with the dataset being collected through the pipeline. Following are the params that define output spec:
* `description`: Overall description of the collected dataset
* `operation-description`: Describe the mission the drone is flying in the scene
* `data-description`: Description of the data being collected
* `data-source`: Source of the data being generation (`projectairsim`)
* `error-sources`: Sources of error in the collected dataset 
* `dataset-name`: Name of the dataset to be collected, `Note` - if enabled, the Pixie dataset will be under this name
* `image-spec`: Properties of individual images in the dataset