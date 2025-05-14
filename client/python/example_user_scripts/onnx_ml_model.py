"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates how to use an Onnx ML model on scene camera images.

The ONNX model file path is specified in the robot_quadrotor_fastphysics_onnx.jsonc
config as part of the camera sensor's settings called "post-process-model-settings".
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

from PIL import Image
from matplotlib.colors import hsv_to_rgb
import numpy as np
import cv2

key = -1


class ImageProcessingWrapper(Drone):
    """
    Class to inherit the Drone class for nonphysics operations
    """

    cur_pose = None
    img_msg = None
    annotation_msg = None

    def callback_rgb_image(self, topic, image_msg):
        self.img_msg = image_msg

    def callback_onnx_ouput(self, topic, image_msg):
        self.annotation_msg = image_msg
        if self.img_msg is not None:
            display_annotated_image(self.img_msg, self.annotation_msg)

    def callback_actual_pose(self, topic, msg):
        self.cur_pose = msg


# This is the number of channels in the output from the ONNX model
num_classes = 21


def get_palette():
    # prepare and return palette
    palette = [0] * num_classes * 3

    for hue in range(num_classes):
        if hue == 0:  # Background color
            colors = (0, 0, 0)
        else:
            colors = hsv_to_rgb((hue / num_classes, 0.75, 0.75))

        for i in range(3):
            palette[hue * 3 + i] = int(colors[i] * 255)

    return palette


def colorize(labels):
    # generate colorized image from output labels and color palette
    result_img = Image.fromarray(labels).convert("P", colors=num_classes)
    result_img.putpalette(get_palette())
    return np.array(result_img.convert("RGB"))


def visualize_output(image, output):
    assert (
        image.shape[0] == output.shape[1] and image.shape[1] == output.shape[2]
    )  # Same height and width
    assert output.shape[0] == num_classes

    # get classification labels
    raw_labels = np.argmax(output, axis=0).astype(np.uint8)

    # comput confidence score
    confidence = float(np.max(output, axis=0).mean())

    # generate segmented image
    result_img = colorize(raw_labels)

    # generate blended image
    blended_img = cv2.addWeighted(image[:, :, ::-1], 0.5, result_img, 0.5, 0)

    return confidence, result_img, blended_img, raw_labels


# Use OpenCV directly to pop-up a sub-window for the camera sensor image
def display_annotated_image(image_msg, annotation_msg, win_name="Drone"):
    """Display the image using OpenCV HighGUI"""
    if image_msg is not None:
        o1 = np.frombuffer(image_msg["data"], dtype="uint8")
        ann = np.frombuffer(annotation_msg["data"], dtype="uint8")

        orig = np.reshape(o1, (image_msg["height"], image_msg["width"], 3))
        output = np.reshape(
            ann, (num_classes, annotation_msg["height"], annotation_msg["width"])
        )
        conf, result_img, blended_img, raw_labels = visualize_output(orig, output)
        cv2.imshow(win_name, blended_img)

        # Use OpenCV to capture keyboard input while sub-window is active
        global key
        key = cv2.waitKeyEx(20)


client = None
world = None
drone = None
private_assets = []


def connect_to_sim_server():
    global client, world, drone

    # Create a Project AirSim client and connect
    server = "127.0.0.1"
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a World object to interact with the sim world
    world = World(client, "scene_onnx.jsonc", delay_after_load_sec=0)

    # Create a Drone object to interact with a drone in the sim world
    drone = ImageProcessingWrapper(client, world, "Drone1")


# Async main function to wrap async drone commands
async def main():
    try:
        # Connect to simulation environment
        connect_to_sim_server()

        # Subscribe to the actual scene camera which is being passed into the ONNX Model
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"], drone.callback_rgb_image
        )

        # ------------------------------------------------------------------------------

        # Subscribe to the topic that publishes the output of the ONNX model
        # This callback overlays the result of the model on top of the acutal image
        client.subscribe(
            f"{drone.sensors_topic}/DownCamera/post_process_onnx_image",
            drone.callback_onnx_ouput,
        )

        # ------------------------------------------------------------------------------

        # Wait a bit for image data to start displaying at client
        await asyncio.sleep(1)

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Takeoff
        projectairsim_log().info("**** Takeoff started ****")
        meters_up = 10
        vel = 2.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=cur_pos["z"] - meters_up,
            velocity=vel,
        )

        await task

        # Command the Drone to move down / land
        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()
        await land_task

        await asyncio.sleep(3)

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
