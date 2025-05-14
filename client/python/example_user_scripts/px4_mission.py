"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates flying a FastPhysics quadrotor using a PX4 controller running the mission
already loaded into the vehicle by ground control software such as QGroundControl.
This script demonstrates how to command the drone to run the mission, interrupt the
mission to perform a maneuver, and then resume the mission.

Note: The PX4 controller should be running for the Iris airframe before
      starting this script by connecting the hardware board by serial port
      for HITL or running `make px4_sitl_default none_iris` in the PX4 source
      directory for SITL.

      When using QGroundControl to monitor the mission, it can be launched
      either before or after the sim and PX4 and will auto-connect over UDP.
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

        input(
            "Ready to begin mission.  Wait for PX4 to commence GPS fusion and press enter."
        )

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        projectairsim_log().info("Invoking drone.enable_api_control()")
        drone.enable_api_control()
        projectairsim_log().info("Invoking drone.Arm")
        drone.arm()

        # ------------ Start Mission ---------------------------------------------------

        projectairsim_log().info("Mission Mode: starting")
        mission_task = await drone.set_mission_mode_async()
        await mission_task
        projectairsim_log().info("Mission Mode: running")

        # ------------ Interrupt Mission -----------------------------------------------

        # Wait a few seconds and interrupt the mission
        input("Press enter to perform avoidance movement.")
        kinematics = drone.get_ground_truth_kinematics()

        # Calculate move 90 degrees to left of current heading
        v_north_avoidance = kinematics["twist"]["linear"]["y"]
        v_east_avoidance = -kinematics["twist"]["linear"]["x"]

        request_task = await drone.request_control_async()
        await request_task

        projectairsim_log().info("Performing avoidance movement")
        avoidance_task = await drone.move_by_velocity_async(
            v_north=v_north_avoidance, v_east=v_east_avoidance, v_down=0.0, duration=5.0
        )
        await avoidance_task
        projectairsim_log().info("Avoidance movement complete")

        # ------------ Resume Mission --------------------------------------------------

        projectairsim_log().info("Mission Mode: restarting")
        mission_task = await drone.set_mission_mode_async()
        await mission_task
        projectairsim_log().info("Mission Mode: running")

        # ------------------------------------------------------------------------------

        input("When mission is complete, press enter to exit.")

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
    asyncio.run(main(scene_to_run))  # Runner for async main function
