# Client Demo Runbook: One Scene, SUV, PX4 Drone, ROS2, and GPU LiDAR

Use this runbook for a client-facing demonstration that the expected features
work from one running Project AirSim scene:

- Vehicle Integration
- SUV Trajectory Following via ROS2
- PX4 Drone Takeoff
- ROS2 Bridge
- Drone Sensor Visualization in RViz
- Drone GPU LiDAR Functional Validation

The demo uses one long-running Project AirSim client process for ROS2 checks:
the ROS2 C++ bridge node. A short Python loader script loads the scene first and
exits before the bridge starts. Do not run a second bridge node or a scene-loading
launch file during this demo.

## Assumptions

- The Docker image has already been built from the packaged Blocks environment.
  If the ROS2 bridge source changed, rebuild the image before running the demo;
  otherwise Docker may still contain the old bridge binary. The loader, scene
  configs, and RViz config are read from the mounted repository in this runbook.
- Docker, NVIDIA Container Toolkit, GPU access, and X11 display forwarding are
  available on the demo machine.
- PX4 SITL runs locally on the host with the Iris airframe.
- ROS2 is used only inside the Docker container. The host terminal does not need
  native ROS2 setup.
- The combined client demo scene uses Project AirSim `engine-driven` clock mode
  so the Unreal packaged environment drives simulation time.
- The scene contains `UnrealVehicle` and PX4-controlled `Drone1`.
- `Drone1` starts forward of the SUV so it is visible in the simulator viewport
  after scene load.
- The SUV has a forward-looking third-person `SuvChaseCamera` for the simulator
  viewport.
- Drone sensors, not car sensors, are used for RViz and GPU LiDAR validation.

Run all host commands from the repository root unless a step says otherwise.
Replace `<tag>` with the image tag produced by `docker/build_image.sh`.

## 0. Prepare Terminals

Use five terminals for the full demo:

- Terminal 0: runs local PX4 SITL on the host.
- Terminal 1: runs the Docker container and visible Blocks simulator.
- Terminal 2: runs exactly one command, the ROS2 C++ bridge client.
- Terminal 3: Docker exec shell for the Python scene loader, then ROS2 topic
  checks and service calls.
- Terminal 4: Docker exec shell for standalone RViz.

Terminal 4 is optional if RViz is not being shown. Do not use Terminal 4 to
launch another bridge.

In each host terminal except the PX4 terminal:

```bash
cd "$(git rev-parse --show-toplevel)"
export HOST_REPO="$PWD"
export IMAGE="projectairsim-blocks:<tag>"
export DEMO_CONTAINER="projectairsim-client-demo"
```

## 1. Start Local PX4 SITL

In Terminal 0, on the host, start PX4 before loading the Project AirSim scene:

```bash
cd /path/to/PX4-Autopilot
make px4_sitl none_iris
```

Expected result: PX4 prints that it is waiting for the simulator to connect on
TCP port `4560`.

Leave PX4 running for the whole demo. If you stop the Blocks simulator, restart
PX4 before starting another PX4 flight session.

## 2. Launch Packaged Blocks from Docker

In Terminal 1:

```bash
mkdir -p "$HOST_REPO/docker-logs"
xhost +local:docker

docker run --rm -it \
  --name "$DEMO_CONTAINER" \
  --gpus all \
  --network host \
  -e NVIDIA_DRIVER_CAPABILITIES=all \
  -e PROJECTAIRSIM_HEADLESS=0 \
  -e DISPLAY="$DISPLAY" \
  -e PYTHONDONTWRITEBYTECODE=1 \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$HOST_REPO/docker-logs:/workspace" \
  -v "$HOST_REPO:/workspace/repo" \
  "$IMAGE" run-env
```

The `--network host` argument is required for this demo: the containerized Blocks
environment connects to local PX4 on `127.0.0.1:4560`, and PX4 uses UDP port
`14540` for offboard/API traffic.

Expected result: the Blocks simulation window opens with no loaded demo scene
yet.

Leave this container running for the whole demo.

## 3. Open Docker Command Shells

In Terminal 2, Terminal 3, and Terminal 4, open a shell inside the running
container:

```bash
docker exec -it "$DEMO_CONTAINER" bash -lc '
source /opt/ros/humble/setup.bash
source /opt/projectairsim/ros/install/setup.bash
export REPO=/workspace/repo
export SIM_CONFIG="$REPO/client/python/example_user_scripts/sim_config"
export PYTHONPATH="$REPO/client/python/projectairsim/src:${PYTHONPATH:-}"
export PYTHONDONTWRITEBYTECODE=1
cd /workspace
bash --noprofile --norc
'
```

All remaining commands run inside one of these Docker exec shells unless a step
explicitly says "host terminal".


## 4. Validate CPU vs GPU LiDAR performance improvement

Because the ROS node cannot currently coexist with Python scripts, we will first validate the improvement introduced by GPU lidar. Its performance improves compared to CPU lidar the more points per second it captures.
The
[`lidar_compare_cpu_gpu.py`](../client/python/example_user_scripts/lidar_compare_cpu_gpu.py)
example loads equivalent CPU (`generic_cylindrical`) and GPU
(`gpu_cylindrical`) scenes sequentially and checks that both implementations
produce non-empty point-cloud frames consistently under equivalent sensor
settings.

