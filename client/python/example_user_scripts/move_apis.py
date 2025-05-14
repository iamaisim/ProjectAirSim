"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates various flight command APIs to move a drone.
"""

import asyncio
import math
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, quaternion_to_rpy
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

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to path location index Lambda with 2 arguments
        client.subscribe(drone.robot_info["current_waypoint_number"], 
                         lambda _, msg: print("Current Waypoint Number: ", msg))

        image_display.start()

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------ takeoff_async ----------------------------------------------------

        # Drone takeoff
        projectairsim_log().info("takeoff_async: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("takeoff_async: completed")

        #  ------------ move_by_velocity_async --------------------------------------------
        
        # Command the drone to move up in NED coordinate system for 4 seconds
        projectairsim_log().info("Using move_by_velocity_async API")
        move_by_vel_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        await move_by_vel_task

        # ------------ move_by_velocity_z_async --------------------------------------------

        # Command the Drone to move North and Up in NED coordinate system for 3 seconds
        projectairsim_log().info("Using move_by_velocity_z_async API")
        move_by_vel_z_task = await drone.move_by_velocity_z_async(
            v_north=2.0, v_east=0.0, duration=3.0, z=-20
        )
        await move_by_vel_z_task

        #  ------------ move_by_velocity_body_frame_async --------------------------------------------

        # Command the drone to move up and forward relative to its position for 4 seconds
        projectairsim_log().info("Using move_by_velocity_body_frame_async API")
        move_by_vel_task = await drone.move_by_velocity_body_frame_async(
            v_forward=5.0, v_right=0.0, v_down=-1.0, duration=4.0
        )
        await move_by_vel_task

        # ------------ move_by_velocity_body_frame_z_async --------------------------------------------

        # Command the Drone to move left relative to its position for 6 seconds while attaining a specific height and turning 45 degrees
        projectairsim_log().info("Using move_by_velocity_body_frame_z_async API")
        move_by_vel_z_task = await drone.move_by_velocity_body_frame_z_async(
            v_forward=0.0,
            v_right=-5.0,
            duration=6.0,
            z=-20,
            yaw_is_rate=False,
            yaw=math.radians(45.0),
        )
        await move_by_vel_z_task

        # ------------ move_by_heading_async ----------------------------------------------

        # Command the Drone to turn at the optional yaw rate to and move with the given heading
        projectairsim_log().info("Using move_by_heading_async API")
        move_by_heading_task = await drone.move_by_heading_async(
            heading=math.radians(90.0),
            speed=5.0,
            duration=3,
            yaw_rate=math.radians(5.0),
        )
        await move_by_heading_task

        # ------------ move_to_position_async ---------------------------------------------

        # Command the Drone to move to (10, -5, -10) in NED coordinate system with
        # velocity 3 m/s
        projectairsim_log().info("Using move_to_position_async API")
        move_to_position_task = await drone.move_to_position_async(
            north=10, east=-5, down=-10, velocity=3
        )
        await move_to_position_task
        # ------------ move_on_path_async -------------------------------------------------

        # Command the Drone to move on defined path with velocity 3 m/s
        projectairsim_log().info("Using move_on_path_async API")
        path = [[10, -5, -10], [13, 0, -12], [6, -8, -5]]
        move_on_path_task = await drone.move_on_path_async(path=path, velocity=3)
        await move_on_path_task

        # ------------ rotate_to_yaw_async ------------------------------------------------

        # Command the Drone to rotate to yaw of 0.5 rad
        projectairsim_log().info("Using rotate_to_yaw_async API")
        rotate_to_yaw_task = await drone.rotate_to_yaw_async(yaw=0.5)
        await rotate_to_yaw_task

        # ------------ rotate_by_yaw_rate_async --------------------------------------------

        # Command the Drone to rotate by 0.1 rad/s for 5 seconds
        projectairsim_log().info("Using rotate_by_yaw_rate_async API")
        rotate_by_yawrate_task = await drone.rotate_by_yaw_rate_async(
            yaw_rate=0.1, duration=5
        )
        await rotate_by_yawrate_task

        # ------------ go_home_async -----------------------------------------------------

        projectairsim_log().info("go_home_async: starting")
        go_home_task = await drone.go_home_async(velocity=2.0)
        await go_home_task
        projectairsim_log().info("go_home_async: completed")

        # ------------ land_async -------------------------------------------------------

        # Command the Drone to land
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
