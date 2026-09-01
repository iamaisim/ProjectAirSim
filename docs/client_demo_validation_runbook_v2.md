# Client Demo Validation Runbook V2

Use this runbook to build/package an environment, build the Docker image, and
run simple LiDAR demo scenes with a live visualizer.

## Assumptions

- Run host commands from the repository root.
- The environment package and Docker image are built by following
  [`docker/README.md`](../docker/README.md).
- Docker, NVIDIA Container Toolkit, GPU access, and X11 display forwarding are
  available.
- Each run uses one scene config at a time. Stop the script before starting
  the next run.
- Replace `<tag>` with the Docker image tag printed by `docker/build_image.sh`.

## 1. Build, Package, and Build Docker Image

Follow [`docker/README.md`](../docker/README.md):

- Package the Unreal environment.
- Build the Docker image from the packaged environment.
- Note the final image name and tag.

For Blocks, the final image is usually:

```bash
projectairsim-blocks:<tag>
```

## 2. Prepare Terminals

Before starting, stop and prune all Docker containers:

```bash
docker ps -aq | xargs -r docker stop && docker container prune -f
```

Use two terminals:

- Terminal 1: runs the Docker container and visible Blocks simulator.
- Terminal 2: runs visualizer commands inside the container.

In both host terminals:

```bash
cd {REPO_PATH}/ProjectAirSim
export HOST_REPO="$PWD"
export IMAGE="projectairsim-blocks:<tag>"
export DEMO_CONTAINER="projectairsim-lidar-demo"
```

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
  -e XDG_RUNTIME_DIR=/tmp/runtime-projectairsim \
  -e SDL_AUDIODRIVER=dummy \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$HOST_REPO/docker-logs:/workspace" \
  -v "$HOST_REPO:/workspace/repo" \
  "$IMAGE" run-env
```

In Terminal 2:

```bash
docker exec -it "$DEMO_CONTAINER" bash -lc '
export REPO=/workspace/repo
export PYTHONPATH="$REPO/client/python/projectairsim/src:${PYTHONPATH:-}"
export PYTHONDONTWRITEBYTECODE=1
cd "$REPO/client/python/example_user_scripts"
bash --noprofile --norc
'
```

## Explanation of following tests
### Test environment
![Release v2 test environment](./gm_releases/release_v2_test_env.png)

Four cylinders surrounded by a wall. Two cylinders have overlap/non-blocking visibility geometry and depth-stencil-id=13. Since CPU LiDAR ignores the overlap/non-blocking components, and GPU LiDAR ignores depth-stencil-id=13 because of the configuration we're using in these tests, we should see a different behabior in these two cylinders with respect to the other ones.

Two scripts are going to be used. Both visualize + allow controlling the drone to move in the scene.
- `gpu_lidar_live_visualizer.py`: Visualizer for first and second returns. Used for both GPU and CPU LiDAR. It'll show in green the first return points, and in purple the second points.
- `lidar_intensity_factor_visualizer.py`: Intensity visualizer for both first and second-return points.

Controls for scripts: arrows move, 8/2 up/down, 4/6 yaw, q/esc quit.

The commands below start the drone with `takeoff_async()` before opening the
visualizer, so it begins flying at the default takeoff hover height, about 2 m
above the ground.


## 3. GPU LiDAR, One Return

Scene config expectations:

- Drone with `lidar-type` set to `gpu_cylindrical`.
- `number-of-returns` set to `1`.
- No intensity factor table.

In Terminal 2:

```bash
python3 gpu_lidar_live_visualizer.py \
  --scene-config gpu_lidar_returns_1_scene.jsonc \
  --move-to-capture-pose \
  --forward-seconds 0
```

Expected result: the scene loads and the visualizer shows first-return points in green.
Since there's no return 2, it just ignores the cylinders with depth stencil = 13, as configured by the jsonc file.

![GPU LiDAR one return](./gm_releases/gpu_1_return.png)

## 4. GPU LiDAR, Two Returns

Scene config expectations:

- Drone with `lidar-type` set to `gpu_cylindrical`.
- `number-of-returns` set to `2`.
- GPU hidden stencil values configured for the second return.
- No intensity factor table.

In Terminal 2:

```bash
python3 gpu_lidar_live_visualizer.py \
  --scene-config gpu_lidar_returns_2_scene.jsonc \
  --move-to-capture-pose \
  --forward-seconds 0
```

Expected result: the visualizer shows both first and second return, in green and purple correspondingly.

![GPU LiDAR two returns](./gm_releases/gpu_2_return.png)

## 5. GPU LiDAR, Two Returns, Intensity Factor

Scene config expectations:

- Same as the GPU two-return scene.
- Intensity factor table configured.

In Terminal 2:

```bash
python3 lidar_intensity_factor_visualizer.py \
  --scene-config gpu_lidar_returns_2_intensity_scene.jsonc \
  --intensity-min 0   --intensity-max 2.5 \
  --move-to-capture-pose \
  --forward-seconds 0
```

Expected result: the visualizer shows both return points with
published intensity values.

![GPU LiDAR intensity](./gm_releases/gpu_intensity.png)

## 6. CPU LiDAR, One Return

Scene config expectations:

- Drone with `lidar-type` set to `generic_cylindrical`.
- `number-of-returns` set to `1`.
- No intensity factor table.

In Terminal 2:

```bash
python3 gpu_lidar_live_visualizer.py \
  --scene-config cpu_lidar_returns_1_scene.jsonc \
  --move-to-capture-pose \
  --forward-seconds 0
```

Expected result: the scene loads and the visualizer shows first-return points in green.

![CPU LiDAR one return](./gm_releases/cpu_1_return.png)

## 7. CPU LiDAR, Two Returns

Scene config expectations:

- Drone with `lidar-type` set to `generic_cylindrical`.
- `number-of-returns` set to `2`.
- No intensity factor table.

In Terminal 2:

```bash
python3 gpu_lidar_live_visualizer.py \
  --scene-config cpu_lidar_returns_2_scene.jsonc \
  --move-to-capture-pose \
  --forward-seconds 0
```

Expected result: the visualizer shows both return points, using green for the first-return points and purple for the second-return points.

![CPU LiDAR two returns](./gm_releases/cpu_2_return.png)

## 8. CPU LiDAR, Two Returns, Intensity Factor

Scene config expectations:

- Same as the CPU two-return scene.
- Intensity factor table configured.

In Terminal 2:

```bash
python3 lidar_intensity_factor_visualizer.py \
  --scene-config cpu_lidar_returns_2_intensity_scene.jsonc \
  --intensity-min 0   --intensity-max 2.5 \
  --move-to-capture-pose \
  --forward-seconds 0
```

Expected result: the visualizer shows both return points with
published intensity values.

![CPU LiDAR intensity](./gm_releases/cpu_intensity.png)

## Cleanup

Close the visualizer window, then stop the Docker container with `Ctrl+C` in
Terminal 1.
