"""
Copyright (C) Microsoft Corporation. All rights reserved.
ProjectAirSim utilities
"""

import logging
import math
from typing import Dict, List, Tuple
import msgpack
import numpy as np
import collections
import cv2
import os
import commentjson
import jsonschema
from jsonschema import validate
from pykml import parser
import pkg_resources

from projectairsim.geodetic_converter import GeodeticConverter


def get_pitch_between_traj_points(point1, point2):
    """Computes the pitch angle between two points in NED coordinates (x, y, z)

    Args:
        point1, point2: the two points

    Returns:
        float: the pitch angle between the points, in radians
    """
    dx = point2[0] - point1[0]
    dy = point2[1] - point1[1]
    dz = point2[2] - point1[2]
    return math.atan2(-dz, math.hypot(dx, dy))


#
def get_heading_between_traj_points(point1, point2):
    """Computes the heading/yaw angle between two points in NED coordinates (x, y, z)

    Args:
        point1, point2: the two points

    Returns:
        float: the yaw angle between the points, in radians
    """
    dx = point2[0] - point1[0]
    dy = point2[1] - point1[1]
    return math.atan2(dy, dx)


def generate_perpendicular_unit_vector(direction_vector):
    """Generates an arbitrary unit vector perpendicular to a given 3d vector

    Args:
        vector (List): the vector to be perpendicular to

    Returns:
        List: the generated vector
    """
    # generate two helpers by rotating 90 degrees
    helper_1 = [direction_vector[1], -direction_vector[0], direction_vector[2]]
    helper_2 = [direction_vector[2], -direction_vector[1], -direction_vector[0]]

    # The cross product of two vectors is guaranteed to be perpendicular to both
    perpendicular_vector_1 = np.cross(helper_1, direction_vector)
    perpendicular_vector_2 = np.cross(helper_2, direction_vector)

    # Use the norm and redundancy to ensure that we have a nonzero vector
    if np.linalg.norm(perpendicular_vector_1) > np.linalg.norm(perpendicular_vector_2):
        return [
            i / np.linalg.norm(perpendicular_vector_1) for i in perpendicular_vector_1
        ]
    else:
        return [
            i / np.linalg.norm(perpendicular_vector_2) for i in perpendicular_vector_2
        ]


def rotate_vector_about_axis(vector, axis, angle):
    """Rotates a vector about an axis

    Args:
        vector (List): the vector to be rotated
        axis (List): the axis to rotate about
        angle (float): the rotation angle, in radians

    Returns:
        List: the rotated vector
    """
    # Normalize the axis vector
    axis = axis / np.linalg.norm(axis)

    # Compute the cross product matrix of the axis vector
    K = np.array(
        [[0, -axis[2], axis[1]], [axis[2], 0, -axis[0]], [-axis[1], axis[0], 0]]
    )

    # Compute the rotation matrix using Rodrigues' formula
    R = np.eye(3) + np.sin(angle) * K + (1 - np.cos(angle)) * np.dot(K, K)

    # Rotate the vector using the rotation matrix
    return np.dot(R, vector)


def get_point_to_line_segment_distance(point, line_start, line_end):
    """Computes the minimum distance between a 3d point and the line segment defined by two points

    Args:
        point (List): the point
        line_start, line_end (List): the start and end points of the line segment

    Returns:
        float: the distance
    """
    # Calculate vector between the line start and the point to measure
    point_to_line_start_vector = np.subtract(point, line_start)
    # Calculate length and direction of the line segment
    segment_length = np.linalg.norm(np.subtract(line_end, line_start))
    line_direction = [i / segment_length for i in np.subtract(line_end, line_start)]
    # Get the projection length and compute distance based on where the point is relative to the line segment
    length_on_line = np.dot(point_to_line_start_vector, line_direction)
    if length_on_line < 0:
        return np.linalg.norm(point_to_line_start_vector)
    elif length_on_line > segment_length:
        return np.linalg.norm(np.subtract(point, line_end))
    else:
        return np.linalg.norm(
            np.subtract(
                point_to_line_start_vector, [length_on_line * i for i in line_direction]
            )
        )


