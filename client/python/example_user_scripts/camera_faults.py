"""
Copyright (C) Microsoft Corporation. All rights reserved.

Visualizes effects of chromatic_aberration and out of focus camera faults
through a non-physics drone RGB camera.

Usage:
    python camera_faults.py [--save-pose] <bool to save path as pose or not>
                               [--save-path] <path to save CSV file with poses>
"""

import argparse
import cv2
import numpy as np
import csv
import os

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.types import ImageType
from projectairsim.utils import (
    rpy_to_quaternion,
    quaternion_to_rpy,
    projectairsim_log,
)
from projectairsim.types import Pose, Vector3, Quaternion

parser = argparse.ArgumentParser("NonPhysics Drone")

parser.add_argument(
    "--save-pose",
    help="Determine if poses are saved to a CSV file",
    default=False,
    type=bool,
)

parser.add_argument(
    "--save-path", help="Path to directory for saving CSV file with poses", default="./"
)

args = parser.parse_args()


class NonphysicsDrone(Drone):
    # Save latest pose and RGB image callback data to the drone object
    cur_pose = None
    cur_rgb_image = None

    # For callbacks, just copy data and return to prevent holding up topic sender
    def callback_rgb_image(self, topic, image_msg):
        self.cur_rgb_image = image_msg

    def callback_actual_pose(self, topic, pose_msg):
        self.cur_pose = pose_msg


# Use OpenCV directly to pop-up a sub-window for the camera sensor image
def display_image(image, win_name="Drone"):
    """Display the image using OpenCV HighGUI"""
    if image is not None:
        nparr = np.frombuffer(image["data"], dtype="uint8")
        img_np = np.reshape(nparr, [image["height"], image["width"], 3])
        cv2.imshow(win_name, img_np)

        # Use OpenCV to capture keyboard input while sub-window is active
        global key
        key = cv2.waitKeyEx(20)


key = -1  # initialize global keyboard value


def write_to_csv(list, file_name):
    csv_path = os.path.join(args.save_path, file_name)

    with open(csv_path, "w", newline="") as f:
        write = csv.writer(f)
        write.writerows(list)


# Regular synchronous main function since no async API commands are used
if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()
    drone_pose = []

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_nonphysics_drone.jsonc")

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = NonphysicsDrone(client, world, "Drone1")

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
        client.subscribe(drone.robot_info["actual_pose"], drone.callback_actual_pose)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"], drone.callback_rgb_image
        )

        # ------------------------------------------------------------------------------

        projectairsim_log().info(
            "\nWith active window as OpenCV-displayed camera image:\n"
            "  Esc = quit\n"
            "  W = move up\n"
            "  S = move down\n"
            "  A = rotate counter-clockwise\n"
            "  D = rotate clockwise\n"
            "  up arrow = move north\n"
            "  down arrow = move south\n"
            "  left arrow = move west\n"
            "  right arrow = move east\n"
            "  B = Toggle chromatic aberration\n"
            "  C = enable out of focus fault injection\n"
            "  E = Set Camera reliability to zero (drops all images)\n"
            "  F = Set Camera reliability to one (receive all images)\n"

        )

        # ------------------------------------------------------------------------------
        chromatic_aberration = 2.5
        focal_length = 0.05
        while True:
            if key == 27:  # Esc key to quit
                cv2.destroyAllWindows()
                break

            # if key != -1:
            #     projectairsim_log().debug(f"Key press ID# = {key}")

            display_image(drone.cur_rgb_image, "RGB-Image")

            update_pose = False

            if drone.cur_pose is not None:
                pos = drone.cur_pose["position"]
                rot = drone.cur_pose["orientation"]
                roll, pitch, yaw = quaternion_to_rpy(
                    rot["w"], rot["x"], rot["y"], rot["z"]
                )
                pose_step = 0.2  # m
                rot_step = 0.02  # rad

                if key == 2490368 or key == 65362:  # up key
                    pos["x"] = pos["x"] + pose_step
                    update_pose = True
                if key == 2621440 or key == 65364:  # down key
                    pos["x"] = pos["x"] - pose_step
                    update_pose = True
                if key == 2555904 or key == 65363:  # right key
                    pos["y"] = pos["y"] + pose_step
                    update_pose = True
                if key == 2424832 or key == 65361:  # left key
                    pos["y"] = pos["y"] - pose_step
                    update_pose = True
                if key == 119:  # W key
                    pos["z"] = pos["z"] - pose_step
                    update_pose = True
                if key == 115:  # S key
                    pos["z"] = pos["z"] + pose_step
                    update_pose = True

                if key == 97:  # A key
                    yaw = yaw - rot_step
                    update_pose = True
                if key == 100:  # D key
                    yaw = yaw + rot_step
                    update_pose = True
                if key == 98:
                    chromatic_aberration = (chromatic_aberration + 2.5) % 5.0
                    drone.set_chromatic_aberration_intensity(
                        camera_id="DownCamera",
                        image_type_id=ImageType.SCENE,
                        intensity=chromatic_aberration,
                    )
                if key == 99:
                    focal_length = (focal_length + 0.05) % 0.2
                    drone.set_depth_of_field_focal_region(
                        camera_id="DownCamera",
                        image_type_id=ImageType.SCENE,
                        max_focal_distance=focal_length,
                    )
                if key == 101:
                    topic = drone.sensors["DownCamera"]["scene_camera"]
                    client.update_subscription_options(topic, reliability=0.0)

                if key == 102:
                    topic = drone.sensors["DownCamera"]["scene_camera"]
                    client.update_subscription_options(topic, reliability=1.0)
                
                if update_pose:
                    quat = rpy_to_quaternion(roll, pitch, yaw)
                    rot["w"] = quat[0]
                    rot["x"] = quat[1]
                    rot["y"] = quat[2]
                    rot["z"] = quat[3]
                    now = world.get_sim_time()
                    translation = Vector3({"x": pos["x"], "y": pos["y"], "z": pos["z"]})
                    rotation = Quaternion(
                        {"w": rot["w"], "x": rot["x"], "y": rot["y"], "z": rot["z"]}
                    )
                    transform = {"translation": translation, "rotation": rotation}
                    pose = Pose(transform)
                    drone_pose.append([pos["x"], pos["y"], pos["z"], roll, pitch, yaw])
                    drone.set_pose(pose, True)

            # Loop until user presses Esc key

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
        # If set from command-line, save the poses to a csv file
        if args.save_pose:
            file_name = "hello_nonphysics_drone_poses.csv"
            write_to_csv(drone_pose, file_name)
            projectairsim_log().info(
                "-------------------------------------------------"
            )
            projectairsim_log().info(
                "Poses written to CSV file with the following column order:"
            )
            projectairsim_log().info("x, y, z, roll, pitch, yaw")
            projectairsim_log().info(
                "CSV file saved to \n %s", os.path.join(args.save_path, file_name)
            )
            projectairsim_log().info(
                "-------------------------------------------------"
            )
