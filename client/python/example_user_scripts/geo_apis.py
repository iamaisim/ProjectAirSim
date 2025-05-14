"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates various flight command APIs to move a drone.
"""

import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.types import Quaternion


# Async main function to wrap async drone commands
async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        # Demonstrate setting the drone's position in geo coordinates
        lat = 47.641450
        lon = -122.140150
        alt = 125.0
        rot = Quaternion({"w": 1.0, "x": 0, "y": 0, "z": 0})
        projectairsim_log().info("moving drone")
        drone.set_geo_pose(lat, lon, alt, rot)

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Demonstrate moving to a geo position
        projectairsim_log().info("move_to_geo_position_async: starting")
        move_task = await drone.move_to_geo_position_async(
            latitude=47.641460, longitude=-122.140140, altitude=130.0, velocity=1.0
        )
        await move_task
        projectairsim_log().info("move_to_geo_position_async: completed")

        # Demonstrate moving on a geo path
        projectairsim_log().info("move_on_geo_path_async: starting")
        path = [
            [47.641460, -122.140220, 135.0],
            [47.641450, -122.140100, 144.0],
            [47.641440, -122.140300, 126.0],
        ]
        move_path_task = await drone.move_on_geo_path_async(path, velocity=1.0)
        await move_path_task
        projectairsim_log().info("move_on_geo_path_async: completed")

        # Command the Drone to land and shut down
        land_task = await drone.land_async()
        await land_task
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
