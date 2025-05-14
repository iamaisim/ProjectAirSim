# EAP Quick Start

## Package contents

An Project AirSim EAP package should contain something like the following contents:

```
projectairsim_nnnn_2022xxxx
│   NOTICE.txt
│   README.md
│
├───client
│       ProjectAirSim_<user specific>_Scripts.zip
│       ProjectAirSim_Client_Binary_Wheel.zip
│       ProjectAirSim_Example_User_Scripts.zip
|       ProjectAirSim_ROS.zip
│
├───docs
│       External_Docs.zip
│
└───server
    ├───ue_environments
    │       GeoSpecificEnv_UEProject.zip
    │       SyntheticEnv_UEProject.zip
    │
    └───ue_plugin_only
    │       ProjectAirSim_UE_Plugin_Linux64.zip
    │       ProjectAirSim_UE_Plugin_Win64.zip
    │
    ├───unity_environments
    │       GeoSpecificEnv_UnityProject.zip
    │       SyntheticEnv_UnityProject.zip
    │
    └───unity_plugin_only
            ProjectAirSim_Unity_Plugin_Win64.zip
```

* `client`:  Setup file for the Project AirSim Python Client API library, and Python scripts with sim configuration files that use the Project AirSim Client API. *(Note: User-specific script packages may also contain an Unreal content-only plugin with custom assets that needs to be copied into the Unreal project Plugins folder. See the README file included with the scripts for more details.)*
* `docs`: Project AirSim documentation.
* `server`: Project AirSim simulation server files.
    * `ue_plugin_only`: Project AirSim plugins for Unreal Engine to use in your own custom environments.
    * `unreal_environments`: Unreal Engine projects including Project AirSim plugins that are ready to package and run.

To use geospecific environments, additional GIS tile assets for each region of interest need to be downloaded:

```
geo_specific_tiles
├───dfw_tiles
│       dfw-tiles-200km-2021xxxx.zip
│
└───san_diego_tiles
        san-diego-tiles-200km-2021xxxx.zip
```