def point_distance(start, end):
    """Helper method for calculating distance

    Args:
        start, end (list): the quaternion values

    Returns:
        float: the euclidean distance between two points
    """
    return np.linalg.norm(np.subtract(end, start))


def calculate_path_length(path_points):
    """Calculates the length of a path

    Args:
        path_points (list): the 3d coordinate values of the path

    Returns:
        float: the total length of the path
    """
    distances = []
    for i in range(len(path_points) - 1):
        distances.append(point_distance(path_points[i + 1], path_points[i]))
    time = sum(distances)
    return time


def calculate_path_time(path_points, speeds):
    """Calculates the time it will take to traverse a path

    Args:
        path_points (list): the 3d coordinate values of the path
        speeds (list): the speeds of travel on each leg of the path

    Returns:
        float: the total time taken
    """
    distances = []
    for i in range(len(path_points) - 1):
        distances.append(
            np.linalg.norm(np.subtract(path_points[i + 1], path_points[i]))
        )
    time = sum(np.divide(distances, speeds[: len(distances)]))
    return time

def get_voxel_grid_idx(coordinate: list, grid_center: list, edge_lens: tuple, res: float):
    """For a given coordinate in NED, returns the index of the corresponding voxel in the occupancy map

    Args:
        coordinate (list): (x, y, z) in NEU
        grid_center (list): the center point of the grid
        edge_lens (tuple): the edge lengths of the grid
        res (list): the resolution of the grid
    Returns:
        int: index of the corresponding voxel in the occupancy map
    """
    coordinate = (coordinate[0], coordinate[1], -coordinate[2])

    x = coordinate[0] - grid_center[0]
    y = coordinate[1] - grid_center[1]
    z = coordinate[2] - grid_center[2]
    num_cells_x = edge_lens[0] // res
    num_cells_y = edge_lens[1] // res
    num_cells_z = edge_lens[2] // res

    x_idx = round(x / res + (num_cells_x / 2))
    y_idx = round(y / res + (num_cells_y / 2))
    z_idx = round(z / res + (num_cells_z / 2))

    grid_idx = x_idx + num_cells_x * (z_idx + num_cells_z * y_idx)

    return int(grid_idx)

def get_point_distance_along_path(path: List[List[float]], desired_distance: float):
    """Given a path and a distance, returns the point at that distance along the path

    Args:
        path (list[list[float]]): the path coordinates
        desired_distance (float): the distance along the path to find the point
    Returns:
        list: the point on the path that is the given distance from the start
    """
    total_distance = 0.0
    segment_length = 0.0
    # Determine where to place intercept point
    leg_start = 0
    while total_distance < desired_distance:
        on_segment_distance = desired_distance - total_distance
        segment_length = point_distance(path[leg_start], path[leg_start + 1])
        total_distance += segment_length
        if total_distance < desired_distance:
            leg_start += 1
    # Interpolate to get a random point on the path
    leg_end = leg_start + 1
    leg_segment = np.subtract(path[leg_end], path[leg_start])
    interpolation_factor = on_segment_distance / segment_length
    chosen_x = path[leg_start][0] + leg_segment[0] * interpolation_factor
    chosen_y = path[leg_start][1] + leg_segment[1] * interpolation_factor
    chosen_z = path[leg_start][2] + leg_segment[2] * interpolation_factor
    return ({"x": chosen_x, "y": chosen_y, "z": chosen_z}, leg_segment, leg_start)


def quaternion_to_rpy(w, x, y, z):
    """Helper method for converting quaternion to/from roll/pitch/yaw

    https:#en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    This conversion returns the Euler angles based on the order of body-fixed frame ZYX (yaw, pitch, then roll),
    which is equivalent to world-fixed frame XYZ (roll, pitch, then yaw).

    Args:
        w, x, y, z (float): the quaternion values

    Returns:
        tuple: a (roll, pitch, yaw) tuple, in radians
    """
    ysqr = y * y

    # roll (x-axis rotation)
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + ysqr)
    roll = math.atan2(t0, t1)

    # pitch (y-axis rotation)
    t2 = +2.0 * (w * y - z * x)
    if t2 > 1.0:
        t2 = 1
    if t2 < -1.0:
        t2 = -1.0
    pitch = math.asin(t2)

    # yaw (z-axis rotation)
    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (ysqr + z * z)
    yaw = math.atan2(t3, t4)

    return (roll, pitch, yaw)


