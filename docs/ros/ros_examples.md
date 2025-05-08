# Project AirSim ROS Bridge Examples

Examples of using the Project AirSim ROS Bridge as part of a ROS network are included in the Project AirSim distribution under the `ros/examples` directory.

## Requirements
The Project AirSim ROS Bridge examples requires the following:

* A version of Python supported by the [Project AirSim Client API](../client_setup.md#python-client).
* The following Python packages in addition to those required by the Project AirSim ROS Bridge: [moveit](http://wiki.ros.org/moveit) and[moveit_msgs](http://wiki.ros.org/moveit_msgs).

## Setup
1. Setup the [Project AirSim ROS Bridge](ros.md#setup).
2. The files for Project AirSim ROS Bridge examples are in the `ros/examples` and `ros/examples_externals` directory of the distribution files and are provided as the ROS catkin package `projectairsim_ros_examples`.  These directories must be added to your ROS catkin workspace and and built.  It's recommended to add a soft link in the `catkin_ws/src` directory to the `ros` directory of the extracted Project AirSim distribution files which will add all Project AirSim ROS packages to your ROS catkin workspace.
4. Activate the [Python virtual environment setup for Project AirSim](../client_setup.md).

## Building
To build the Project AirSim ROS Bridge Examples package run `catkin_make`.

## Running
The Project AirSim ROS Bridge Examples must be run from within a [Python virtual environment setup for Project AirSim](../client_setup.md).

## Individual Examples

[build_map](example_build_map.md): Creates a 3D occupancy map (octomap) using Project AirSim, the MoveIt! motion library, and RViz.

[navigate_map](example_navigate_map.md): Navigate the 3D occupancy map constructed by [build_map](example_build_map.md) using Project AirSim, the MoveIt! motion library, and RViz.

## Simple Takeoff and land example (services)
```shell
ros2 service call /airsim_node/Drone1/takeoff projectairsim_ros/srv/Takeoff
```
The drone takes off in the simulator
```shell
ros2 service call /airsim_node/Drone1/land projectairsim_ros/srv/Land
```
The drone lands.

## TakeoffGroup and LandGroup example (services)
Can be tested with scene_two_drones.jsonc

```shell
ros2 service call /airsim_node/takeoff_group projectairsim_ros/srv/TakeoffGroup "{vehicle_names: ['Drone1', 'Drone2']}"
```
The drone takes off in the simulator
```shell
ros2 service call /airsim_node/land_group projectairsim_ros/srv/LandGroup "{vehicle_names: ['Drone1', 'Drone2']}"
```
The drone lands.


## Arm and Disarm Services
```shell
ros2 service call /airsim_node/Drone1/arm projectairsim_ros/srv/Arm
```
The drone takes off in the simulator
```shell
ros2 service call /airsim_node/Drone1/disarm projectairsim_ros/srv/Disarm
```

## ArmGroup and DisarmGroup Services
```shell
ros2 service call /airsim_node/arm_group projectairsim_ros/srv/ArmGroup  "{vehicle_names: ['Drone1', 'Drone2']}"
```
The drone takes off in the simulator
```shell
ros2 service call /airsim_node/disarm_group projectairsim_ros/srv/DisarmGroup  "{vehicle_names: ['Drone1', 'Drone2']}"


## Working with Mesh and Segmentation IDs Services
Note: Mesh ID and Object ID are the same thing

Get the RGB color given a mesh ID
```shell
ros2 service call /airsim_node/get_color_from_mesh projectairsim_ros/srv/GetColorFromMeshId "{object_id: 'TemplateCube_Rounded_6'}"
```

Get the RGB color given a segmentation ID
```shell
ros2 service call /airsim_node/get_color_from_seg_id projectairsim_ros/srv/GetColorFromSegId "{segmentation_id: 3}"
```

Get the mesh name associated with a segmentation ID
```shell
ros2 service call /airsim_node/get_mesh_from_seg_id projectairsim_ros/srv/GetMeshIdsFromSegId "{segmentation_id: 95}"
```

Get the segmentation ID associated with a mesh ID
```shell
ros2 service call /airsim_node/get_seg_id_from_mesh projectairsim_ros/srv/GetSegIdFromMeshId "{object_id: 'TemplateCube_Rounded_6'}"
```

Get the segmentation ID associated with a color
```shell
ros2 service call /airsim_node/get_seg_id_from_color projectairsim_ros/srv/GetSegIdFromColor "{r: '43', g: '47', b: '206'}"
```

## Create Voxel Grid Service
```shell
ros2 service call /airsim_node/create_voxel_grid projectairsim_ros/srv/CreateVoxelGrid \
"{position_x: 0.0,position_y: 0.0,position_z: -20.0,ncells_x: 100,ncells_y: 100,ncells_z: 100,resolution: 0.5,n_z_resolution: 2,output_file: '/home/airsim_user/ProjectAirSim/ros/node/maps/voxelgrid.binvox'}"
```
The resolution of the grid in the z axis is resolution * n_z_resolution (In the example 0.5 m * 2 = 1 m)

## Create Segmented Voxel Grid service
```shell
ros2 service call /airsim_node/create_segmented_voxel_grid projectairsim_ros/srv/CreateVoxelGrid \
"{position_x: 0.0,position_y: 0.0,position_z: -20.0,ncells_x: 100,ncells_y: 100,ncells_z: 100,resolution: 1,n_z_resolution: 1,output_file: '/home/airsim_user/ProjectAirSim/ros/node/maps/segmentedvoxelgrid.binvox'}"
```

The resolution of the grid in the z axis is resolution * n_z_resolution

## Create Occupancy Grid Service
```shell
ros2 service call /airsim_node/create_occupancy_grid projectairsim_ros/srv/OccupancyGrid "{position_x: 0.0,position_y: 0.0,position_z: -2.0,ncells_x: 1000,ncells_y: 1000,res: 1.0, n_z_resolution: 1}"
```

The resolution of the grid in the z axis is resolution * n_z_resolution

## Save Occupancy Grid Service
```shell
ros2 service call /map_saver/save_map nav2_msgs/srv/SaveMap "{map_topic: '/occupancy_grid', map_url: '/home/airsim_user/ProjectAirSim/ros/node/maps/my_map', image_format: 'pgm', map_mode: 'trinary', free_thresh: '0.25', occupied_thresh: '0.65'}"
```

## MoveOnPath Service
```shell
ros2 service call /airsim_node/Drone1/move_on_path projectairsim_ros/srv/MoveOnPath \ "{
path: [
{
header: {
stamp: {
sec: 0,
nanosec: 0
},
frame_id: 'world'
},
pose: {
position: {
x: 0.0,
y: 0.0,
z: -10.0
},
orientation: {
x: 0.0,
y: 0.0,
z: 0.0,
w: 1.0
}
}
},
{
header: {
stamp: {
sec: 0,
nanosec: 0
},
frame_id: 'map'
},
pose: {
position: {
x: 50.0,
y: 10.0,
z: -2.0
},
orientation: {
x: 0.0,
y: 0.0,
z: 0.707,
w: 0.707
}
}
}
],
velocity: 1.5,
timeout_sec: 30,
lookahead: -1.0,
adaptive_lookahead: 1.0,
drive_train_type: 1,
yaw_is_rate: true,
yaw: 0.0, 
}"
```

## MoveToPosition
```shell
ros2 service call /airsim_node/Drone1/move_to_position projectairsim_ros/srv/MoveToPosition \ "{x: 0.0, y: 0.0, z: -10.0, velocity: 1.5, lookahead: -1.0, adaptive_lookahead: 1.0}"
```

## MoveOnPath Action
```shell
ros2 action send_goal /airsim_node/Drone1/move_on_path projectairsim_ros/action/MoveOnPath \ "{
path: [
{
header: {
stamp: {
sec: 0,
nanosec: 0
},
frame_id: 'world'
},
pose: {
position: {
x: 0.0,
y: 0.0,
z: -10.0
},
orientation: {
x: 0.0,
y: 0.0,
z: 0.0,
w: 1.0
}
}
},
{
header: {
stamp: {
sec: 0,
nanosec: 0
},
frame_id: 'map'
},
pose: {
position: {
x: 50.0,
y: 10.0,
z: -2.25
},
orientation: {
x: 0.0,
y: 0.0,
z: 0.5,
w: 0.707
}
}
}
],
velocity: 10,
timeout_sec: 30,
lookahead: -1.0,
adaptive_lookahead: 1.0,
drive_train_type: 1,
yaw_is_rate: true,
yaw: 0.0, 
}"
---


---

Copyright (C) Microsoft Corporation.  All rights reserved.
