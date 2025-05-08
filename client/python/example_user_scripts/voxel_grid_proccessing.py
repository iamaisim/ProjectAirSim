"""
Copyright (C) Microsoft Corporation. All rights reserved.

"""

import asyncio
from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import get_voxel_grid_idx
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import projectairsim_log


async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
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

        l = w = h = 100  # length, width, height of the map in m
        resolution = 1  # edge len of each cube cell in m
        projectairsim_log().info(f"Generating Map")

        occupancy_grid = world.create_voxel_grid(center_pos, l, w, h, resolution, use_segmentation = True, write_file = True) # Generate a voxel grid with segmentation

        # Obtain an element of the voxel grid
        # Ask user for the coordinates of the cell they want to access
        i = int(input("Enter the x coordinate of the cell you want to access: "))
        j = int(input("Enter the y coordinate of the cell you want to access: "))
        k = int(input("Enter the z coordinate of the cell you want to access: "))
        
        # The voxel grid is a 1D array, so we need to convert the 3D coordinates to a 1D index
        idx = get_voxel_grid_idx([i, j, k], center, (l, w, h), resolution)
        cell = occupancy_grid[idx]
        if cell == 0:
            projectairsim_log().info(f"Cell at ({i}, {j}, {k}) is free")
        else:
            projectairsim_log().info(f"Cell at ({i}, {j}, {k}) is {cell}")


    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
