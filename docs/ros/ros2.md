# Project AirSim ROS2 C++ Bridge

Project AirSim provides a ROS2 C++ bridge package named
`projectairsim_ros2_cpp`. The package is located under:

```text
ros/projectairsim_ros2_cpp
```

The bridge is an `rclcpp` node that connects to Project AirSim through the
Project AirSim C++ client API, discovers simulator topics, republishes supported
payloads as typed ROS2 messages, and exposes command services/actions.

## Requirements

The bridge has been built and tested with ROS2 Humble. Source your ROS2
installation before building or running the node:

```bash
source /opt/ros/humble/setup.bash
```

The package depends on:

| Dependency | Used for |
|---|---|
| `ament_cmake` | ROS2 CMake package build |
| `rosidl_default_generators` | Custom service/action generation |
| `rclcpp` | ROS2 C++ node |
| `rclcpp_action` | `MoveOnPath` action server |
| `geometry_msgs` | poses and path goals |
| `nav_msgs` | occupancy grid service responses |
| `rosgraph_msgs` | simulation clock topic |
| `sensor_msgs` | sensor topics |
| `std_msgs` | topic info and raw JSON fallback topics |
| `tf2_ros` | TF broadcasting |

## Build

From the repository root:

```bash
source /opt/ros/humble/setup.bash
cd ros
colcon build --packages-select projectairsim_ros2_cpp
source install/setup.bash
```

## Test

The ROS2 C++ package includes C++ contract tests for the generated topics,
services, and action interfaces.

```bash
source /opt/ros/humble/setup.bash
cd ros
colcon test --packages-select projectairsim_ros2_cpp
colcon test-result --all --verbose
```

## Run

Start Project AirSim first. Then, from `ros`:

```bash
source /opt/ros/humble/setup.bash
source install/setup.bash

ros2 run projectairsim_ros2_cpp projectairsim_ros2_cpp_node --ros-args \
  -p scene_config:=scene_drone_sensors.jsonc \
  -p sim_config_path:=../client/python/example_user_scripts/sim_config
```

If Project AirSim is not reachable at `127.0.0.1`, set `address` to the host
running the simulator:

```bash
ros2 run projectairsim_ros2_cpp projectairsim_ros2_cpp_node --ros-args \
  -p address:=<projectairsim_host_ip> \
  -p scene_config:=scene_drone_sensors.jsonc \
  -p sim_config_path:=../client/python/example_user_scripts/sim_config
```

If a scene is already loaded and should not be changed by the bridge, omit
`scene_config`. In that mode the node connects to the existing simulator scene,
adopts the discovered scene topic root, and starts topic and clock publishing
when Project AirSim reports topic information.

## Parameters

| Parameter | Default | Description |
|---|---|---|
| `address` | `127.0.0.1` | Project AirSim server address. Set this when the simulator runs on another host or network namespace. |
| `port_topics` | `8989` | Project AirSim pub-sub topic port. |
| `port_services` | `8990` | Project AirSim service request port. |
| `scene_config` | empty | Scene config file to load at startup. Relative names are resolved against `sim_config_path`. When empty, the bridge connects to the currently loaded scene. |
| `sim_config_path` | `client/python/example_user_scripts/sim_config` | Directory containing scene, robot, and environment config files. |
| `delay_after_load_sec` | `2.0` | Delay after loading a scene before topic discovery. |
| `projectairsim_topic_root` | `/Sim` | Native Project AirSim topic root to rewrite. |
| `ros_topic_root` | `/ProjectAirsim` | ROS topic root used when publishing bridged topics. |
| `publish_unmatched_as_json` | `false` | Publish unsupported Project AirSim topics as `std_msgs/String` JSON. |
| `publish_tf` | `true` | Broadcast TF transforms from `/actual_pose` and camera pose payloads. |
| `tf_world_frame_id` | `map` | Parent frame used for TF transforms. |
| `refresh_topics_period_sec` | `0.0` | Periodic topic discovery interval. `0.0` disables polling; use a positive value only if scenes/topics can change outside this node. |
| `vehicle_name` | `Drone1` | Vehicle used for single-drone services/actions. |
| `service_root` | `/projectairsim` | Root namespace for command services and actions. |

