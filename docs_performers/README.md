<!--
Copyright (c) Microsoft Corporation.
Licensed under the MIT License.
-->
# Project AirSim
For additional support, please file an [issue](https://github.com/microsoft/ProjectAirSim/issues) or post to the program support Slack [channel](https://app.slack.com/client/T05L71S22LE/C05MWA9P1RT).

## How To Guide

## Using Docker
### Basic configuration
#### Pull AirSim from the registry container
```
docker pull [NAME]:[TAG]
```
_Example_
```
docker pull hameritt.azurecr.io/project_airsim_str:v1.9.6-Neighborhood
```
#### Display docker images (confirmation of image repository, tage, IMAGE ID, and creation timestamp)
```
docker images
```
#### Allow root user access to the running X server
```
xhost +local:root
```
#### Run AirSim
```
docker run --network host --gpus=all -v "/tmp/.X11-unix:/tmp/.X11-unix:rw" -e DISPLAY [NAME]:[TAG]
```
_Example_
```
docker run --network host --gpus=all -v "/tmp/.X11-unix:/tmp/.X11-unix:rw" -e DISPLAY hameritt.azurecr.io/project_airsim_str:v1.9.6-Neighborhood
```
#### Run AirSim in headless mode
```
docker run --network host --gpus=all -v "/tmp/.X11-unix:/tmp/.X11-unix:rw" -e DISPLAY [NAME]:[TAG] --unrealoptions="-RenderOffscreen"
```
_Example_
```
docker run --network host --gpus=all -v "/tmp/.X11-unix:/tmp/.X11-unix:rw" -e DISPLAY hameritt.azurecr.io/project_airsim_str:v1.9.6-Neighborhood --unrealoptions="-RenderOffscreen"
```
#### Display docker containers (confirmation of containers and status; note the target CONTAINER ID)
```
docker ps -a
```
#### Connect to terminal session of the target CONTAINER ID
```
docker exec -it [CONTAINER ID] bash
```
### Advanced configuration
#### Display docker containers (confirmation of containers and status; note the target CONTAINER ID)
```
docker ps -a
```
#### Stop CONTAINER ID (stop AirSim)
```
docker stop [CONTAINER ID]
```
#### Start CONTAINER ID (start AirSim)
```
docker start [CONTAINER ID]
```
#### Remove CONTAINER ID (stop the container first)
```
docker rm [CONTAINER ID]
```
#### Display docker images (confirmation of image repository, tage, IMAGE ID, and creation timestamp)
```
docker images
```
#### Remove docker image (stop and remove associated docker containers firt)
```
docker rmi [IMAGE ID]
```
### Create and save occupancy grid
To create and save an occupancy grid, you can use the following ROS service:
```shell
ros2 service call /airsim_node/create_occupancy_grid projectairsim_ros/srv/OccupancyGrid "{position_x: 0.0, position_y: 0.0, position_z: -2.0, ncells_x: 1000, ncells_y: 1000, res: 1.0, n_z_resolution: 1}"
```
To save the occupancy grid:
```shell
ros2 service call /map_saver/save_map nav2_msgs/srv/SaveMap "{map_topic: '/occupancy_grid', map_url: '/home/airsim_user/ProjectAirSim/ros/node/my_map', image_format: 'pgm', map_mode: 'trinary', free_thresh: '0.25', occupied_thresh: '0.65'}"
```
### Create and save (segmented) binvox data
To create segmented voxel grid data:
```shell
ros2 service call /airsim_node/create_segmented_voxel_grid projectairsim_ros/srv/CreateVoxelGrid "{position_x: 0.0, position_y: 0.0, position_z: -20.0, ncells_x: 100, ncells_y: 100, ncells_z: 100, resolution: 1, n_z_resolution: 1, output_file: '/home/airsim_user/ProjectAirSim/ros/node/maps/segmentedvoxelgrid.binvox'}"
```
### Create a pedestrian actor
Sample code to create a pedestrian actor:
```python
from projectairsim import ProjectAirSimClient, World, EnvActor

client = ProjectAirSimClient()
client.connect()
world = World(client, "scene_env_actor_human.jsonc", delay_after_load_sec=2)
env_actor = EnvActor(client, world, "pedestrian")
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/env_actors_human.py
### Create a car actor
Sample code to create a car actor:
```python
from projectairsim import ProjectAirSimClient, World, EnvActor

client = ProjectAirSimClient()
client.connect()
world = World(client, "scene_env_actor_car.jsonc", delay_after_load_sec=2)
env_actor = EnvActor(client, world, "car1")
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/env_actors_cars.py
### Author a trajectory for car or pedestrian actor
Sample code to author a trajectory:
```python
def set_env_car_traj():
    # Read CSV file and import the trajectory
    time = []
    pose_x = []
    pose_y = []
    pose_z = []
    pose_yaw = []
    with open('car_path.csv', 'r') as f:
        next(f)
        for line in f:
            line = line.split(',')
            pose_x.append(float(line[0]))
            pose_y.append(float(line[1]))
            pose_z.append(float(line[2]) * -1)
            pose_yaw.append(float(line[3]))
            time.append(float(line[4]))

    pose_roll = [0] * len(time)
    pose_pitch = [0] * len(time)

    world.import_ned_trajectory("traj1", time, pose_x, pose_y, pose_z, pose_roll, pose_pitch, pose_yaw)
    env_actor.set_trajectory("traj1", to_loop=True)
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/env_actors_cars.py
### Set weather
Sample code to set weather effects:
```python
world.enable_weather_visual_effects()
world.set_weather_visual_effects_param(param=WeatherParameter.SNOW, value=0.9)
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/weather.py
### Set time of day
Sample code to set time of day:
```python
world.set_time_of_day(
    status=True,
    datetime="2021-09-20 17:00:00",
    is_dst=False,
    clock_speed=5.0,
    update_interval=0.5,
    move_sun=True,
)
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/time_of_day.py

### Set actor IDs (list annotations) for camera detection
This can be configured in the camera settings:
```json
{
    "camera_name": "DownCamera",
    "image_type": 0,
    "pixels_as_float": false,
    "compress": true,
    "annotations": {
        "object_id": "TemplateCube_Rounded_6"
    }
}
```
### Using move on path service
Sample code to use the move on path service in python:
```python
def test_case_move_on_path(order, max_retries=5):
    order.append('test_case_move_on_path')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=30.0, y=0.0, z=-30.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose2 = PoseStamped()
        pose2.header = Header()
        pose2.header.frame_id = 'world'
        pose2.pose = Pose()
        pose2.pose.position = Point(x=-15.0, y=15.0, z=-50.0)
        pose2.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose3 = PoseStamped()
        pose3.header = Header()
        pose3.header.frame_id = 'world'
        pose3.pose = Pose()
        pose3.pose.position = Point(x=10.0, y=0.0, z=-25.0)
        pose3.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose4 = PoseStamped()
        pose4.header = Header()
        pose4.header.frame_id = 'world'
        pose4.pose = Pose()
        pose4.pose.position = Point(x=0.0, y=0.0, z=-8.0)
        pose4.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1, pose2, pose3, pose4]
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 45.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MoveOnPath response is None" )
        else:
            LOGGER.warning( f"MoveOnPath {response.success}" )

        retries += 1

    assert response != None
    assert response.success == True
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/move_apis.py

