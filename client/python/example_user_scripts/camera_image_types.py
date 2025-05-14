"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Uses the drone camera to take one picture with each image type and saves them to a
specified folder.
"""

import asyncio
import os
import argparse
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, unpack_image
from projectairsim.types import ImageType

parser = argparse.ArgumentParser("Image Types")

parser.add_argument(
    "--save-path", help="Path to directory for saving images", required=True
)

args = parser.parse_args()

save_path = args.save_path


async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        world = World(client, "scene_drone_sensors.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        drone.arm()

        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-4.0, duration=1.0
        )
        await move_up_task

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Press enter to save images")
        input()

        images = drone.get_images(
            "DownCamera",
            [
                ImageType.SCENE,
                ImageType.DEPTH_PLANAR,
                ImageType.DEPTH_PERSPECTIVE,
                ImageType.SEGMENTATION,
                ImageType.DEPTH_VIS,
                ImageType.DISPARITY_NORMALIZED,
                ImageType.SURFACE_NORMALS,
            ],
        )
        print("Saving images to %s" % save_path)

        for index, image in enumerate(images.values()):
            img_np = unpack_image(image)
            if image["encoding"] == "16UC1":
                file_save_path = os.path.join(save_path, str(index) + ".pfm")
            else:
                file_save_path = os.path.join(save_path, str(index) + ".png")
            cv2.imwrite(file_save_path, img_np)

        # ------------------------------------------------------------------------------

        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
