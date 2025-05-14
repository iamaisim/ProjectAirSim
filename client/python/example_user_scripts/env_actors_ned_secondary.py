"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates the use of environment actors with trajectories imported and set via
client API. A trajectory can be used by multiple actors with some global offset.
"""

import asyncio
import time

from projectairsim import ProjectAirSimClient, World, EnvActor, Drone
from projectairsim.utils import projectairsim_log
from typing import List
from math import cos, sin, radians

async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_env_actor.jsonc", delay_after_load_sec=2, actual_load = False)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        #env_actor1 = EnvActor(client, world, "ActorWithConfigTraj")

        # client.subscribe(
        #     "/Sim/SceneBasicDrone/env_actors/ActorWithApiTraj/actual_kinematics",
        #     lambda _, kinematics: print(kinematics),
        # )
        
        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------

        projectairsim_log().info("takeoff_async: starting")
        takeoff_task = (
            await drone.takeoff_async()
        )  # schedule an async task to start the command

        # Example 1: Wait on the result of async operation using 'await' keyword
        await takeoff_task
        projectairsim_log().info("takeoff_async: completed")

        # ------------------------------------------------------------------------------

        # Command the drone to move up in NED coordinate system at 1 m/s for 4 seconds
        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        projectairsim_log().info("Move-Up invoked")

        await move_up_task
        projectairsim_log().info("Move-Up completed")

        # ------------------------------------------------------------------------------

        # Command the Drone to move down in NED coordinate system at 1 m/s for 4 seconds
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
        )  # schedule an async task to start the command
        projectairsim_log().info("Move-Down invoked")

        # Example 2: Wait for move_down_task to complete before continuing
        while not move_down_task.done():
            await asyncio.sleep(0.005)
        projectairsim_log().info("Move-Down completed")

        # ------------------------------------------------------------------------------

        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("land_async: completed")

        # ------------------------------------------------------------------------------

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

        # ------------------------------------------------------------------------------


    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
