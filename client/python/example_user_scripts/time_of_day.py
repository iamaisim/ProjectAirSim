"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates using the set_time_of_day API.
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
import projectairsim
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
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # ------------------------------------------------------------------------------

        # Set time of day in the sim
        # The following params are inpits to the set_time_of_day() method:
        # status: determines wether or not the time of day lighting effects are applied
        # datetime: the date and time
        # is_dst: apply day light saving or not
        # clock_speed: scale of how fast time of day advances vs sim time
        # update_interval: how much sim time should pass between each new sun position update
        # move_sun: move the visual position of the sun in the scene based on the rate set by 'clock_speed' and 'update_interval'
        projectairsim_log().info("Setting time to 5 pm (Dynamic Sun Position)")
        world.set_time_of_day(
            status=True,
            datetime="2021-09-20 17:00:00",
            is_dst=False,
            clock_speed=5.0,
            update_interval=0.5,
            move_sun=True,
        )

        # ------------------------------------------------------------------------------

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
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

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("takeoff_async: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("takeoff_async: completed")

        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up_task
        projectairsim_log().info("Move-Up completed")

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Changing Time to 9:00 am (Static Sun Position)")

        world.set_time_of_day(
            status=True,
            datetime="2021-09-20 09:00:00",
            is_dst=False,
            clock_speed=1.0,
            update_interval=1.0,
            move_sun=False,
        )

        # ------------------------------------------------------------------------------

        move_north_task = await drone.move_by_velocity_async(
            v_north=3.0, v_east=0.0, v_down=0.0, duration=4.0
        )
        projectairsim_log().info("Move-North invoked")
        await move_north_task

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Changing Time to 6:00 pm (Static Sun Position)")

        world.set_time_of_day(
            status=True,
            datetime="2021-09-20 18:00:00",
            is_dst=False,
            clock_speed=1.0,
            update_interval=1.0,
            move_sun=False,
        )

        # ------------------------------------------------------------------------------

        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=6.0
        )
        projectairsim_log().info("Move-Down invoked")
        await move_down_task
        projectairsim_log().info("Move-Down completed")

        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("land_async: completed")

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
