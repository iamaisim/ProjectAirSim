"""
Copyright (C) Microsoft Corporation. All rights reserved.
"""

from ssl import VERIFY_CRL_CHECK_CHAIN
import numpy as np
import time
import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

# Async main function to wrap async drone commands
async def main():
    # Create a AirSimVNext Client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a world object to interact with the Project AirSim world and load a scene
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # Create a drone object to interact with a Drone in the loaded Project AirSim world
        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        drone.arm()
        world.toggle_trace()

        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        hover_task = await drone.hover_async()
        await hover_task

        move_task = asyncio.create_task(drone.move_by_velocity_async(1, 1, 0, 12))
        await move_task

        world.set_trace_line([1.0, 0.0, 0.0, 1.0], 5)
        time.sleep(2)
        world.set_trace_line([0.0, 1.0, 0.0, 0.8], 10)
        time.sleep(2)
        world.set_trace_line([0.0, 0.0, 1.0, 0.6], 20)
        time.sleep(2)
        world.set_trace_line([1.0, 1.0, 0.0, 0.4], 30)
        time.sleep(2)
        world.set_trace_line([0.0, 1.0, 1.0, 0.2], 40)
        time.sleep(2)
        world.set_trace_line([1.0, 0.0, 1.0, 0.1], 50)
        time.sleep(2)

        drone.disarm()
        drone.disable_api_control()

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        pending_tasks = [t for t in asyncio.all_tasks() if t is not asyncio.current_task()]
        for task in pending_tasks:
            task.cancel()
            try:
                await task
            except asyncio.CancelledError:
                pass
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
