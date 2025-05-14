"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Implements the MissionPlanner class that can manage and execute .plan files
"""
import json
import math
import pathlib
from typing import List
from enum import IntEnum

from projectairsim import World, Drone
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.types import Pose, Vector3, Quaternion
from projectairsim.planners import AStarPlanner


class MAVLinkCMDs(IntEnum):
    """MAVLink command type enum values:
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
    """

    MAV_CMD_NAV_TAKEOFF_LOCAL = 24
    MAV_CMD_NAV_LAND_LOCAL = 23
    MAV_CMD_NAV_PATHPLANNING = 81
    MAV_CMD_DO_CHANGE_SPEED = 178


class MissionPlanner:
    """This class encompasses the mission plan execution functionality for the Project AirSim client"""

    def __init__(self, plan_path: pathlib.Path, voxel_res: int = 1):
        """INitializes the MissionPlanner class

        Args:
            plan_path (pathlib.Path): path to the mission plan file
            voxel_res (int, optional): Edge length of generated voxels. Defaults to 1.
        """
        self.plan_path = plan_path
        self.res = voxel_res
        self.plan_data = self.read_mission()
        self.mission_data = self.plan_data.get("mission")
        self.coordinates = self.get_mission_coordinates()
        self.mission_center, self.mission_size = self.get_mission_boundaries()
        projectairsim_log().info(
            f"Mission center: {self.mission_center}, Mission size: {self.mission_size}"
        )
        self.mission_length, self.mission_width, self.mission_height = self.mission_size
        self.occupancy_map = None

    def read_mission(self):
        """Reads the mission plan file and returns the JSON data

        Returns:
            mission_data (dict): JSON data from the mission plan file
        """
        mission_data = None
        try:
            # Open the file for reading
            with open(self.plan_path, "r") as file:
                # Read the JSON data from the file
                mission_data = json.load(file)

        except FileNotFoundError:
            projectairsim_log().info(f"The file '{self.plan_path}' was not found.")
        except json.JSONDecodeError as e:
            projectairsim_log().info(f"Error decoding JSON data: {str(e)}")
        except Exception as e:
            projectairsim_log().info(f"An error occurred: {str(e)}")

        return mission_data

    def get_mission_coordinates(self):
        """Given a mission plan, returns a list of coordinates (x,y,z) of the mission waypoints
        Returns:
            coordinates (list): list of (x, y, z) coordinates
        """
        self.commands = self.mission_data.get("items")
        coordinates = []
        for command in self.commands:
            command_id = command.get("command")
            if (
                command_id == MAVLinkCMDs.MAV_CMD_NAV_LAND_LOCAL.value
                or command_id == MAVLinkCMDs.MAV_CMD_NAV_PATHPLANNING.value
                or command_id == MAVLinkCMDs.MAV_CMD_NAV_TAKEOFF_LOCAL.value
            ):  # only supporting takeoff, nav, landing
                params = command.get("params")
                coordinate = params[-3:]
                coordinates.append(coordinate)
            else:
                if command_id != MAVLinkCMDs.MAV_CMD_DO_CHANGE_SPEED.value:
                    projectairsim_log().info(
                        f"Command {command_id} not supported. It will be ignored during mission execution"
                    )

        return coordinates

    def get_mission_boundaries(self):
        """Given a list of coordinates (x,y,z), returns the center and size of the bounding box
        Args:
            coordinates (list): list of (x, y, z) coordinates
        Returns:
            mission_center (tuple): center of the bounding box
            mission_size (tuple): (l, w, h) of the bounding box
        """
        min_x = min(coord[0] for coord in self.coordinates)
        max_x = max(coord[0] for coord in self.coordinates)
        min_y = min(coord[1] for coord in self.coordinates)
        max_y = max(coord[1] for coord in self.coordinates)
        min_z = min(coord[2] for coord in self.coordinates)
        max_z = max(coord[2] for coord in self.coordinates)

        center_x = (min_x + max_x) // 2
        center_y = (min_y + max_y) // 2
        center_z = (min_z + max_z) // 2

        # Get edge len and add buffer
        buffer = 50  # m
        length = math.ceil(max_x - min_x) + buffer
        width = math.ceil(max_y - min_y) + buffer
        height = math.ceil(max_z - min_z) + buffer

        mission_size = (length, width, height)
        # For some reason, odd dimensions do not work...
        mission_size = [n + 1 if n % 2 != 0 else n for n in mission_size]

        return (center_x, center_y, center_z), mission_size

    def get_pose_transform(self, pose: List, rot: List):
        """Given a pose (x, y, z) and a rotation (roll, pitch, yaw), returns a Pose object

        args:
            pose (List): (x, y, z) coordinate
            rot (List): (roll, pitch, yaw) rotation

        returns:
            Pose: Pose object
        """
        quat = rpy_to_quaternion(rot[0], rot[1], rot[2])
        translation = Vector3({"x": pose[0], "y": pose[1], "z": pose[2]})
        rotation = Quaternion({"w": quat[0], "x": quat[1], "y": quat[2], "z": quat[3]})
        transform = {"translation": translation, "rotation": rotation}

        return Pose(transform)

    def generate_mission_occupancy_map(self, world: World):
        """Given a World object, generates a voxel grid for the mission

        Args:
            world (World): PAS world object
        Returns:
            occupancy_map (list): PAS voxel grid list in binvox RLE encoding
        """
        center_pose: Pose = self.get_pose_transform(self.mission_center, (0, 0, 0))
        self.occupancy_map = world.create_voxel_grid(
            center_pose,
            self.mission_length,
            self.mission_width,
            self.mission_height,
            self.res,
        )

        return self.occupancy_map

    def generate_planned_paths(self):
        """Given the coordinates in the .plan file, generates a path for each pair of consecutive coordinates

        Returns:
            self.mission_paths (dict): holds the paths as key-value pairs of (start_coord, goal_coord): path
        """
        path_planner = AStarPlanner(
            self.occupancy_map, self.mission_center, self.mission_size, self.res
        )

        self.mission_paths = {}
        for i in range(len(self.coordinates) - 1):
            start_cor = self.coordinates[i]
            goal_cor = self.coordinates[i + 1]

            projectairsim_log().info(f"Generating path from {start_cor} to {goal_cor}")
            path = path_planner.generate_plan(start_cor, goal_cor)
            if path == []:
                continue
            self.mission_paths[(tuple(start_cor), tuple(goal_cor))] = path

        return self.mission_paths

    async def execute_mission(self, world: World, drone: Drone):
        """Generates required paths and executes mission given a Drone object

        Args:
            drone (Drone): Drone object that will execute the plan in the sim
        """
        self.generate_mission_occupancy_map(world)
        projectairsim_log().info(f"Voxel grid generated.")

        projectairsim_log().info(f"Generating paths...")
        self.generate_planned_paths()

        cur_speed = self.mission_data.get("cruiseSpeed")
        projectairsim_log().info(f"Speed set to {cur_speed} m/s")
        cur_coord = None
        for command in self.commands:
            command_id = command.get("command")

            if command_id == MAVLinkCMDs.MAV_CMD_DO_CHANGE_SPEED.value:
                projectairsim_log().info(
                    f"Executing MAVLink command {MAVLinkCMDs.MAV_CMD_DO_CHANGE_SPEED.name} ({MAVLinkCMDs.MAV_CMD_DO_CHANGE_SPEED.value})"
                )
                cur_speed = command.get("params")[1]
                projectairsim_log().info(f"Speed changed to {cur_speed} m/s")

            elif command_id == MAVLinkCMDs.MAV_CMD_NAV_TAKEOFF_LOCAL.value:  # take-off
                coord = command.get("params")[-3:]
                yaw = command.get("params")[3]
                projectairsim_log().info(
                    f"Executing MAVLink command {MAVLinkCMDs.MAV_CMD_NAV_TAKEOFF_LOCAL.name} ({MAVLinkCMDs.MAV_CMD_NAV_TAKEOFF_LOCAL.value})"
                )
                projectairsim_log().info(f"Moving drone to take-off position {coord}")
                task = await drone.takeoff_async()
                await task
                task = await drone.move_to_position_async(
                    north=coord[0],
                    east=coord[1],
                    down=coord[2],
                    velocity=cur_speed,
                    yaw_is_rate=False,
                    yaw=yaw,
                )
                await task

                projectairsim_log().info(f"Taking off at {coord}")
                task = await drone.takeoff_async()
                await task
                cur_coord = coord

            elif (
                command_id == MAVLinkCMDs.MAV_CMD_NAV_PATHPLANNING.value
            ):  # planned nav
                projectairsim_log().info(
                    f"Executing MAVLink command {MAVLinkCMDs.MAV_CMD_NAV_PATHPLANNING.name} ({MAVLinkCMDs.MAV_CMD_NAV_PATHPLANNING.value})"
                )
                goal_coord = command.get("params")[-3:]
                yaw = command.get("params")[3]
                if (
                    tuple(cur_coord),
                    tuple(goal_coord),
                ) not in self.mission_paths.keys():
                    projectairsim_log().info(
                        f"No path found from {cur_coord} to {goal_coord}. Stopping mission."
                    )
                    return
                projectairsim_log().info(
                    f"Navigating drone from {cur_coord} to {goal_coord}"
                )
                path = self.mission_paths[(tuple(cur_coord), tuple(goal_coord))]
                task = await drone.move_on_path_async(
                    path, cur_speed, yaw_is_rate=False, yaw=yaw
                )
                await task
                cur_coord = goal_coord

            elif command_id == MAVLinkCMDs.MAV_CMD_NAV_LAND_LOCAL.value:  # land
                coord = command.get("params")[-3:]
                yaw = command.get("params")[3]
                projectairsim_log().info(
                    f"Executing MAVLink command {MAVLinkCMDs.MAV_CMD_NAV_LAND_LOCAL.name} ({MAVLinkCMDs.MAV_CMD_NAV_LAND_LOCAL.value})"
                )
                projectairsim_log().info(f"Moving drone to landing position {coord}")

                if (tuple(cur_coord), tuple(coord)) not in self.mission_paths:
                    projectairsim_log().info(
                        f"No path found from {cur_coord} to {coord}. Stopping mission."
                    )
                    return
                path = self.mission_paths[(tuple(cur_coord), tuple(coord))]
                task = await drone.move_on_path_async(
                    path, cur_speed, yaw_is_rate=False, yaw=yaw
                )
                await task
                projectairsim_log().info(f"Landing at {coord}")
                task = await drone.land_async()
                await task
                projectairsim_log().info(f"Landed at {coord}")

            projectairsim_log().info(f"Mission execution complete.")

        return