To use the move on path service, you can use the following ROS service:
```shell
ros2 service call /airsim_node/Drone1/move_on_path projectairsim_ros/MoveOnPath "{path: [{header: {stamp: {sec: 0, nanosec: 0}, frame_id: 'world'}, pose: {position: {x: 20.0, y: 0.0, z: -20.0}, orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}}}], velocity: 10.0, timeout_sec: 30, lookahead: -1.0, adaptive_lookahead: 1.0, drive_train_type: 0, yaw_is_rate: true, yaw: 0.0, wait_on_last_task: true}"
```
This will command the drone to move along the specified path with a given velocity.
### Use the APIs to query segmentation information (color, seg_ID, object_ID)
You can query segmentation information using the following services:
- Get the RGB color given a mesh ID:
  ```shell
  ros2 service call /airsim_node/get_color_from_mesh projectairsim_ros/srv/GetColorFromMeshId "{object_id: 'outer_wall_quart_win_7'}"
  ```
- Get the RGB color given a segmentation ID:
  ```shell
  ros2 service call /airsim_node/get_color_from_seg_id projectairsim_ros/srv/GetColorFromSegId "{segmentation_id: 3}"
  ```
- Get the mesh name associated with a segmentation ID:
  ```shell
  ros2 service call /airsim_node/get_mesh_from_seg_id projectairsim_ros/srv/GetMeshIdsFromSegId "{segmentation_id: 95}"
  ```
