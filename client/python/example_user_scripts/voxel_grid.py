"""
Copyright (C) Microsoft Corporation. All rights reserved.

This script generates a segmented voxel grid.
"""

from projectairsim import ProjectAirSimClient, World
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import projectairsim_log
from projectairsim.planners import AStarPlanner

client = ProjectAirSimClient()

client.connect()

# Create a World object to interact with the sim world and load a scene
world = World(client, "scene_basic_drone.jsonc")

center = (0, 0, 0)  # In UE Coordinates - Not NED
center_trans = Vector3({"x": center[0], "y": center[1], "z": center[2]})
center_rot = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
center_pos = Pose(
    {
        "translation": center_trans,
        "rotation": center_rot,
        "frame_id": "DEFAULT_ID",
    }
)

l = w = h = 200  # length, width, height of the map in m
resolution = 1  # edge len of each cube cell in m
projectairsim_log().info(f"Generating Map")

#world.set_segmentation_id_by_name("Landscape[\w]*", 0, True, True)

occupancy_grid = world.create_voxel_grid(center_pos, l, w, h, resolution, use_segmentation=True, write_file= True)

client.disconnect()