def rpy_to_quaternion(roll, pitch, yaw):
    """Helper method for converting quaternion to/from roll/pitch/yaw

    https:#en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    The Euler angles given should be based on the order of body-fixed frame ZYX (yaw, pitch, then roll),
    which is equivalent to world-fixed frame XYZ (roll, pitch, then yaw).
    Pitch must be within -89.9 to +89.9 degrees due to gimbal lock restrictions.

    Args:
       roll, pitch, yaw (float): the rpy, in radians

    Returns:
        tuple: a (w,x,y,z) tuple, representing a Quaternion
    """
    # pitch angle must be within -90 ~ +90 deg, apply clipping
    kPitchLimit = math.radians(89.9)
    if abs(pitch) > kPitchLimit:
        projectairsim_log().warning("Pitch exceeded +/-89.9 deg, value will be clipped")
        pitch = max(-kPitchLimit, min(pitch, kPitchLimit))

    t0 = math.cos(yaw * 0.5)
    t1 = math.sin(yaw * 0.5)
    t2 = math.cos(roll * 0.5)
    t3 = math.sin(roll * 0.5)
    t4 = math.cos(pitch * 0.5)
    t5 = math.sin(pitch * 0.5)

    w_val = t0 * t2 * t4 + t1 * t3 * t5  # w
    x_val = t0 * t3 * t4 - t1 * t2 * t5  # x
    y_val = t0 * t2 * t5 + t1 * t3 * t4  # y
    z_val = t1 * t2 * t4 - t0 * t3 * t5  # z

    return (w_val, x_val, y_val, z_val)


# Helper function to normalize angles for comparison
def norm_rad(angle):
    """Helper function to normalize angles for comparison

    Args:
        angle (float): the angle, in radians

    Returns:
        float: the angle, normalized to between 0 and 2pi
    """
    return math.atan2(math.sin(angle), math.cos(angle))


def decode(data):
    """Recursively decode data based on type

    Args:
        data: the data to decode

    Returns:
        The decoded data
    """
    if isinstance(data, bytes):
        data_decoded = decode_bytes(data)
        # return decode(data_decoded)
        return data_decoded
    elif isinstance(data, Dict):
        data_decoded = decode_dict(data)
        # return decode(data_decoded)
        return data_decoded
    elif isinstance(data, list) or isinstance(data, collections.abc.KeysView):
        data_decoded = decode_list(data)
        # return decode(data_decoded)
        return data_decoded
    elif isinstance(data, (int, float, str)):  # `int` covers `bool` as well
        return data
    else:
        return data


def decode_bytes(data: bytes):
    """Helper function for the general decode to decode bytes"""
    try:
        # First try unpacking as msgpack binary data
        data_decoded = msgpack.unpackb(data, raw=False)
    except msgpack.FormatError:
        # If not valid msgpack data, try unpacking as a binary string
        data_decoded = data.decode()
    finally:
        return data_decoded


def decode_dict(data: Dict):
    """Helper function for the general decode to decode Dicts"""
    data_decoded = {
        key.decode()
        if isinstance(key, bytes)
        else key: decode(val)
        if isinstance(val, (bytes, Dict, list, collections.abc.KeysView))
        else val
        for key, val in data.items()
    }
    return data_decoded


def decode_list(data: list) -> List:
    """Helper function for the general decode to decode Lists"""
    data_decoded = [decode(x) for x in data]
    return data_decoded


def convert_string_with_spaces_to_float_list(data: str) -> List[float]:
    """Helper function to convert a string input of numbers
    separated by spaces to a List of floats. Useful for reading
    in config parameters.
    """
    floats = [float(x) for x in data.split()]
    return floats


