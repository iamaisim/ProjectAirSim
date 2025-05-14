"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates using a manual controller to keyboard-control a non-physics quadrotor's
rotor commands.
"""

import cv2
import numpy as np

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log


class NonphysicsDrone(Drone):
    # Save latest RGB image callback data to the drone object
    cur_rgb_image = None

    # For callbacks, just copy data and return to prevent holding up topic sender
    def callback_rgb_image(self, topic, image_msg):
        self.cur_rgb_image = image_msg


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
rotor1_control_signal = 0.0
rotor2_control_signal = 0.0
rotor3_control_signal = 0.0
rotor4_control_signal = 0.0


# Regular synchronous main function since no async API commands are used
if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()
    drone_pose = []

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_manual_controller_drone.jsonc")

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = NonphysicsDrone(client, world, "Drone1")

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"], drone.callback_rgb_image
        )

        # ------------------------------------------------------------------------------

        projectairsim_log().info(
            "\nWith active window as OpenCV-displayed camera image:\n"
            "  Esc = quit\n"
            "  Q/A = Increase/decrease rotor1 control signal\n"
            "  W/S = Increase/decrease rotor2 control signal\n"
            "  E/D = Increase/decrease rotor3 control signal\n"
            "  R/F = Increase/decrease rotor4 control signal\n"
        )

        # ------------------------------------------------------------------------------

        while True:
            if key == 27:  # Esc key to quit
                cv2.destroyAllWindows()
                break

            # if key != -1:
            #     projectairsim_log().debug(f"Key press ID# = {key}")

            display_image(drone.cur_rgb_image, "RGB-Image")

            # ----------------------- Rotor 1 -----------------------

            if key == 113 or key == 97:  # 'Q' to increase or 'A' key to decrease
                delta_signal = 0.1 if key == 113 else -0.1
                rotor1_control_signal += delta_signal
                rotor1_control_signal = max(min(rotor1_control_signal, 1.0), 0.0)

                result = drone.set_control_signals(
                    {"Prop_FL_actuator": rotor1_control_signal}
                )

                projectairsim_log().info(
                    f"Set Prop_FL_actuator control signal = {rotor1_control_signal}"
                )

            # ----------------------- Rotor 2 -----------------------

            if key == 119 or key == 115:  # 'W' to increase or 'S' key to decrease
                delta_signal = 0.1 if key == 119 else -0.1
                rotor2_control_signal += delta_signal
                rotor2_control_signal = max(min(rotor2_control_signal, 1.0), 0.0)

                result = drone.set_control_signals(
                    {"Prop_FR_actuator": rotor2_control_signal}
                )

                projectairsim_log().info(
                    f"Set Prop_FR_actuator control signal = {rotor2_control_signal}"
                )

            # ----------------------- Rotor 3 -----------------------

            if key == 101 or key == 100:  # 'E' to increase or 'D' key to decrease
                delta_signal = 0.1 if key == 101 else -0.1
                rotor3_control_signal += delta_signal
                rotor3_control_signal = max(min(rotor3_control_signal, 1.0), 0.0)

                result = drone.set_control_signals(
                    {"Prop_RL_actuator": rotor3_control_signal}
                )

                projectairsim_log().info(
                    f"Set Prop_RL_actuator control signal = {rotor3_control_signal}"
                )

            # ----------------------- Rotor 4 -----------------------

            if key == 114 or key == 102:  # 'R' to increase or 'F' key to decrease
                delta_signal = 0.1 if key == 114 else -0.1
                rotor4_control_signal += delta_signal
                rotor4_control_signal = max(min(rotor4_control_signal, 1.0), 0.0)

                result = drone.set_control_signals(
                    {"Prop_RR_actuator": rotor4_control_signal}
                )

                projectairsim_log().info(
                    f"Set Prop_RR_actuator control signal = {rotor4_control_signal}"
                )

            # Loop until user presses Esc key

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
