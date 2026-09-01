"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Python client for ProjectAirSim Drone robots/actors.
"""

import asyncio
import math

from projectairsim import ProjectAirSimClient, World
from projectairsim.robot import Robot
from projectairsim.utils import projectairsim_log, geo_to_ned_coordinates
from typing import List, Dict
from projectairsim.types import LandedState


class YawControlMode:
    MaxDegreeOfFreedom = 0
    ForwardOnly = 1


class Drone(Robot):
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
        super().__init__(client, world, name)
        self.log_topics()
        self.vel_cmd = {"axes_0": 0.0, "axes_1": 0.0, "axes_2": 0.0, "axes_3": 0.0}
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

    def set_robot_info_topics(self):
        """Sets up robot info topics for the Drone. Called automatically"""
        super().set_robot_info_topics()
        self.robot_info["rotor_info"] = f"{self.parent_topic}/rotor_info"

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
        """Arms drone

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
        """Disarms drone

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
        """Checks if the drone can be armed

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

    def get_landed_state(self) -> LandedState:
        """Checks if the drone is landed

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


    #  TODO: Add sensor msg types


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

    # get_jsbsim_property
    def get_jsbsim_property(self, property_name: str) -> float:
        """Get JSBSim property

        Args:
            property_name (str): Property name

        Returns:
            float: Property value
        """
        get_jsbsim_property_req: Dict = {
            "method": f"{self.parent_topic}/GetJSBSimProperty",
            "params": {"_property_name": property_name},
            "version": 1.0,
        }
        property_value = self.client.request(get_jsbsim_property_req)
        return property_value

    # set_jsbsim_property
    def set_jsbsim_property(self, property_name: str, property_value: float) -> bool:
        """Set JSBSim property

        Args:
            property_name (str): Property name
            property_value (float): Property value

        Returns:
            bool: True if property is set
        """
        set_jsbsim_property_req: Dict = {
            "method": f"{self.parent_topic}/SetJSBSimProperty",
            "params": {
                "_property_name": property_name,
                "_value": property_value,
            },
            "version": 1.0,
        }
        property_set = self.client.request(set_jsbsim_property_req)
        return property_set
