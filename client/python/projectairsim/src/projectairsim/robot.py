"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Shared base class for ProjectAirSim robot clients.
"""

from typing import Dict


class Robot(object):
    def __init__(self, client, world, name: str):
        """Base robot client with common topic/sensor/kinematics APIs.

        Args:
            client: ProjectAirSim client object
            world: ProjectAirSim world object
            name (str): Name of the robot actor in the scene
        """
        self.client = client
        self.world = world
        self.name = name
        self.world_parent_topic = world.parent_topic
        self.home_geo_point = getattr(world, "home_geo_point", None)
        self.set_topics(world)

    def set_topics(self, world):
        """Sets up common topics for the robot. Called automatically."""
        self.parent_topic = f"{self.world_parent_topic}/robots/{self.name}"
        self.sensors_topic = f"{self.parent_topic}/sensors"
        self.set_sensor_topics(world)
        self.set_robot_info_topics()

    def set_sensor_topics(self, world):
        """Build sensor topic map from scene config."""
        self.sensors = {}
        scene_config_data = world.get_configuration()
        data = None

        for actor in scene_config_data["actors"]:
            if actor["name"] == self.name:
                data = actor["robot-config"]

        if data is None:
            raise Exception("Actor " + self.name + " not found in the config")

        if "sensors" not in data:
            return

        capture_setting_dict = {
            0: "scene_camera",
            1: "depth_planar_camera",
            2: "depth_camera",
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
                sub_cameras = sensor.get("capture-settings", [])
                for sub_camera in sub_cameras:
                    if sub_camera.get("capture-enabled", False):
                        image_type = capture_setting_dict[sub_camera["image-type"]]
                        self.sensors[name][image_type] = f"{sensor_root_topic}/{image_type}"
                        self.sensors[name][
                            f"{image_type}_info"
                        ] = f"{sensor_root_topic}/{image_type}_info"
            elif sensor_type == "radar":
                self.sensors[name][
                    "radar_detections"
                ] = f"{sensor_root_topic}/radar_detections"
                self.sensors[name]["radar_tracks"] = f"{sensor_root_topic}/radar_tracks"
            elif sensor_type == "imu":
                self.sensors[name]["imu_kinematics"] = f"{sensor_root_topic}/imu_kinematics"
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

    def set_robot_info_topics(self):
        """Sets up common robot info topics. Called automatically."""
        self.robot_info = {}
        self.robot_info["actual_pose"] = f"{self.parent_topic}/actual_pose"
        self.robot_info["collision_info"] = f"{self.parent_topic}/collision_info"

    def get_ground_truth_kinematics(self) -> Dict:
        """Get ground truth kinematics.

        Returns:
            Dict: the Kinematics
        """
        req = {
            "method": f"{self.parent_topic}/GetGroundTruthKinematics",
            "params": {},
            "version": 1.0,
        }
        return self.client.request(req)
