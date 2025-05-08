"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates capturing images with bounding boxes from a non-physics drone.

Objects to be detected are defined in the camera settings at
robot_quadrotor_nonphysics_annotations.jsonc
"""

import cv2
import numpy as np

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import (
    rpy_to_quaternion,
    quaternion_to_rpy,
    projectairsim_log,
)
from projectairsim.image_utils import draw_bbox3D
from projectairsim.types import Pose, Vector3, Quaternion


# Use OpenCV directly to pop-up a sub-window for the camera sensor image
def display_annotated_image(image_msg, win_name="Drone"):
    """Display the image using OpenCV HighGUI"""
    if image_msg is not None:
        # Convert image data to numpy array
        nparr = np.frombuffer(image_msg["data"], dtype="uint8")
        img_np = np.reshape(nparr, [image_msg["height"], image_msg["width"], 3])
        
        # Make a mutable copy of the image
        img_np_mutable = img_np.copy()
        
        # Draw bounding boxes
        for annotation in image_msg["annotations"]:
            bbox_center = annotation["bbox2d"]["center"]
            bbox_size = annotation["bbox2d"]["size"]
            v1 = (
                int(bbox_center["x"] - (bbox_size["x"] / 2.0)),
                int(bbox_center["y"] - (bbox_size["y"] / 2.0)),
            )
            v2 = (
                int(bbox_center["x"] + (bbox_size["x"] / 2.0)),
                int(bbox_center["y"] + (bbox_size["y"] / 2.0)),
            )
            # Draw the rectangle on the image
            cv2.rectangle(img_np_mutable, v1, v2, (0, 255, 0), 3)
            # Draw 3D bounding box
            draw_bbox3D(img_np_mutable, annotation, (0, 0, 255), 2)
        
        # Display the image in a window
        cv2.imshow(win_name, img_np_mutable)
        
        # Capture keyboard input
        global key
        key = cv2.waitKeyEx(20)

class NonphysicsDrone(Drone):
    # Save latest pose and RGB image callback data to the drone object
    cur_pose = None
    cur_rgb_image = None

    # For callbacks, just copy data and return to prevent holding up topic sender
    def callback_rgb_image(self, topic, image_msg):
        self.cur_rgb_image = image_msg

    def callback_actual_pose(self, topic, pose_msg):
        self.cur_pose = pose_msg


key = -1  # initialize global keyboard value


# Regular synchronous main function since no async API commands are used
if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_nonphysics_annotations_drone.jsonc")

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = NonphysicsDrone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Subscribe to the drone's sensors with a callback to receive the sensor data
        client.subscribe(drone.robot_info["actual_pose"], drone.callback_actual_pose)

        client.subscribe(
            drone.sensors["Chase"]["scene_camera"], drone.callback_rgb_image
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
        )

        # ------------------------------------------------------------------------------

        while True:
            if key == 27:  # Esc key to quit
                cv2.destroyAllWindows()
                break

            # if key != -1:
            #     projectairsim_log().debug(f"Key press ID# = {key}")

            display_annotated_image(drone.cur_rgb_image)

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
                    drone.set_pose(pose, True)

            # Loop until user presses Esc key

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