The simulator publishes `/Sim/<scene>/clock` from the scene tick. The bridge
subscribes to that native clock stream and republishes every sample on ROS
`/clock`; it does not poll `GetSimTime`. Sensor message headers likewise retain
their native per-sample `time_stamp` values. If a sensor payload has no valid
`time_stamp`, the bridge warns and publishes the conventional
invalid/uninitialized ROS timestamp (`sec: 0`, `nanosec: 0`) instead of
substituting the bridge's current time. ROS time fields are integers, so they
cannot represent NaN.

## Topics

List ROS2 topics:

```bash
ros2 topic list -t
```

The bridge subscribes to native Project AirSim topics such as `/Sim/...` and republishes them under `/ProjectAirsim/...` by default.

Robot state topics use the vehicle name as `frame_id`; camera image and
camera-info topics use a unique sanitized camera frame so they can participate
in TF without colliding with the vehicle frame.

Implemented typed topic conversions:

| Project AirSim topic suffix | ROS2 topic suffix | ROS2 topic type |
|---|---|---|
| `/actual_pose` | `/actual_pose` | `geometry_msgs/msg/PoseStamped` |
| `/gps` | `/gps` | `sensor_msgs/msg/NavSatFix` |
| `/imu`, `/imu_kinematics` | `/imu` | `sensor_msgs/msg/Imu` |
| `/barometer` | `/barometer` | `sensor_msgs/msg/FluidPressure` |
| `/magnetometer` | `/magnetometer` | `sensor_msgs/msg/MagneticField` |
| `/lidar` | `/lidar` | `sensor_msgs/msg/PointCloud2` |
| `/radar_detections` | `/radar_detections` | `projectairsim_ros2_cpp/msg/RadarScan` |
| `/radar_tracks` | `/radar_tracks` | `projectairsim_ros2_cpp/msg/RadarTracks` |
| camera topics | same suffix | `sensor_msgs/msg/Image` |
| `*_camera_info` | `<camera_topic>/camera_info` | `sensor_msgs/msg/CameraInfo` |

The bridge also publishes:

| Topic | Type | Description |
|---|---|---|
| `/projectairsim/topic_info` | `std_msgs/msg/String` | JSON list of Project AirSim topic paths from the first discovery pass after startup or scene load. |
| `/clock` | `rosgraph_msgs/msg/Clock` | Native Project AirSim scene-clock topic, republished without service polling. |
| `/tf` | `tf2_msgs/msg/TFMessage` | Vehicle and camera transforms when `publish_tf=true`. |

To echo a topic:

```bash
ros2 topic echo /ProjectAirsim/<scene_name>/robots/<vehicle_name>/actual_pose
```

If you want raw JSON fallback topics for unsupported Project AirSim topics:

```bash
ros2 run projectairsim_ros2_cpp projectairsim_ros2_cpp_node --ros-args \
  -p publish_unmatched_as_json:=true
```

## Services

List services:

```bash
ros2 service list -t
```

Single-vehicle services use the configured `vehicle_name`, default `Drone1`:

| Service | Type |
|---|---|
| `/projectairsim/Drone1/takeoff` | `projectairsim_ros2_cpp/srv/Takeoff` |
| `/projectairsim/Drone1/land` | `projectairsim_ros2_cpp/srv/Land` |
| `/projectairsim/Drone1/arm` | `projectairsim_ros2_cpp/srv/Arm` |
| `/projectairsim/Drone1/disarm` | `projectairsim_ros2_cpp/srv/Disarm` |
| `/projectairsim/Drone1/move_to_position` | `projectairsim_ros2_cpp/srv/MoveToPosition` |
| `/projectairsim/Drone1/move_on_path` | `projectairsim_ros2_cpp/srv/MoveOnPath` |

Global, group, and scene services:

