"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates flying a quadtiltrotor airtaxi with parameters read in from a config file.
The airtaxi first rotates its heading to 45 degrees using a properly tuned MC_YAWRATE_MAX
value. The scene is then reloaded with an improperly tuned value to demonstrate unstable
yaw behavior.

"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, load_scene_config_as_dict
from projectairsim.image_utils import ImageDisplay

client = None
world = None
drone = None
image_display = None


async def execute_takeoff_and_rotate_maneuver():
    # Subscribe to the downward-facing camera sensor's RGB and Depth images
    rgb_name = "RGB-Image"
    image_display.add_image(rgb_name, subwin_idx=0)
    client.subscribe(
        drone.sensors["DownCamera"]["scene_camera"],
        lambda _, rgb: image_display.receive(rgb, rgb_name),
    )

    image_display.start()

    # ------------------------------------------------------------------------------

    # Set the drone to be ready to fly
    drone.enable_api_control()
    drone.arm()

    # ------------------------------------------------------------------------------

    projectairsim_log().info("takeoff_async: starting")
    takeoff_task = (
        await drone.takeoff_async()
    )  # schedule an async task to start the command

    # Wait on the result of async operation using 'await' keyword
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

    # Command the drone to rotate to a heading of 45 degrees
    rotate_to_yaw_task = await drone.rotate_to_yaw_async(yaw=0.785)
    await rotate_to_yaw_task


# Async main function to wrap async drone commands
async def main():
    global client, world, drone, image_display

    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment
        client.connect()

        scene_config_name = "scene_config_quadtiltrotor_sf_params.jsonc"

        # Create a World object to interact with the sim world and load a scene
        world = World(client, scene_config_name, delay_after_load_sec=2)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Execute maneuver with properly tuned gains
        projectairsim_log().info(
            "Rotating yaw to 45 degrees using MC_YAWRATE_MAX = 0.3."
        )
        await execute_takeoff_and_rotate_maneuver()

        # Shutdown drone to reload scene
        drone.disarm()
        drone.disable_api_control()
        image_display.stop()

        # ------------------------------------------------------------------------------

        # Reload scene to try maneuver again with new MC_YAWRATE_MAX gain
        projectairsim_log().info(
            "Reloading scene using MC_YAWRATE_MAX = 0.6 (doubled) to notice deteriorated performance."
        )

        # Read config as dict to modify it
        config_dict, _ = load_scene_config_as_dict(scene_config_name)

        # Modify MC_YAWRATE_MAX gain
        simple_flight_params = config_dict["actors"][0]["robot-config"]["controller"][
            "simple-flight-api-settings"
        ]["parameters"]
        simple_flight_params["MC_YAWRATE_MAX"] = 0.6

        # Reload scene with modified config file and execute maneuver again
        world.load_scene(config_dict, delay_after_load_sec=2)
        await execute_takeoff_and_rotate_maneuver()

        # Command the Drone to move down in NED coordinate system at 1 m/s for 4 seconds
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
        )  # schedule an async task to start the command
        projectairsim_log().info("Move-Down invoked")

        # Wait for move_down_task to complete before continuing
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
