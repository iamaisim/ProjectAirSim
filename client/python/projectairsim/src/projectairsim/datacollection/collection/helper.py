"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

This script holds helper methods for the collection client scripts

"""

import datetime
import os
import pathlib
import time
from typing import Dict

import cv2
import numpy as np
from PIL import Image
from projectairsim import Drone, EnvActor, ProjectAirSimClient, World
from projectairsim.image_utils import segmentation_color_to_id
from projectairsim.types import Color
from projectairsim.utils import projectairsim_log
from shapely.geometry import Polygon
from skimage import measure

import projectairsim.datacollection.utils as utils
from projectairsim.datacollection.types import COCOAnnotations, GeoLocation, WayPoint


class NonphysicsDrone(Drone):
    """
    Class to inherit the Drone class
    and enable nonphysics operations
    """

    cur_pose = None

    def callback_actual_pose(self, topic, msg):
        """
        Callback for the pose subscription
        Args:
            topic: pose in this case
            msg: the message conveying the pose
        """
        self.cur_pose = msg


# Sets up world and drone objects
def setup_scene(client: ProjectAirSimClient, sim_config: str, config_dir: str):

    world = World(
        client,
        sim_config,
        sim_config_path=os.path.join(config_dir, "sim_config"),
#        delay_after_load_sec=2.0,  # this delay appears to hang tests when a failure occurs
    )
    drone = NonphysicsDrone(client, world, "Drone1")

    return drone, world


def save_image(image: Dict, img_name: str, save_path: pathlib.Path):
    """Callback to Save the image using OpenCV"""
    file_save_path = None
    if image is not None:
        img_np = np.reshape(image["data"], [image["height"], image["width"], 3])
        folder_path = pathlib.Path(save_path, "images")
        if not os.path.exists(folder_path):
            os.mkdir(folder_path)
        file_save_path = str(pathlib.Path(folder_path, img_name))
        cv2.imwrite(file_save_path, img_np)
    else:
        projectairsim_log().info(f"Image data not available for {img_name}")

    return file_save_path


def flatten_bbox2D(annotation):
    if "bbox2d" in annotation and annotation["bbox2d"] is not None:
        bb2D = annotation["bbox2d"]
        return [
            bb2D["center"]["x"],
            bb2D["center"]["y"],
            bb2D["size"]["x"],
            bb2D["size"]["y"],
        ]
    return None


def flatten_bbox3D(annotation):
    if "bbox3d" in annotation and annotation["bbox3d"] is not None:
        bb3D = annotation["bbox3d"]
        return [
            bb3D["center"]["x"],
            bb3D["center"]["y"],
            bb3D["center"]["z"],
            bb3D["size"]["x"],
            bb3D["size"]["y"],
            bb3D["size"]["z"],
        ]
    return None


def get_vertices(annotations, object_id):

    if object_id == "":
        projectairsim_log().info(
            f"No object_id defined for this location. Cannot get 3d bbox"
        )
        return [[0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]

    for annotation in annotations:
        if annotation["object_id"] == object_id:
            if (
                "bbox3d_in_image_space" in annotation
                and annotation["bbox3d_in_image_space"] is not None
            ):
                vertices = annotation["bbox3d_in_image_space"]
                rep = []
                for vertex in vertices:
                    x = vertex["x"]
                    y = vertex["y"]
                    rep.append([x, y])

                return rep

    projectairsim_log().info("Object not in 3d annotation data")
    return [[0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]


def get_2d_bbox(annotations, object_id):

    if object_id == "":
        projectairsim_log().info(
            f"No object_id defined for this location. Cannot get 2d bbox"
        )
        return [0, 0, 0, 0]

    for annotation in annotations:
        if annotation["object_id"] == object_id:
            if "bbox2d" in annotation and annotation["bbox2d"] is not None:
                bbox_2d = annotation["bbox2d"]
                center = bbox_2d["center"]
                dim = bbox_2d["size"]

                return [center["x"], center["y"], dim["x"], dim["y"]]
            else:
                projectairsim_log().info("No 2d bbox found")

    projectairsim_log().info("Object not in 2d annotation data")
    return [0, 0, 0, 0]


def bbox_2d_valid(bbox):
    if bbox != []:
        if bbox[2] == 0 and bbox[3] == 0:
            return False
    else:
        return False

    return True


def convert_to_pascal_voc_format(bbox_2d):
    x_c, y_c, w, h = bbox_2d
    x_min = float(x_c) - float(w) / 2
    y_min = float(y_c) - float(h) / 2
    x_max = float(x_c) + float(w) / 2
    y_max = float(y_c) + float(h) / 2
    return [x_min, y_min, x_max, y_max]


def convert_to_airsim_format(bbox_2d_pascal_voc):

    x_min, y_min, x_max, y_max = bbox_2d_pascal_voc[0]
    x_c = (float(x_min) + float(x_max)) / 2
    y_c = (float(y_min) + float(y_max)) / 2
    w = float(x_max) - float(x_min)
    h = float(y_max) - float(y_min)
    return [x_c, y_c, w, h]


def setup_save_path(save_path):

    os.makedirs(save_path, exist_ok=True)
    projectairsim_log().info("Writing to %s", save_path)


# Adopted from https://github.com/chrise96/image-to-coco-json-converter/blob/master/src/create_annotations.py
def create_sub_masks(image):
    # Initialize a dictionary of sub-masks indexed by RGB colors
    if image is None:
        return {}
    width = image["width"]
    height = image["height"]

    img_np = np.reshape(image["data"], [height, width, 3])
    mask_image = Image.fromarray(img_np.astype("uint8"), "RGB")
    sub_masks = {}

    for x in range(width):
        for y in range(height):
            # Get the RGB values of the pixel
            pixel = mask_image.getpixel((x, y))[:3]

            # Check to see if we have created a sub-mask...
            sub_mask = sub_masks.get(pixel)
            if sub_mask is None:
                # Create a sub-mask (one bit per pixel) and add to the dictionary
                # Note: we add 1 pixel of padding in each direction
                # because the contours module doesn"t handle cases
                # where pixels bleed to the edge of the image
                sub_masks[pixel] = Image.new("1", (width + 2, height + 2))

            # Set the pixel value to 1 (default is 0), accounting for padding
            sub_masks[pixel].putpixel((x + 1, y + 1), 1)

    return sub_masks


# Adopted from https://github.com/chrise96/image-to-coco-json-converter/blob/master/src/create_annotations.py
def create_sub_mask_annotation(sub_mask):
    # Find contours (boundary lines) around each sub-mask
    # Note: there could be multiple contours if the object
    # is partially occluded. (E.g. an elephant behind a tree)
    contours = measure.find_contours(
        np.array(sub_mask), 0.5, positive_orientation="low"
    )
    polygons = []
    for contour in contours:
        # Flip from (row, col) representation to (x, y)
        # and subtract the padding pixel
        for i in range(len(contour)):
            row, col = contour[i]
            contour[i] = (col - 1, row - 1)

        # Make a polygon and simplify it
        poly = Polygon(contour)
        poly = poly.simplify(1.0, preserve_topology=False)

        if poly.is_empty:
            # Go to next iteration, dont save empty values in list
            continue

        if type(poly) == Polygon:
            polygons.append(poly)

    return polygons


def get_segmenation_annotations(seg_image, categories: Dict, id_object_map: Dict):

    sub_masks = create_sub_masks(seg_image)
    segmentation_data = []
    category_id = len(categories)

    for colour, sub_mask in sub_masks.items():

        colour_bgr = colour[::-1]  # convert RGB to BGR
        colour_bgr = Color(colour_bgr)
        seg_id = segmentation_color_to_id(colour_bgr)  # Get seg_id
        object_id = id_object_map.get(seg_id)  # Get object_id from seg_id

        if object_id not in categories:
            categories[object_id] = category_id  # Keep track of all categories
            category_id += 1

        polygons = create_sub_mask_annotation(sub_mask)

        for polygon in polygons:
            segmentation = np.array(polygon.exterior.coords).ravel().tolist()
            annotation = COCOAnnotations(
                category_id=categories[object_id],
                polygon=polygon,
                segmentation=segmentation,
            )
            segmentation_data.append(annotation)

    return segmentation_data, categories


def get_seg_id_object_map(world: World):

    object_id: int
    seg_id: int
    seg_id_map: Dict[
        object_id, seg_id
    ] = world.get_segmentation_id_map()  # Dict with object_id - seg_id mapping
    id_object_map = dict(
        (v, k) for k, v in seg_id_map.items()
    )  # Dict with seg_id - object_id mapping

    return id_object_map


def set_weather(world: World, weather_value: int, weather_intensity):
    try:
        world.reset_weather_effects()
        world.enable_weather_visual_effects()
        world.set_weather_visual_effects_param(
            param=weather_value, value=weather_intensity
        )
        projectairsim_log().info(f"Sleeping for 3 seconds")
        time.sleep(3)  # wait for weather to change
    except Exception as e:
        projectairsim_log().info(f"Error changing weather - {e}")
        return False, world

    return True, world


def set_time(world: World, time_of_day: datetime.datetime):
    if time_of_day == None:
        return False, world
    try:
        world.set_time_of_day(True, time_of_day.isoformat(), False, 1.0, 1.0, False)
        world.set_sun_position_from_date_time(time_of_day, False)
        projectairsim_log().info(f"Sleeping for 1 seconds")
        time.sleep(1)  # wait for time to change
    except Exception as e:
        projectairsim_log().info(f"Error setting time - {e}")
        return False, world

    return True, world


def generate_voxel_grid(
    center: WayPoint,
    edge_len: float,
    res: float,
    scene_config: str,
    server_ip: str,
    config_dir="./",
):

    client = ProjectAirSimClient(address=server_ip)
    client.connect()
    drone, world = setup_scene(client, scene_config, config_dir)
    occupancy_map = world.create_voxel_grid(
        center.get_transform(), edge_len, edge_len, edge_len, res
    )
    client.disconnect()

    return occupancy_map


def setup_env_actor(client: ProjectAirSimClient, world: World, location: GeoLocation):

    if location.env_actor_name == "":  # No env actor setup
        return world, None

    env_actor = EnvActor(client, world, location.env_actor_name)

    if location.env_actor_trajectory == []:
        return world, env_actor

    env_trajectory_ned = utils.get_trajectory_ned(location.env_actor_trajectory)
    traj_duration = location.env_actor_trajectory_settings.get("duration")
    world.import_ned_trajectory(
        traj_name="test",
        time=list(
            np.linspace(
                0,
                traj_duration,
                len(location.env_actor_trajectory),
            )
        ),
        pose_x=env_trajectory_ned[0],
        pose_y=env_trajectory_ned[1],
        pose_z=env_trajectory_ned[2],
    )
    env_actor.set_trajectory(
        "test",
        to_loop=location.env_actor_trajectory_settings.get("loop"),
        time_offset=location.env_actor_trajectory_settings.get("time-offset"),
    )

    return world, env_actor
