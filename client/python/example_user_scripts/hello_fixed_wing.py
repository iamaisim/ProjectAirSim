"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates flying a quadrotor drone with camera sensors.
"""

import asyncio
import math

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay


# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_basic_fixed_wing.jsonc", delay_after_load_sec=2)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "x8")
#
        ## ------------------------------------------------------------------------------
#
        ## Subscribe to chase camera sensor as a client-side pop-up window
        #chase_cam_window = "ChaseCam"
        #image_display.add_chase_cam(chase_cam_window)
        #client.subscribe(
        #    drone.sensors["DownCamera"]["scene_camera"],
        #    lambda _, chase: image_display.receive(chase, chase_cam_window),
        #)
#
        ## Subscribe to the downward-facing camera sensor's RGB and Depth images
        #rgb_name = "RGB-Image"
        #image_display.add_image(rgb_name, subwin_idx=0)
        #client.subscribe(
        #    drone.sensors["DownCamera"]["scene_camera"],
        #    lambda _, rgb: image_display.receive(rgb, rgb_name),
        #)
#
        #depth_name = "Depth-Image"
        #image_display.add_image(depth_name, subwin_idx=2)
        #client.subscribe(
        #    drone.sensors["DownCamera"]["depth_camera"],
        #    lambda _, depth: image_display.receive(depth, depth_name),
        #)
#
        #image_display.start()
#
        ## ------------------------------------------------------------------------------
#
        ## Set the drone to be ready to fly
        ## JSBSim robot currently does not support control the drone at runtime
        #drone.enable_api_control()
        ##set brakes to 1
        #drone.set_brakes(1)
        #drone.arm()
#
        ## ------------------------------------------------------------------------------
#
        ## set takeoff z to 120 meters
        #drone.set_take_off_z(-120)
#
        ## ------------------------------------------------------------------------------
#
        ## Sleep for two seconds to
        #await asyncio.sleep(2)
        #
        ## release brakes
        #drone.set_brakes(0)
#
        #projectairsim_log().info("takeoff_async: starting")
        #takeoff_task = (
        #    await drone.takeoff_async(timeout_sec=1200)
        #)  # schedule an async task to start the command
#
        #await takeoff_task
        #projectairsim_log().info("takeoff_async: completed")
#
        #projectairsim_log().info("Waiting to stabilize altitude... (10 seconds)")
        #await asyncio.sleep(10)
#
        ## ------------------------------------------------------------------------------
        #
        ## Command the drone to move to position 1000,1000,-200
        #move_up_task = await drone.move_to_position_async(
        #    north=1000, east=1000, down=-200, velocity=33.0, lookahead=100, timeout_sec=60
        #)
        #projectairsim_log().info("Move to position 1000,1000,-200 invoked")
#
        #await move_up_task
        #projectairsim_log().info("Move to position completed")
#
        ## ------------------------------------------------------------------------------
#
        ## Command vehicle to fly at a specific heading and speed
        #projectairsim_log().info("Heading 90 invoked")
        #heading_45_task = await drone.move_by_heading_async(
        #    heading=math.radians(90.0), speed=20.0, duration=10
        #)
        #await heading_45_task
        #projectairsim_log().info("Heading 90 complete.")
#
        ## ------------------------------------------------------------------------------
#
        ## Command the drone to move to position 0,0,-100
        #move_up_task = await drone.move_to_position_async(
        #    north=0, east=0, down=-100, velocity=33.0, lookahead=100, timeout_sec=60
        #)
        #projectairsim_log().info("Move to position 0,0,-100 invoked")
#
        #await move_up_task
        #projectairsim_log().info("Move to position completed")
        ## ------------------------------------------------------------------------------
#
        #projectairsim_log().info("land_async: starting")
        #land_task = await drone.land_async()
        #await land_task
        #projectairsim_log().info("land_async: completed")
        ## set brakes to 50%
        #drone.set_brakes(0.5)

        # ------------------------------------------------------------------------------

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function