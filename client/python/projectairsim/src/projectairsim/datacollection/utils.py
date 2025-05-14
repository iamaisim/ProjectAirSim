"""

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.


This script holds helper methods for the data collection module


"""

import copy
import csv
import json
import math
import os
import pathlib
import random
from typing import Dict, List

import commentjson
import cv2
import jsonschema
import numpy as np
from azure.storage.blob import BlobClient, BlobServiceClient

from projectairsim.geodetic_converter import GeodeticConverter


def read_cjson(path: pathlib.Path) -> Dict:
    with open(path) as f:
        config = commentjson.load(f)

        return config


def read_csv(file):
    output = []

    with open(file) as f:
        reader = csv.reader(f)

        for row in reader:
            output.append(row)

    return output


def validate_config(config: Dict, schema_path: pathlib.Path):
    schema = read_cjson(schema_path)

    try:
        jsonschema.validate(instance=config, schema=schema)

    except jsonschema.exceptions.ValidationError as err:
        raise err


def clamp(n):
    if n > math.pi:
        n -= 2 * math.pi

    elif n < -math.pi:
        n += 2 * math.pi

    if n < -math.pi:
        n = -math.pi

    if n >= math.pi:
        n = math.pi

    return n


def check_range(arr: List, lower: int, higher: int):
    res = all(ele >= lower and ele <= higher for ele in arr)

    return res


# Calculate horizontal plane distance between coords

# start_coord: WayPoint, end_coord: WayPoint


def dist_xy(start_coord, end_coord):
    dx = end_coord.pose_x - start_coord.pose_x

    dy = end_coord.pose_y - start_coord.pose_y

    return math.hypot(dx, dy)


# Calculate distance between 2 points in 3d

# start_coord: WayPoint, end_coord: WayPoint


def dist_xyz(start_coord, end_coord):
    start = np.array([start_coord.pose_x, start_coord.pose_y, start_coord.pose_z])

    end = np.array([end_coord.pose_x, end_coord.pose_y, end_coord.pose_z])

    return np.linalg.norm(end - start)


# Normalize angle to 0~2pi


def norm_0_to_2pi(x_rad):
    return (x_rad + 2 * math.pi) % (2 * math.pi)


def download_blob(connection_string: str, container_name: str, file_name: str):
    blob_service_client = BlobServiceClient.from_connection_string(connection_string)

    container_client = blob_service_client.get_container_client(container_name)
    blob_client = container_client.get_blob_client(file_name)

    stream = blob_client.download_blob()

    return stream.readall()


def write_metadata(
    path,
    running=True,
    last_index_instance=0,
    completed=False,
    last_index_run=0,
    is_write_metadata=True,
):
    if is_write_metadata:
        data = {
            "running": running,
            "completed": completed,
            "last_index_instance": last_index_instance,  # Last index collected by the instance
            "last_index_run": last_index_run,  # Image name of the last collected index
        }

        with open(path, "w") as f:
            json.dump(data, f)


def convert_geo_to_UE(geo_coord: list, converter: GeodeticConverter):
    scene_coordinates = list(converter.geodetic_to_ned(geo_coord))

    scene_coordinates = [round(x) for x in scene_coordinates]  # round to standardize

    return scene_coordinates


# Generate a flight path from a start/end coord: WayPoint


def generate_p2p_path(start_coord, end_coord, spacing, min_dist=0):
    trajectory = []  # list of WayPoint objects

    # Set yaw to turn toward end_coord

    cur_coord = copy.deepcopy(start_coord)

    theta = math.atan2(
        end_coord.pose_y - start_coord.pose_y,
        end_coord.pose_x - start_coord.pose_x,
    )

    cur_coord.yaw = (theta) % (2 * math.pi)

    trajectory.append(copy.deepcopy(cur_coord))

    if min_dist < spacing:
        min_dist = spacing

    # Start moving x, y toward end_coord

    while dist_xy(cur_coord, end_coord) > min_dist:
        cur_coord.pose_x = cur_coord.pose_x + spacing * math.cos(theta)

        cur_coord.pose_y = cur_coord.pose_y + spacing * math.sin(theta)

        trajectory.append(copy.deepcopy(cur_coord))

    return trajectory


def process_csv_stream(csv_stream):
    bbox_data = csv_stream.decode("utf-8")

    bbox_data = bbox_data.split()

    for i, row in enumerate(bbox_data):
        bbox_data[i] = row.split(",")

    return bbox_data


