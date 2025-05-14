"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Demo client script for a single FastPhysics drone in AirSimVNext.
"""

import asyncio
import os
import tempfile
import cv2
import numpy as np
from datetime import datetime
from projectairsim.types import ImageType

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, unpack_image

# Async main function to wrap async drone commands
async def main():
    # Create a AirSimVNext Client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a world object to interact with the Project AirSim world and load a scene
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)

        # Create a drone object to interact with a Drone in the loaded Project AirSim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("state: %s", drone.get_ground_truth_kinematics())
        projectairsim_log().info("imu_data: %s", drone.get_imu_data("IMU1"))
        projectairsim_log().info("barometer_data: %s", drone.get_barometer_data("Barometer"))
        projectairsim_log().info("magnetometer_data: %s", drone.get_magnetometer_data("Magnetometer"))
        projectairsim_log().info("gps_data: %s", drone.get_gps_data("GPS"))

        # ------------ TakeoffAsync ------------------------------------------

        projectairsim_log().info("Press enter to Takeoff")
        input()

        # example-1: wait on the result of async operation using 'await' keyword
        projectairsim_log().info("TakeoffAsync: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("TakeoffAsync: completed")

        projectairsim_log().info("state: %s", drone.get_ground_truth_kinematics())

        # -----------Move Drone-----------------------------------------------------------

        projectairsim_log().info("Press enter to move drone to (-10, 10, -10)")
        input()

        # Command the Drone to move to (-10, 10, -10) in NED coordinate system at 5 m/s
        projectairsim_log().info("Using MoveToPositionAsync API")
        move_to_position_task = await drone.move_to_position_async(
            north=-10, east=10, down=-10, velocity=5
        )  # schedule an async task to start the command
        await move_to_position_task

        projectairsim_log().info("state: %s", drone.get_ground_truth_kinematics())

        hover_task = await drone.hover_async()
        await hover_task

        # ---------------Take Images------------------------------------------------------

        projectairsim_log().info("Press enter to start taking images")
        input()

        # get camera images from the vehicle
        # note that we cannot take images in png and raw bitmap from the same camera like in AirSim OSS
        # the image format is determined in the camera config jsonc
        images = drone.get_images("front_center", [ImageType.SCENE, ImageType.DEPTH_PLANAR, ImageType.DEPTH_PERSPECTIVE, ImageType.SEGMENTATION])
        print('Retrieved images: %d' % len(images))

        tmp_dir = os.path.join(tempfile.gettempdir(), "airsim_drone")
        print ("Saving images to %s" % tmp_dir)
        try:
            os.makedirs(tmp_dir)
        except OSError:
            if not os.path.isdir(tmp_dir):
                raise

        for idx, image in enumerate(images.values()):
            img_np = unpack_image(image)
            if image["encoding"] == "16UC1":
                file_save_path = os.path.join(tmp_dir, str(idx) + ".pfm")
            else:
                file_save_path = os.path.join(tmp_dir, str(idx) + ".png")
            cv2.imwrite(file_save_path, img_np)

        projectairsim_log().info("Image Captured")

        # In the original script, the state and drone are reset at the end, cleaning the sim up for the next script to be run
        # In project AirSim there's no need to do this as the sim will be reset when the next script is run regardless

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