def unpack_image(image):
    """Takes a Project AirSim image message and converts it into a format usable by openCV

    Args:
        image: the image message

    Returns:
        The image in openCV decoded form
    """
    # 16UC1 is used for serializing depth images
    if image["encoding"] == "16UC1":
        img_dtype = "uint16"
        img_shape = [image["height"], image["width"]]
    elif image["encoding"] == "AVX":
        img_dtype = "uint8"
        img_shape = [image["height"], image["width"], 4]
    elif image["encoding"] == "GRAY8":
        img_dtype = "uint8"
        img_shape = [image["height"], image["width"], 1]
    elif image["encoding"] == "32FC1":
        img_dtype = "float32"
        img_shape = [image["height"], image["width"], 1]
    else:
        img_dtype = "uint8"
        img_shape = [image["height"], image["width"], 3]

    # TODO Currently, image messages received by pubsub are serialized by
    # msgpack and the data needs to be processed as a buffer, but images
    # received by reqrep get_images() are serialized by JSON and the data
    # needs to be processed as a list. This should be commonized.
    if len(image["data_float"]) > 0:
        nparr = np.array(image["data_float"], dtype=img_dtype)
        # Normalize the depth values from [0, 65504] to [0, 255] and invert them, so closer objects appear brighter
        nparr =255 * ((65504 - nparr)/(65504))
        nparr = nparr.view(img_dtype)
    elif isinstance(image["data"], list):
        nparr = np.array(image["data"], dtype="B")
        nparr = nparr.view(img_dtype)
    else:
        nparr = np.frombuffer(image["data"], dtype=img_dtype)

    if image["encoding"] == "PNG":
        nparr = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    # TODO Assumes 3-channel image format (no alpha channel). May need to
    # reconsider this if its more efficient to pass images including alpha
    # when reading directly from GPU image buffers.
    img_np = np.reshape(nparr, img_shape)

    if image["encoding"] == "AVX":
        img_np = cv2.cvtColor(img_np, cv2.COLOR_RGB2BGR)

    return img_np


def projectairsim_log():
    """Returns the Project AirSim logger. Use the logger's methods to log information."""
    logger = logging.getLogger("projectairsim")

    if not logger.hasHandlers():
        log_formatter = logging.Formatter(
            "%(asctime)s:%(msecs)03d [%(levelname)s] %(message)s", datefmt="%H:%M:%S"
        )

        file_handler = logging.FileHandler("projectairsim_client.log", mode="w")
        file_handler.setLevel(logging.DEBUG)
        file_handler.setFormatter(log_formatter)

        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.DEBUG)
        console_handler.setFormatter(log_formatter)

        # Set logger handler list in a single call to prevent duplicate adds in case
        # logger gets initialized through async calls to projectairsim_log
        logger.handlers[:] = [file_handler, console_handler]

        # TODO Expose log level to client and set default to lower level filter
        logger.setLevel(logging.DEBUG)

    return logger


