# Project AirSim ROS Bridge
{# include enable_internal_docs.tpp #}

Project AirSim can join a ROS1 (Robot Operating System) network using the Project AirSim ROS Bridge.  The bridge is a pure Python ROS1 node that connects to Project AirSim using the Client API.  The bridge is provided as a ROS package and the stand-alone node Python script can be launched using ROS mechanisms such as `rosrun` or `roslaunch` but the script can also be run directly with the Python interpreter.  The core bridge Python modules are provided as a Python package to allow the bridge to be integrated with other client code.

## Requirements
The Project AirSim ROS Bridge requires the following:

* A version of Python supported by the [Project AirSim Client API](../client_setup.md#python-client).
* The following Python packages in addition to those required by the Client API: [rospy](http://wiki.ros.org/rospy), [geometry_msgs](http://wiki.ros.org/geometry_msgs), [radar_msgs](http://wiki.ros.org/radar_msgs), [sensor_msgs](http://wiki.ros.org/sensor_msgs), [std_msgs](http://wiki.ros.org/std_msgs), and [tf](http://wiki.ros.org/tf).

## Setup
1. Setup [Project AirSim Client API](../client_setup.md#python-client).
2. In the same the `client` folder from the Project AirSim distribution, locate the `ProjectAirSim_ROS.zip` compressed archive file and extract the files so that the top-level `ros` directory is a sibling to the
`client` directory of the extracted Project AirSim Client API files.
3. The Project AirSim ROS Bridge is provided as a ROS catkin package, `projectairsim_ros`.  Adding this package to your catkin workspace and building it will ensure dependencies are addressed.  It's recommended to add a soft link in the `catkin_ws/src` directory to the `ros` directory of the extracted Project AirSim distribution files which will add all Project AirSim ROS packages to your ROS catkin workspace.
4. Activate the [Python virtual environment setup for Project AirSim](../client_setup.md).
5. Install the Project AirSim ROS Python library from the `projectairsim_ros-{version}-py3-none-any.whl` file in the distribution:

        python -m pip install path\to\projectairsim_ros-{version}-py3-none-any.whl

	where `version` is the actual version ID.

{# ifdef INTERNAL_DOCS #}
## ROS bridge library in-place install

{# include for_internal_developers.md #}

For convenience, internal developers may wish to install the Project AirSim ROS Bridge Python package (`projectairsim_ros`) "in place" instead of from the Python wheel file.  This allows changes to the package sources to be automatically picked up without having to rebuild and reinstall the wheel each time.  Developers are free to install the wheel, however, if they wish.

Follow the instructions [setup for the Project AirSim ROS Bridge](../ros/ros.md#setup), but instead of installing the *.whl file, do the following:

* If the `projectairsim_ros` package was previously installed from the wheel, uninstall the package first with:
``` bash
    pip uninstall projectairsim_ros
```
* Navigate to the `ros/node` directory and install the package using the `-e` command-line switch:
``` bash
    pip install -e projectairsim_ros
```

{# endif INTERNAL_DOCS #}

## Building
Building the Project AirSim ROS Bridge package is not necessary to use the bridge, although doing so may help ensure dependencies are satisfied. To build the Project AirSim ROS Bridge package, include the Project AirSim ROS Bridge directory `ros` in the ROS catkin workspace and run `catkin_make`.

## Running
The Project AirSim ROS Bridge must be run from within a [Python virtual environment setup for Project AirSim](../client_setup.md).  Once all dependencies are present, launch the Project AirSim ROS Bridge with one of the following ways:
 - Run the main node file with Python:
``` bash
python projectairsim/ros/node/scripts/projectairsim_node.py
```

- Launch by `rosrun`.  Ensure that the Project AirSim `ros/node` directory is in or under a directory in the `ROS_PACKAGE_PATH` environment variable and that the `projectairsim_node.py` is marked executable.  The bridge can then be launched with:
``` bash
rosrun projectairsim_ros projectairsim_node.py
```

- Launch by `roslaunch` from a `.launch` file .  Ensure that the Project AirSim `ros/node` directory is in or under a directory in the `ROS_PACKAGE_PATH` environment variable and that the `projectairsim_node.py` is marked executable.  The bridge can then be launched from a `.launch` file with an entry similar this:
``` xml
    <node pkg="projectairsim_ros" name="projectairsim_node" type="projectairsim_node.py" output="screen"/>
```

**Note:** The Project AirSim ROS Bridge uses the Project AirSim Client API, and currently the Project AirSim ROS Bridge and other client scripts (like flight mission scripts) cannot connect to Project AirSim simultaneously as separate scripts.  For a work-around, see [Running with a mission script](#running-with-a-mission-script), below.

## Command-line arguments

|  Switch  | Value | Default | Description |
| -------- | ------| --------| ----------- |
| <code>&#x2011;&#x2011;address</code> | string | 127.0.0.1 | The IP address of the host running Project AirSim (e.g., <code>&#x2011;&#x2011;address=127.0.0.1</code>). |
| <code>&#x2011;&#x2011;nodename</code> | string | "projectairsim" + random | The name of the Project AirSim ROS Bridge node.  By default, the node uses the name "`projectairsim`" with `rospy_init_node()`'s `anonymous` argument set to `True`.  If this switch is specified, the `anonymous` argument is set to `False`. |
| <code>&#x2011;&#x2011;simconfigpath</code> | string | "sim_config/" | The path to the directory containing the Project AirSim config files.  The path is relative to Project AirSim ROS Bridge node's current working directory. |
| <code>&#x2011;&#x2011;servicesport</code> | integer | 8990 | The Project AirSim services TCP/IP port.  This can be changed by the [-servicesport](../command_line_switches.md#command_line_switches) command-line switch to Project AirSim.
| <code>&#x2011;&#x2011;topicsport</code> | integer | 8989 | The Project AirSim pub-sub TCP/IP port.  This can be changed by the [-topicsport](../command_line_switches.md#command_line_switches) command-line switch to Project AirSim.

## ROS topics and tf transform API
The following ROS topics and transform frames are subscribed to and advertised by the Project AirSim ROS Bridge.  Parts of the topic and frame ID names change depending on the simulation configuration:

| Symbol | Replaced By |
| ------ | ----------- |
| <code>*&lt;scene_name&gt;*</code> | The `id` value in the scene configuration file. |
| <code>*&lt;robot_name&gt;*</code> | The `name` value of an entry in the `actor` section of the [scene configuration file](../config_scene.md#actor-settings). |
| <code>*&lt;camera_name&gt;*</code><br><code>*&lt;lidar_name&gt;*</code><br><code>*&lt;radar_name&gt;*</code> | The `id` value of an entry in the `sensor` section of the [robot configuration file](../config_robot.md#sensor-settings). |
| <code>*&lt;image_type&gt;*</code> | The type of image set by the `image-type` value of an entry in the `capture-settings` array for the camera under the `sensor` section of the [robot configuration file](../config_robot.md#sensor-settings). See [Image type](#image-type), below.|

### Image type
The mapping between the `image-type` value and the *`<image_type>`* value is:
| `image_type` | *<code>&lt;image_type&gt;</code>* |
| :----------: | --------------- |
| 0 | `scene_camera` |
| 1 | `depth_planar_camera` |
| 2 | `depth_camera` |
| 3 | `segmentation_camera` |
| 4 | `depth_vis_camera` |
| 5 | `disparity_normalized_camera` |
| 6 | `surface_normals` |

### Subscribed topics

<code>/ProjectAirSim/node/*&lt;node_name&gt;*/load_scene</code>
([std_msgs/String](http://docs.ros.org/en/api/std_msgs/html/msg/String.html))
<ul><li style="list-style-type: none;">Project AirSim will load a new simulation scene when this topic is set to the name of a scene configuration file.  The scene configuration file must be in the directory specified by the <code>&#x2011;&#x2011;simconfigpath</code> command-line switch.  <b>Warning:</b> All topics containing the scene name are unregistered and new topics are advertised and subscribed to as appropriate for the the new scene.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/cmd_vel</code>
([geometry_msgs/Twist](http://docs.ros.org/en/api/geometry_msgs/html/msg/Twist.html))
<ul><li style="list-style-type: none;">Request to move the robot at the specified linear and angular velocities.  As a safety feature, a message must be published at least every three seconds or the robot will stop movement.  Note the robot may ignore velocity parameters it does not support or cannot support at the moment.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/desired_pose</code>
([geometry_msgs/PoseStamped](http://docs.ros.org/en/api/geometry_msgs/html/msg/PoseStamped.html))
<ul><li style="list-style-type: none;">Request to move the robot to the specified pose.  The robot is immediately placed in the specified pose.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/camera_name&gt;*/desired_pose</code>
([geometry_msgs/PoseStamped](http://docs.ros.org/en/api/geometry_msgs/html/msg/PoseStamped.html))
<ul><li style="list-style-type: none;">Request to move the camera sensor to the specified pose.  The camera is immediately placed in the specified pose.  Note that this topic is on the camera root path and not on child image type paths.  Setting the camera pose affects all image types produced by the camera sensor.  Only available when an actor is configured with a camera sensor.</li></ul>

### Published topics
<code>/airsim_node/*&lt;robot_name&gt;*/actual_pose</code>
([geometry_msgs/PoseStamped](http://docs.ros.org/en/api/geometry_msgs/html/msg/PoseStamped.html))
<ul><li style="list-style-type: none;">Robot's current pose.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/camera_name&gt;*/*&lt;camera_pose</code>
([geometry_msgs/PoseStamped](http://docs.ros.org/en/api/geometry_msgs/html/msg/PoseStamped.html))
<ul><li style="list-style-type: none;">Camera's current pose.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/camera_name&gt;*/*&lt;image_type&gt;*/image</code>
([sensor_msgs/Image](http://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html))
<ul><li style="list-style-type: none;">Images as captured by the camera sensor.  The available image types are: <code>scene_camera</code>, <code>depth_camera</code>, and <code>depth_planar_camera</code>.  This topic is paired with a matching <code>camera_info</code> topic.  Only available when an actor is configured with a camera sensor.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/camera_name&gt;*/*&lt;image_type&gt;*/camera_info</code>
([sensor_msgs/CameraInfo](http://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html))
<ul><li style="list-style-type: none;">Information about the camera image.  The available image types are: <code>scene_camera</code>, <code>depth_camera</code>, and <code>depth_planar_camera</code>.  This topic is paired with a matching <code>image</code> topic.  Only available when an actor is configured with a camera sensor.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/lidar_name&gt;*/lidar</code>
([sensor_msgs/PointCloud2](http://docs.ros.org/en/api/sensor_msgs/html/msg/PointCloud2.html))
<ul><li style="list-style-type: none;">A point cloud of the returns seen by the LIDAR sensor.  Only available when an actor is configured with a LIDAR sensor.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/radar_name&gt;*/radar_detections</code>
([radar_msgs/RadarScan](http://docs.ros.org/en/noetic/api/radar_msgs/html/msg/RadarScan.html))
<ul><li style="list-style-type: none;">Relative directions and velocities of RADAR signal returns.  Only available when an actor is configured with a RADAR sensor.</li></ul>

<code>/airsim_node/*&lt;robot_name&gt;*/radar_name&gt;*/radar_tracks</code>
([radar_msgs/RadarTracks](http://docs.ros.org/en/noetic/api/radar_msgs/html/msg/RadarTracks.html))
<ul><li style="list-style-type: none;">Estimated 3D trajectories of objects detected by the RADAR sensor.  Only available when an actor is configured with a RADAR sensor.</li></ul>

<code>/airsim_streaming_camera/image_raw</code>
<ul><li style="list-style-type: none;"> Currently, only one camera can stream video at a time, but support for multiple simultaneous camera streams is under development. To enable camera streaming, use the following <code>capture-settings</code> configuration: <code>"streaming-enabled": true</code>. If multiple cameras have streaming enabled, you can switch between them using the <code>switch_streaming_view</code>API.  
If you experience low resolution or incorrect image output, it may be due to frame loss in the H.264 stream. Try reducing the [Pixel Streaming transmission frequency](docs/sensors/camera_streaming.md#setting-the-maximum-fps-for-pixel-streaming) to mitigate the issue.</li></ul>

### Broadcasted transform frames
<code>airsim_node/*&lt;robot_name&gt;*</code>
<ul><li style="list-style-type: none;">Robot's base transform frame from the <code>map</code> parent frame ID.</li></ul>

<code>airsim_node/*&lt;robot_name&gt;*/camera_name&gt;*/*&lt;image_type&gt;*</code>
<ul><li style="list-style-type: none;">The camera image's transform frame from the <code>map</code> parent frame ID.</li></ul>

### Coordinate conversion
The bridge handles the conversion between data representations used by ROS and Project AirSim, both based on the NED (North-East-Down: right-handed coordinates, X-forward, Y-right, Z-down) coordinate system.

For instance, the `actual_pose` ROS topic reports the vehicle’s position and orientation in NED coordinates, and the Project AirSim Client API also provides the vehicle’s position in NED coordinates. Similarly, when setting the pose of the vehicle through the `desired_pose` ROS topic, you specify the pose in NED coordinates, and the bridge ensures that this data is correctly passed to Project AirSim.

## Running with a mission script
The Project AirSim ROS Bridge uses the Project AirSim Client API.  Currently, Project AirSim only supports one client at a time thus precluding other clients such as flight mission scripts from connecting to an Project AirSim instance already connected to an Project AirSim ROS Bridge.

The work-around is to combine the Project AirSim ROS Bridge with the mission script so that they can share a single `ProjectAirSimClient` instance and use the Python `asyncio` facility to run both the bridge and the mission script at the same time.  The `projectairsimnode.py` script does little more than create an instance of the `ProjectAirSimWrapper` class and enter the ROS main loop.  The `ProjectAirSimWrapper` class implements all of the functionality of the Project AirSim ROS Bridge.

The example script `hello_ros.py` in the `ros/node/scripts` directory shows one way to launch the ROS node with a mission script.  To run this example:
1. Launch Project AirSim.
2. Open a command prompt or terminal.
3. Activate the Python virtual environment created for Project AirSim clients as discussed in the [Project AirSim client setup](../client_setup.md) page.
4. Ensure `roscore` is running and, if necessary, that the `ROS_MASTER_URI` environment variable is set correctly.
5. Use the `cd` command to go to the `ros/node/scripts` directory.
6. Run the script with the following command:
``` bash
python hello_ros.py
```

The script will load a simulation scene and fly the drone from the launch pad to the orange ball, back to the launch pad, and repeat as long as the script is running.  Meanwhile the Project AirSim ROS Bridge node will have connected to the ROS network and the `rostopic list` command will show all of the topics it has made available.

 Here's a step-by-step breakdown of how `hello_ros.py` works:
 - After parsing the command line, a ROS node is created using `rospy.init_node()`.
 - The logger used by the Project AirSim client library is redirected through the ROS logger.  Ordinarily, ROS redirects all non-ROS logger output to a file.  See **Note 1**.
 - An `ProjectAirSimClient` object is created and connected to the running instance of Project AirSim.
 - Project AirSim is directed to load the scene config file `scene_drone_sensors.jsonc`.  The bridge looks for simulation config files in the directory specified by the `--simconfigpath` command-line switch which defaults to `../../../client/python/example_user_scripts/sim_config`.  Project AirSim will load the scene which uses the `Blocks` environment with one drone.
 - Next the `ProjectAirSimWrapper` object is created&mdash;this object runs the Project AirSim ROS Bridge node. The `ProjectAirSimClient` object we created earlier is passed-in so the constructor won't create its own.  For convenience, you can instead allow it to create the `ProjectAirSimClient` object and then retrieve it--see **Note 2**.
 - Now that the simulation environment and the ROS node are setup and running, the script launches the `main()` function to run asynchronously and enters the `rospy` main loop with `rospy.spin()`.  The `main()` function runs the mission script and the `asyncio` facility allows it to run in parallel with the ROS node.
 - `main()` creates an asynchronous task for the mission script.  The mission code flies the drone in a continuous pattern, repeating forever.  Otherwise it is very similar to the other Project AirSim Python client script examples, except that the client and world objects are created externally and passed-in.
- `main()` checks once a second for ROS being shutdown (by receiving the Ctrl-C signal is most common).  When `rospy.is_shutdown()` returns `True`, `main()`  cancels the mission task--that task receives a CancelledError exception and exits.  `main()` then disconnects the `AirSimVNetClient` object from Project AirSim and also exits.  It's important that `main()` uses `await` in between calls to `rospy.is_shutdown()`--see **Note 3**.
- In the meanwhile, the `ProjectAirSimWrapper` object has also been notified that ROS is being shutdown.  It stops its ROS node activities and frees its resources.  The call to `rospy.spin()` in the main Python thread then returns and the main Python thread also exits.  When all threads have exited, the app closes.

**Notes:**
1. By employing a custom filter, the `_redirecting_log_filter()` function, the standard logger used by Project AirSim is redirected to the `rosout.projectairsim` logger.  It is a child of the `rosout` logger and with the default ROS logging configuration it sends logging output from Project AirSim to the command prompt/terminal in addition to the log file.

2. When `ProjectAirSimWrapper` creates the `ProjectAirSimClient` object, it's available from the `projectairsim_client` attribute.  With it, you can load a scene by creating a `World` object, then calling `ProjectAirSimWrapper.update_topics()` so that it can update the node's topics to match the scene.  This is slightly less efficient since `ProjectAirSimWrapper` will have automatically done this once already when it was created.  If `ProjectAirSimWrapper` creates the client, it will disconnect it when ROS is shutdown.  If you pass in a client, it will *not* disconnect it automatically so you can do so when appropriate.

3. The Python `asyncio` facility uses cooperative multitasking.    The calls to `await asyncio.sleep()` allows other asychronous tasks like the mission task to run while `main()` sleeps.  The `await` statements in the mission script (or any other `asyncio` task) won't proceed while another task (like `main()`) is running without relenquishing control by, for instance, calling `await` statements.  Newly created tasks may not even start.

## Examples

Samples of using the Project AirSim ROS Bridge are provided in the `ros/examples` directory.  See [Project AirSim ROS Bridge Examples](ros_examples.md) for more information.

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
