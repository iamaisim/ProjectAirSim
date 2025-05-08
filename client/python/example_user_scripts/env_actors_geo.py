"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates the use of environment actors with geographic coordinates (Latitude,
Longitude, Altitude).

Script allows for the the user to follow the path of the environment actor
manually with a non-physics drone.
"""

import cv2
import numpy as np

from projectairsim import ProjectAirSimClient, Drone, World, EnvActor
from projectairsim.utils import (
    rpy_to_quaternion,
    quaternion_to_rpy,
    get_trajectory_from_kml,
    projectairsim_log
)
from projectairsim.types import Pose, Vector3, Quaternion


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

# Regular synchronous main function since no async API commands are used
if __name__ == "__main__":
    # Create a ProjectAirSim client
    client = ProjectAirSimClient()
    drone_pose = []

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_gis_seattle_env_actor.jsonc")

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = NonphysicsDrone(client, world, "Drone1")

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
        client.subscribe(drone.robot_info["actual_pose"], drone.callback_actual_pose)

        client.subscribe(
            drone.sensors["Chase"]["scene_camera"], drone.callback_rgb_image
        )

        # ------------------------------------------------------------------------------

        # Create a EnvActor object to interact with the environment actor
        env_actor = EnvActor(client, world, "Actor1")

        # Import Seattle route from KML file and modify altitude
        traj_file = "sim_config/seattle.kml"
        traj_name = "seattle_art_museum_to_great_wheel"
        latitudes, longitudes, altitudes = get_trajectory_from_kml(traj_file)

        # coords obtained from Google My Maps which uses altitude of zero
        altitudes = [200] * len(latitudes)
        time = [10 * i for i in range(len(latitudes))]

        world.import_geo_trajectory(traj_name, time, latitudes, longitudes, altitudes)
        env_actor.set_trajectory(traj_name, False)

        projectairsim_log().info(
            env_actor.name + " moving along " + traj_name + " route..."
        )

        # ------------------------------------------------------------------------------

        # NonPhysics Drone to follow the env actor manually

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
                pose_step = 2.0  # m
                rot_step = 0.05  # rad

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
                    drone_pose.append([pos["x"], pos["y"], pos["z"], roll, pitch, yaw])
                    drone.set_pose(pose, True)

        # Loop until user presses Esc key

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
