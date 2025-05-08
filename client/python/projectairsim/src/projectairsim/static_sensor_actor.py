"""
Copyright (C) Microsoft Corporation. All rights reserved.
Python client for ProjectAirSim static sensor actor (non-physics robot).
"""

import json

from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log
from typing import List, Dict


class StaticSensorActor(object):
    def __init__(self, client: ProjectAirSimClient, world: World, name: str):
        """ProjectAirSim Static Sensor Actor Interface

        Arguments:
            client {ProjectAirSimClient} -- ProjectAirSim client object
            world {World} -- ProjectAirSim world object
            name {str} -- Name of the static sensor actor in the scene
        """
        projectairsim_log().info(f"Initalizing static sensor actor  '{name}'...")
        self.client = client
        self.name = name
        self.world_parent_topic = world.parent_topic
        self.set_topics(world)
        self.log_topics()

    def set_topics(self, world: World):
        self.parent_topic = f"{self.world_parent_topic}/robots/{self.name}"
        self.sensors_topic = f"{self.parent_topic}/sensors"
        self.set_sensor_topics(world)

    def set_sensor_topics(self, world: World):
        self.sensors = {}
        scene_config_data = world.get_configuration()
        data = None

        for actor in scene_config_data["actors"]:
            if actor["name"] == self.name:
                data = actor["robot-config"]

        if data is None:
            raise Exception("Actor " + self.name + " not found in the config")

        capture_setting_dict = {
            0: "scene_camera",  # TODO rename scene_camera topic to rgb_camera
            1: "depth_camera",
            2: "depth_planar_camera",
            3: "segmentation_camera",
            4: "depth_vis_camera",
            5: "disparity_normalized_camera",
            6: "surface_normals_camera",
        }

        for sensor in data["sensors"]:
            name = sensor["id"]
            sensor_type = sensor["type"]
            sensor_root_topic = f"{self.sensors_topic}/{name}"
            self.sensors[name] = {}
            if sensor_type == "camera":
                sub_cameras = sensor["capture-settings"]
                # Based on 'image-type' within the camera, set up the topic paths
                for sub_camera in sub_cameras:
                    image_type = capture_setting_dict[sub_camera["image-type"]]
                    self.sensors[name][image_type] = f"{sensor_root_topic}/{image_type}"
            elif sensor_type == "radar":
                self.sensors[name][
                    "radar_detections"
                ] = f"{sensor_root_topic}/radar_detections"
                self.sensors[name]["radar_tracks"] = f"{sensor_root_topic}/radar_tracks"
            elif sensor_type == "imu":
                self.sensors[name][
                    "imu_kinematics"
                ] = f"{sensor_root_topic}/imu_kinematics"
            elif sensor_type == "gps":
                self.sensors[name]["gps"] = f"{sensor_root_topic}/gps"
            elif sensor_type == "airspeed":
                self.sensors[name]["airspeed"] = f"{sensor_root_topic}/airspeed"
            elif sensor_type == "barometer":
                self.sensors[name]["barometer"] = f"{sensor_root_topic}/barometer"
            elif sensor_type == "magnetometer":
                self.sensors[name]["magnetometer"] = f"{sensor_root_topic}/magnetometer"
            elif sensor_type == "lidar":
                self.sensors[name]["lidar"] = f"{sensor_root_topic}/lidar"
            elif sensor_type == "distance-sensor":
                self.sensors[name]["distance_sensor"] = f"{sensor_root_topic}/distance_sensor"
            elif sensor_type == "battery":
                self.sensors[name]["battery"] = f"{sensor_root_topic}/battery"
            else:
                raise Exception(
                    f"Unknown sensor type '{sensor_type}' found in config "
                    f"for sensor '{name}'"
                )

    def log_topics(self):
        projectairsim_log().info("-------------------------------------------------")
        projectairsim_log().info(
            "The following topics can be subscribed to for "
            f"static sensor actor '{self.name}':"
        )
        for sensor in self.sensors.keys():
            for name in self.sensors[sensor].keys():
                projectairsim_log().info(f'    sensors["{sensor}"]["{name}"]')
        projectairsim_log().info("-------------------------------------------------")

    def get_images(self, camera_id: str, image_type_ids: List[int]) -> Dict:
        """Get a set of images from a camera sensor (server will wait until the next
           set of captured images is available with a sim timestamp >= sim time when
           the request was received)
        Returns:
            Dict of image data for the requested images with keys by type IDs (Dict)
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        get_images_req = {
            "method": f"{req_camera_path}/GetImages",
            "params": {"image_type_ids": image_type_ids},
            "version": 1.0,
        }
        get_images_data = self.client.request(get_images_req)

        # Since JSON doesn't support int keys, the returned dict has the ImageType int
        # keys as strings. Convert keys from strings of int to native int types for
        # easier direct access with the IntEnum ImageType.
        str_keys = [*get_images_data]
        for key in str_keys:
            get_images_data[int(key)] = get_images_data.pop(key)

        return get_images_data
