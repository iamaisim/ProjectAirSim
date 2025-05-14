"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates flying a FastPhysics quadrotor using a PX4 controller.

Note: PX4 controller also should be running for the Iris airframe before starting this
      script, by connecting the hardware board by serial port for HITL, or running
      `make px4_sitl_default none_iris` in the PX4 source directory for SITL.

      If using QGC for mission control, it can be launched either before or after the
      sim and PX4, and should auto-connect over UDP.
"""

import asyncio
import argparse

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay


# Async main function to wrap async drone commands
async def main(scenefile):
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to position up to 2 pop-up sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, scenefile, delay_after_load_sec=2)

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

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        projectairsim_log().info("Invoking enable_api_control")
        drone.enable_api_control()
        projectairsim_log().info("Invoking Arm")
        drone.arm()

        # ------------------------------------------------------------------------------

        takeoff_task = await drone.takeoff_async()
        projectairsim_log().info("takeoff_async invoked")
        await takeoff_task
        projectairsim_log().info("takeoff_async completed")

        # Command the drone to move up and east in NED coordinate system for 4 seconds
        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=1.0, v_down=-1.0, duration=4.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up_task
        projectairsim_log().info("Move-Up completed")

        # Command the Drone to move down and west in NED coordinate system for 4 seconds
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-1.0, v_down=1.0, duration=4.0
        )
        projectairsim_log().info("Move-Down invoked")
        await move_down_task
        projectairsim_log().info("Move-Down completed")

        land_task = await drone.land_async()
        projectairsim_log().info("land_async invoked")
        await land_task
        projectairsim_log().info("land_async completed")

        # ------------------------------------------------------------------------------

        # Shut down the drone
        projectairsim_log().info("Invoking Disarm")
        drone.disarm()
        projectairsim_log().info("Invoking disable_api_control")
        drone.disable_api_control()

        # ------------------------------------------------------------------------------

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()


if __name__ == "__main__":
    scene_files = {"hitl": "scene_px4_hitl.jsonc", "sitl": "scene_px4_sitl.jsonc"}

    parser = argparse.ArgumentParser(description="Basic drone exercise with PX4.")
    parser.add_argument(
        "mode",
        choices=["hitl", "sitl"],
        nargs="?",
        help=(
            'the PX4 configuration: "hitl" for hardware-in-the loop (hardware via'
            'serial port), "sitl" for software-in-the-loop (simulation) over '
            "TCP/IP"
        ),
        default="sitl",
    )
    args = parser.parse_args()

    scene_to_run = scene_files.get(args.mode)
    projectairsim_log().info('Using scene "' + scene_to_run + '"')
    asyncio.run(main(scene_to_run))  # Runner for async main function
