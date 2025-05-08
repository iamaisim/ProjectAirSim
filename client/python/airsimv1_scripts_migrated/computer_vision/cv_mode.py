import pprint
import os
import time
import math
import tempfile
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, rpy_to_quaternion, unpack_image
from projectairsim.types import Pose, Quaternion, Vector3, ImageType

def camera_callback(camera_info, camera_name):
    projectairsim_log().info("CameraInfo %s: %s" % (camera_name, pp.pprint(camera_info)))
    client.unsubscribe(drone.sensors["front_center"]["scene_camera_info"])

def to_quaternion(pitch, roll, yaw):
    quat = rpy_to_quaternion(pitch, roll, yaw)
    return Quaternion({"w": quat[0], "x": quat[1], "y": quat[2], "z": quat[3]})

pp = pprint.PrettyPrinter(indent=4)

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_computer_vision.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")
    projectairsim_log().info('Press enter to set front_center gimbal to 15-degree pitch')
    input()
    camera_pose = Pose({"translation":Vector3({"x":0, "y":0, "z":0}), "rotation":to_quaternion(math.radians(15), 0, 0)}) #radians
    drone.set_camera_pose("front_center", camera_pose)

    #camera info is unimplemented
    projectairsim_log().info('Press enter to get camera parameters')
    input()
    cam_list = ["front_center","front_right","front_left","bottom_center","back_center"]
    for cam_name in cam_list:
        client.subscribe(
            drone.sensors[cam_name]["scene_camera_info"],
            lambda _, camera_info, camera_name=cam_name: camera_callback(camera_info, camera_name),
            )

    #wait for cameras to log
    time.sleep(1)

    tmp_dir = os.path.join(tempfile.gettempdir(), "airsim_cv_mode")
    projectairsim_log().info ("Saving images to %s" % tmp_dir)
    try:
        os.makedirs(tmp_dir)
    except OSError:
        if not os.path.isdir(tmp_dir):
            raise

    projectairsim_log().info('Press enter to get images')
    input()
    for x in range(3): # do few times
        z = x * -20 - 5 # some random number
        drone.set_pose(Pose({"translation":Vector3({"x":z, "y":z, "z":z}), "rotation":to_quaternion(x / 3.0, 0, x / 3.0)}))

        # DepthVis isn't implemented, so we substitute DepthPlanar
        responses = drone.get_images("front_center", [ImageType.DEPTH_PLANAR])
        responses.update(drone.get_images("front_right", [ImageType.DEPTH_PERSPECTIVE]))
        responses.update(drone.get_images("front_left", [ImageType.SEGMENTATION]))
        responses.update(drone.get_images("bottom_center", [ImageType.SCENE]))
        # uncomment this once these image types have been implemented
        #responses.update(drone.GetImages("back_center", [ImageType.DISPARITY_NORMALIZED, ImageType.SURFACE_NORMALS]))

        for idx, response in enumerate(responses.values()):
            if response["encoding"] == "16UC1":
                filename = os.path.join(tmp_dir, str(x) + "_" + str(idx) + ".pfm")
            else:
                filename = os.path.join(tmp_dir, str(x) + "_" + str(idx) + ".png")
            img = unpack_image(response)
            cv2.imwrite(filename, img)

        pose = drone.get_ground_truth_pose()
        pp.pprint(pose)

        time.sleep(3)
finally:
    client.disconnect()
