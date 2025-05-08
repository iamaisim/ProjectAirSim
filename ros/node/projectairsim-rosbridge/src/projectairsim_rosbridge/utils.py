"""
Copyright (C) Microsoft Corporation. All rights reserved.
ROS bridge for Project AirSim: Callback, topic path, and type conversion utilities
"""

import re

import geometry_msgs.msg as rosgeommsg
from builtin_interfaces.msg import Time

from geometry_msgs.msg import TransformStamped
import quaternion
import numpy as np


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

# Regular expression to extract robot's parent frame ID, everything
# between the two forward-slashes the succeed 'robots'
_re_get_parent_frame_id = re.compile(r'/robots/([^/]*)')

# Regular expression to extract robot's parent frame ID, everything
# between the two forward-slashes the succeed 'robots'
_re_get_sensor_name_id = re.compile(r'/sensors/([^/]*)')

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

# Regular expression to extract camera name, everything
# between the two ("/") immediately proceeding depth_planar_camera
#_re_get_sensor_frame_id = re.compile("/(.*/sensors/[^/]*)")
_re_get_sensor_name = re.compile(r'([^/]*)/(depth_planar_camera|scene_camera|depth_camera|depth_vis_camera|disparity_normalized_camera|segmentation_camera|surface_normals_camera|lidar)$')