def get_bbox_dict(bbox_data):
    headers = bbox_data[0]

    bbox_data = bbox_data[1:]

    valid_data = None  # Checks if the row has valid 2d/3d bbox data

    bbox_dict = {}

    for row in bbox_data:
        valid_data = True

        row_dict = {}

        for i, header in enumerate(headers):
            try:
                row_dict[header] = row[i]

            except Exception as e:
                print(f"Ran into error '{e}' for {row[0]}. Moving On")

                valid_data = False

                break

        if valid_data:
            bbox_dict[row[0]] = row_dict

    return bbox_dict


def setup_data_local(dir_path: str, num_images: int, **kwargs):
    # flag = {"aug_flag": kwargs["aug_flag"]}

    csv_path = pathlib.Path(dir_path, "bounding_boxes.csv")

    image_dir = pathlib.Path(dir_path, "images")

    if not csv_path.exists():
        print(f"bounding_boxes.csv does not exist")

        return {}, [], []

    bbox_data = read_csv(csv_path)

    bbox_dict = get_bbox_dict(bbox_data)

    image_list_full = os.listdir(image_dir)

    valid_image_list = []

    for image in image_list_full:
        if image in bbox_dict.keys():
            valid_image_list.append(image)

    blob_list = (
        valid_image_list
        if num_images == 0
        else random.sample(valid_image_list, min(num_images, len(valid_image_list)))
    )

    blob_list.sort(key=lambda f: int("".join(filter(str.isdigit, f))))

    video_images = []

    for image_name in blob_list:
        image_path = pathlib.Path(image_dir, image_name)

        video_images.append(cv2.imread(str(image_path), cv2.IMREAD_COLOR))

    return bbox_dict, video_images, blob_list


def setup_data_azure(config_dir: str, connection_string: str, num_images: int):
    config_path = pathlib.Path(config_dir, "datacollector_config.jsonc")

    schema_path = pathlib.Path(
        config_dir, "schemas", "datacollector_config_schema.jsonc"
    )

    config: Dict = read_cjson(config_path)

    validate_config(config, schema_path)

    output_spec = config.get("output-spec", {})

    azure_spec = output_spec.get("azure-spec", {})

    container_name = azure_spec.get("container-name")

    # Setup container client

    blob_service_client = BlobServiceClient.from_connection_string(connection_string)

    container_client = blob_service_client.get_container_client(container_name)

    # Download and process bounding_boxes.csv

    print("Downloading CSV file")

    # blob_client = container_client.get_blob_client("bounding_boxes.csv")

    csv_blob = BlobClient.from_connection_string(
        conn_str=connection_string,
        container_name=container_name,
        blob_name="bounding_boxes.csv",
    )

    csv_stream = csv_blob.download_blob().readall()

    bbox_data = process_csv_stream(csv_stream)

    bbox_dict = get_bbox_dict(bbox_data)

    # Download and process n random images from the dataset

    print("Downloading Images")
    blob_list_original = container_client.list_blobs()

    image_list = []

    for blob in blob_list_original:
        blob_name: str = blob.name

        if blob_name.endswith(".png") and blob_name.startswith("images"):
            if blob_name.split("/")[1] in bbox_dict.keys():
                image_list.append(blob.name)

    blob_list = (
        image_list
        if num_images == 0
        else random.sample(image_list, min(num_images, len(image_list)))
    )

    blob_list.sort(key=lambda f: int("".join(filter(str.isdigit, f))))

    video_images = []

    for blob_name in blob_list:
        blob_client = container_client.get_blob_client(blob_name)

        img_data = blob_client.download_blob().content_as_bytes()

        image = np.frombuffer(img_data, np.uint8)

        image = cv2.imdecode(image, cv2.IMREAD_COLOR)

        video_images.append(image)

    return bbox_dict, video_images, blob_list, container_client


# trajectory: List[WayPoint]


def get_trajectory_ned(trajectory):
    pose_n = []

    pose_e = []

    pose_d = []

    for pose in trajectory:
        pose_n.append(pose.pose_x)

        pose_e.append(pose.pose_y)

        pose_d.append(pose.pose_z)

    return (pose_n, pose_e, pose_d)


# Gets edge lenght of a hypothetical cube that encloses the two points

# start: WayPoint, end: WayPoint


def get_edge_len(start, end):
    del_x = abs(start.pose_x - end.pose_x)

    del_y = abs(start.pose_y - end.pose_y)

    del_z = abs(start.pose_z - end.pose_z)

    return max(del_x, del_y, del_z)
