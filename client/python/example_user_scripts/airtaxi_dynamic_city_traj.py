"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Demonstrates flying a FastPhysics VTOL quadtiltrotor air taxi using a SimpleFlight
controller in the Dallas/Fort Worth (DFW) GIS scene in the Dynamic City environment.
"""

import asyncio
import math

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.types import Vector3, Quaternion, Pose

private_assets = []


def load_private_assets(world):
    """Load private 3D assets and add to the world.
    Arguments:
        world   World to contain the new assets
    """
    global private_assets

    try:
        takeoff_pad_name_1: str = "TakeoffPad"
        pad_asset_path_1: str = "BasicLandingPad"
        pad_enable_physics_1: bool = False
        pad_translation_1 = Vector3({"x": 407.1, "y": 83.2, "z": -11.1})
        pad_rotation_1 = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
        pad_scale_1 = [10, 10, 3]
        pad_pose_1: Pose = Pose(
            {
                "translation": pad_translation_1,
                "rotation": pad_rotation_1,
                "frame_id": "DEFAULT_ID",
            }
        )

        takeoff_pad_name_2: str = "TakeoffPad"
        pad_asset_path_2: str = "BasicLandingPad"
        pad_enable_physics_2: bool = False
        pad_translation_2 = Vector3({"x": -664.7, "y": 81.1, "z": -18.6})
        pad_rotation_2 = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
        pad_scale_2 = [10, 10, 3]
        pad_pose_2: Pose = Pose(
            {
                "translation": pad_translation_2,
                "rotation": pad_rotation_2,
                "frame_id": "DEFAULT_ID",
            }
        )

        world.spawn_object(
            takeoff_pad_name_1, pad_asset_path_1, pad_pose_1, pad_scale_1, pad_enable_physics_1
        )
        world.spawn_object(
            takeoff_pad_name_2, pad_asset_path_2, pad_pose_2, pad_scale_2, pad_enable_physics_2
        )

        private_assets.append(takeoff_pad_name_1)
        private_assets.append(takeoff_pad_name_2)
    except Exception as exc:
        projectairsim_log().warning(exc)
        pass


def get_current_pos(drone):
    """Return a drone's current position in NED coordinates
    Arguments:
        drone   The drone to query
    Returns:
        (Return)    Current position of the drone
    """
    return drone.get_ground_truth_kinematics()["pose"]["position"]

# Async main function to wrap async drone commands
async def main(scenefile):
    speed = 20.0  # Horizontal flight speed (meters/second)
    transition_duration = (
        5.0  # Duration of transition from horizontal to vertical mode (seconds)
    )
    transition_speed = 5.0  # Horizontal speed to maintain lift while transitioning from horizontal flight (meters/second)
    yaw_rate = math.radians(4.5)  # Yaw rate limit (radians/sec)

    # Create a simulation client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, scenefile, delay_after_load_sec=0)

        load_private_assets(world)
        projectairsim_log().info("**** Spawned private assets ****")
        world.resume()
        await asyncio.sleep(5)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        projectairsim_log().info("Executing fly sequence.")
        drone.enable_api_control()
        drone.arm()

        # ------------ takeoff_async ----------------------------------------------------

        # Command the vehicle to take off and hover a short distance above the
        # ground and wait for the command to complete with the "await" keyword
        projectairsim_log().info("Takeoff started")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("Takeoff complete.")

        # Move up to a higher altitude
        move_task = await drone.move_by_velocity_async(0, 0, -2.0, 4)
        await move_task

        await asyncio.sleep(4)

        # ------------------------------------------------------------------------------

        # Command the vehicle to move up in NED coordinates to a specific height
        projectairsim_log().info("Move up invoked")
        vel = 5.0
        cur_pos = get_current_pos(drone)
        move_up_task = await drone.move_to_position_async(
            north=cur_pos["x"], east=cur_pos["y"], down=-60, velocity=vel
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
            heading=math.radians(180.0), speed=speed, duration=10
        )
        await move_forward_task
        projectairsim_log().info("Move forward completed.")

        # ------------------------------------------------------------------------------

        # Command vehicle to fly at a specific heading and speed
        projectairsim_log().info(
            f"Heading 90 (yaw {math.degrees(yaw_rate):.2} deg/s) invoked"
        )
        heading_45_task = await drone.move_by_heading_async(
            heading=math.radians(180.0), speed=speed+ 4.0, duration=10, yaw_rate=yaw_rate
        )
        await heading_45_task
        projectairsim_log().info("Heading 90 complete.")

        # ------------------------------------------------------------------------------

        # Command drone to fly at a specific heading, horizontal speed, and descent speed
        projectairsim_log().info(
            f"Heading 180 (yaw {math.degrees(yaw_rate):.2} deg/s) and descend invoked"
        )
        heading_180_task = await drone.move_by_heading_async(
            heading=math.radians(180.0),
            speed=speed + 4.0,
            v_down=1.0,
            duration=20,
            yaw_rate=yaw_rate,
        )
        await heading_180_task
        projectairsim_log().info("Heading 180 and descend complete.")

        # ------------------------------------------------------------------------------
        # Disable fixed-wing flight and switch back to multirotor flight mode
        # We have to keep moving forward to maintain lift until we're fully back in multirotor flight
        projectairsim_log().info("Disabling fixed-wing flight")
        disable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.Multirotor)
        await disable_fw_task
        transition_task = await drone.move_by_velocity_async(
            v_north=transition_speed, v_east=0, v_down=0, duration=transition_duration
        )
        await transition_task
        projectairsim_log().info("Transition from fixed-wing flight complete.")

        # ------------------------------------------------------------------------------
        # Descend towards ground
        projectairsim_log().info("Move down invoked")
        cur_pos = get_current_pos(drone)
        move_down_task = await drone.move_to_position_async(
            north=-658.2,
            east=82.1,
            down=-22.6,
            velocity=10.0,
        )
        await move_down_task
        projectairsim_log().info("Move down complete.")

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
    scene_to_run = "scene_quadtiltrotor_dfw_dynamic_city.jsonc"
    projectairsim_log().info('Using scene "' + scene_to_run + '"')
    asyncio.run(main(scene_to_run))  # Runner for async main function
