"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates using an A* motion planner to set a flight path.
"""

import asyncio
from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import projectairsim_log
from projectairsim.planners import AStarPlanner


async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_basic_drone.jsonc")

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Takeoff
        projectairsim_log().info("**** Takeoff started ****")
        meters_up = 5
        vel = 2.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=cur_pos["z"] - meters_up,
            velocity=vel,
        )
        await task

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

        occupancy_grid = world.create_voxel_grid(center_pos, l, w, h, resolution, use_segmentation=False)

        planner = AStarPlanner(occupancy_grid, center, (l, w, h), resolution)

        projectairsim_log().info(f"Map Generated. Planning Path")

        # Change in x,y,z components need to be in steps of 1m
        # Start and Goals Locations in NED
        start_pos = (-1, 8, -6)
        goal_pos = (30, -48, -10)

        # Method takes in coordinates in NEU
        if not planner.check_coordinate_validity(start_pos, is_NED=True):
            projectairsim_log().info(f"Start position {start_pos} is invalid")
            return

        if not planner.check_coordinate_validity(goal_pos, is_NED=True):
            projectairsim_log().info(f"Goal position {goal_pos} is invalid")
            return

        path = planner.generate_plan(start_pos, goal_pos)
        projectairsim_log().info(f"Plan Generated")

        vel = 4
        projectairsim_log().info(f"Moving to start pos {start_pos}")
        task = await drone.move_to_position_async(
            north=start_pos[0],
            east=start_pos[1],
            down=start_pos[2],
            velocity=vel,
        )
        await task

        projectairsim_log().info(f"Moving on planned path")
        task = await drone.move_on_path_async(path, vel)
        await task
        projectairsim_log().info(f"Goal Location {goal_pos} Reached")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