| Service | Type |
|---|---|
| `/projectairsim/request` | `projectairsim_ros2_cpp/srv/RawRequest` |
| `/projectairsim/get_client_info` | `projectairsim_ros2_cpp/srv/GetClientInfo` |
| `/projectairsim/get_topic_info` | `projectairsim_ros2_cpp/srv/GetTopicInfo` |
| `/projectairsim/get_drones` | `projectairsim_ros2_cpp/srv/GetDrones` |
| `/projectairsim/publish` | `projectairsim_ros2_cpp/srv/Publish` |
| `/projectairsim/unsubscribe` | `projectairsim_ros2_cpp/srv/Unsubscribe` |
| `/projectairsim/cancel_all_requests` | `projectairsim_ros2_cpp/srv/CancelAllRequests` |
| `/projectairsim/takeoff_group` | `projectairsim_ros2_cpp/srv/TakeoffGroup` |
| `/projectairsim/land_group` | `projectairsim_ros2_cpp/srv/LandGroup` |
| `/projectairsim/arm_group` | `projectairsim_ros2_cpp/srv/ArmGroup` |
| `/projectairsim/disarm_group` | `projectairsim_ros2_cpp/srv/DisarmGroup` |
| `/projectairsim/reset` | `projectairsim_ros2_cpp/srv/Reset` |
| `/projectairsim/load_scene` | `projectairsim_ros2_cpp/srv/LoadScene` |
| `/projectairsim/create_voxel_grid` | `projectairsim_ros2_cpp/srv/CreateVoxelGrid` |
| `/projectairsim/occupancy_grid` | `projectairsim_ros2_cpp/srv/OccupancyGrid` |
| `/projectairsim/get_clock` | `projectairsim_ros2_cpp/srv/GetClock` |
| `/projectairsim/get_origin_geo_point` | `projectairsim_ros2_cpp/srv/GetOriginGeoPoint` |
| `/projectairsim/set_segmentation_id_by_name` | `projectairsim_ros2_cpp/srv/SetSegmentationIDByName` |
| `/projectairsim/get_segmentation_id_by_name` | `projectairsim_ros2_cpp/srv/GetSegmentationIDByName` |
| `/projectairsim/get_segmentation_id_map` | `projectairsim_ros2_cpp/srv/GetSegmentationIDMap` |
| `/projectairsim/set_object_material` | `projectairsim_ros2_cpp/srv/SetObjectMaterial` |
| `/projectairsim/set_object_texture` | `projectairsim_ros2_cpp/srv/SetObjectTexture` |
| `/projectairsim/swap_object_texture` | `projectairsim_ros2_cpp/srv/SwapObjectTexture` |

Examples:

```bash
ros2 service call /projectairsim/Drone1/arm projectairsim_ros2_cpp/srv/Arm \
  "{wait_on_last_task: true}"
```

```bash
ros2 service call /projectairsim/Drone1/takeoff projectairsim_ros2_cpp/srv/Takeoff \
  "{wait_on_last_task: true}"
```

```bash
ros2 service call /projectairsim/Drone1/move_to_position projectairsim_ros2_cpp/srv/MoveToPosition \
  "{x: 5.0, y: 0.0, z: -3.0, velocity: 2.0, timeout_sec: 10.0, drive_train_type: 0, yaw_is_rate: true, yaw: 0.0, lookahead: -1.0, adaptive_lookahead: 1.0, wait_on_last_task: true}"
```

```bash
ros2 service call /projectairsim/Drone1/land projectairsim_ros2_cpp/srv/Land \
  "{wait_on_last_task: true}"
```

Reload a scene at runtime:

```bash
ros2 service call /projectairsim/load_scene projectairsim_ros2_cpp/srv/LoadScene \
  '{scene_file: "scene_drone_sensors.jsonc", is_primary_client: true}'
```

Use `is_primary_client: true` when this bridge should ask Project AirSim to load
the scene. Use `is_primary_client: false` when another client has already loaded
the scene and this bridge should attach to it.

Create a voxel grid file:

```bash
ros2 service call /projectairsim/create_voxel_grid projectairsim_ros2_cpp/srv/CreateVoxelGrid \
  '{position_x: 0.0, position_y: 0.0, position_z: -4.0, ncells_x: 20, ncells_y: 20, ncells_z: 10, resolution: 1.0, n_z_resolution: 10, output_file: "/tmp/projectairsim_voxel_grid.binvox"}'
```

