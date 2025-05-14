"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates a simple mission on Microsoft Redmond campus with validation module.
In the demo, the drone has a source and a target (both have a landing pad spawned).
The objectives of the mission are evaluated using "mission_validation_module"
"""

import asyncio
import math

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.image_utils import ImageDisplay
from projectairsim.types import Quaternion
from projectairsim.validate import ValidationTaskModule
import mission_validation_module


# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_mission_validation.jsonc", delay_after_load_sec=0)
        world.pause()
        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Create a validation bench to enforce different validations
        validation_task_module = ValidationTaskModule(drone, world)
        # Inject the validation tasks into the validation bench
        mission_validation_module.inject_validation_tasks(validation_task_module)

        await mission_script(drone, world, client)
        # ------------------------------------------------------------------------------

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        mission_validation_module.summarize_validation_tasks()
        client.disconnect()


async def mission_script(drone: Drone, world: World, client: ProjectAirSimClient):
    image_display = ImageDisplay()
    world.spawn_object_at_geo(
        object_name="TakeoffPad",
        asset_path="BasicLandingPad",
        latitude=47.641586,
        longitude=-122.14108,
        altitude=103.3,
        rotation=[1, 0, 0, 0],
        object_scale=[2, 2, 2],
        enable_physics=False,
    )

    binary_gltf: bool = True
    file1 = open("assets/wind_turbine.glb", "rb")
    gltf_byte_array = file1.read()
    file1.close()

    world.spawn_object_from_file_at_geo(
        object_name="Turbine",
        file_format="gltf",
        byte_array=gltf_byte_array,
        is_binary=binary_gltf,
        latitude=47.643994,
        longitude=-122.139372,
        altitude=89.0,
        rotation=rpy_to_quaternion(0, 0, math.radians(90)),
        object_scale=[1.0, 1.0, 1.0],
        enable_physics=False,
    )

    world.spawn_object_at_geo(
        object_name="TakeoffPad",
        asset_path="BasicLandingPad",
        latitude=47.644276,
        longitude=-122.139614,
        altitude=89.5,
        rotation=[1, 0, 0, 0],
        object_scale=[2, 2, 2],
        enable_physics=False,
    )

    world.resume()
    await asyncio.sleep(3)

    # ------------------------------------------------------------------------------

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

    rot = Quaternion({"w": 1.0, "x": 0, "y": 0, "z": 0})

    await asyncio.sleep(3)
    drone.set_geo_pose(47.641586, -122.14108, 104, rot)
    # ------------------------------------------------------------------------------

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
        v_north=0.0, v_east=0.0, v_down=-3.0, duration=5.0
    )
    projectairsim_log().info("Move-Up invoked")

    await move_up_task
    projectairsim_log().info("Move-Up completed")

    move_task = await drone.move_to_geo_position_async(
        latitude=47.6438, longitude=-122.1395, altitude=120.0, velocity=20.0
    )

    await move_task
    await drone.rotate_by_yaw_rate_async(yaw_rate=0.3, duration=4.0)

    move_task = await drone.move_by_velocity_async(0, 0, 3.0, duration=11)
    await move_task

    await drone.rotate_by_yaw_rate_async(yaw_rate=0.3, duration=4.0)

    move_task = await drone.move_by_velocity_async(0.0, 2.0, 0.0, duration=2)
    await move_task

    move_task = await drone.move_by_velocity_async(0, 0, -3.0, duration=2.0)
    await move_task

    move_task = await drone.move_by_velocity_async(0.0, 1.0, 0.0, duration=2.5)
    await move_task

    move_task = await drone.move_by_velocity_async(0.0, 0, 3.0, duration=2.0)
    await move_task

    move_task = await drone.move_by_velocity_async(0.0, 2.0, 0.0, duration=2.5)
    await move_task

    move_task = await drone.move_by_velocity_async(4.25, -3.0, -1.0, duration=8)
    await move_task

    # ------------------------------------------------------------------------------

    # Command the Drone to move down in NED coordinate system at 1 m/s for 4 seconds
    move_down_task = await drone.move_by_velocity_async(
        v_north=0.0, v_east=0.0, v_down=4.0, duration=6.0
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
    image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
