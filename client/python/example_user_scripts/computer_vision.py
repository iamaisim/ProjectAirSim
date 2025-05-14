"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates running the sim for keyboard-controlled computer vision.

Note that the set_pose API can be used for programmatic control of
drone movement in addition to the keyboard control demonstrated.
"""

import argparse
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import (
    rpy_to_quaternion,
    quaternion_to_rpy,
    projectairsim_log,
    unpack_image,
)
from projectairsim.types import Pose, Vector3, Quaternion


class CVDrone(Drone):
    # Save latest pose and RGB image callback data to the drone object
    cur_pose = None
    cur_rgb_image = None

    # For callbacks, just copy data and return to prevent holding up topic sender
    def callback_rgb_image(self, topic, image_msg):
        self.cur_rgb_image = image_msg

    def callback_actual_pose(self, topic, pose_msg):
        self.cur_pose = pose_msg


parser = argparse.ArgumentParser("Computer Vision")

parser.add_argument(
    "--save-path", help="Path to directory for saving images", default="./"
)

args = parser.parse_args()

# Use OpenCV directly to pop-up a sub-window for the camera sensor image
def display_image(image, save, number, win_name="Camera"):
    """Display the image using OpenCV HighGUI"""
    if image is not None:
        img_np = unpack_image(image)
        cv2.imshow(win_name, img_np)
        if save:
            cv2.imwrite(args.save_path + str(number) + ".png", img_np)

        # Use OpenCV to capture keyboard input while sub-window is active
        global key
        key = cv2.waitKeyEx(20)


key = -1  # initialize global keyboard value

# Regular synchronous main function since no async API commands are used
if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()
    camera_pose = []

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_computer_vision.jsonc")

        # Create a CVDrone object to interact with the "vehicle" in the loaded sim world
        camera = CVDrone(client, world, "CV")

        # ------------------------------------------------------------------------------

        # Subscribe to the vehicle's sensors with a callback to receive the sensor data
        client.subscribe(camera.robot_info["actual_pose"], camera.callback_actual_pose)

        client.subscribe(
            camera.sensors["Camera"]["scene_camera"], camera.callback_rgb_image
        )

        # ------------------------------------------------------------------------------

        print(
            "\nWith active window as OpenCV-displayed camera image:\n"
            "  Esc = quit\n"
            "  up arrow = move forward\n"
            "  down arrow = move backward\n"
            "  left arrow = move left\n"
            "  right arrow = move right\n"
            "  page up = move up\n"
            "  page down = move down\n"
            "  W = pitch up\n"
            "  S = pitch down\n"
            "  A = yaw to the left\n"
            "  D = yaw to the right\n"
            "  Q = roll to the left\n"
            "  E = roll to the right\n"
            "  Z = increase movement speed\n"
            "  X = decrease movement speed\n"
            "  1 = increase rotation speed\n"
            "  2 = decrease rotation speed\n"
            "  R = zoom in\n"
            "  F = zoom out\n"
            "  P = start/stop recording images\n"
        )

        # ------------------------------------------------------------------------------

        focal_length = 11.9
        aperture = 2.0

        pose_step = 0.2  # m
        pose_step_step = 0.05
        rot_step = 0.02  # rad
        rot_step_step = 0.005

        recording = False
        imgcount = 0

        while True:
            if key == 27:  # Esc key to quit
                cv2.destroyAllWindows()
                break

            # if key != -1:
            #     projectairsim_log().debug(f"Key press ID# = {key}")

            display_image(camera.cur_rgb_image, recording, imgcount, "RGB-Image")
            if recording:
                imgcount += 1

            update_pose = False

            if camera.cur_pose is not None:
                pos = camera.cur_pose["position"]
                rot = camera.cur_pose["orientation"]
                rpy = quaternion_to_rpy(
                    rot["w"], rot["x"], rot["y"], rot["z"]
                )
                roll = rpy[0]
                pitch = rpy[1]
                yaw = rpy[2]

                # translation
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
                if key == 2162688:  # page up key
                    pos["z"] = pos["z"] - pose_step
                    update_pose = True
                if key == 2228224:  # page down key
                    pos["z"] = pos["z"] + pose_step
                    update_pose = True

                # rotation
                if key == 97:  # A key
                    yaw = yaw - rot_step
                    update_pose = True
                if key == 100:  # D key
                    yaw = yaw + rot_step
                    update_pose = True
                if key == 119:  # W key
                    pitch = pitch + rot_step
                    update_pose = True
                if key == 115:  # S key
                    pitch = pitch - rot_step
                    update_pose = True
                if key == 113:  # Q key
                    roll = roll - rot_step
                    update_pose = True
                if key == 101:  # E key
                    roll = roll + rot_step
                    update_pose = True

                # meta-control
                if key == 122:  # Z key
                    pose_step += pose_step_step
                    projectairsim_log().info(
                        "adjusted translation speed to %f", pose_step
                    )
                if key == 120 and pose_step > 0:  # X key
                    pose_step -= pose_step_step
                    projectairsim_log().info(
                        "adjusted translation speed to %f", pose_step
                    )
                if key == 49:  # 1 key
                    rot_step += rot_step_step
                    projectairsim_log().info("adjusted rotation speed to %f", rot_step)
                if key == 50 and rot_step > 0:  # 2 key
                    rot_step -= rot_step_step
                    projectairsim_log().info("adjusted rotation speed to %f", rot_step)
                if key == 112:  # P key
                    if recording:
                        recording = False
                        projectairsim_log().info("Stopped saving images.")
                    elif not recording:
                        recording = True
                        projectairsim_log().info("Started saving images.")

                # Zoom
                if key == 114:  # R key
                    focal_length = focal_length + 0.1
                    camera.set_focal_length("Camera", 0, focal_length)
                    projectairsim_log().info(
                        "adjusted focal length to %f", focal_length
                    )
                if key == 102:  # f key
                    focal_length = focal_length - 0.1
                    camera.set_focal_length("Camera", 0, focal_length)
                    projectairsim_log().info(
                        "adjusted focal length to %f", focal_length
                    )

                if update_pose:
                    quat = rpy_to_quaternion(roll, pitch, yaw)
                    rot["w"] = quat[0]
                    rot["x"] = quat[1]
                    rot["y"] = quat[2]
                    rot["z"] = quat[3]
                    translation = Vector3({"x": pos["x"], "y": pos["y"], "z": pos["z"]})
                    rotation = Quaternion(
                        {"w": rot["w"], "x": rot["x"], "y": rot["y"], "z": rot["z"]}
                    )
                    transform = {"translation": translation, "rotation": rotation}
                    pose = Pose(transform)
                    camera_pose.append([pos["x"], pos["y"], pos["z"], roll, pitch, yaw])
                    camera.set_pose(pose, True)

            # Loop until user presses Esc key

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
