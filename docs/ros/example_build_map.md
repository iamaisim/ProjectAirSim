# Project AirSim ROS Bridge Examples: build_map

`build_map` demonstrates using the Project AirSim with the ROS MoveIt! motion package and the RViz visualizer to create a 3D occupancy map.  This map indicates the parts of the world that are already occupied so no part of the robot may also occupy those areas.  MoveIt's 3D occupancy map uses the ROS OctoMap library which stores voxels in a 3D manner using a eight-way tree structure (an "octree".)

In this example, the Project AirSim robot is configured with a LIDAR sensor and the 3D point cloud from the sensor is fed to MoveIt!'s Depth Image Octomap Updater plug-in to update the OctoMap in real time.

The robot is moved automatically using a "mission script".  At the end of the movements, the occupancy map is retrieved from MoveIt! and saved to a file.

## Requirements
The `build_map` example requires the following:

* A version of Python supported by the [Project AirSim Client API](../client_setup.md#python-client).
* The following Python packages in addition to those required by the Project AirSim ROS Bridge: [moveit](http://wiki.ros.org/moveit) and[moveit_msgs](http://wiki.ros.org/moveit_msgs).

## Setup
See the [setup for the Project AirSim ROS Bridge Examples](ros_examples.md#setup).

## Building
See the [building the Project AirSim ROS Bridge Examples](ros_examples.md#building).

## Running
1. Activate the [Python virtual environment for Project AirSim](../client_setup.md).
2. Run Project AirSim.
3. Run the example with:
``` bash
    roslaunch projectairsim_ros_examples build_map.launch
```
4.  RViz will run to show the robot and the OctoMap as it is created.  The robot will automatically launch, fly a path, return, and land.  The example will then save the OctoMap to a file exit.

## ROS parameters
The main launch file, `build_map.launch`, accepts several ROS arguments to help run the example in your environment.  To pass an argument from the command-line, use the syntax "<i>argument</i><code>:=</code><i>value</i>" in the `roslaunch` command.  For instance, to specify the IP address where AirSim is running:
``` bash
    roslaunch projectairsim_ros_examples build_map.launch airsim_ip_address:=192.168.0.1
```


|  Argument  | Value | Default | Description |
| ---------- | ------| --------| ----------- |
| <code>airsim_ip_address</code> | string | 127.0.0.1 | The IP address of the host running Project AirSim (e.g., <code>airsim_ip_address:=127.0.0.1</code>). |
| <code>sim_config_path</code> | string | (`projectairsim_ros_examples` package dir)../../../client/python/example_user_scripts/sim_config | The path to the directory containing the Project AirSim config files. |

## Technical description
This example uses MoveIt!'s Depth Image Octomap Updater plug-in to build the 3D occupancy map.

The `build_map.py` script creates the Project AirSim ROS Bridge node and loads the scene config file "scene_drone_sensors.jsonc" which configures the robot with multiple sensors including LIDAR.  The script also runs a "mission script" to fly the robot around and exits when the drone returns to its launch point.

The configuration file `config/sensors_lidar.yaml` adds the PointCloud Octomap Updater plug-in to the MoveIt! group and points the plug-in to the robot's LIDAR sensor data topic published by the Project AirSim ROS Bridge.  This topic publishes PointCloud2 messages supported by the Octomap Updater plug-in.  As the robot flies around, the Updater plug-in processes the LIDAR data and marks voxels in the occupancy map as occupied wherever a LIDAR point is within 100 meters of the vehicle.

For performance reasons the occupany map is configured with a resolution of 1.0 meter per voxel in `sensor_manager.launch.xml`.

In the meanwhile, RViz is running with the MoveIt! MotionPlanning plugin to display the drone and the occupancy map as it is updated.

When `build_map.py` is done flying the vehicle, it uses the OctoHandler class in `common/octomap_handler.py` to retrieve the occupancy map from MoveIt! and save it to a file.  It does this by subscribing to the `/move_group/monitored_planning_scene` topic for one update.  When the update message is received, it serializes the octomap in the message using the Python pickle module and saves it to a file.

The `build_map.py` node is marked "required" in the primary ROS launch file `build_map.launch`.  When `build_map.py` completes the mission script and exits, `roslaunch` automatically shuts down the rest of the nodes.

## 3rd party attributions

`example_externals/moveit_simple_controller_manager` and `example_externals/action_controller` are derived from work by Benoit Courty, Alessio Tonioni, and Wel Selby.  The original work may be found at https://github.com/benoit-cty/ROS-Autonomous-Quadcopter-Flight, and https://github.com/wilselby/ROS_quadrotor_simulator.

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
