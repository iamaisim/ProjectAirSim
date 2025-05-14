"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Script for capturing images corresponding to provided poses and get a bounding box
representation of the landing pad in the image.
This script assumes the robot's physics is turned off in the config.

"""

import os
import pathlib
import time
from datetime import datetime
from typing import Dict, Tuple

import projectairsim.datacollection.collection.helper as helper
import projectairsim.datacollection.randomization.randomization as randomization
import projectairsim.datacollection.utils as utils
from projectairsim import ProjectAirSimClient, World
from projectairsim.datacollection.types import (
    Annotation,
    BboxData,
    Config,
    Dataset,
    GeoLocation,
    ImageData,
    Modality,
    WayPoint,
)
from projectairsim.geodetic_converter import GeodeticConverter
from projectairsim.types import ImageType, Pose, Quaternion, Vector3
from projectairsim.utils import projectairsim_log, rpy_to_quaternion

world: World = None
client: ProjectAirSimClient = None
drone: helper.NonphysicsDrone = None


# Spawn assets on given locations
def spawn_assets(
    env_spec: Dict,
    location: GeoLocation,
    connection_string: str,
    rand_index: int,
):
    global world
    assets: Dict = env_spec["assets"]

    if location.asset == "":
        projectairsim_log().info(
            f"No asset to spawn for location {location.config_dir}"
        )
        # continue
        return

    if location.asset not in assets:
        projectairsim_log().info(
            f"Asset {location.asset} not configured. Moving On"
        )
        # continue
        return

    asset: Dict = assets.get(location.asset)

    # Limitation of current datacollection: one asset per scene. The following active
    # augmentation code assumes this notion. Optimally we'd loop through multiple
    # entities and spawn all of them in their randomized positions.
    entity: randomization.Entity = None
    randomized_pose: WayPoint = None
    converter = GeodeticConverter(
        location.latitude, location.longitude, location.altitude
    )
    if rand_index > -1:
        for randomized_entity in location.randomized_entities:
            if randomized_entity.type == location.asset:
                entity = randomized_entity
                entity.convert_coordinates_from_geo_to_UE(converter)
                randomized_pose = entity.poses[rand_index]
                break

    if randomized_pose and randomized_pose.altered == "scale":
        asset_scale = randomized_pose.scale_xyz
    else:
        asset_scale = asset.get("scale")

    # Set value with randomization. Optimally, we'd simply grab the pose and not worry
    # about individual components like we're doing here.
    wxyz: Tuple[float, float, float, float]  # Angular rotation.
    if randomized_pose and (
        randomized_pose.altered == "rotate" or randomized_pose.altered == "flip"
    ):
        roll = randomized_pose.roll
        pitch = randomized_pose.pitch
        yaw = randomized_pose.yaw
        wxyz = rpy_to_quaternion(roll, pitch, yaw)
    else:
        wxyz = asset.get("rotation")
    asset_rotation = Quaternion(
        {"w": wxyz[0], "x": wxyz[1], "y": wxyz[2], "z": wxyz[3]}
    )
    asset_name = location.object_id
    coordinates = entity.origin_xyz if entity else location.scene_coordinates

    if randomized_pose and randomized_pose.altered == "translate":
        coordinates = randomized_pose.pose_list

    asset_translation = Vector3(
        {"x": coordinates[0], "y": coordinates[1], "z": coordinates[2]}
    )
    asset_pose: Pose = Pose(
        {
            "translation": asset_translation,
            "rotation": asset_rotation,
            "frame_id": "DEFAULT_ID",
        }
    )
    seg_id = int(asset.get("seg-id"))

    if asset["source"] == "GLTF":
        file = open(asset["file-name"], "rb")
        gltf_data = file.read()
        file.close()
        world.spawn_object_from_file(
            asset_name,
            "gltf",
            gltf_data,
            True,
            asset_pose,
            asset_scale,
            False,
        )
    elif asset["source"] == "UnrealProject":
        world.spawn_object(
            asset_name,
            asset["file-name"],
            asset_pose,
            asset_scale,
            False,
        )
    elif asset["source"] == "Azure":
        blob_data = utils.download_blob(
            connection_string, asset["container-name"], asset["file-name"]
        )
        azure_data = bytearray(blob_data)
        world.spawn_object_from_file(
            asset_name,
            "gltf",
            azure_data,
            True,
            asset_pose,
            asset_scale,
            False,
        )
    world.set_segmentation_id_by_name(asset_name, seg_id, False, True)
    return


# Function to collect images corresponding to the poses
def collect_data(
    geo_locations: Dict[str, GeoLocation],
    config: Config,
    server_ip: str,
    save_path: pathlib.Path,
    connection_string: str,
    start_index: int,
):
    global client, world, drone
    helper.setup_save_path(save_path)

    dataset = Dataset()

    # Create client
    client = ProjectAirSimClient(server_ip)
    client.connect()

    index = start_index
    try:
        # Loop over each geo location
        for location_name in geo_locations.keys():
            location = geo_locations[location_name]
            # Reset time, weather, scene for each location
            cur_weather = None
            cur_time_of_day = None
            cur_config = None

            # Run each location with augmentation
            # Run through default setup, then do augmentations
            # Would much rather loop through the poses, but this is least invasive.
            # -1 indicates to skip randomizations
            starting_index = -1
            number_of_randomizations = 0
            if location.randomized_entities:
                # Limitation of current datacollection: one asset per scene. The
                # following active augmentation code assumes this notion. Optimally we'd
                # loop through multiple entities and spawn all of them in their
                # randomized positions.
                entity: randomization.Entity

                # Find an entity that matches with the location asset.
                for randomized_entity in location.randomized_entities:
                    if randomized_entity.type == location.asset:
                        # Count the number of unique poses.
                        number_of_randomizations = len(randomized_entity.poses)
                        break
            for rand_index in range(starting_index, number_of_randomizations):
                # When rand_index == -1, the default configs are used. After,
                # rand_index will spawn the asset in its randomized configuration.
                cur_config = None
                # Loop over the desired poses and capture images
                for trajectory_index, pose in enumerate(location.trajectory):
                    pose: WayPoint
                    weather: Dict = pose.weather
                    weather_value = weather.get("value", 0)
                    weather_type = weather.get("type", "ENABLED")
                    weather_intensity = weather.get("intensity", 0.5)
                    time_of_day: datetime = pose.time

                    # Change sim scene config if required
                    if location.scene_config != cur_config:
                        # Setup scene
                        drone, world = helper.setup_scene(
                            client, location.scene_config, config.dir
                        )
                        world.enable_weather_visual_effects()
                        projectairsim_log().info(
                            f"Changing scene to {location.scene_config}. Sleeping for 5 seconds"
                        )
                        time.sleep(5)
                        # Spawn Assets and get their segmentation ID's
                        spawn_assets(
                            config.env_spec,
                            location,
                            connection_string,
                            rand_index,
                        )
                        seg_id_object_map = helper.get_seg_id_object_map(world)
                        projectairsim_log().info(
                            "**** Spawned private assets ****"
                        )
                        # Setup Env Actor
                        world, env_actor = helper.setup_env_actor(
                            client, world, location
                        )
                        # Subscribe to the drone's pose
                        client.subscribe(
                            drone.robot_info["actual_pose"],
                            drone.callback_actual_pose,
                        )
                        # Update cur-config
                        cur_config = location.scene_config
                        # Note scene and robot configs for downstream visibility
                        dataset.add_config(
                            location.name,
                            os.path.basename(world.scene_config_path),
                            os.path.basename(world.robot_config_paths[0]),
                        )

                    # Change weather if required
                    if weather_type != cur_weather:
                        cur_weather = weather_type
                        projectairsim_log().info(
                            f"Changing weather to {weather_type} at index-{index}, randomization_index-{rand_index}"
                        )
                        weather_change_success, world = helper.set_weather(
                            world, weather_value, weather_intensity
                        )

                    # Change time of day if required
                    if time_of_day != cur_time_of_day:
                        cur_time_of_day = time_of_day
                        projectairsim_log().info(
                            f"Changing time of day to '{time_of_day}' at index-{index}, randomization_index-{rand_index}"
                        )
                        time_change_success, world = helper.set_time(
                            world, time_of_day
                        )

                    # Set drone to current pose
                    pose.yaw = utils.clamp(pose.yaw)
                    drone.set_pose(pose.get_transform(), True)
                    projectairsim_log().info(
                        f"---------------------------------------"
                    )

                    # Output current status logs
                    projectairsim_log().info(
                        f"Collecting data at pose #{trajectory_index}/{len(location.trajectory)}"
                    )
                    projectairsim_log().info(
                        f"[Index:{index}]; [Location:{location.name}]; [Rand-Index:{rand_index}]"
                    )
                    projectairsim_log().info(f"Pose: {drone.cur_pose}")

                    # Get rgb/seg images from drone
                    try:
                        images = drone.get_images(
                            camera_id="DownCamera",
                            image_type_ids=[
                                ImageType.SCENE,
                                ImageType.SEGMENTATION,
                            ],
                        )
                    except Exception as e:
                        projectairsim_log().warning(
                            f"GetImages failed with the following exception: {e}",
                            exc_info=True,
                        )
                        continue

                    rgb = images[ImageType.SCENE]
                    image_bbox_data = rgb["annotations"]

                    image_data = ImageData(
                        id=index,
                        randomization_index=rand_index,
                        location=location,
                        weather=cur_weather,
                        time_of_day=cur_time_of_day.isoformat()
                        if cur_time_of_day is not None
                        else "",
                        pose=pose.pose_list,
                    )

                    bbox_data = BboxData(category=location.asset)

                    # Save data according to enabled modalities/annotations
                    for modality in config.data_spec.get("modalities"):
                        if modality == Modality.RGB:
                            rgb_img_path = helper.save_image(
                                rgb, image_data.name, save_path
                            )
                            image_data.add_image_properties(
                                rgb["height"], rgb["width"]
                            )

                            # Save and Wait for images to be saved
                            while not os.path.exists(rgb_img_path):
                                time.sleep(0.0001)
                            projectairsim_log().info(
                                f"RGB Image Saved to {rgb_img_path}"
                            )

                    for annotation in config.data_spec.get("annotations"):
                        if annotation == Annotation.TWO_D_BBOX:
                            bbox_2d = helper.get_2d_bbox(
                                image_bbox_data, location.object_id
                            )  # [x_c, y_c, w, h]
                            bbox_data.add_2d_bbox(bbox_2d)

                        elif annotation == Annotation.THREE_D_BBOX:
                            bbox_3d = helper.get_vertices(
                                image_bbox_data, location.object_id
                            )  # [x0,y0,x1,y1....x7,y7]
                            bbox_data.add_3d_bbox(bbox_3d)

                        elif annotation == Annotation.SEGMENTATION:
                            seg = images[ImageType.SEGMENTATION]
                            (
                                segmentation_data,
                                categories,
                            ) = helper.get_segmenation_annotations(
                                seg, dataset.categories, seg_id_object_map
                            )
                            image_data.add_segmentation_data(segmentation_data)
                            dataset.update_categories(categories)

                    image_data.add_bbox_data(bbox_data)
                    dataset.images.append(image_data)
                    index += 1

        return dataset
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
    finally:
        client.disconnect()
        return dataset
