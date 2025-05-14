"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates flying a FastPhysics VTOL quadtiltrotor air taxi using a PX4 controller.
"""

import argparse
import asyncio
import math

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log


# Async main function to wrap async drone commands
async def main(scenefile):
    # Create a simulation client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, scenefile, delay_after_load_sec=5)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        projectairsim_log().info("Executing fly sequence.")
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------

        # Command the vehicle to take off and hover a short distance above the
        # ground and wait for the command to complete with the "await" keyword
        projectairsim_log().info("Takeoff started")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("Takeoff complete.")

        # ------------------------------------------------------------------------------

        # Command the vehicle to move up in NED coordinates to a specific height
        projectairsim_log().info("Move up invoked")
        vel = 20.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        move_up_task = await drone.move_to_position_async(
            north=cur_pos["x"], east=cur_pos["y"], down=-40, velocity=vel
        )
        await move_up_task
        projectairsim_log().info("Move up completed.")

        # ------------------------------------------------------------------------------

        # Enable fixed-wing flight mode
        projectairsim_log().info("Enabling fixed-wing flight")
        enable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.FixedWing)
        await enable_fw_task

        # ------------------------------------------------------------------------------

        # Command vehicle to move forward to enter fixed-wing flight
        projectairsim_log().info("Move forward invoked")
        move_forward_task = await drone.move_by_heading_async(
            heading=0.0, speed=20.0, duration=15
        )
        await move_forward_task
        projectairsim_log().info("Move forward completed.")

        # ------------------------------------------------------------------------------

        # Command vehicle to fly at a specific heading and speed
        projectairsim_log().info("Heading 90 invoked")
        heading_45_task = await drone.move_by_heading_async(
            heading=math.radians(90.0), speed=20.0, duration=15
        )
        await heading_45_task
        projectairsim_log().info("Heading 90 complete.")

        # ------------------------------------------------------------------------------

        # Command drone to fly at a specific heading, horizontal speed, and descent speed
        projectairsim_log().info("Heading 180 and descend invoked")
        heading_180_task = await drone.move_by_heading_async(
            heading=math.radians(180.0), speed=24.0, v_down=2.0, duration=16
        )
        await heading_180_task
        projectairsim_log().info("Heading 180 and descend complete.")

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Heading 270 invoked")
        heading_270_task = await drone.move_by_heading_async(
            heading=math.radians(270.0), speed=20.0, duration=4
        )
        await heading_270_task
        projectairsim_log().info("Heading 270 complete.")

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Heading 0 invoked")
        heading_0_task = await drone.move_by_heading_async(
            heading=math.radians(0.0), speed=20.0, duration=0
        )
        await heading_0_task
        projectairsim_log().info("Heading 0 complete.")

        # ------------------------------------------------------------------------------

        # Disable fixed-wing flight and switch back to multirotor flight mode
        projectairsim_log().info("Disabling fixed-wing flight")
        disable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.Multirotor)
        await disable_fw_task

        # ------------------------------------------------------------------------------

        # Zero velocity and hold position
        projectairsim_log().info("Hold position invoked")
        hover_task = await drone.move_by_velocity_async(
            v_north=0, v_east=0, v_down=0, duration=7.5
        )
        await hover_task
        projectairsim_log().info("Hold position complete.")

        # ------------------------------------------------------------------------------

        # Descend towards ground
        projectairsim_log().info("Move down invoked")
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        move_down_task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=-5,
            velocity=5,
        )
        await move_down_task
        projectairsim_log().info("Move down complete.")

        # ------------------------------------------------------------------------------

        # Go back to home position
        projectairsim_log().info("Go home invoked")
        task = await drone.go_home_async()
        await task
        projectairsim_log().info("Go home complete.")

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Land started")
        land = await drone.land_async()
        await land
        projectairsim_log().info("Land complete.")

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


if __name__ == "__main__":
    scene_files = {
        "hitl": "scene_quadtiltrotor_fastphysics_px4_hitl.jsonc",
        "sitl": "scene_quadtiltrotor_fastphysics_px4_sitl.jsonc",
    }

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