def get_sensor_name(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the name of the camera 

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The camera name None if the projectairsim_topic_name
            doesn't contain a camera name
    """
    match = _re_get_sensor_name.search(projectairsim_topic_name)
    return None if match is None else match.group(1)

# Regular expression to extract camera name, everything
# between the two ("/") immediately proceeding depth_planar_camera
_re_get_sensor_name = re.compile(r'([^/]*)/(depth_planar_camera|scene_camera|depth_camera|depth_vis_camera|disparity_normalized_camera|segmentation_camera|surface_normals_camera|lidar)$')


def get_sensor_name(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the name of the camera 

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The camera name None if the projectairsim_topic_name
            doesn't contain a camera name
    """
    match = _re_get_sensor_name.search(projectairsim_topic_name)
    return None if match is None else match.group(1)

    
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
    match = get_robot_name(projectairsim_topic_name)
    return None if match is None else '/airsim_node/' + match


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


def get_robot_parent_frame_id(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the robot's parent frame is.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The transform frame ID for the robot's base, or None
            if projectairsim_topic_name does not contain a robot name
    """
    match = _re_get_parent_frame_id.search(projectairsim_topic_name)
    return None if match is None else match.group(1)    

def get_sensor_name_id(projectairsim_topic_name: str) -> str:
    """
    Given an Project AirSim topic name, return the robot's parent frame is.

    Arguments:
        projectairsim_topic_name - Project AirSim topic name

    Returns:
        (return)  The transform frame ID for the robot's base, or None
            if projectairsim_topic_name does not contain a robot name
    """
    match = _re_get_sensor_name_id.search(projectairsim_topic_name)
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
    Convert a vector from ROS orientation (RHS, X-forward, Y-right, Z-down) to
    Project Airsim orientation
    """
    return {"x": ros_vector.x, "y": ros_vector.y, "z": ros_vector.z}


def to_projectairsim_angular_rotation(ros_vector):
    """
    Convert an angular rotation vector from ROS orientation (RHS, X-forward, Y-right, Z-down)
    to Project AirSim orientation
    """
    return {"x": ros_vector.x, "y": ros_vector.y, "z": ros_vector.z}


def to_projectairsim_quaternion(ros_quaternion: rosgeommsg.Quaternion):
    """
    Convert a vector from ROS orientation (RHS, X-forward, Y-right, Z-down) to Project
    AirSim orientation
    """
    return {
        "x": ros_quaternion.x,
        "y": ros_quaternion.y,
        "z": ros_quaternion.z,
        "w": ros_quaternion.w,
    }


def to_ros_point(projectairsim_vector):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return rosgeommsg.Point(
        x=float(projectairsim_vector["x"]),
        y=float(projectairsim_vector["y"]),
        z=float(projectairsim_vector["z"]),
    )


def to_ros_position_list(projectairsim_vector):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return (
        projectairsim_vector["x"],
        projectairsim_vector["y"],
        projectairsim_vector["z"],
    )


def to_ros_position_list2list(projectairsim_list):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return (projectairsim_list[0], projectairsim_list[1], projectairsim_list[2])


def to_ros_position_vector3(projectairsim_vector):
    """
    Convert a vector from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return rosgeommsg.Vector3(
        x=float(projectairsim_vector["x"]),
        y=float(projectairsim_vector["y"]),
        z=float(projectairsim_vector["z"]),
    )


def to_ros_quaternion(projectairsim_quaternion):
    """
    Convert a quaternion from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return rosgeommsg.Quaternion(
        x=float(projectairsim_quaternion["x"]),
        y=float(projectairsim_quaternion["y"]),
        z=float(projectairsim_quaternion["z"]),
        w=float(projectairsim_quaternion["w"]),
    )


def to_ros_quaternion_list(projectairsim_quaternion):
    """
    Convert a quaternion from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return (
        projectairsim_quaternion["x"],
        projectairsim_quaternion["y"],
        projectairsim_quaternion["z"],
        projectairsim_quaternion["w"],
    )


def to_ros_quaternion_list2list(projectairsim_list):
    """
    Convert a quaternion from Project AirSim orientation (RHS, X-forward, Y-right, Z-down)
    to ROS orientation
    """
    return (
        projectairsim_list[0],
        projectairsim_list[1],
        projectairsim_list[2],
        projectairsim_list[3],
    )

def to_ros_timestamp(projectairsim_timestamp):
    """
    Convert a timestamp from Project AirSim to ROS2 timestamp message type.
    """
    ros_timestamp = Time()
    ros_timestamp.sec = projectairsim_timestamp // 1000000000  # Extract seconds
    ros_timestamp.nanosec = projectairsim_timestamp % 1000000000  # Extract nanoseconds

    return ros_timestamp

#convert
def get_camera_optical_tf_from_body_tf(body_tf):
    # Assuming body_tf is a dictionary with 'translation' and 'rotation'
    optical_tf = body_tf  # Same translation

    # Create quaternion from body_tf rotation
    opticalQ = quaternion.quaternion(optical_tf.rotation.w, optical_tf.rotation.x, optical_tf.rotation.y, optical_tf.rotation.z)

    # Apply the transformation
    cam_optical_offset = quaternion.quaternion(0.5, 0.5, 0.5, 0.5)
    opticalQ *= cam_optical_offset

    # Update optical_tf rotation
    optical_tf.rotation.w = opticalQ.real
    optical_tf.rotation.x = opticalQ.imag[0]
    optical_tf.rotation.y = opticalQ.imag[1]
    optical_tf.rotation.z = opticalQ.imag[2]

    return optical_tf

# --------------------------------------------------------------------------
# Parse sim config
# --------------------------------------------------------------------------

#parses configs and creates dict corresponding sensor with settings
class config_parser:

    def __init__(self):
        self.sensors = []

    def parse_params(self, projectairsim_world):
        actor_dict = {}
        sensor_dict = {}
        temp = projectairsim_world.sim_config
        actors = projectairsim_world.sim_config['actors']

        for index, item in enumerate(actors):
            sensor_configs = projectairsim_world.sim_config['actors'][index]['robot-config']['sensors']
            drone_name = item['name']
            drone_origin = item['origin']
            for item in sensor_configs:
                sensor_id = item['id']
                sensor_type = item['type']
                sensor_params = self.initialize_sensor_params()

                if sensor_type in ['camera', 'lidar']:
                    self.extract_sensor_settings(item, sensor_params, sensor_type)

                sensor_dict[sensor_id] = sensor_params

            if drone_name not in actor_dict:
                actor_dict[drone_name] = {}
                actor_dict[drone_name]['params'] = sensor_dict
                actor_dict[drone_name]['drone_origin'] = drone_origin

        return actor_dict

    #set dict object with sensor settings to be saved
    def initialize_sensor_params(self):
        return {key: None for key in ['width', 'height', 'x_coord', 'y_coord', 'z_coord', 'roll', 'pitch', 'yaw']}

    #extract sets of sensor settings to be parsed
    def extract_sensor_settings(self, item, sensor_params, sensor_type):
        #get capture settings for sensor (image height, width, etc.)
        capture_settings = item.get('capture-settings', [{}])[0]
        #get origin settings for sensor (xyz, rpy)
        origin_dict = item['origin']

        self.extract_settings(capture_settings, sensor_params)
        self.extract_origin(origin_dict, sensor_params)     

    #save camera capture settings to dict
    def extract_settings(self, settings_dict, sensor_params):
        keys_to_find = ['width', 'height']
        for key in keys_to_find:
            if key in settings_dict:
                sensor_params[key] = settings_dict[key]

    #save origin settings to dict
    def extract_origin(self, origin_dict, sensor_params):
        keys_to_find = ['xyz', 'rpy-deg']
        for key in keys_to_find:
            if key in origin_dict:
                if key == 'xyz':
                    self.extract_xyz_settings(origin_dict[key], sensor_params)
                elif key == 'rpy-deg':
                    self.extract_rpy_settings(origin_dict[key], sensor_params)        

    #extract individual x, y, z parameters from xyz array
    def extract_xyz_settings(self, xyz, sensor_params):
        x_coord, y_coord, z_coord = map(float, xyz.split())
        sensor_params['x_coord'] = x_coord
        sensor_params['y_coord'] = y_coord
        sensor_params['z_coord'] = z_coord

    #extract individual r, p, y parameters from rpy array
    def extract_rpy_settings(self, rpy_deg, sensor_params):
        roll, pitch, yaw = map(float, rpy_deg.split())
        sensor_params['roll'] = roll
        sensor_params['pitch'] = pitch
        sensor_params['yaw'] = yaw            
