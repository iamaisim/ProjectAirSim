import pprint
import tempfile
import os
import time
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, rpy_to_quaternion, unpack_image
from projectairsim.types import Pose, Quaternion, Vector3, ImageType

def to_quaternion(pitch, roll, yaw):
    quat = rpy_to_quaternion(pitch, roll, yaw)
    return Quaternion({"w": quat[0], "x": quat[1], "y": quat[2], "z": quat[3]})

pp = pprint.PrettyPrinter(indent=4)

def camera_callback(camera_info, camera_name):
    projectairsim_log().info("CameraInfo %s: %s" % (camera_name, pp.pprint(camera_info)))
    client.unsubscribe(drone.sensors["front_center"]["scene_camera_info"])

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_computer_vision.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")
    
    projectairsim_log().info('Press enter to get camera parameters')
    input()
    cam_list = ["front_center","front_right", "front_left"]
    for cam_name in cam_list:
        client.subscribe(
            drone.sensors[cam_name]["scene_camera_info"],
            lambda _, camera_info, camera_name=cam_name: camera_callback(camera_info, camera_name),
            )

    #wait for cameras to log
    time.sleep(1)

    projectairsim_log().info('Press enter to get images')
    input()
    tmp_dir = os.path.join(tempfile.gettempdir(), "airsim_drone")
    projectairsim_log().info ("Saving images to %s" % tmp_dir)
    try:
        for n in range(3):
            os.makedirs(os.path.join(tmp_dir, str(n)))
    except OSError:
        if not os.path.isdir(tmp_dir):
            raise

    for x in range(50): # do few times
        drone.set_pose(Pose({"translation":Vector3({"x":x, "y":0, "z":-2}), "rotation":to_quaternion(0, 0, 0)}), True)
        time.sleep(0.1)

        responses = drone.get_images(camera_id="front_center", image_type_ids=[ImageType.SCENE])
        responses.update(drone.get_images(camera_id="front_right", image_type_ids=[ImageType.SCENE]))
        responses.update(drone.get_images(camera_id="front_left", image_type_ids=[ImageType.SCENE]))

        for i, response in enumerate(responses.values()):
            if response["encoding"] == "16UC1":
                projectairsim_log().info("Type %s, size %d, pos %s" % (response["encoding"], len(response["data"]), pprint.pformat([response["pos_x"],response["pos_y"],response["pos_z"]])))
                filename = os.path.normpath(os.path.join(tmp_dir, str(x) + "_" + str(i) + '.pfm'))
            else:
                projectairsim_log().info("Type %s, size %d, pos %s" % (response["encoding"], len(response["data"]), pprint.pformat([response["pos_x"],response["pos_y"],response["pos_z"]])))
                filename = os.path.normpath(os.path.join(tmp_dir, str(i), str(x) + "_" + str(i) + '.png'))
            img = unpack_image(response)
            cv2.imwrite(filename, img)

        pose = drone.get_ground_truth_pose()
        pp.pprint(pose)

        time.sleep(3)

    # return the camera to the original position
    drone.set_pose(Pose({"translation":Vector3({"x":0, "y":0, "z":0}), "rotation":to_quaternion(0, 0, 0)}), True)
finally:
    client.disconnect()