Return a ROS2 occupancy grid. The 3D voxel grid is projected into a 2D
`nav_msgs/OccupancyGrid`; a cell is occupied if any voxel in its Z column is
occupied.

```bash
ros2 service call /projectairsim/occupancy_grid projectairsim_ros2_cpp/srv/OccupancyGrid \
  "{position_x: 0.0, position_y: 0.0, position_z: -4.0, ncells_x: 20, ncells_y: 20, ncells_z: 10, res: 1.0, n_z_resolution: 10}"
```

Get simulation time in nanoseconds:

```bash
ros2 service call /projectairsim/get_clock projectairsim_ros2_cpp/srv/GetClock "{}"
```

Get the scene origin geo point:

```bash
ros2 service call /projectairsim/get_origin_geo_point projectairsim_ros2_cpp/srv/GetOriginGeoPoint "{}"
```

Call any Project AirSim C++ client request-backed API by method path and JSON
parameters:

```bash
ros2 service call /projectairsim/request projectairsim_ros2_cpp/srv/RawRequest \
  '{method: "/Sim/GetBuildCommitHash", json_parameters: "{}"}'
```

`/projectairsim/request` is a generic escape hatch for Project AirSim server
request paths. Prefer typed ROS2 services when one exists. For generic calls,
use a method path and JSON payload supported by the Project AirSim server.

## Actions

The bridge provides a `MoveOnPath` action:

| Action | Type |
|---|---|
| `/projectairsim/Drone1/move_on_path` | `projectairsim_ros2_cpp/action/MoveOnPath` |

List actions:

```bash
ros2 action list -t
```

Send a path goal:

```bash
ros2 action send_goal /projectairsim/Drone1/move_on_path projectairsim_ros2_cpp/action/MoveOnPath \
  "{path: [{pose: {position: {x: 5.0, y: 0.0, z: -3.0}, orientation: {w: 1.0}}}, {pose: {position: {x: 5.0, y: 5.0, z: -3.0}, orientation: {w: 1.0}}}], velocity: 2.0, timeout_sec: 20.0, lookahead: -1.0, adaptive_lookahead: 1.0, drive_train_type: 0, yaw_is_rate: true, yaw: 0.0, wait_on_last_task: true}"
```

## Common Workflow

Use three terminals. Run the ROS2 commands from the repository root unless
noted otherwise:

1. Start Project AirSim.
2. Start the bridge:

```bash
source /opt/ros/humble/setup.bash
cd ros
source install/setup.bash
ros2 run projectairsim_ros2_cpp projectairsim_ros2_cpp_node --ros-args \
  -p scene_config:=scene_drone_sensors.jsonc \
  -p sim_config_path:=../client/python/example_user_scripts/sim_config
```

3. Inspect and command from ROS2:

```bash
source /opt/ros/humble/setup.bash
cd ros
source install/setup.bash
ros2 topic list -t
ros2 service list -t
ros2 action list -t
```

Then call services such as arm, takeoff, move, and land.

## Resolved Limitations And Remaining Gaps

The ROS2 C++ bridge provides typed ROS2 coverage for the high-priority areas
below:

| Area | Status |
|---|---|
| Radar typed messages | Implemented as `projectairsim_ros2_cpp/msg/RadarScan` and `RadarTracks`. |
| Camera info topics | Implemented as `<camera_topic>/camera_info` with `sensor_msgs/msg/CameraInfo`. |
| TF broadcasting | Implemented for vehicle pose and camera pose payloads; controlled by `publish_tf`. |
| Segmentation and object material/texture services | Implemented as typed ROS2 services. |

Remaining gaps are narrower typed wrappers for Project AirSim request paths that
currently require `/projectairsim/request`, plus richer typed conversions for
specialized sensor/status payloads that do not yet have dedicated ROS message
contracts.

---

Copyright (C) IAMAI CONSULTING CORP

MIT License. All rights reserved.