- Get the segmentation ID associated with a mesh ID:
  ```shell
  ros2 service call /airsim_node/get_seg_id_from_mesh projectairsim_ros/srv/GetSegIdFromMeshId "{object_id: 'outer_wall_quart_win_7'}"
  ```
- Get the segmentation ID associated with a color:
  ```shell
  ros2 service call /airsim_node/get_seg_id_from_color projectairsim_ros/srv/GetSegIdFromColor "{r: '43', g: '47', b: '206'}"
  ```
### Create a smoke actor
Sample code to create a smoke actor:
```python
from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log

if __name__ == "__main__":
    client = ProjectAirSimClient()
    try:
        client.connect()
        world = World(client, "scene_env_objects_fire_and_smoke.jsonc", delay_after_load_sec=2)
    except Exception as e:
        projectairsim_log("Error occurred: {}".format(e))
        client.disconnect()
        exit(1)
    finally:
        client.disconnect()
        projectairsim_log("Disconnected from the simulation environment")
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/fire_detection.py
### Create a fire actor
Similar to creating a smoke actor, you can use the same script above to create a fire actor by ensuring the scene configuration file includes fire actors.
### Using drone controls commands
#### Arm
```shell
ros2 service call /airsim_node/Drone1/arm projectairsim_ros/srv/Arm
```
#### Arm group
```shell
ros2 service call /airsim_node/arm_group projectairsim_ros/srv/ArmGroup "{vehicle_names: ['Drone1', 'Drone2']}"
```
#### Disarm
```shell
ros2 service call /airsim_node/Drone1/disarm projectairsim_ros/srv/Disarm
```
#### Disarm group
```shell
ros2 service call /airsim_node/disarm_group projectairsim_ros/srv/DisarmGroup "{vehicle_names: ['Drone1', 'Drone2']}"
```
#### Land
```shell
ros2 service call /airsim_node/Drone1/land projectairsim_ros/srv/Land
```
#### Land group
```shell
ros2 service call /airsim_node/land_group projectairsim_ros/srv/LandGroup "{vehicle_names: ['Drone1', 'Drone2']}"
```
#### Takeoff
```shell
ros2 service call /airsim_node/Drone1/takeoff projectairsim_ros/srv/Takeoff
```
#### Takeoff group
```shell
ros2 service call /airsim_node/takeoff_group projectairsim_ros/srv/TakeoffGroup "{vehicle_names: ['Drone1', 'Drone2']}"
```
#### Move to position
```python
    move_to_position_task = await drone.move_to_position_async(
       north=10, east=-5, down=-10, velocity=3
    )
    await move_to_position_task
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/move_apis.py
#### Move on path
```python
    path = [[10, -5, -10], [13, 0, -12], [6, -8, -5]]
    move_on_path_task = await drone.move_on_path_async(path=path, velocity=3)
    await move_on_path_task
```
Example: ~/ProjectAirSim/client/python/example_user_scripts/move_apis.py
