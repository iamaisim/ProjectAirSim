# Project AirSim ROS Bridge Examples: navigate_map

`navigate_map` demonstrates using the Project AirSim with the ROS MoveIt! motion package and the RViz visualizer to navigate a 3D occupancy map.  This map indicates the parts of the world that are already occupied so no part of the robot may also occupy those areas.  MoveIt's 3D occupancy map uses the ROS OctoMap library which stores voxels in a 3D manner using a eight-way tree structure (an "octree".)

In this example, the occupancy created by the [build_map](example_build_map.md) is loaded back into MoveIt! and RViz displays the map, the drone, and a destination target widget.  After the user moves the target widget to the desired destination, MoveIt! is commanded via the MoveIt! RViz plug-in to create a movement plan (avoiding any obstacles) and then execute the plan moving the robot to the destination.

## Requirements
The `navigate_map` example requires the following:

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
    roslaunch projectairsim_ros_examples navigate_map.launch
```
4. RViz will run to show the robot, a destination target widget, and the OctoMap that was previously created by [build_map](example_build_map.md).  The robot will automatically launch and wait.
5. Using the mouse, move the target widget to a location on the map.  By default the target must be within 50 meters of the current robot location, but this can be changed by setting the Workspace Size in the RViz MotionPlanning plug-in, Context tab.  Note that setting too large of a value may impact performance.
6. In the MotionPlanning plug-in, Planning tab, click "Plan".  MoveIt! will create a motion plan and an animated "ghost" robot will move along the plan in the RViz display.
7. Click "Execute" and the robot will move along the planned path in both RViz and the Project AirSim chase cam view.
8. To exit the example, close RViz and click "Close without Saving".

## ROS parameters
The main launch file, `navigate_map.launch`, accepts several ROS arguments to help run the example in your environment.  To pass an argument from the command-line, use the syntax "<i>argument</i><code>:=</code><i>value</i>" in the `roslaunch` command.  For instance, to specify the IP address where AirSim is running:
``` bash
    roslaunch projectairsim_ros_examples navigate_map.launch airsim_ip_address:=192.168.0.1
```


|  Argument  | Value | Default | Description |
| ---------- | ------| --------| ----------- |
| <code>airsim_ip_address</code> | string | 127.0.0.1 | The IP address of the host running Project AirSim (e.g., <code>airsim_ip_address:=127.0.0.1</code>). |
| <code>sim_config_path</code> | string | (`projectairsim_ros_examples` package dir)../../../client/python/example_user_scripts/sim_config | The path to the directory containing the Project AirSim config files. |


## Technical description
MoveIt! is primarly designed for controlling the movement of components (e.g., arms, effectors, sensors) attached to a robot base through linkages and joints and not generally for the movement of entire vehicles.  To use MoveIt! for vehicle movement, the robot semantic description, `config/quad.srdf`, declares a floating "virtual joint" (named `virtual_joint`) attached from the `world` coordinate frame to `base_link`, the robot's base transform frame.

To have the virtual joint track the robot in the Project AirSim simulation scene, the primary launch script (`navigate_map.launch`) uses a `static_transform_publisher` node to link `base_link` to the Project AirSim robot's transform frame (broadcast by the Project AirSim ROS Bridge) with a null offset.  Since MoveIt's fixed coordinate frame is `world` while Project AirSim's is `map`, `navigate_map.launch` creates another `static_transform_publisher` node to link `map` to `world`, also with a null offset.  In this way, a change in the Project AirSim's robot transform causes `base_link` (and `virtual_joint`) to move in a corresponding way relative to MoveIt's `world` coordinate frame.

`navigate_map.launch` also runs the `navigate_map.py` script which creates the Project AirSim ROS Bridge node and loads the simulation scene config file `scene_drone_sensors.jsonc` which configures the robot with multiple sensors including LIDAR.  `navigate_map.py` also runs a short "mission script" to enable and launch the robot into the air, but unlike the `build_map.py` script, `navigate_map.py` continues running the bridge node even after the mission script completes.

On startup `navigate_map.py` loads the occupancy map created by the `build_map` example using the OctoHandler class in `common/octomap_handler.py`.  OctoHandler sends the MoveIt! action service, `apply_planning_scene`, a PlanningScene message with the previously created occupancy map data, populating MoveIt!'s occupancy map.

The configuration file `config/sensors_null.yaml` adds the PointCloud Octomap Updater plug-in to the MoveIt! group to enable the occupancy map but points the plug-in to a nonexistent topic so that it will not disturb the occupancy map loaded by `navigate_map.py`.

Once MoveIt! creates a motion plan for a joint, it can command the robot to execute the plan by sending a `RobotTrajectory` message for the joint to the motion controller manager.  The controller manager maps the joint (in this case, `virtual_joint`) to a robot joint controller and maps the `RobotTrajectory` message to a joint movement message understood by the controller, usually `JointTrajectoryAction`.  MoveIt! has a default controller manager that supports several different joint movement messages, but none support both the 3D linear and yaw motions needed by the quadcopter robot.

The `example_externals/moveit_simple_controller_manager` plug-in replaces the default controller manager and adds support for the custom `MultiDofFollowJointTrajectory` message which supports both linear and angular motion.  This plug-in is installed by `trajectory_execution.launch.xml` &#8594; `quad_moveit_controller.launch.xml` launch files.

The configuration file `config/controllers.yaml` adds the complementary `example_externals/action_controller` robot controller plug-in to the controller manager.  `action_controller` accepts `MultiDofFollowJointTrajectory` and is customized for commanding the Project AirSim robot.  At each pose along the trajectory, it computes linear and angular velocities based on the target pose, target time, current robot pose (from the robot's `actual_pose` topic,) and current time.  For each target pose, it sends ROS `geometry_msgs/Twist` messages to the robot's `cmd_vel` topic, waits for the robot to reach the target position, moves to the next target pose, and repeats until the robot has reached the final target pose.

RViz provides the GUI to MoveIt! via MoveIt's MotionPlanner RViz plug-in.  RViz is marked as "required" in `moveit_rviz.launch` so when RViz is shutdown by the user, `roslaunch` automatically shuts down the rest of the nodes.

## 3rd party attributions

`example_externals/moveit_simple_controller_manager` and `example_externals/action_controller` are derived from work by Benoit Courty, Alessio Tonioni, and Wel Selby.  The original work may be found at https://github.com/benoit-cty/ROS-Autonomous-Quadcopter-Flight, and https://github.com/wilselby/ROS_quadrotor_simulator.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI Consulting Corp.

MIT License. All rights reserved.
