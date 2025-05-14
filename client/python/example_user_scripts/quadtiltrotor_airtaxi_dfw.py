"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates flying a FastPhysics VTOL quadtiltrotor air taxi using a SimpleFlight
controller in the Dallas/Fort Worth (DFW) GIS scene.
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
        takeoff_pad_name: str = "TakeoffPad"
        pad_asset_path: str = "BasicLandingPad"
        pad_enable_physics: bool = False
        pad_translation = Vector3({"x": 1111.0, "y": -781.0, "z": -0.1})
        pad_rotation = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
        pad_scale = [10, 10, 3]
        pad_pose: Pose = Pose(
            {
                "translation": pad_translation,
                "rotation": pad_rotation,
                "frame_id": "DEFAULT_ID",
            }
        )

        world.spawn_object(
            takeoff_pad_name, pad_asset_path, pad_pose, pad_scale, pad_enable_physics
        )

        private_assets.append(takeoff_pad_name)
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


def radius_of_turn(yaw_rate, speed):
    """Return the radius of the circle traveled by the vehicle at the given constant yaw rate and speed.

    Derivation:
        Time to travel the circle, t = 2pi / yaw_rate
        Distance traveled, d = speed * t = speed * 2pi / yaw_rate
        Radius of circle with circumference d, r = d / 2pi = speed * 2pi / yaw_rate / 2pi = speed / yaw_rate

    Arguments:
        yaw_rate    Turn rate (radians per second)
        speed       Horizontal travel rate (units per second)

    Returns:
        (Return)    Radius of circle (units from speed unit)
    """
    return speed / yaw_rate * 0.95  # Factor compensates for speed drop during turn


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

        # Set the home position
        home_pos = get_current_pos(drone)

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

        # ------------ Multirotor box pattern ------------------------------------------

        projectairsim_log().info("Multirotor box pattern started")
        vel = 2.0
        move_task = await drone.move_by_velocity_async(vel, 0, 0, 4)
        await move_task

        move_task = await drone.move_by_velocity_async(0, -vel, 0, 4)
        await move_task

        move_task = await drone.move_by_velocity_async(-vel, 0, 0, 4)
        await move_task

        move_task = await drone.move_by_velocity_async(0, vel, 0, 4)
        await move_task
        projectairsim_log().info("Multirotor box pattern complete")

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
            heading=0.0, speed=speed, duration=10
        )
        await move_forward_task
        projectairsim_log().info("Move forward completed.")

        # ------------------------------------------------------------------------------

        # Command vehicle to fly at a specific heading and speed
        projectairsim_log().info(
            f"Heading 90 (yaw {math.degrees(yaw_rate):.2} deg/s) invoked"
        )
        heading_45_task = await drone.move_by_heading_async(
            heading=math.radians(90.0), speed=speed, duration=10, yaw_rate=yaw_rate
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
            duration=15,
            yaw_rate=yaw_rate,
        )
        await heading_180_task
        projectairsim_log().info("Heading 180 and descend complete.")

        # ------------------------------------------------------------------------------

        projectairsim_log().info(
            f"Heading 270 (yaw {math.degrees(yaw_rate):.2} deg/s) invoked"
        )

        # At the end of the turn from heading 180 to 270, the drone will be one turn
        # radius further west along the Y axis.  Since the command duration starts after
        # the turn is complete, we shorten the distance to travel (delta_y) by one turn
        # radius.  In turning to heading 0, the turn will again end with the drone one
        # turn radius further west along the Y axis (and one turn radius further north
        # along the X.)  We shorten the distance by another turn radius so at the end of
        # the turn to heading 0, we'll end up on the home position's Y coordinate.
        cur_pos = get_current_pos(drone)
        turn_radius = radius_of_turn(yaw_rate, speed)
        delta_y = (home_pos["y"] + turn_radius) - (cur_pos["y"] - turn_radius)
        duration = math.fabs(delta_y) / speed

        heading_270_task = await drone.move_by_heading_async(
            heading=math.radians(270.0),
            speed=speed,
            duration=duration,
            yaw_rate=yaw_rate,
        )
        await heading_270_task
        projectairsim_log().info("Heading 270 complete.")

        # ------------------------------------------------------------------------------

        projectairsim_log().info(
            f"Heading 0 (yaw {math.degrees(yaw_rate):.2} deg/s) invoked"
        )
        heading_0_task = await drone.move_by_heading_async(
            heading=math.radians(0.0), speed=20.0, duration=0, yaw_rate=yaw_rate
        )
        await heading_0_task
        projectairsim_log().info("Heading 0 complete.")

        # ------------------------------------------------------------------------------

        # We may not be perfectly aligned with the home position's Y coordinate, so
        # calculate the heading and distance to the home position
        cur_pos = get_current_pos(drone)
        delta_x = home_pos["x"] - cur_pos["x"]
        delta_y = home_pos["y"] - cur_pos["y"]

        # Precompensate for additional distance traveled during transition to multicopter mode
        delta_x -= transition_duration * 5.0 + speed * 2.5

        heading = math.atan2(delta_y, delta_x)
        distance = math.sqrt(delta_x * delta_x + delta_y * delta_y)
        duration = distance / speed

        projectairsim_log().info(
            f"Heading {math.degrees(heading):.2} towards home invoked"
        )
        heading_home_task = await drone.move_by_heading_async(
            heading=heading, speed=speed, duration=duration
        )
        await heading_home_task
        projectairsim_log().info("Heading towards home complete.")

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

        # Go back to home position
        projectairsim_log().info("Go home invoked")
        task = await drone.go_home_async(timeout_sec=120, velocity=2.5)
        await task
        projectairsim_log().info("Go home complete.")

        # ------------------------------------------------------------------------------

        # Descend towards ground
        projectairsim_log().info("Move down invoked")
        cur_pos = get_current_pos(drone)
        move_down_task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=-5,
            velocity=1.0,
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
    scene_to_run = "scene_quadtiltrotor_dfw.jsonc"
    projectairsim_log().info('Using scene "' + scene_to_run + '"')
    asyncio.run(main(scene_to_run))  # Runner for async main function
