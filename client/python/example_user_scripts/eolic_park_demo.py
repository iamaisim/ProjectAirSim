"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

A demo for the Eolic Park environment where the drone travels around the assets.
Note: (to take in consideration) the floor is around z=375.0
"""

import asyncio
import time

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.image_utils import ImageDisplay
from projectairsim.utils import projectairsim_log


async def demo_sensors(drone: Drone):
    # Set the drone to be ready to fly
    drone.enable_api_control()
    drone.arm()

    takeoff_task = await drone.takeoff_async()

    # Wait for the move_up_task to complete
    await takeoff_task

    # Command the drone to move up in NED coordinate system for 5 seconds
    move_up_task = await drone.move_by_velocity_async(
        v_north=0.0, v_east=0.0, v_down=-1, duration=5.0
    )
    # Wait for the move_up_task to complete
    await move_up_task

    # Command the drone to move up in NED coordinate system for 5 seconds
    move_to_pos1= await drone.move_to_position_async(
        north=28.10,
        east=71.70,
        down=367.20,
        velocity=7.0,
    )

    # Wait for the move_up_task to complete
    await move_to_pos1

    # Command the drone to move North in NED coordinate system for 10 seconds
    move_up= await drone.move_to_position_async(
        north=83.90,
        east=82.50,
        down=299.90,
        velocity=7.0,
        yaw= 0.105768,  # 90 degrees
    )

    # Wait for the move_up_task to complete
    await move_up
    
    move_around_turbine= await drone.move_to_position_async(
        north=131.90,
        east=119.70,
        down=300.60,
        velocity=7.0,
        yaw= 0.154674,  # 180 degrees in radians
    )

    # Wait for the move_up_task to complete
    await move_around_turbine

    hover = await drone.hover_async()
    time.sleep(2)
    await hover

    drone.disable_api_control()
    drone.disarm()


if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_drone_eolic_park.jsonc")

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Run the sensors demonstration routine
        asyncio.run(demo_sensors(drone))

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()