def load_scene_config_as_dict(
    config_name: str,
    sim_config_path: str = "sim_config/",
    sim_instance_idx: int = -1,
) -> Tuple[Dict, List]:
    """Loads the scene configuration

    (Re)loads any specified scene config and returns the complete sim and robot config as a Dict as well as
    the file paths as a tuple of the scene config, a list of robot configs, and a list of environment actor configs

    Args:
        config_name (str): the filename of the config
        sim_config_path (str): the folder in which to look for the config
        sim_instance_idx (int): the index of the scene instance (for distributed sim only)

    Returns:
        Tuple[Dict, List]: a tuple containing the dict and the file paths of all loaded configuration files
    """
    projectairsim_log().info(f"Loading scene config: {config_name}")
    total_path = os.path.join(sim_config_path, config_name)
    filepaths = [total_path, [], [], []]

    robot_config_schema = pkg_resources.resource_string(
        __name__, "schema/robot_config_schema.jsonc"
    )
    scene_config_schema = pkg_resources.resource_string(
        __name__, "schema/scene_config_schema.jsonc"
    )

    with open(total_path) as f:
        data = commentjson.load(f)  # read and write the JSON as a dict

    if sim_instance_idx != -1:
        # Append sim instance index to scene ID for distribution mapping
        data["id"] = f"{data['id']}-{sim_instance_idx}"

    validate_json(data, scene_config_schema)

    if "actors" in data:  # read and write the robot-config param in each actor
        for actor in data["actors"]:
            if actor["type"] == "robot":
                actor_configs = actor["robot-config"]
                combined_config = {}
                if not isinstance(actor_configs, list):
                    actor_configs = [actor_configs]
                for actor_path in actor_configs:
                    total_actor_path = os.path.join(sim_config_path, actor_path)
                    filepaths[1].append(total_actor_path)
                    with open(total_actor_path) as e:
                        temp = commentjson.load(e)
                        validate_json(temp, robot_config_schema)
                        combined_config = merge_dicts(combined_config,temp)
                actor["robot-config"] = combined_config
                
    if "environment-actors" in data:
        for env_actor in data["environment-actors"]:
            if env_actor["type"] in ["env_actor", "env_car", "env_human"]:
                env_actor_configs = env_actor["env-actor-config"]
                combined_config = {}
                if not isinstance(env_actor_configs, list):
                    env_actor_configs = [env_actor_configs]
                for env_actor_path in env_actor_configs:
                    total_env_actor_path = os.path.join(sim_config_path, env_actor_path)
                    filepaths[2].append(env_actor_path)
                    with open(total_env_actor_path) as e:
                        temp = commentjson.load(e)
                        if temp.get("script") is not None:
                            validate_trajectory_json(temp["script"])
                        combined_config = merge_dicts(combined_config, temp)
                env_actor["env-actor-config"] = combined_config

    if "environment-objects" in data:
        # read and write the env-object-config param in each env object
        for env_object in data["environment-objects"]:
            if env_object["type"] == "env_particle_effect":
                env_object_configs = env_object["env-object-config"]
                combined_config = {}
                if not isinstance(env_object_configs, list):
                    env_object_configs = [env_object_configs]
                for env_object_path in env_object_configs:
                    total_env_object_path = os.path.join(sim_config_path, env_object_path)
                    filepaths[3].append(env_object_path)
                    with open(total_env_object_path) as e:
                        temp = commentjson.load(e)
                        if temp.get("script") is not None:
                            validate_trajectory_json(temp["script"])
                        combined_config = merge_dicts(combined_config, temp)
                env_object["env-object-config"] = combined_config

    if "tiles-dir" in data and data.get("tiles-dir-is-client-relative"):
        # Convert client-relative path into an absolute path before sending to sim
        resolved_path = os.path.abspath(data["tiles-dir"])
        data["tiles-dir"] = resolved_path
        projectairsim_log().info(
            f"Resolved client-relative tiles-dir path as: {resolved_path}"
        )

    return (data, filepaths)

def merge_dicts(d1, d2):
    """Recursively merges dict d2 into dict d1"""
    for k, v in d2.items():
        if isinstance(v, collections.abc.Mapping):
            d1[k] = merge_dicts(d1.get(k, {}), v)
        elif isinstance(v, list) and isinstance(d1.get(k), list):
            d1[k] = merge_lists(d1[k], v)
        else:
            d1[k] = v
    return d1

def merge_lists(l1, l2):
    """Merges two lists of dictionaries, matching elements by the 'name' key if present"""
    result = l1[:]
    for item2 in l2:
        if isinstance(item2, dict) and "name" in item2:
            # Try to find the corresponding item in l1 based on 'name'
            matching_item = next((item1 for item1 in result if item1.get("name") == item2["name"]), None)
            if matching_item:
                # Merge the dictionaries
                merge_dicts(matching_item, item2)
            else:
                # If no match, append the item from l2
                result.append(item2)
        else:
            # If it's not a dictionary or doesn't have a 'name', just append it
            result.append(item2)
    return result

def validate_json(json_data, file_name) -> None:
    """Validates a JSON according to a given schema

    Args:
        json_data (object): the json to validate
        file_name (str): the name of the schema file
    """
    json_schema = get_schema(file_name)

    try:
        validate(instance=json_data, schema=json_schema)
    except jsonschema.exceptions.ValidationError as err:
        raise err


def validate_trajectory_json(env_actor_script: commentjson) -> None:
    """Validates a json trajectory

    Args:
        env_actor_script (commentjson): the json to validate
    """
    trajectory = env_actor_script["trajectory"]
    validate_trajectory(list(trajectory.values()))


