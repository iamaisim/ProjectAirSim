"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Python client for ProjectAirSim Drone robots/actors.
"""

import json
import asyncio
import math

from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log, geo_to_ned_coordinates
from typing import List, Dict
from projectairsim.types import Pose, Quaternion, Vector3, LandedState, ImageType


class YawControlMode:
    MaxDegreeOfFreedom = 0
    ForwardOnly = 1


class Drone(object):
    class VTOLMode:
        Multirotor = 0  # Multirotor (helicopter) mode
        FixedWing = 1  # Fixed-wing mode when possible, multirotor otherwise

    def __init__(self, client: ProjectAirSimClient, world: World, name: str):
        """ProjectAirSim Drone Actor Interface

        Args:
            client (ProjectAirSimClient): ProjectAirSim client object
            world (World): ProjectAirSim world object
            name (str): Name of the Drone actor in the scene
        """
        projectairsim_log().info(f"Initalizing Drone '{name}'...")
        self.client = client
        self.world = World
        self.name = name
        self.world_parent_topic = world.parent_topic
        self.set_topics(world)
        self.log_topics()
        self.vel_cmd = {"axes_0": 0.0, "axes_1": 0.0, "axes_2": 0.0, "axes_3": 0.0}
        self.home_geo_point = world.home_geo_point
        self.axis_mapping = {
            "north": "axes_0",
            "east": "axes_1",
            "down": "axes_2",
            "yaw": "axes_3",
        }
        projectairsim_log().info(
            f"Drone '{self.name}' initialized for "
            f"World scene '{self.world_parent_topic}'"
        )

    def set_topics(self, world: World):
        """Sets up all topics for the Drone. Called automatically.

        Args:
            world (World): the associated ProjectAirSim World object
        """
        self.parent_topic = f"{self.world_parent_topic}/robots/{self.name}"
        self.sensors_topic = f"{self.parent_topic}/sensors"
        self.set_sensor_topics(world)
        self.set_robot_info_topics()

    def set_sensor_topics(self, world: World):
        """Sets up sensor topics for the drone. Called automatically.

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
            0: "scene_camera",  # TODO rename scene_camera topic to rgb_camera
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
                sub_cameras = sensor["capture-settings"]
                # Based on 'image-type' within the camera, set up the topic paths
                for sub_camera in sub_cameras:
                    if sub_camera["capture-enabled"]:
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
        """Sets up robot info topics for the Drone. Called automatically"""
        self.robot_info = {}
        self.robot_info["actual_pose"] = f"{self.parent_topic}/actual_pose"
        self.robot_info["current_waypoint_number"] = f"{self.parent_topic}/multirotor_api/current_waypoint_number" # for move_on_path_async
        self.robot_info["collision_info"] = f"{self.parent_topic}/collision_info"
        self.robot_info["rotor_info"] = f"{self.parent_topic}/rotor_info"
        self.robot_info["gt_kinematics"] = f"{self.parent_topic}/gt_kinematics"

    def log_topics(self):
        """Logs a human-readable list of all topics associated with the drone"""
        projectairsim_log().info("-------------------------------------------------")
        projectairsim_log().info(
            f"The following topics can be subscribed to for robot '{self.name}':",
        )
        for sensor in self.sensors.keys():
            for name in self.sensors[sensor].keys():
                projectairsim_log().info(f'    sensors["{sensor}"]["{name}"]')
        for name in self.robot_info.keys():
            projectairsim_log().info(f'    robot_info["{name}"]')
        projectairsim_log().info("-------------------------------------------------")

    def enable_api_control(self) -> bool:
        """Enable drone control using API calls

        Returns:
            bool: True if ApiControl is enabled
        """
        enable_api_control_req: Dict = {
            "method": f"{self.parent_topic}/EnableApiControl",
            "params": {},
            "version": 1.0,
        }
        api_control_enabled = self.client.request(enable_api_control_req)
        return api_control_enabled

    def disable_api_control(self) -> bool:
        """Disable drone control using API calls

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

    def is_api_control_enabled(self) -> bool:
        """Check if drone control using API calls is enabled

        Returns:
            bool: True if ApiControl is enabled
        """
        is_api_control_enabled_req: Dict = {
            "method": f"{self.parent_topic}/IsApiControlEnabled",
            "params": {},
            "version": 1.0,
        }
        api_control_enabled = self.client.request(is_api_control_enabled_req)
        return api_control_enabled

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

    def get_ready_state(self) -> Dict:
        """Checks if the vehicle is ready

        Returns:
            Dict: whether the vehicle is ready, and a status message
        """
        is_ready_req: Dict = {
            "method": f"{self.parent_topic}/GetReadyState",
            "params": {},
            "version": 1.0,
        }
        is_ready = self.client.request(is_ready_req)
        return is_ready

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

    async def takeoff_async(
        self, timeout_sec=20, callback: callable = None
    ) -> asyncio.Task:
        """
        Takeoff vehicle to 3m above ground. Vehicle should not be moving when this API is used

        Args:
            timeout_sec (int): Timeout for the vehicle to reach desired altitude
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        params: Dict = {"timeout_sec": timeout_sec}

        req: Dict = {
            "method": f"{self.parent_topic}/Takeoff",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def land_async(
        self, timeout_sec=3e38, callback: callable = None
    ) -> asyncio.Task:
        """
        Land the vehicle

        Args:
            timeout_sec (int): Timeout for the vehicle to land
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        params: Dict = {"timeout_sec": timeout_sec}

        req: Dict = {
            "method": f"{self.parent_topic}/Land",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    def get_landed_state(self) -> LandedState:
        """Checks if the vehicle is landed

        Returns:
            LandedState: 0 if drone is landed, 1 if flying
        """
        landed_state_req: Dict = {
            "method": f"{self.parent_topic}/GetLandedState",
            "params": {},
            "version": 1.0,
        }
        landed_state = self.client.request(landed_state_req)
        return landed_state

    async def go_home_async(
        self, timeout_sec=60, velocity=0.5, callback: callable = None
    ) -> asyncio.Task:
        """
        Return vehicle to Home i.e. Launch location

        Args:
            timeout_sec (int): Timeout for the vehicle to reach home
            velocity (float): the speed at which to travel (m/s)
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        params: Dict = {"timeout_sec": timeout_sec, "velocity": velocity}

        req: Dict = {
            "method": f"{self.parent_topic}/GoHome",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def hover_async(self, callback: callable = None) -> asyncio.Task:
        """
        Hovers the vehicle at the current Z

        Args:
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        params: Dict = {}

        req: Dict = {
            "method": f"{self.parent_topic}/Hover",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def move_by_heading_async(
        self,
        heading: float,
        speed: float,
        v_down: float = 0.0,
        duration: float = 0.001,
        heading_margin: float = math.radians(5.0),
        yaw_rate: float = 0,
        timeout_sec: float = 3e38,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move by heading, horizontal speed, and vertical velocity

        Args:
            heading (float): Heading in world coordinates (radians)
            speed (float): Desired speed in world (NED) X-Y plane (m/s)
            v_down (float): Desired velocity in world (NED) Z axis (m/s)
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
                "vz": v_down,
                "duration": duration,
                "heading_margin": heading_margin,
                "yaw_rate": yaw_rate,
                "timeout_sec": timeout_sec,
            },
            "version": 1.0,
        }
        taskcr = await self.client.request_async(req, callback)
        return taskcr

    async def move_by_velocity_async(
        self,
        v_north: float,
        v_east: float,
        v_down: float,
        duration: float = 0.001,
        yaw_control_mode: YawControlMode = YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move by velocity. Control returns back to the caller immediately.

        Args:
            v_north (float): desired velocity in world (NED) X axis (m/s)
            v_east (float): desired velocity in world (NED) Y axis (m/s)
            v_down (float): desired velocity in world (NED) Z axis (m/s)
            duration (float): Desired amount of time (seconds), to send this command for
            yaw_control_mode (YawControlMode): the yaw control mode for the command
            yaw_is_rate (bool): whether yaw is absolute or rate, optional
            yaw (float): yaw angle (rad) or rate (rad/s)
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "vx": v_north,
            "vy": v_east,
            "vz": v_down,
            "duration": duration,
            "drivetrain": yaw_control_mode,
            "yaw_is_rate": yaw_is_rate,
            "yaw": yaw,
        }

        req: Dict = {
            "method": f"{self.parent_topic}/MoveByVelocity",
            "params": params,
            "version": 1.0,
        }

        async_task = await self.client.request_async(req, callback)
        return async_task

    async def move_by_velocity_z_async(
        self,
        v_north: float,
        v_east: float,
        z: float,
        duration: float = 0.001,
        yaw_control_mode: YawControlMode = YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move by velocity at a specific Z. Control returns back to the caller immediately.

        Args:
            v_north (float): desired velocity in world (NED) X axis (m/s)
            v_east (float): desired velocity in world (NED) Y axis (m/s)
            z (float): desired z (m) [in NED]
            duration (float): Desired amount of time (seconds), to send this command for
            yaw_control_mode (YawControlMode): the yaw control mode for the command
            yaw_is_rate (bool): whether the yaw is absolute or a rate
            yaw (float): yaw angle (rad) or rate (rad/s)
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "vx": v_north,
            "vy": v_east,
            "z": z,
            "duration": duration,
            "drivetrain": yaw_control_mode,
            "yaw_is_rate": yaw_is_rate,
            "yaw": yaw,
        }

        req: Dict = {
            "method": f"{self.parent_topic}/MoveByVelocityZ",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def move_by_velocity_body_frame_async(
        self,
        v_forward: float,
        v_right: float,
        v_down: float,
        duration: float = 0.001,
        yaw_control_mode: YawControlMode = YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move by velocity. Control returns back to the caller immediately.

        Args:
            v_forward (float): desired velocity in drone forward (X) axis (m/s)
            v_right (float): desired velocity in drone right (Y) axis (m/s)
            v_down (float): desired velocity in drone Z axis (m/s)
            duration (float): Desired amount of time (seconds), to send this command for
            yaw_control_mode (YawControlMode):  the yaw control mode for the command
            yaw_is_rate (bool): whether yaw is absolute or rate, optional
            yaw (float): yaw angle (rad) or rate (rad/s)
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "vx": v_forward,
            "vy": v_right,
            "vz": v_down,
            "duration": duration,
            "drivetrain": yaw_control_mode,
            "yaw_is_rate": yaw_is_rate,
            "yaw": yaw,
        }

        req: Dict = {
            "method": f"{self.parent_topic}/MoveByVelocityBodyFrame",
            "params": params,
            "version": 1.0,
        }

        async_task = await self.client.request_async(req, callback)
        return async_task

    async def move_by_velocity_body_frame_z_async(
        self,
        v_forward: float,
        v_right: float,
        z: float,
        duration: float = 0.001,
        yaw_control_mode: YawControlMode = YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move by velocity at a specific Z. Control returns back to the caller immediately.

        Args:
            v_forward (float): desired velocity in drone forward (X) axis (m/s)
            v_right (float): desired velocity in drone right (Y) axis (m/s)
            z (float): desired z (m) [in NED]
            duration (float): Desired amount of time (seconds), to send this command for
            yaw_control_mode (YawMode): the yaw control mode for the command
            yaw_is_rate (bool): whether yaw is absolute or rate, optional
            yaw (float): yaw angle (rad) or rate (rad/s)
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "vx": v_forward,
            "vy": v_right,
            "z": z,
            "duration": duration,
            "drivetrain": yaw_control_mode,
            "yaw_is_rate": yaw_is_rate,
            "yaw": yaw,
        }

        req: Dict = {
            "method": f"{self.parent_topic}/MoveByVelocityBodyFrameZ",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def move_to_position_async(
        self,
        north: float,
        east: float,
        down: float,
        velocity: float,
        timeout_sec: float = 3e38,
        yaw_control_mode: YawControlMode = YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        lookahead: float = -1.0,
        adaptive_lookahead: float = 1.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move to position. Control returns back to the caller immediately.

        Args:
            north (float): the desired position north-coordinate (m)
            east (float): the desired position east-coordinate (m)
            down (float): the desired position down-coordinate (m)
            velocity (float): the desired velocity (m/s)
            timeout_sec (float): timeout for the command
            yaw_control_mode (YawControlMode): yaw control mode
            yaw_is_rate (bool): whether the yaw is absolute or a rate
            yaw (float): the desired yaw, in radians, or yaw rate, in radians/second
            lookahead (float): the amount of lookahead for the command
            adaptive_lookahead (float): the amount of adaptive lookahead for the command
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "x": north,
            "y": east,
            "z": down,
            "velocity": velocity,
            "timeout_sec": timeout_sec,
            "drivetrain": yaw_control_mode,
            "yaw_is_rate": yaw_is_rate,
            "yaw": yaw,
            "lookahead": lookahead,
            "adaptive_lookahead": adaptive_lookahead,
        }

        # print("params: {}".format(params))
        req: Dict = {
            "method": f"{self.parent_topic}/MoveToPosition",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def move_to_geo_position_async(
        self,
        latitude: float,
        longitude: float,
        altitude: float,
        velocity: float,
        timeout_sec: float = 3e38,
        yaw_control_mode: YawControlMode = YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        lookahead: float = -1.0,
        adaptive_lookahead: float = 1.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move to position given in lat-lon-alt coordinates. Control returns back to the caller immediately.

        Args:
            latitude {float}: the desired position latitude
            longitude {float}: the desired position longitude
            altitude {float}: the desired altitude (m)
            velocity {float}: the desired velocity (m/s)
            timeout_sec {float}: timeout for the command
            yaw_control_mode {YawControlMode}: yaw control mode
            yaw_is_rate {bool}: whether the yaw is absolute or a rate
            yaw {float}: the desired yaw, in radians, or yaw rate, in radians/second
            lookahead {float}: the amount of lookahead for the command
            adaptive_lookahead {float}: the amount of adaptive lookahead for the command
            callback {callable}: callback to invoke on command completion or error

        Returns:
            {asyncio.Task}: An awaitable task wrapping the async coroutine
        """
        geo_point = [latitude, longitude, altitude]
        coords_ned = geo_to_ned_coordinates(self.home_geo_point, geo_point)

        return await self.move_to_position_async(
            coords_ned[0],
            coords_ned[1],
            coords_ned[2],
            velocity,
            timeout_sec,
            yaw_control_mode,
            yaw_is_rate,
            yaw,
            lookahead,
            adaptive_lookahead,
            callback,
        )

    async def move_on_path_async(
        self,
        path,
        velocity: float,
        timeout_sec: float = 3e38,
        yaw_control_mode=YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        lookahead=-1,
        adaptive_lookahead=1,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move on a path. Control returns back to the caller immediately.

        This API uses a carrot following algorithm with lookahead values to control the velocity
        toward the waypoints ahead. The default values (lookahead=-1, adaptive_lookahead=1)
        mean that the algorithm will automatically determine the appropriate lookahead.

        Args:
            path (List[List[float]]): a list of path points, in NED coordinates
            velocity (float): the desired velocity, in m/s
            timeout_sec (sec): operation timeout
            yaw_control_mode (YawControlMode): the yaw control mode for the command
            yaW_is_rate (bool): whether the yaw is absolute or a rate
            yaw (float): the desired yaw, in radians, or yaw rate, in radians/second
            lookahead (float): the amount of lookahead for the command
            adaptive_lookahead (float): the amount of adaptive lookahead for the command
            callback (callable): callback to invoke on command completion or error

        Returns:
           asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {
            "path": path,
            "velocity": velocity,
            "timeout_sec": timeout_sec,
            "drivetrain": yaw_control_mode,
            "yaw_is_rate": yaw_is_rate,
            "yaw": yaw,
            "lookahead": lookahead,
            "adaptive_lookahead": adaptive_lookahead,
        }

        # print("params: {}".format(params))
        req: Dict = {
            "method": f"{self.parent_topic}/MoveOnPath",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def move_on_geo_path_async(
        self,
        path,
        velocity: float,
        timeout_sec: float = 3e38,
        yaw_control_mode=YawControlMode.MaxDegreeOfFreedom,
        yaw_is_rate: bool = True,
        yaw: float = 0.0,
        lookahead=-1,
        adaptive_lookahead=1,
        callback: callable = None,
    ) -> asyncio.Task:
        """Move on a path given in geo coordinates. Control returns back to the caller immediately.

        Args:
            path {List[List[float]]}: a list of path points, in lat-lon-alt coordinates
            velocity {float}: the desired velocity, in m/s
            timeout_sec {sec}: operation timeout
            yaw_control_mode {YawControlMode}: the yaw control mode for the command
            yaW_is_rate {bool}: whether the yaw is absolute or a rate
            yaw {float}: the desired yaw, in radians, or yaw rate, in radians/second
            lookahead {float}: the amount of lookahead for the command
            adaptive_lookahead {float}: the amount of adaptive lookahead for the command
            callback {callable}: callback to invoke on command completion or error

        Returns:
           {asyncio.Task}: An awaitable task wrapping the async coroutine
        """

        path_ned = []

        for point in path:
            coords_ned = geo_to_ned_coordinates(self.home_geo_point, point)
            path_ned.append(coords_ned)

        return await self.move_on_path_async(
            path_ned,
            velocity,
            timeout_sec,
            yaw_control_mode,
            yaw_is_rate,
            yaw,
            lookahead,
            adaptive_lookahead,
            callback,
        )

    async def rotate_to_yaw_async(
        self,
        yaw: float,
        timeout_sec: float = 3e38,
        margin: float = math.radians(5.0),
        yaw_rate: float = 0.0,
        callback: callable = None,
    ) -> asyncio.Task:
        """Rotate to a yaw. Control returns back to the caller immediately.

        Args:
            yaw (float): the desired yaw (radians)
            timeout_sec (float): the operation timeout (seconds)
            margin (float): the acceptable margin of error (radians)
            yaw_rate (float): Desired yaw rate to heading, <= 0 means as quickly as possible (radians/s)
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {"yaw": yaw, "timeout_sec": timeout_sec, "margin": margin, "yaw_rate": yaw_rate}

        # print("params: {}".format(params))
        req: Dict = {
            "method": f"{self.parent_topic}/RotateToYaw",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    async def rotate_by_yaw_rate_async(
        self, yaw_rate: float, duration: float, callback: callable = None
    ) -> asyncio.Task:
        """Rotate by yaw rate. Control returns back to the caller immediately.

        Args:
            yaw_rate (float): the desired yaw rate, in radians per second
            duration (float): the duration for which to perform the command, in seconds
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """

        params: Dict = {"yaw_rate": yaw_rate, "duration": duration}

        # print("params: {}".format(params))
        req: Dict = {
            "method": f"{self.parent_topic}/RotateByYawRate",
            "params": params,
            "version": 1.0,
        }

        async_task_cr = await self.client.request_async(req, callback)
        return async_task_cr

    def set_pose(self, pose: Pose, reset_kinematics=True) -> bool:
        """Sets Pose for NonPhysics Drone

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

    def set_geo_pose(
        self,
        latitude: float,
        longitude: float,
        altitude: float,
        rotation: Quaternion,
        reset_kinematics=True,
    ) -> bool:
        """Sets the pose of a Drone using latitude, longitude, and altitude

        Args:
            latitude {float}: the desired latitude
            longitude {float}: the desired longitude
            altitude {float}: the desired altitude, in meters
            reset_kinematics {bool}: if true, the object's velocity etc will be reset

        Returns:
            {bool} true if pose successfully set
        """

        geo_point = [latitude, longitude, altitude]
        coords_ned = geo_to_ned_coordinates(self.home_geo_point, geo_point)
        translation_ned = Vector3(
            {"x": coords_ned[0], "y": coords_ned[1], "z": coords_ned[2]}
        )
        pose_ned = Pose(
            {
                "translation": translation_ned,
                "rotation": rotation,
                "frame_id": "DEFAULT_ID",
            }
        )
        return self.set_pose(pose_ned, reset_kinematics)

    def get_ground_truth_pose(self) -> Pose:
        """Gets Pose for Drone

        Returns:
            Pose: the drone pose
        """

        get_pose_req = {
            "method": f"{self.parent_topic}/GetGroundTruthPose",
            "params": {},
            "version": 1.0,
        }
        pose = self.client.request(get_pose_req)
        return pose

    def get_ground_truth_geo_location(self) -> Dict:
        """Gets the geo location of the drone

        Returns:
            Dict: the drone's geo location, as a dict with keys "latitude", "longitude", and "altitude"
        """
        get_geo_location_req = {
            "method": f"{self.parent_topic}/GetGroundTruthGeoLocation",
            "params": {},
            "version": 1.0,
        }
        pose = self.client.request(get_geo_location_req)
        return pose

    def get_estimated_geo_location(self) -> Dict:
        """Gets the estimated geo location of the drone

        Returns:
            Dict: the drone's geo location from the drone's state estimator, as a dict with keys "latitude", "longitude", and "altitude"
        """
        get_geo_location_req = {
            "method": f"{self.parent_topic}/GetEstimatedGeoLocation",
            "params": {},
            "version": 1.0,
        }
        pose = self.client.request(get_geo_location_req)
        return pose

    #  TODO: Add sensor msg types
    def get_imu_data(self, sensor_name: str) -> Dict:
        """Get latest IMU sensor data

        Args:
            sensor_name (str): the configured name of the sensor

        Returns:
            Dict: ImuData as a dict
        """
        imu_data_req = {
            "method": f"{self.sensors[sensor_name]['imu_kinematics']}",
            "params": {},
            "version": 1.0,
        }
        imu_data = self.client.request(imu_data_req)
        return imu_data

    def get_gps_data(self, sensor_name: str) -> Dict:
        """Get latest GPS sensor data

        Args:
            sensor_name (str): the configured name of the sensor

        Returns:
            Dict: GPSData as a dict
        """
        gps_data_req = {
            "method": f"{self.sensors[sensor_name]['gps']}",
            "params": {},
            "version": 1.0,
        }
        gps_data = self.client.request(gps_data_req)
        return gps_data

    def get_airspeed_data(self, sensor_name: str) -> Dict:
        """Get latest Airspeed sensor data

        Args:
            sensor_name (str): the configured name of the sensor

        Returns:
            (Dict) the Airspeed data as a dict
        """
        airspeed_data_req = {
            "method": f"{self.sensors[sensor_name]['airspeed']}",
            "params": {},
            "version": 1.0,
        }
        airspeed_data = self.client.request(airspeed_data_req)
        return airspeed_data

    def get_battery_state(self, sensor_name: str) -> Dict:
        """Get latest Battery sensor data

        Returns:
            Dict: BatteryState
        """
        req_battery_path = f"{self.sensors_topic}/Battery"
        battery_state_req = {
            "method": f"{req_battery_path}/GetBatteryStatus",
            "params": {},
            "version": 1.0,
        }
        battery_state = self.client.request(battery_state_req)
        return battery_state

    def set_battery_remaining(self, desired_battery_remaining: float) -> bool:
        """Set Desired battery remaining battery charge level

        Args:
            desired_battery_remaining (float): the remaining battery, as a percent from 0.0-100.0

        Returns:
            bool: whether the operation was successful
        """
        req_battery_path = f"{self.sensors_topic}/Battery"
        battery_set_level_req = {
            "method": f"{req_battery_path}/SetBatteryRemaining",
            "params": {"desired_battery_remaining": desired_battery_remaining},
            "version": 1.0,
        }
        success = self.client.request(battery_set_level_req)
        return success

    def get_battery_drain_rate(self, sensor_name: str) -> Dict:
        """Get the current battery drain rate

        Returns:
            float: the battery drain rate, in the same units as the configuration value
        """
        req_battery_path = f"{self.sensors_topic}/Battery"
        battery_get_drain_req = {
            "method": f"{req_battery_path}/GetBatteryDrainRate",
            "params": {},
            "version": 1.0,
        }
        drain_rate = self.client.request(battery_get_drain_req)
        return drain_rate

    def set_battery_drain_rate(self, desired_drain_rate: float) -> bool:
        """Set Desired battery drain rate

        Args:
            desired_drain_rate (float): the desired battery drain rate, in the same units as the configuration value

        Returns:
            bool: whether the value was set
        """
        req_battery_path = f"{self.sensors_topic}/Battery"
        battery_set_drain_req = {
            "method": f"{req_battery_path}/SetBatteryDrainRate",
            "params": {"desired_drain_rate": desired_drain_rate},
            "version": 1.0,
        }
        success = self.client.request(battery_set_drain_req)
        return success

    def set_battery_health_status(self, is_desired_state_healthy: bool) -> bool:
        """Set battery health status (true = healthy, false = unhealthy)

        Args:
            is_desired_state_healthy (bool): true for healthy, false for unhealthy

        Returns:
            bool: whether the value was set
        """
        req_battery_path = f"{self.sensors_topic}/Battery"
        battery_set_health_req = {
            "method": f"{req_battery_path}/SetBatteryHealthStatus",
            "params": {"battery_health_indicator": is_desired_state_healthy},
            "version": 1.0,
        }
        success = self.client.request(battery_set_health_req)
        return success

    def get_barometer_data(self, sensor_name: str) -> Dict:
        """Get latest Barometer sensor data

        Args:
            sensor_name (str): the configured sensor name

        Returns:
            Dict: Dict of BarometerData
        """
        barometer_data_req = {
            "method": f"{self.sensors[sensor_name]['barometer']}",
            "params": {},
            "version": 1.0,
        }
        barometer_data = self.client.request(barometer_data_req)
        return barometer_data

    def get_magnetometer_data(self, sensor_name: str) -> Dict:
        """Get latest Magnetometer sensor data

        Args:
            sensor_name (str): the configured sensor name

        Returns:
            Dict: The MagnetometerData
        """
        magnetometer_data_req = {
            "method": f"{self.sensors[sensor_name]['magnetometer']}",
            "params": {},
            "version": 1.0,
        }
        magnetometer_data = self.client.request(magnetometer_data_req)
        return magnetometer_data

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

    def get_estimated_kinematics(self) -> Dict:
        """Get estimated kinematics from the flight controller's state estimator

        Returns:
            Dict: the Kinematics
        """
        est_kinematics_req = {
            "method": f"{self.parent_topic}/GetEstimatedKinematics",
            "params": {},
            "version": 1.0,
        }
        est_kinematics_data = self.client.request(est_kinematics_req)
        return est_kinematics_data

    def set_ground_truth_kinematics(self, kinematics: Dict) -> bool:
        """Set ground truth kinematics

        Args:
            kinematics (Dict): the kinematics as a dict

        Returns:
            bool: whether the kinematics were successfully set
        """
        set_gt_kinematics_req = {
            "method": f"{self.parent_topic}/SetGroundTruthKinematics",
            "params": {"kinematics": kinematics},
            "version": 1.0,
        }
        set_gt_kinematics_data = self.client.request(set_gt_kinematics_req)
        return set_gt_kinematics_data

    def get_images(self, camera_id: str, image_type_ids: List[int]) -> Dict:
        """Get a set of images from a camera sensor (server will wait until the next
           set of captured images is available with a sim timestamp >= sim time when
           the request was received)

        Args:
            camera_id (str): the camera id to retrieve images from
            image_type_ids (List[int]): set of ImageTypes to retrieve

        Returns:
            Dict: dict of image data for the requested images with keys by type IDs
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

    def camera_look_at_object(
        self, camera_id: str, object_name: str, wait_for_pose_update: bool = True
    ) -> bool:
        """Focuses camera onto an object of interest. A warning is
            logged if the object is not in the scene and the camera pose
            will not change.
            To reset the camera pose, call reset_camera_pose().

        Args:
            camera_id (str): the id of the camera to focus
            object_name (str): the name of the object to focus on
            wait_for_pose_update (bool): if true, the request will not return until
                the camera renderer has finished moving to the new pose

        Returns:
            bool: True if successful
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        camera_look_at_obj_req = {
            "method": f"{req_camera_path}/LookAtObject",
            "params": {
                "object_name": object_name,
                "wait_for_pose_update": wait_for_pose_update,
            },
            "version": 1.0,
        }
        result = self.client.request(camera_look_at_obj_req)
        return result

    def camera_draw_frustum(
        self, camera_id: str, to_enable: bool, image_type: ImageType = ImageType.SCENE
    ) -> bool:
        """Draws view frustum in the scene for the given camera.

        Args:
            camera_id (str): the id of the camera to display the frustum for
            image_type (ImageType): the image-type for a specific capture setting
            to_enable (bool): True to enable, False to disable drawing

        Returns:
            {bool}: True if successful
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        camera_draw_frustum_req = {
            "method": f"{req_camera_path}/DrawFrustum",
            "params": {"image_type": image_type, "to_enable": to_enable},
            "version": 1.0,
        }
        result = self.client.request(camera_draw_frustum_req)
        return result

    async def request_control_async(self, callback: callable = None) -> asyncio.Task:
        """Request the drone exit automatic mode and enable manual mode

        Args:
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        req: Dict = {
            "method": f"{self.parent_topic}/RequestControl",
            "params": {},
            "version": 1.0,
        }
        taskcr = await self.client.request_async(req, callback)
        return taskcr

    def set_camera_pose(
        self, camera_id: str, pose: Pose, wait_for_pose_update: bool = True
    ) -> bool:
        """Sets the relative pose of a drone camera

        Args:
            camera_id (str): the name of the camera
            pose (Pose): the desired camera relative pose
            wait_for_pose_update (bool): if true, the request will not return until
                the camera renderer has finished moving to the new pose

        Returns:
            bool: true if pose successfully set
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        set_camera_pose_req = {
            "method": f"{req_camera_path}/SetPose",
            "params": {"pose": pose, "wait_for_pose_update": wait_for_pose_update},
            "version": 1.0,
        }
        pose_set = self.client.request(set_camera_pose_req)
        return pose_set

    def set_focal_length(
        self, camera_id: str, image_type_id: int, focal_length: float
    ) -> bool:
        """Sets the focal length of a drone camera, affecting the image 'zoom'

        Args:
            camera_id (str): the name of the camera
            image_type_id (int): the ImageType for which to set the focal length
            focal_length (float): the desired focal length, in mm

        Returns:
            bool: true if focal length successfully set
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        set_focal_length_req = {
            "method": f"{req_camera_path}/SetFocalLength",
            "params": {"image_type_id": image_type_id, "focal_length": focal_length},
            "version": 1.0,
        }
        focal_length_set = self.client.request(set_focal_length_req)
        return focal_length_set

    def set_depth_of_field_transition_threshold(
        self, camera_id: str, image_type_id: int, transition_threshold: float
    ) -> bool:
        """Sets the transition area around the focal region, where the image is still not entirely blurry

        Args:
            camera_id (str): the name of the camera
            image_type_id (int): the ImageType for which to set the focal length
            transition_threshold (float): the threshold around the focal region where images still clear in meters.

        Returns:
            bool: true if focal length successfully set
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        set_focal_length_req = {
            "method": f"{req_camera_path}/SetDepthOfFieldTransitionRegion",
            "params": {"image_type_id": image_type_id, "transition_threshold": transition_threshold},
            "version": 1.0,
        }
        focal_length_set = self.client.request(set_focal_length_req)
        return focal_length_set

    def set_depth_of_field_focal_region(
        self, camera_id: str, image_type_id: int, max_focal_distance: float
    ) -> bool:
        """Sets the focal region, beyond this distance, images are blurry.

        Args:
            camera_id (str): the name of the camera
            image_type_id (int): the ImageType for which to set the focal length
            focal_length (float): the desired focal region area, in meters

        Returns:
            bool: true if focal length successfully set
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        set_focal_length_req = {
            "method": f"{req_camera_path}/SetDepthOfFieldFocalRegion",
            "params": {"image_type_id": image_type_id, "focal_length": max_focal_distance},
            "version": 1.0,
        }
        focal_length_set = self.client.request(set_focal_length_req)
        return focal_length_set

    def set_chromatic_aberration_intensity(
        self, camera_id: str, image_type_id: int, intensity: float
    ) -> bool:
        """Sets the chromatic aberration intensity to simulate camera faults

        Args:
            camera_id (str): the name of the camera
            image_type_id (int): the ImageType for which to set the focal length
            intensity (float): desired intensity range 0 to 1

        Returns:
            bool: true if focal length successfully set
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        set_focal_length_req = {
            "method": f"{req_camera_path}/SetChromaticAberrationIntensity",
            "params": {"image_type_id": image_type_id, "intensity": intensity},
            "version": 1.0,
        }
        focal_length_set = self.client.request(set_focal_length_req)
        return focal_length_set

    def set_field_of_view(
        self, camera_id: str, image_type_id: int, field_of_view: float
    ) -> bool:
        """Sets the field of view of a drone camera

        Args:
            camera_id (str): the name of the camera
            image_type_id (int): the ImageType for which to set the fov
            field_of_view (float): the desired camera fov, in degrees

        Returns:
            bool: true if fov successfully set
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        set_field_of_view_req = {
            "method": f"{req_camera_path}/SetFieldOfView",
            "params": {"image_type_id": image_type_id, "field_of_view": field_of_view},
            "version": 1.0,
        }
        field_of_view_set = self.client.request(set_field_of_view_req)
        return field_of_view_set

    async def set_mission_mode_async(self, callback: callable = None) -> asyncio.Task:
        """Set drone to execute previously loaded mission profile

        Args:
            callback (callable): callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        req: Dict = {
            "method": f"{self.parent_topic}/SetMissionMode",
            "params": {},
            "version": 1.0,
        }
        taskcr = await self.client.request_async(req, callback)
        return taskcr

    async def set_vtol_mode_async(
        self, vtol_mode: VTOLMode, callback: callable = None
    ) -> asyncio.Task:
        """Set drone flight mode on VTOL-quad-tailsitter vehicles

        Args:
            vtol_mode (VTOLMode): VTOLMode class value specifying the flight mode
            callback (callback): Callback to invoke on command completion or error

        Returns:
            asyncio.Task: An awaitable task wrapping the async coroutine
        """
        req: Dict = {
            "method": f"{self.parent_topic}/SetVTOLMode",
            "params": {"vtol_mode": vtol_mode},
            "version": 1.0,
        }
        taskcr = await self.client.request_async(req, callback)
        return taskcr

    def set_control_signals(self, control_signal_map: Dict) -> bool:
        """set_control_signals for Manual Controller type

        Args:
            control_signal_map (Dict): Dictionary with keys = actuator ID strings, and
                                 values = control signal floats

        Returns:
            bool: True if all control outputs are set successfully
        """
        set_control_signal_req: Dict = {
            "method": f"{self.parent_topic}/SetControlSignals",
            "params": {"control_signal_map": control_signal_map},
            "version": 1.0,
        }
        success = self.client.request(set_control_signal_req)
        return success


    def update_actuator_fault_state(self, actuator_id, fault_configured: bool) -> bool:
        """update actuator fault state. Helpful for fault testing

        Args:
            actuator_id: actuator id of the targeted actuator
            fault_configured: desired state of the fault simulation on actuator

        Returns:
            bool: True if actuated fault state updated successfully
        """
        toggle_actuator_fault_req: Dict = {
            "method": f"{self.parent_topic}/actuators/{actuator_id}/ToggleFault",
            "params": {"enable": fault_configured},
            "version": 1.0,
        }
        success = self.client.request(toggle_actuator_fault_req)
        return success

    def reset_camera_pose(
        self, camera_id: str, wait_for_pose_update: bool = True
    ) -> bool:
        """Reset camera pose on drone to config setting

        Args:
            camera_id (str): the id of the camera to reset
            wait_for_pose_update (bool): if true, the request will not return until
                the camera renderer has finished moving to the new pose

        Returns:
            bool: True if camera pose is reset successfully
        """
        req_camera_path = f"{self.sensors_topic}/{camera_id}"
        reset_camera_pose_req: Dict = {
            "method": f"{req_camera_path}/ResetCameraPose",
            "params": {"wait_for_pose_update": wait_for_pose_update},
            "version": 1.0,
        }
        success = self.client.request(reset_camera_pose_req)
        return success

    def set_external_force(self, ext_force: List[float]) -> bool:
        """
        Set arbitrary external force on drone, which can be used
        to model forces arising from the environment.

        Args:
            ext_force (List[float]): the force [x, y, z] in World frame (NED), in Newtons

        Returns:
            bool: True if external force is set successfully
        """
        set_external_force_req: Dict = {
            "method": f"{self.parent_topic}/SetExternalForce",
            "params": {"ext_force": ext_force},
            "version": 1.0,
        }
        success = self.client.request(set_external_force_req)
        return success
