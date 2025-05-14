"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Demo client script for a single FastPhysics drone in AirSimVNext.
"""

import asyncio
import math
from datetime import datetime

from projectairsim import ProjectAirSimClient, World, Drone
from projectairsim.utils import projectairsim_log

# Async main function to wrap async drone commands
async def main():
    # Create a AirSimVNext Client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a world object to interact with the AirSimVNext world and load a scene
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # Create a drone object to interact with a Drone in the loaded AirSimVNext world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------ TakeoffAsync ------------------------------------------

        # example-1: wait on the result of async operation using 'await' keyword
        projectairsim_log().info("TakeoffAsync: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("TakeoffAsync: completed")

        # ------------------------------------------------------------------------------

        # Command the drone to move up in NED coordinate system for 4 seconds
        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )  # schedule an async task to start the command
        projectairsim_log().info("Move Up invoked")

        await move_up_task
        projectairsim_log().info("Move Up completed")

        # ------------------------------------------------------------------------------

        # Command the drone to move east for 5 seconds
        move_east_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=1.0, v_down=0.0, duration=5.0, yaw=0.0, yaw_is_rate=False
        )  # schedule an async task to start the command
        projectairsim_log().info("Move East invoked")
        
        await move_east_task
        projectairsim_log().info("Move East completed")

        # ------------------------------------------------------------------------------

        # Command the drone to move north for 5 seconds
        move_north_task = await drone.move_by_velocity_async(
            v_north=1.0, v_east=0.0, v_down=0.0, duration=5.0, yaw=-math.pi/2, yaw_is_rate=False
        )  # schedule an async task to start the command
        projectairsim_log().info("Move North invoked")

        await move_north_task
        projectairsim_log().info("Move North completed")

        # ------------------------------------------------------------------------------

        # Command the drone to move west for 5 seconds
        move_west_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-1.0, v_down=0.0, duration=5.0, yaw=-math.pi, yaw_is_rate=False
        )  # schedule an async task to start the command
        projectairsim_log().info("Move West invoked")

        await move_west_task
        projectairsim_log().info("Move West completed")

        # ------------------------------------------------------------------------------

        # Command the drone to move south for 5 seconds
        move_south_task = await drone.move_by_velocity_async(
            v_north=-1.0, v_east=0.0, v_down=0.0, duration=5.0, yaw=-math.pi*3/2, yaw_is_rate=False
        )  # schedule an async task to start the command
        projectairsim_log().info("Move South invoked")
        await move_south_task
        projectairsim_log().info("Move South completed")

        # ------------------------------------------------------------------------------

        # Command the Drone to move down in NED coordinate system for 4 seconds
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
        )  # schedule an async task to start the command
        projectairsim_log().info("Move Down invoked")

        # Example #2 to wait for move_down_task to complete before continuing
        while not move_down_task.done():
            await asyncio.sleep(0.005)
        projectairsim_log().info("Move Down completed")
    

        # ------------ LandAsync ------------------------------------------

        projectairsim_log().info("LandAsync: starting")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("LandAsync: completed")

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