def validate_trajectory(params) -> None:
    """Internal function for validating an imported trajectory

    Args:
        params (List): a list of trajectory parameters
    """
    curr_idx = 0
    # Check that first entry (name) is a string
    if not isinstance(params[curr_idx], str):
        raise TypeError("Trajectory name must be a string.")

    curr_idx += 1
    base_len = len(params[curr_idx])  # length of time array
    params = params[curr_idx + 1 :]  # splice out name and time

    # Check if any of the param arrays are empty
    if not all(len(param) != 0 for param in params):
        raise ValueError("Trajectory params cannot be empty arrays.")

    # Check length of each param array with base value
    if not all(len(param) == base_len for param in params):
        raise ValueError("Trajectory params do not all have the same length.")

    # Check that each entry in param arrays is either a float or int
    for param_list in params:
        if not all(isinstance(param, (int, float)) for param in param_list):
            raise TypeError(
                "All list values in trajectory params should be either int or float."
            )


def get_schema(file_name) -> Dict:
    """Loads a json schema from disk and returns it as a Dict

    Args:
        file_name (str): the name of the file to load
    """
    schema = commentjson.loads(file_name)
    return schema


def geo_to_ned_coordinates(home_geo_point: Dict, lat_lon_alt_coord: List):
    """Helper function to convert lat-lon-alt to scene coordinates

    Args:
        home_geo_point (Dict): the geo point that corresponds to 0,0,0 in NED. Keys should be "latitude", "longitude", and "altitude"
        lat_lon_alt_coord (List): a list of [latitude, longitude, altitude]

    Returns:
        (List): a list of coordinates [n,e,d]
    """

    converter = GeodeticConverter(
        home_geo_point["latitude"],
        home_geo_point["longitude"],
        home_geo_point["altitude"],
    )

    scene_coordinates = list(
        converter.geodetic_to_ned(
            [lat_lon_alt_coord[0], lat_lon_alt_coord[1], lat_lon_alt_coord[2]]
        )
    )

    return scene_coordinates


def get_trajectory_from_kml(filename: str) -> List[tuple]:
    """Reads in a kml file and exports a list of (lat, lon, alt) coordinates

    Args:
        filename (str): the name of the kml file

    Returns:
        List[tuple]: a list of tuples (lat, lon, alt)
    """
    tree = parser.parse(filename)
    root = tree.getroot()
    ns = root.nsmap[None]  # get namespace

    lineStrings = tree.findall(".//{{{0}}}LineString".format(ns))

    for attributes in lineStrings:
        coordinates = attributes.coordinates.text.split()

    lon = []
    lat = []
    alt = []
    for coord in coordinates:
        coord = coord.split(",")
        lon.append(float(coord[0]))
        lat.append(float(coord[1]))
        alt.append(float(coord[2]))
    return lat, lon, alt


def get_ned_trajectory_from_csv(
    filename: str,
    delimiter_str: str = ",",
    n_lines_to_skip: int = 1,
) -> None:
    """Retrieves a NED trajectory from a CSV file

    First row of the file is the header. Numerical data begins from
    n_lines_to_skip + 1. Each row corresponds to trajectory data at a
    given time instance, where the column order is
    1) time (s)
    2) pose_x (m)
    3) pose_y (m)
    4) pose_z (m)
    5) pose_roll (rad)
    6) pose_pitch (rad)
    7) pose_yaw (rad)
    8) vel_lin_x (m/s)
    9) vel_lin_y (m/s)
    10) vel_lin_z (m/s)

    Args:
        filename (str): the filename
        delimiter_str (str): the delimiter separating the data
        n_lines_to_skip (int): the number of lines at the top of the file to ignore
    """
    num_cols_expected = 10
    traj_data = np.genfromtxt(
        filename, delimiter=delimiter_str, skip_header=n_lines_to_skip
    )
    traj_data = np.transpose(traj_data)

    # Check for valid input
    if np.isnan(np.min(traj_data)):
        raise ValueError("File contains NaN!")
    elif np.shape(traj_data)[1] != num_cols_expected:
        raise ValueError(
            "Expected columns for: time, x, y, z, roll, pitch, yaw, x_vel, y_vel, z_vel."
        )