Run this as a standalone check from the repository root while the Project
AirSim Unreal application is running. The script reloads the simulator scene,
so run it separately from the combined client demonstration workflow above.

In Terminal 3, inside the Docker exec shell:
```bash
cd "$REPO"
python client/python/example_user_scripts/lidar_compare_cpu_gpu.py
```

By default, the script captures for up to 5 seconds and collects a maximum of
10 frames from each sensor. It prints the point count for every captured frame:

```text
CPU: 10 frames, 22,652 total points
GPU: 10 frames, 30,282 total points

Mode: frame-by-frame (10 frames)

 Frame   Points CPU   Points GPU
----------------------------------
     0         2577         3365
```

Expected result:

- Both implementations continuously produce non-empty point-cloud frames.
- Point counts remain within a consistent order of magnitude across the
  capture instead of dropping to zero or changing without bound.
- The CPU and GPU averages provide a concise consistency check across all
  captured frames.

Exact point-count equality is not expected. CPU and GPU LiDAR use different
capture and publication mechanisms, and frames are paired by arrival order
rather than by a shared timestamp or scan azimuth. This check is intended to
detect missing, empty, or clearly inconsistent output, not require identical
point counts.

## 5. Load the Demo Scene, Then Launch the ROS2 Bridge

Use the Python loader once to load the scene. The loader exits before the bridge
starts, so the C++ bridge is the only long-running Project AirSim client during
the ROS2 checks.

In Terminal 3, inside the Docker exec shell:

```bash
cd "$REPO/client/python/example_user_scripts"
python3 load_client_demo_validation_scene.py
```

Expected result:

- The scene `scene_client_demo_validation.jsonc` loads.
- The simulator window shows one scene containing `UnrealVehicle` and `Drone1`.
- `Drone1` is initialized forward of the SUV and visible in the viewport.
- The simulator viewport uses the SUV third-person `SuvChaseCamera`.
- Three cube markers are visible at the planned SUV `MoveOnPath` waypoints.
- PX4 reports simulator connection and home/EKF initialization in Terminal 0.
- The loader prints `Client demo validation scene is ready for ROS2 bridge`.

After the Python loader exits, launch the ROS2 C++ bridge in Terminal 2 and
leave it running until cleanup.

In Terminal 2, inside the Docker exec shell:

```bash
ros2 run projectairsim_ros2_cpp projectairsim_ros2_cpp_node \
  --ros-args \
  -p sim_config_path:="$SIM_CONFIG" \
  -p vehicle_name:=UnrealVehicle \
  -p publish_clock_period_sec:=0.0 \
  -p refresh_topics_period_sec:=0.0
```

Expected result:

- The bridge prints
  `Attached to current Project AirSim scene /Sim/SceneClientDemoValidation`.
- The bridge registers typed SUV services under `/projectairsim/UnrealVehicle/...`.
- The bridge registers shared services such as `/projectairsim/takeoff_group`.
- The bridge republishes discovered scene topics under
  `/ProjectAirsim/SceneClientDemoValidation/...`.

## 6. Show ROS2 Topics and Services

In Terminal 3:

```bash
ros2 service list -t | grep projectairsim
ros2 topic list -t | grep SceneClientDemoValidation
```

Expected topics and services include:

```text
/ProjectAirsim/SceneClientDemoValidation/robots/UnrealVehicle/actual_pose
/ProjectAirsim/SceneClientDemoValidation/robots/Drone1/actual_pose
/ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneForwardCamera/scene_camera
/ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneGpuLidar/lidar
/projectairsim/takeoff_group
/projectairsim/UnrealVehicle/move_on_path
/projectairsim/UnrealVehicle/move_on_path_service
```

Echo one SUV pose sample and one drone pose sample:

```bash
ros2 topic echo --once \
  /ProjectAirsim/SceneClientDemoValidation/robots/UnrealVehicle/actual_pose
```

```bash
ros2 topic echo --once \
  /ProjectAirsim/SceneClientDemoValidation/robots/Drone1/actual_pose
```

Expected result: each command prints one `geometry_msgs/msg/PoseStamped`
message.

## 6. Show Drone Sensors in Standalone RViz

Keep the ROS2 bridge from Step 4 running. Do not use
`projectairsim_rviz.launch.py`, because that launch file starts another bridge
client. RViz alone is a ROS visualization process and does not load or own a
Project AirSim scene.

In Terminal 4, inside the Docker exec shell:

```bash
rviz2 -d "$REPO/ros/projectairsim_ros2_cpp/rviz/client_demo_validation.rviz"
```

Expected result: RViz opens with displays bound to `Drone1`:

```text
Drone Pose:
/ProjectAirsim/SceneClientDemoValidation/robots/Drone1/actual_pose

Drone GPU LiDAR:
/ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneGpuLidar/lidar

Drone Forward Camera:
/ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneForwardCamera/scene_camera
```

