"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates using a Livox Mid-70 or Avia lidar with a rosette scan pattern.
"""

import asyncio
import argparse

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay
from projectairsim.lidar_utils import LidarDisplay


# Async main function to wrap async drone commands
async def main(lidar_type : str):
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    # ----------------------------------------------------------------------------------

    # Initialize a LidarDisplay object for a point cloud visualization sub-window
    lidar_subwin = image_display.get_subwin_info(2)
    lidar_display = LidarDisplay(
        x=lidar_subwin["x"], y=lidar_subwin["y"] + 30, view=LidarDisplay.VIEW_FORWARD
    )  # add 30 y-pix for window title bar

    # ----------------------------------------------------------------------------------

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(
            client, "scene_lidar_" + lidar_type + ".jsonc", delay_after_load_sec=2
        )

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to the drone's sensors with a callback to receive the sensor data
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=1)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        image_display.start()

        # ------------------------------------------------------------------------------

        client.subscribe(
            drone.sensors["lidar1"]["lidar"],
            lambda _, lidar: lidar_display.receive(lidar),
        )

        lidar_display.start()

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Fly the drone around the scene
        projectairsim_log().info("Move up")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-3.0, duration=12.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=12.0
        )
        await move_task

        projectairsim_log().info("Move north-east")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=4.0, v_down=0.0, duration=8.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=3.0
        )
        await move_task

        projectairsim_log().info("Move down")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=3.0, duration=12.0
        )
        await move_task

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()

        # ------------------------------------------------------------------------------

        lidar_display.stop()

        # ------------------------------------------------------------------------------


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Livox")

    parser.add_argument(
        "--type",
        help="Type of Lidar to use. 'mid70' and 'avia' supported",
        required=True,
    )

    args = parser.parse_args()

    if args.type not in ["mid70", "avia"]:
        print("Unsupported Lidar type. Must be 'mid70' or 'avia'")
        exit(0)

    asyncio.run(main(args.type))  # Runner for async main function
