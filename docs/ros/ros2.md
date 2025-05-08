# ROS2 Setup for developers(WIP)


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
