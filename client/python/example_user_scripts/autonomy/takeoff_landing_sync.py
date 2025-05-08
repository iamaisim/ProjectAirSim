"""
Copyright (C) Microsoft Corporation. All rights reserved.
ProjectAirSim:: Autonomy building-blocks:: Perception App1: Visual Drone Takeoff-Landing(Sync)
"""
import asyncio
import time
import commentjson
import os
import jsonschema
import cv2
import numpy as np
from projectairsim.autonomy.perception import PosePredictor

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.types import Pose, Quaternion, Vector3, ImageType
from projectairsim.utils import projectairsim_log


client: ProjectAirSimClient = None
world: World = None
drone: Drone = None
private_assets = []
window_created = False


def read_cjson(json_file: str):
    with open(json_file) as f:
        data = commentjson.load(f)
    return data


def validate_config(config, schema):
    try:
        jsonschema.validate(instance=config, schema=schema)
    except jsonschema.exceptions.ValidationError as err:
        raise err


# Load and validate App config
app_config_fname = "perception_app_config.jsonc"
app_config_schema_fname = "perception_app_config_schema.jsonc"
config_path = os.path.join("configs", app_config_fname)
config_schema_path = os.path.join("configs", "schema", app_config_schema_fname)
config = read_cjson(config_path)
schema = read_cjson(config_schema_path)
validate_config(config, schema)

# Load the Perception module
perception_arch = config.get("perception-arch")
perception_pretrained_model = config.get("perception-pretrained-model")
perception_gpu = config.get("perception-gpu")
perception_module = PosePredictor(
    perception_arch,
    perception_pretrained_model,
    perception_gpu,
)


def predict(image_packed):

    img = np.array(image_packed["data"], dtype="uint8")
    w, h = image_packed["width"], image_packed["height"]
    img_np = np.reshape(img, [h, w, 3])
    # Convert BGR to RGB
    img_np = img_np[:, :, ::-1].copy()
    # [x, y, w, h] unscaled
    output = perception_module.predict(img_np)
    # Scale outputs;
    # Currently GT uses CVRect; Where (x, y) is (col, row)
    if config.get("bb-normalized"):
        output = list(np.multiply(output, [w, h, w, h]))
    return output


def connect_to_sim_server():
    global client, world, drone

    # Create a Project AirSim client and connect
    server = "127.0.0.1"  # Assumes Sim server is running on localhost
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a world object to interact with the RobSim World
    world = World(
        client,
        config.get("scene"),
        delay_after_load_sec=0,
        sim_config_path=r"./configs",
    )
    projectairsim_log().info("**** Loaded Dallas/Fort Worth scene ****")

    load_private_assets()

    projectairsim_log().info("**** Spawned private assets ****")
    world.resume()

    # Configure segmentation ID's for each landing pad
    landing_pads = config.get("landing-pad")
    for landing_pad in landing_pads:
        world.set_segmentation_id_by_name(
            landing_pad["name"], landing_pad["seg_ID"], False, True
        )

    # Create a Drone object to interact with a drone in the sim world
    drone = Drone(client, world, "Drone1")


# Async main function to wrap async drone commands
async def main():
    try:
        # Connect to simulation environment
        connect_to_sim_server()

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        moves = config.get("moves")
        for i, move in enumerate(moves):
            if "weather" in move:
                world.reset_weather_effects()
                world.enable_weather_visual_effects()
                world.set_weather_visual_effects_param(move.get("weather"), 1.0)
            if "time-of-day" in move:
                world.set_time_of_day(True, move.get("time-of-day"), False, 1.0, 1.0, True)

            name = move.get("name")
            projectairsim_log().info(f"**** {name} Started ****")

            for _ in range(move["num-loops"]):
                images = drone.get_images(
                    camera_id="DownCamera",
                    image_type_ids=[ImageType.SCENE],
                )
                world.pause()
                rgb = images[ImageType.SCENE]
                prediction = predict(rgb)
                display(rgb, "RGB_Image", prediction)

                v_north = move["v-north"]
                v_east = move["v-east"]
                v_down = move["v-down"]
                duration = move["task-duration"]

                world.resume()
                task = await drone.move_by_velocity_async(
                    v_north, v_east, v_down, duration
                )
                await task

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

        unload_private_assets()
        projectairsim_log().info("**** Unloaded private assets ****")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


def display_image_with_bb(
    image, win_name: str, bb_coords: list, resize_x: int = None, resize_y: int = None
):
    """Display the image using OpenCV HighGUI"""

    if image is None:
        return

    # Unpack image data
    img = np.array(image["data"], dtype="uint8")
    img_cv = np.reshape(img, (image["height"], image["width"], 3))
    x_c, y_c, w, h = list(bb_coords)
    cv2.rectangle(
        img_cv,
        (int(x_c - w / 2), int(y_c - h / 2)),
        (int(x_c + w / 2), int(y_c + h / 2)),
        (0, 0, 233),
        2,
    )

    # Resize image if requested
    if resize_x is not None and resize_y is not None:
        img_cv = cv2.resize(
            img_cv, (resize_x, resize_y), interpolation=cv2.INTER_LINEAR
        )

    # Display image
    cv2.imshow(win_name, img_cv)


def display(image, image_name, bb_coords):
    global window_created
    # Create window if not already visible
    if not window_created:
        cv2.namedWindow(
            image_name,
            flags=cv2.WINDOW_GUI_NORMAL + cv2.WINDOW_AUTOSIZE,
        )
        window_created = True

    display_image_with_bb(
        image,
        image_name,
        bb_coords,
    )

    key = cv2.waitKey(1)  # expensive, can take minimum 5~15 ms
    if key == 27:  # Esc key
        cv2.destroyAllWindows()


def load_private_assets():
    global world, private_assets

    landing_pads = config.get("landing-pad")
    for landing_pad in landing_pads:

        takeoff_pad_name: str = landing_pad["name"]
        pad_asset_path: str = "BasicLandingPad"
        pad_enable_physics: bool = False
        pad_translation = Vector3(
            {"x": landing_pad["x"], "y": landing_pad["y"], "z": landing_pad["z"]}
        )  # Marriott
        pad_rotation = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
        pad_scale = [3, 3, 3]
        pad_pose: Pose = Pose(
            {
                "translation": pad_translation,
                "rotation": pad_rotation,
                "frame_id": "DEFAULT_ID",
            }
        )
        world.spawn_object(
            takeoff_pad_name, pad_asset_path, pad_pose, pad_scale, pad_enable_physics
        )
        private_assets.append(takeoff_pad_name)


def unload_private_assets():
    global world, private_assets
    for asset in private_assets:
        world.destroy_object(asset)
    private_assets.clear()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