*Note: Once the GIS tiles have been downloaded and extracted, the file path to their location needs to be updated in the sim config scene JSONC file for the client script that will run the simulation. See [Run the sim in a geospecific scene](#run-the-sim-in-a-geospecific-scene) for an example of this.*

## Read the docs

To get started, first take a look at the documentation in `docs\External_Docs.zip`. This contains a set of static HTML web page files. To view them, you can simply open `index.html` in a web browser, or start a local web host to serve them by running `python -m http.server` from the folder where the files are extracted to.

*Note: The search functionality of the docs is not supported when viewing the files locally.*

## Install system prerequisites

See [Installing system prerequisites](system_specs.md#installing-system-prerequisites) for information about Windows/Linux system setup needed before running Project AirSim.

## Set up the client

To install the Python client package, install the binary wheel provided in `client\ProjectAirSim_Client_Binary_Wheel.zip`:

```
<from activated Python environment>

pip install projectairsim-{version}-py3-none-any.whl
```
where "{version}" is the client version ID in the form "vX.Y.Z".  For instance, the wheel files might be named `projectairsim-0.1.10-py3-none-any.whl`.


*Note regarding LIDAR visualization by Open3D:*

*In order to use the client library's LIDAR visualization `LidarDisplay` utility, the Open3D pip package is needed but since [Open3D doesn't have a Python 3.9 package posted yet](https://github.com/isl-org/Open3D/issues/3983), Open3D is not included as a dependency in the binary wheel so that the Project AirSim client can be used with Python 3.9.*

*To use the LIDAR visualizations and example scripts, Open3D needs to be manually installed. For Python 3.7-3.8, you can simply run `pip install open3d`. For Python 3.9, you can download the [Open3D development wheel](http://www.open3d.org/docs/latest/getting_started.html#development-version-pip) and install it directly.*

Please see [Project AirSim Client Setup](client_setup.md) for more details about client setup.

## Run the sim in a synthetic scene

Building/packaging the sim scenes requires Unreal Engine 4.27. Please refer to [Unreal Engine](https://www.unrealengine.com/en-US/) for more details on installing UE 4.27.

To get started with running the sim in a synthetic scene:

1. Extract the provided `SyntheticEnv` sim environment provided in `server\environments\SyntheticEnv_UEProject.zip`.

    *Note: If you get a Windows error during extraction about exceeding the maximum 260 character file path limit, you can either extract the contents to a folder closer to the drive's root path, or disable this limitation as described [here](https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=powershell#enable-long-paths-in-windows-10-version-1607-and-later)*

2. The provided environment projects are pre-configured with the Project AirSim plugins already included, so to use them you can just open the `.uproject` file in the Unreal Editor (select "yes" to rebuild the project and plugins if prompted during opening) and start the sim by pressing Play in the Editor, or you can use the provided `package.<bat|sh>` scripts to package them into stand-alone binaries with the following steps:

    a) Set an environment variable called `UE_ROOT` with a value containing the path to your Unreal Engine's root folder (e.g. `C:\Program Files\Epic Games\UE_4.27`). The root folder is the path that contains the `Engine` subfolder.

    b) Run the package script with the desired variation (`DebugGame`, `Development`, or `Shipping`) as an argument (e.g. `package.bat Development`)

    c) After packaging completes, the stand-alone binary will be saved under a `Packages\` folder and can be launched using the game executable (e.g. `Packages\Development\WindowsNoEditor\SyntheticEnv.exe`)

3. Launch the environment sim server by pressing `Play` in the Unreal Editor or by executing the packaged sim binary with optional off-screen rendering (see [Command Line Switches](command_line_switches.md) for info about additional runtime options):

        SyntheticEnv.exe -RenderOffScreen

4. Extract the provided example client scripts provided in `client\ProjectAirSim_Example_User_Scripts.zip` and run a sim client:

        <from activated Python environment>

        python hello_drone.py

    The scene/robot configuration files referenced by these scripts are located under the `sim_config\` sub-folder included with the example scripts. See [Configuration JSONC Settings](config.md) for more details.

## Run the sim in a geospecific scene

To get started with running the sim in a geospecific scene:

1. Set up the provided `GeoSpecificEnv` sim environment provided in `server\environments\GeoSpecificEnv_UEProject.zip` following the same process as steps 1-2 from [running the sim in a synthetic scene](#run-the-sim-in-a-synthetic-scene).

2. Extract the provided GIS tiles for a geospecific scene (e.g. `geo_specific_tiles\dfw_tiles\dfw-tiles-200km-2021xxxx.zip`) to a local folder (e.g. `C:\dfw_tiles\`).

3. Extract the provided example client scripts provided in `client\ProjectAirSim_Example_User_Scripts.zip` and open the the geospecific scene example script to find the referenced scene config file that it loads with the `World` object. For example, in `hello_gis_dfw.py` the `World` object loads `scene_gis_dfw.jsonc`:

        world = World(client, "scene_gis_dfw.jsonc", delay_after_load_sec=0)

    This scene config JSONC file is loaded from the `sim_config/` subfolder in the folder with the example scripts.

4. Update the scene config file's `"tiles-dir"` field to the local path where you extracted the GIS tiles.

    e.g. `sim_config\scene_gis_dfw.jsonc`

        "scene-type": "CustomGIS",
        "tiles-dir": "<your path to the extracted GIS tile files>"

4. Launch the geospecific environment sim server following the same process as step 3 from [running the sim in a synthetic scene](#run-the-sim-in-a-synthetic-scene).

5. Run the sim client, and the tiles should start loading into the scene around the robot as the sim is running:

        <from activated Python environment>

        python hello_gis_dfw.py

## Add the UE Plugin to your own environment

The UE plugins by themselves are also provided in `server\ue_plugin_only` and are ready to be dropped into any custom Unreal project. See [Using Project AirSim Plugin in Custom Environments](use_plugin.md) for more details.

## Set up the ROS bridge

The Project AirSim ROS Bridge enables Project AirSim to be used with ROS, the Robot Operating System.  The Project AirSim ROS Bridge and examples of using Project AirSim with ROS are provided in `client\ProjectAirSim_ROS.zip`.  Please see [Project AirSim ROS Bridge](ros/ros.md) for setup and use.


# Enable AirSim in Unity Environments
If you want to run `GeoSpecificEnv_UnityProject` or `SyntheticEnv_UnityProject`, you need to do a simple extra step to run AirSim in them. Once you have your project open:
1. Open the desired scene in the `Scenes` folder.
2. Drag the `ProjectAirSimLoader.prefab` from the `ProjectAirSimAssets/` folder into the scene hierarchy.

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
