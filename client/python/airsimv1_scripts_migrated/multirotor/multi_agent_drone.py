from http.client import responses
import cv2
import numpy as np
import os
import pprint
import tempfile
import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, unpack_image
from projectairsim.types import ImageType

async def main():
    try:
        # connect to the AirSim simulator
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_multi_agent_classic.jsonc", delay_after_load_sec=2)
        drone1 = Drone(client, world, "Drone1")
        drone2 = Drone(client, world, "Drone2")

        drone1.enable_api_control()
        drone2.enable_api_control()
        drone1.arm()
        drone2.arm()

        projectairsim_log().info('Press enter to takeoff')
        input()

        #move_task = asyncio.create_task(drone.MoveByVelocityAsync(1, 1, 0, 12))
        #await move_task
        takeoff_task_1 = await drone1.takeoff_async()
        takeoff_task_2 = await drone2.takeoff_async()
        await asyncio.wait({takeoff_task_1, takeoff_task_2})

        state1 = drone1.get_ground_truth_kinematics()
        s = pprint.pformat(state1)
        projectairsim_log().info("state: %s" % s)
        state2 = drone2.get_ground_truth_kinematics()
        s = pprint.pformat(state2)
        projectairsim_log().info("state: %s" % s)

        projectairsim_log().info('Press enter to move vehicles')
        input()
        move_task_1 = await drone1.move_to_position_async(-5, 5, -10, 5)
        move_task_2 = await drone2.move_to_position_async(5, -5, -10, 5)
        await asyncio.wait({move_task_1, move_task_2})

        projectairsim_log().info('Press enter to take images')
        input()
        # get camera images from the drone. DepthVis isn't implemented, so we substitute DepthPlanar
        responses1 = drone1.get_images("front_center", [ImageType.DEPTH_PLANAR])
        responses1.update(drone1.get_images("front_right", [ImageType.SCENE]))
        projectairsim_log().info('Drone1: Retrieved images: %d' % len(responses1))
        responses2 = drone2.get_images("front_center", [ImageType.DEPTH_PLANAR])
        responses2.update(drone2.get_images("front_right", [ImageType.SCENE]))
        projectairsim_log().info('Drone2: Retrieved images: %d' % len(responses2))

        tmp_dir = os.path.join(tempfile.gettempdir(), "airsim_drone")
        projectairsim_log().info ("Saving images to %s" % tmp_dir)
        try:
            os.makedirs(tmp_dir)
        except OSError:
            if not os.path.isdir(tmp_dir):
                raise

        responses = [v for d in (responses1, responses2) for v in d.values()]
        for idx, response in enumerate(responses):
            if response["encoding"] == "16UC1":
                filename = os.path.join(tmp_dir, str(idx) + ".pfm")
            else:
                filename = os.path.join(tmp_dir, str(idx) + ".png")
            img = unpack_image(response)
            cv2.imwrite(filename, img)

        projectairsim_log().info('Press enter to finish script')
        input()
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())

