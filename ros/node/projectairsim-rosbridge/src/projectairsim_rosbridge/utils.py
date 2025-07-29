"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
ROS bridge for Project AirSim: Callback, topic path, and type conversion utilities
"""

import re

import geometry_msgs.msg as rosgeommsg


# --------------------------------------------------------------------------
class Callbacks:
    """
    Manages a list of callback functions.
    """

    def __init__(self):
        """
        Constructor.
        """
        self.callbacks = []

    def __bool__(self):
        """
        Return true if there are callbacks registered, false otherwise.
        """
        return bool(self.callbacks)

    def __call__(self, *args, **kwargs):
        """
        Invoke all registered callbacks.

        Arguments:
            *args - Position arguments to callback function
            **kwargs - Named arguments to callback function
        """
        for callback in self.callbacks:
            callback(*args, **kwargs)

    def __len__(self):
        """
        Return the number of registered callback functions.
        """
        return len(self.callbacks)

    def add(self, callback) -> bool:
        """
        Add a callback function.  If the function is already on the list it is not added again.

        Arguments:
            callback - The function to add

        Return:
            (Return) - True if callback was added, False if callback was already on the list.
        """
        if not callable(callback):
            raise TypeError(f"specified callback object is not callable: {callback}")

        if not self.callbacks:
            self.callbacks = [callback]
            subscriber_added = True
        else:
            if callback in self.callbacks:
                subscriber_added = False
            else:
                self.callbacks.append(callback)
                subscriber_added = True

        return subscriber_added

    def remove(self, callback):
        """
        Remove a callback function.

        Arguments:
            callback - The function to remove
        """
        self.callbacks.remove(callback)


# --------------------------------------------------------------------------
# Topic Path Extraction Functions
# --------------------------------------------------------------------------

# Regular expression to extract robot's base transform frame ID, everything
# past the initial forward-slash ("/") up to the forward-slash terminating
# the name
_re_get_robot_base_frame_id = re.compile("/(.*/robots/[^/]*)")

# Regular expression to extract robot name, everything up to forward-slash
# ("/") or end-of-string terminating the name
_re_get_robot_name = re.compile(".*/robots/([^/]*)")

# Regular expression to extract robot path, everything up to forward-slash
# ("/") terminating the name
_re_get_robot_path = re.compile("(.*/robots/[^/]*)")

# Regular expression to extract robot's sensor path, everything up to
# forward-slash ("/") terminating the name
_re_get_sensor_path = re.compile("(.*/sensors/[^/]*)")

# Regular expression to extract robot's sensor transform frame ID, everything
# past the initial forward-slash ("/") up to the forward-slash terminating
# the name
_re_get_sensor_frame_id = re.compile("/(.*/sensors/[^/]*)")


def get_robot_path(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the robot path which contains the
    initial portion of the topic name ending with the robot name minus the
    terminating forward-slash.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The robot name path or None if the projectairsim_topic_name
            doesn't contain a robot name path
    """
    match = _re_get_robot_path.match(projectairsim_topic_name)
    return None if match is None else match.group(1)


def get_robot_frame_id(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the transform frame ID for the
    robot's base.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The transform frame ID for the robot's base, or None
            if projectairsim_topic_name does not contain a robot name
    """
    match = _re_get_robot_base_frame_id.match(projectairsim_topic_name)
    return None if match is None else match.group(1)


def get_robot_name(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the robot name minus the
    leading and terminating forward-slashes.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The robot name  or None if the projectairsim_topic_name
            doesn't contain a robot name
    """
    match = _re_get_robot_name.match(projectairsim_topic_name)
    return None if match is None else match.group(1)


def get_sensor_path(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the sensor path which contains
    the initial portion of the topic name ending with the sensor name minus
    the terminating forward-slash.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The sensor name path or None if the projectairsim_topic_name
            doesn't contain a sensor name path
    """
    match = _re_get_sensor_path.match(projectairsim_topic_name)
    return None if match is None else match.group(1)


def get_sensor_frame_id(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the transform frame ID for a
    robot's sensor.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The transform frame ID for the robot's base, or None
            if projectairsim_topic_name does not contain a sensor name
    """
    match = _re_get_sensor_frame_id.match(projectairsim_topic_name)
    return None if match is None else match.group(1)


# --------------------------------------------------------------------------
# Project AirSim / ROS Coordinate Conversion Functions
# --------------------------------------------------------------------------


def to_projectairsim_position(ros_vector):
    """
    Convert a vector from ROS orientation (RHS, X-forward, Y-left, Z-up) to
    Project Airsim orientation (RHS, X-forward, Y-right, Z-down)
    """
    return {"x": ros_vector.x, "y": -ros_vector.y, "z": -ros_vector.z}


def to_projectairsim_angular_rotation(ros_vector):
    """
    Convert an angular rotation vector from ROS orientation (RHS, X-forward, Y-left,
    Z-up) to Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    """
    return {"x": ros_vector.x, "y": -ros_vector.y, "z": -ros_vector.z}


def to_projectairsim_quaternion(ros_quaternion: rosgeommsg.Quaternion):
    """
    Convert a vector from ROS orientation (RHS, X-forward, Y-left, Z-up) to Project
    AirSim orientation (RHS, X-forward, Y-right, Z-down)
    """
    return {
        "x": ros_quaternion.x,
        "y": -ros_quaternion.y,
        "z": -ros_quaternion.z,
        "w": ros_quaternion.w,
    }


def to_ros_point(projectairsim_vector):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return rosgeommsg.Point(
        x=projectairsim_vector["x"],
        y=-projectairsim_vector["y"],
        z=-projectairsim_vector["z"],
    )


def to_ros_position_list(projectairsim_vector):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return (
        projectairsim_vector["x"],
        -projectairsim_vector["y"],
        -projectairsim_vector["z"],
    )


def to_ros_position_list2list(projectairsim_list):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return (projectairsim_list[0], -projectairsim_list[1], -projectairsim_list[2])


def to_ros_position_vector3(projectairsim_vector):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return rosgeommsg.Vector3(
        x=projectairsim_vector["x"],
        y=-projectairsim_vector["y"],
        z=-projectairsim_vector["z"],
    )


def to_ros_quaternion(projectairsim_quaternion):
    """
    Convert a quaternion from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return rosgeommsg.Quaternion(
        x=projectairsim_quaternion["x"],
        y=-projectairsim_quaternion["y"],
        z=-projectairsim_quaternion["z"],
        w=projectairsim_quaternion["w"],
    )


def to_ros_quaternion_list(projectairsim_quaternion):
    """
    Convert a quaternion from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return (
        projectairsim_quaternion["x"],
        -projectairsim_quaternion["y"],
        -projectairsim_quaternion["z"],
        projectairsim_quaternion["w"],
    )


def to_ros_quaternion_list2list(projectairsim_list):
    """
    Convert a quaternion from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation (RHS, X-forward, Y-left, Z-up)
    """
    return (
        projectairsim_list[0],
        -projectairsim_list[1],
        -projectairsim_list[2],
        projectairsim_list[3],
    )
