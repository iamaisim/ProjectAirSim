"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Python client for ProjectAirSim Rover robots/actors.
"""

import asyncio
import math
import time

from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log, geo_to_ned_coordinates
from typing import List, Dict
from projectairsim.types import Pose


class Rover(object):
    def __init__(self, client: ProjectAirSimClient, world: World, name: str):
        """ProjectAirSim Rover Actor Interface

        Args:
            client (ProjectAirSimClient): ProjectAirSim client object
            world (World): ProjectAirSim world object
            name (str): Name of the Rover actor in the scene
        """
        projectairsim_log().info(f"Initalizing Rover '{name}'...")
        self.client = client
        self.world = World
        self.name = name
        self.world_parent_topic = world.parent_topic
        self.set_topics(world)
        self.vel_cmd = {"axes_0": 0.0, "axes_1": 0.0, "axes_2": 0.0, "axes_3": 0.0}
        self.home_geo_point = world.home_geo_point
        self.axis_mapping = {
            "north": "axes_0",
            "east": "axes_1",
            "down": "axes_2",
            "yaw": "axes_3",
        }
        projectairsim_log().info(
            f"Rover '{self.name}' initialized for "
            f"World scene '{self.world_parent_topic}'"
        )

    def arm(self) -> bool:
        """Arms vehicle

        Returns:
            bool: True if drone is armed
        """
        arm_req: Dict = {
            "method": f"{self.parent_topic}/Arm",
            "params": {},
            "version": 1.0,
        }
        armed = self.client.request(arm_req)
        return armed

    def can_arm(self) -> bool:
        """Checks if the vehicle can be armed

        Returns:
            bool: True if drone can be armed
        """
        can_arm_req: Dict = {
            "method": f"{self.parent_topic}/CanArm",
            "params": {},
            "version": 1.0,
        }
        can_arm = self.client.request(can_arm_req)
        return can_arm

    def cancel_last_task(self) -> bool:
        """Cancels vehicle's last controller task

        Returns:
            bool: True if task has been cancelled
        """
        cancel_req: Dict = {
            "method": f"{self.parent_topic}/CancelLastTask",
            "params": {},
            "version": 1.0,
        }
        cancelled = self.client.request(cancel_req)
        return cancelled

    def disable_api_control(self) -> bool:
        """Disable Wheeled Ground Robot control using API calls

        Returns:
            bool: True if ApiControl is disabled
        """
        disable_api_control_req: Dict = {
            "method": f"{self.parent_topic}/DisableApiControl",
            "params": {},
            "version": 1.0,
        }
        api_control_disabled = self.client.request(disable_api_control_req)
        return api_control_disabled

    def disarm(self) -> bool:
        """Disarms vehicle

        Returns:
            bool: True if drone is disarmed
        """
        disarm_req: Dict = {
            "method": f"{self.parent_topic}/Disarm",
            "params": {},
            "version": 1.0,
        }
        disarmed = self.client.request(disarm_req)
        return disarmed

    def enable_api_control(self) -> bool:
        """Enable Rover control using API calls

        Returns:
            bool: True if ApiControl is enabled.
        """
        enable_api_control_req: Dict = {
            "method": f"{self.parent_topic}/EnableApiControl",
            "params": {},
            "version": 1.0,
        }
        api_control_enabled = self.client.request(enable_api_control_req)
        return api_control_enabled

    def get_ground_truth_kinematics(self) -> Dict:
        """Get ground truth kinematics

        Returns:
            Dict: the Kinematics
        """
        gt_kinematics_req = {
            "method": f"{self.parent_topic}/GetGroundTruthKinematics",
            "params": {},
            "version": 1.0,
        }
        gt_kinematics_data = self.client.request(gt_kinematics_req)
        return gt_kinematics_data

    def log_topics(self):
        """Logs a human-readable list of all topics associated with the Rover"""
        projectairsim_log().info("-------------------------------------------------")
        projectairsim_log().info(
            f"The following topics can be subscribed to for Rover'{self.name}':",
        )
        for sensor in self.sensors.keys():
            for name in self.sensors[sensor].keys():
                projectairsim_log().info(f'    sensors["{sensor}"]["{name}"]')
        for name in self.robot_info.keys():
            projectairsim_log().info(f'    rover_info["{name}"]')
        projectairsim_log().info("-------------------------------------------------")

    async def move_to_position_async(
        self,
        north: float,
        east: float,
        velocity: float,
        timeout_sec: float = 3e38,
        yaw_rate_max: float = -1,  # Unlimited
        lookahead: float = -1.0,
        adaptive_lookahead: float = 1.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move to position. Control returns back to the caller immediately.

        Args:
            north (float): the desired position north-coordinate (m)
            east (float): the desired position east-coordinate (m)
            velocity (float): the desired velocity (m/s)
            timeout_sec (float): timeout for the command (seconds)
            yaw_rate_max (float): the maximum desired yaw rate, ignored if < 0 (radians/second)
            lookahead (float): the amount of lookahead for the command
            adaptive_lookahead (float): the amount of adaptive lookahead for the command
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "x": north,
            "y": east,
            "velocity": velocity,
            "timeout_sec": timeout_sec,
            "yaw_rate_max": yaw_rate_max,
            "lookahead": lookahead,
            "adaptive_lookahead": adaptive_lookahead,
        }
        req: Dict = {
            "method": f"{self.parent_topic}/MoveToPosition",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def move_by_heading_async(
        self,
        heading: float,
        speed: float,
        duration: float = 3,
        heading_margin: float = math.radians(5.0),
        yaw_rate: float = 5,
        timeout_sec: float = 3e38,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move by heading, horizontal speed, and vertical velocity

        Args:
            heading (float): Heading in world coordinates (radians)
            speed (float): Desired speed in world (NED) X-Y plane (m/s)
            duration (float): How long to fly at heading (seconds)
            heading_margin (float): How close to specified heading vehicle must be before starting flight duration countdown, in radians
            yaw_rate (float): Desired yaw rate to heading, <= 0 means as quickly as possible (radians/s)
            timeout_sec (float): Command timeout (seconds)
            callback (callable): Callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        req: Dict = {
            "method": f"{self.parent_topic}/MoveByHeading",
            "params": {
                "heading": heading,
                "speed": speed,
                "duration": duration,
                "heading_margin": heading_margin,
                "yaw_rate": yaw_rate,
                "timeout_sec": timeout_sec,
            },
            "version": 1.0,
        }
        taskcr = await self.client.request_async(req, callback)
        return taskcr

    def set_pose(self, pose: Pose, reset_kinematics=True) -> bool:
        """Sets Pose for NonPhysics rover

        Args:
            pose (Pose): the desired pose
            reset_kinematics (bool): if true, the object's velocity etc will be reset

        Returns:
            bool: whether the operation was successful
        """

        set_pose_req = {
            "method": f"{self.parent_topic}/SetPose",
            "params": {"pose": pose, "reset_kinematics": reset_kinematics},
            "version": 1.0,
        }
        pose_set = self.client.request(set_pose_req)
        return pose_set

    async def set_rover_controls(
        self, engine, steering_angle, brake, callback: callable = None
    ) -> asyncio.Task:
        """
        Control the rover using engine and steering angle.
        Args:
            engine: Value for engine input.
            steering_angle: Negative yaw is to the left. Positive yaw to the right.
            brake: Force to stop the car
        """

        params: Dict = {
            "engine": engine,
            "steering_angle": steering_angle,
            "brake": brake,
        }

        set_rover_controls_req: Dict = {
            "method": f"{self.parent_topic}/SetRoverControls",
            "params": params,
            "version": 1.0,
        }

        async_task = await self.client.request_async(set_rover_controls_req, callback)
        return async_task

    def set_sensor_topics(self, world: World):
        """Sets up sensor topics for the rover. Called automatically.

        Args:
            world (World): the associated ProjectAirSim World object
        """
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
                if sub_camera["capture-enabled"]:
                    sub_cameras = sensor["capture-settings"]
                    # Based on 'image-type' within the camera, set up the topic paths
                    for sub_camera in sub_cameras:
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
                self.sensors[name][
                    "distance_sensor"
                ] = f"{sensor_root_topic}/distance_sensor"
            elif sensor_type == "battery":
                self.sensors[name]["battery"] = f"{sensor_root_topic}/battery"
            else:
                raise Exception(
                    f"Unknown sensor type '{sensor_type}' found in config "
                    f"for sensor '{name}'"
                )

    def set_robot_info_topics(self):
        """Sets up robot info topics for the rover. Called automatically"""
        self.robot_info = {}
        self.robot_info["actual_pose"] = f"{self.parent_topic}/actual_pose"
        self.robot_info["collision_info"] = f"{self.parent_topic}/collision_info"
        self.robot_info["rotor_info"] = f"{self.parent_topic}/rotor_info"

    def set_topics(self, world: World):
        """Sets up all topics for the rover. Called automatically.

        Args:
            world (World): the associated ProjectAirSim World object
        """
        self.parent_topic = f"{self.world_parent_topic}/robots/{self.name}"
        self.sensors_topic = f"{self.parent_topic}/sensors"
        # self.set_sensor_topics(world)
        self.set_robot_info_topics()

    async def wait_until_stopped_async(
        self, timeout_sec: float = 30, speed_tolerance: float = 1e-5
    ):
        """Waits until the rover has stopped moving.

        Args:
            timeout_sec (float): Maximum time to wait (seconds)
            speed_tolerance (float): Maximum vehicle speed for the vehicle to be considered stopped
        """
        dist_sq = 1
        speed_tolerance_sq = speed_tolerance**2
        position_last = self.get_ground_truth_kinematics()["pose"]["position"]
        sec_start = time.monotonic()
        sec_last = sec_start
        while True:
            await asyncio.sleep(0.1)
            position = self.get_ground_truth_kinematics()["pose"]["position"]

            sec_cur = time.monotonic()
            if (sec_cur - sec_start) >= timeout_sec:
                raise TimeoutError()

            dx = position["x"] - position_last["x"]
            dy = position["y"] - position_last["y"]
            position_last = position

            dist_sq = dx**2 + dy**2
            dist_tolerance = speed_tolerance * (sec_cur - sec_last)
            if abs(dist_sq) <= (dist_tolerance**2):
                break

            sec_last = sec_cur
