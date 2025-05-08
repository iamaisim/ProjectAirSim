"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates flying a quadrotor drone in a GIS scene using Blackshark as an out-of-the-box example.
This needs to be run with a Blackshark environment uproject that has the Airsim plugin 
rather than the GeoSpecific env binary.
"""

import asyncio

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
        world = World(client, "scene_hello_blackshark.jsonc", delay_after_load_sec=0)
        home_pt = world.home_geo_point
        world.pause()

        world.spawn_object_at_geo(
            object_name="TakeoffPad",
            asset_path="BasicLandingPad",
            latitude=home_pt["latitude"],
            longitude=home_pt["longitude"],
            altitude=home_pt["altitude"]-25.0,
            rotation=[1, 0, 0, 0],
            object_scale=[2, 2, 2],
            enable_physics=False,
        )

        world.resume()
        await asyncio.sleep(3)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Subscribe to chase camera sensor as a client-side pop-up window
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to the downward-facing camera sensor's RGB and Depth images
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=2)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        image_display.start()

        await asyncio.sleep(3)

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()
        await asyncio.sleep(30)

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
            v_north=0.0, v_east=0.0, v_down=-3.0, duration=12.0
        )
        projectairsim_log().info("Move-Up invoked")

        await move_up_task
        projectairsim_log().info("Move-Up completed")

        # ------------------------------------------------------------------------------

        # Command the Drone to move down in NED coordinate system at 1 m/s for 4 seconds
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=3.0, duration=13.0
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

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