The forward camera display should show the drone's own forward view, so the demo
can follow what the PX4 drone sees during takeoff.

## 8. View ROS2 Topic Rates

In Terminal 3, run each command for a few seconds, then press `Ctrl-C`:

```bash
ros2 topic hz \
  /ProjectAirsim/SceneClientDemoValidation/robots/Drone1/actual_pose
```

```bash
ros2 topic hz \
  /ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneForwardCamera/scene_camera
```

```bash
ros2 topic hz \
  /ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneGpuLidar/lidar
```

Expected result:

- Drone pose, forward camera, and GPU LiDAR topics publish continuously.
- The displayed average rates are nonzero.

## 9. Demonstrate SUV Trajectory Following via ROS2

Keep the ROS2 bridge from Step 4 running.

In Terminal 3, send a `MoveOnPath` service request:

```bash
ros2 service call /projectairsim/UnrealVehicle/move_on_path_service \
  projectairsim_ros2_cpp/srv/MoveOnPath \
  "{path: [
    {pose: {position: {x: -485.0, y: 0.0, z: 0.0}, orientation: {w: 1.0}}},
    {pose: {position: {x: -450.0, y: 0.0, z: 0.0}, orientation: {w: 1.0}}},
    {pose: {position: {x: -380.0, y: 20.0, z: 0.0}, orientation: {w: 1.0}}}
  ],
  velocity: 2.0,
  timeout_sec: 120.0,
  drive_train_type: 0,
  yaw_is_rate: true,
  yaw: -1.0,
  lookahead: -1.0,
  adaptive_lookahead: 1.0,
  wait_on_last_task: false}"
```

Expected result:

- The service response reports `success: true` and returns immediately.
- The bridge terminal prints `Received MoveOnPath service request for UnrealVehicle with 3 waypoints`.
- The SUV moves through the requested path in the same scene.
- RViz continues showing `Drone1` sensors from the same bridge while the SUV
  trajectory runs.

To show the SUV pose changing while it moves:

```bash
ros2 topic echo \
  /ProjectAirsim/SceneClientDemoValidation/robots/UnrealVehicle/actual_pose
```

Press `Ctrl-C` after several pose samples.

## 10. Demonstrate PX4 Drone Takeoff

Keep PX4, the simulator, the same scene, RViz, and the ROS2 bridge running.

In Terminal 3, command `Drone1` takeoff:

```bash
ros2 service call /projectairsim/takeoff_group \
  projectairsim_ros2_cpp/srv/TakeoffGroup \
  "{vehicle_names: ['Drone1'], wait_on_last_task: true}"
```

Expected result:

- The service response reports `success: true`.
- PX4 arms, takes off, and stabilizes.
- RViz shows the drone pose moving and the forward camera view changing.

## 11. Demonstrate Drone GPU LiDAR Functional Validation

Keep the same simulator container, scene, PX4 instance, RViz, and ROS2 bridge
running. The GPU LiDAR sensor is attached to `Drone1` in
`SceneClientDemoValidation`.

Verify that the GPU LiDAR topic is live from Terminal 3:

```bash
ros2 topic echo --once \
  /ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneGpuLidar/lidar
```

```bash
ros2 topic hz \
  /ProjectAirsim/SceneClientDemoValidation/robots/Drone1/sensors/DroneGpuLidar/lidar
```

Expected result:

- `echo --once` prints one `sensor_msgs/msg/PointCloud2` message.
- `topic hz` reports a nonzero PointCloud2 publish rate.
- RViz shows a live point cloud from `Drone1` in the same scene and same bridge.


## Client-Facing Evidence Checklist

Use this checklist during the demo:

| Expected capability | What the client should see |
|---|---|
| Vehicle Integration | The Python loader loads one combined scene, the C++ bridge attaches to it, the SUV and PX4 drone appear in Blocks, and both poses are available through ROS2. |
| ROS2 Bridge | Docker exec shells run `ros2 topic list`, `ros2 service list`, `ros2 topic echo`, and `ros2 topic hz` against live bridge data. |
| Sensor Visualization in RViz | Standalone RViz shows live `Drone1` pose, forward camera, and GPU LiDAR data from `SceneClientDemoValidation`. |
| SUV Trajectory Following via ROS2 | `UnrealVehicle` `move_on_path_service` returns success and the SUV follows the requested path. |
| PX4 Drone Takeoff | `Drone1` takes off through PX4 while RViz shows its forward view. |
| GPU LiDAR Functional Validation | The same bridge publishes live `Drone1` GPU PointCloud2 data and ROS2 topic commands show nonzero LiDAR output. |

## Cleanup

Close RViz or stop terminal processes with `Ctrl-C`.

From a host terminal:

```bash
docker exec "$DEMO_CONTAINER" pkill -f projectairsim_ros2_cpp_node || true
docker exec "$DEMO_CONTAINER" pkill -f rviz2 || true
docker stop "$DEMO_CONTAINER"
```

In the PX4 terminal, press `Ctrl-C` or type `shutdown`.
