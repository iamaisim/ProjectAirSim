"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates debug plotting methods.
"""

import asyncio
import numpy as np

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.types import Pose, Vector3, Quaternion
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.image_utils import ImageDisplay

# Async main function to wrap async drone commands
async def main():

    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    world = None

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        points = [
            [x, y, -5] for x, y in zip(np.linspace(0, -10, 20), np.linspace(0, -20, 20))
        ]
        color_rgba = [1.0, 0.0, 0.0, 1.0]
        size = 10
        duration = 10
        is_persistent = True

        # plot red points for 10 s
        projectairsim_log().info("Displaying points in world!")
        world.plot_debug_points(points, color_rgba, size, duration, is_persistent)
        await asyncio.sleep(5)

        # clear points
        world.flush_persistent_markers()
        projectairsim_log().info("Points cleared!")

        # plot magenta arrows for 10 s
        points_start = [
            [x, y, z]
            for x, y, z in zip(
                np.linspace(0, 10, 20), np.linspace(0, 0, 20), np.linspace(-3, -10, 20)
            )
        ]
        points_end = [
            [x, y, z]
            for x, y, z in zip(
                np.linspace(0, 10, 20), np.linspace(10, 20, 20), np.linspace(-5, -8, 20)
            )
        ]
        color_rgba = [1.0, 0.0, 1.0, 1.0]
        thickness = 3
        size = 15
        is_persistent = False
        projectairsim_log().info("Displaying arrows in world!")
        world.plot_debug_arrows(
            points_start,
            points_end,
            color_rgba,
            thickness,
            size,
            duration,
            is_persistent,
        )
        await asyncio.sleep(duration)

        # plot solid red line
        points = [
            [x, y, -5] for x, y in zip(np.linspace(0, -10, 10), np.linspace(0, -20, 10))
        ]
        color_rgba = [1.0, 0.0, 0.0, 1.0]
        thickness = 5
        projectairsim_log().info("Displaying solid line in world!")
        world.plot_debug_solid_line(points, color_rgba, thickness, duration, is_persistent)

        # plot green dashed line. Make sure there are an even number of points!
        points = [
            [x, y, -7] for x, y in zip(np.linspace(0, -10, 10), np.linspace(0, -20, 10))
        ]
        color_rgba = [0.0, 1.0, 0.0, 1.0]
        projectairsim_log().info("Displaying dashed line in world!")
        world.plot_debug_dashed_line(
            points, color_rgba, thickness, duration, is_persistent
        )

        # plot strings
        strings = ["Microsoft AirSim" for i in range(5)]
        positions = [
            [x, y, -1] for x, y in zip(np.linspace(0, 20, 5), np.linspace(0, 0, 5))
        ]
        scale = 1
        color_rgba = [1.0, 1.0, 1.0, 1.0]
        projectairsim_log().info("Displaying text in world!")
        world.plot_debug_strings(strings, positions, scale, color_rgba, duration)
        await asyncio.sleep(duration)

        # plot transforms
        translations = [
            [x, y, -3] for x, y in zip(np.linspace(0, 15, 10), np.linspace(0, 15, 10))
        ]
        rotations = [
            rpy_to_quaternion(roll=0, pitch=0, yaw=yaw)
            for yaw in np.linspace(0, np.pi, 10)
        ]
        poses = []

        for trans, rot in zip(translations, rotations):
            trans = Vector3({"x": trans[0], "y": trans[1], "z": trans[2]})
            rot = Quaternion({"w": rot[0], "x": rot[1], "y": rot[2], "z": rot[3]})
            poses.append(
                Pose(
                    {
                        "translation": trans,
                        "rotation": rot,
                        "frame_id": "DEFAULT_ID",
                    }
                )
            )
        scale = 35
        projectairsim_log().info("Displaying transforms in world!")
        world.plot_debug_transforms(poses, scale, thickness, duration, is_persistent)
        await asyncio.sleep(duration)

        # plot transforms with names
        names = ["yaw = " + str(round(yaw, 1)) for yaw in np.linspace(0, np.pi, 10)]
        text_scale = 1
        projectairsim_log().info("Displaying transforms with text in world!")
        world.plot_debug_transforms_with_names(
            poses, names, scale, thickness, text_scale, color_rgba, duration
        )

        # ------------------------------------------------------------------------------

        # Set trace line params
        color_rgba = [0.0, 0.0, 1.0, 1.0]
        thickness = 3
        world.set_trace_line(color_rgba, thickness)
        projectairsim_log().info("Set trace line color to blue!")
        projectairsim_log().info("Press 't' while Unreal viewport window is active to see path")

        # ------------------------------------------------------------------------------

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

        projectairsim_log().info("Move forward invoked")
        move_forward_task = await drone.move_by_heading_async(
            heading=0.0, speed=2.0, duration=5
        )
        await move_forward_task
        projectairsim_log().info("Move forward completed.")

        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
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

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # ------------------------------------------------------------------------------

        if world is not None:
            # clear any residual debug traces
            world.flush_persistent_markers()

        # ------------------------------------------------------------------------------

        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
