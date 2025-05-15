# ROS2 Setup for developers

Project AirSim can join a ROS2 network using the Project AirSim ROS2 Bridge. The bridge is a pure Python ROS2 node that connects to Project AirSim using the Client API. It is provided as a Python package and can be run as a ROS2 node or directly with the Python interpreter. This bridge is compatible with the Pixel Streaming infrastructure and supports camera, lidar, radar, and pose topics.

### 1. Setup python client(venv)

[Setup python client](../client_setup.md)

### 2. Install ROS(humble)
How to install [ros_humble](https://docs.ros.org/en/humble/Installation.html)
...

Install [radar_msgs](http://wiki.ros.org/radar_msgs)
```
sudo apt-get install ros-humble-radar-msgs
```
...
Build and activate projectairsim-interfaces(custom message/service types)
```
cd ./ros/node 
colcon build
. install/setup.bash
```


### 3. Install bridge
In a virtual environment, install `projectairsim-rosbridge` and `projectairsim-ros2`

```
cd ./ros/node/
pip install -e projectairsim-rosbridge
pip install -e projectairsim-ros2
```

### 4. Activate node
Executing `projectairsim_bridge_ros2.py` will initialize the ros2 node that will start listening and broadcasting topics. 
In a virtual environment, 
```
python ./scripts/ros2/projectairsim_bridge_ros2.py --simconfigpath PATH_TO_SIM_CONFIGS --address <IPv4_server_address>
```

### 5. Publish topic messages

In a separate terminal window:
```
source /opt/ros/humble/setup.bash
```

#### To get a list of available topics:
```
ros2 topic list
```

#### To load a new scene config
This will make available new topics, relevant to scene.
```
ros2 service call /airsim_node/load_scene projectairsim_ros/srv/LoadScene "{scene_file: 'scene_basic_drone.jsonc', is_primary_client: 1}"
```
NODENAME changes. You can find NODENAME in the topic list.

### 6. Activate video streaming node

Executing 'webrtc_node.py' will initialize the ros2 node that will connect with pixel streaming video server.

```
ros2 run projectairsim_ros webrtc_node.py
```

### ROS2 Topics and TF Frames

#### Subscribed Topics
- `/airsim_node/<robot_name>/cmd_vel` ([geometry_msgs/Twist](http://docs.ros.org/en/api/geometry_msgs/html/msg/Twist.html))
- `/airsim_node/<robot_name>/desired_pose` ([geometry_msgs/PoseStamped](http://docs.ros.org/en/api/geometry_msgs/html/msg/PoseStamped.html))
- `/airsim_node/<robot_name>/<camera_name>/desired_pose` ([geometry_msgs/PoseStamped](http://docs.ros.org/en/api/geometry_msgs/html/msg/PoseStamped.html))
- `/airsim_node/load_scene` ([projectairsim_ros/srv/LoadScene])

#### Published Topics
- `/airsim_node/<robot_name>/actual_pose` ([geometry_msgs/PoseStamped])
- `/airsim_node/<robot_name>/<camera_name>/<image_type>/image` ([sensor_msgs/Image])
- `/airsim_node/<robot_name>/<camera_name>/<image_type>/camera_info` ([sensor_msgs/CameraInfo])
- `/airsim_node/<robot_name>/<lidar_name>/lidar` ([sensor_msgs/PointCloud2])
- `/airsim_node/<robot_name>/<radar_name>/radar_detections` ([radar_msgs/RadarScan])
- `/airsim_node/<robot_name>/<radar_name>/radar_tracks` ([radar_msgs/RadarTracks])
- `/airsim_streaming_camera/image_raw` (H.264 stream forwarding)

#### TF Broadcasted Frames
- `airsim_node/<robot_name>`: robot base frame from `map`
- `airsim_node/<robot_name>/<camera_name>/<image_type>`: camera image frame from `map`

#### Image Types Mapping
| image_type ID | Name                     |
|---------------|--------------------------|
| 0             | scene_camera             |
| 1             | depth_planar_camera      |
| 2             | depth_camera             |
| 3             | segmentation_camera      |
| 4             | depth_vis_camera         |
| 5             | disparity_normalized_camera |
| 6             | surface_normals          |
