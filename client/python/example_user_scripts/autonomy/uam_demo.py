"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

London UAM Demo Script.
Uses 2D Landing Pad Detection Autonomy Block to detect landing pads in the scene and
land on the detected landing pad using a PID controller.

"""
import argparse
import asyncio
import math

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.image_utils import ImageDisplay
from projectairsim.types import ImageType, Pose, Quaternion, Vector3
from projectairsim.utils import projectairsim_log
from projectairsim.autonomy.utils import PredictionsDisplayAsync

client: ProjectAirSimClient = None
world: World = None
drone: Drone = None
private_assets = []
landing_pad_z = None


def connect_to_sim_server():
    global client, world, drone, private_assets

    # Create a ProjectAirSim Client and connect
    server = "127.0.0.1"  # Assumes Sim server is running on localhost
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a world object to interact with the RobSim World
    world = World(
        client,
        "scene_synthetic_london.jsonc",
        delay_after_load_sec=0,
        sim_config_path=r"./configs",
    )
    projectairsim_log().info("**** Loaded London scene ****")

    load_private_assets()

    projectairsim_log().info("**** Spawned private assets ****")
    world.resume()

    # Configure segmentation ID's for each landing pad
    seg_ID = 125
    for landing_pad in private_assets:
        world.set_segmentation_id_by_name(landing_pad, seg_ID, False, True)

    # Create a drone object to interact with a Drone in the ProjectAirSim World
    drone = Drone(client, world, "Drone1")



async def main(model_endpoint):

    try:

        # Initialize Predictions and Image Display objects to display camera sub-windows
        prediction_display = PredictionsDisplayAsync(model_endpoint=model_endpoint)
        image_display = ImageDisplay()

        # Connect to simulation environment
        connect_to_sim_server()

        # RGB Cam with Autonomy Blocks Perception display
        perception_window_name = "Autonomy Blocks::Perception"
        prediction_display.add_image(perception_window_name, subwin_idx=0)

        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, image: prediction_display.receive(image, perception_window_name),
        )

        # Segmentation Cam
        seg_image_window_name = "Segmented-Image"
        image_display.add_image(seg_image_window_name, subwin_idx=1)
        client.subscribe(
            drone.sensors["DownCamera"]["segmentation_camera"],
            lambda _, seg: image_display.receive(seg, seg_image_window_name),
        )

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        await asyncio.sleep(2)

        prediction_display.start()
        image_display.start()

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Takeoff started")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("Takeoff complete.")

        # Move up to a higher altitude
        projectairsim_log().info("Move up invoked")
        move_task = await drone.move_by_velocity_async(0, 0, -2.0, 2)
        await move_task    

        vel = 5.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        move_up_task = await drone.move_to_position_async(
            north=cur_pos["x"], east=cur_pos["y"], down=-70, velocity=vel
        )
        await move_up_task
        projectairsim_log().info("Move up completed.")

        # ------------------------------------------------------------------------------
        
        prediction_display.display_bb = False # turn off model inference

        # ------------------------------------------------------------------------------

        # Enable fixed-wing flight mode
        projectairsim_log().info("Enabling fixed-wing flight")
        enable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.FixedWing)
        await enable_fw_task

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Initiating flight across river")
        task = await drone.move_by_heading_async(
            heading=math.radians(60), speed=20, v_down=-0.5, duration=18
        )
        await task

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Initiating flight towards Shard")
        task = await drone.move_by_heading_async(
            heading=math.radians(14), speed=20, v_down=-0.5, duration=60
        )
        await task

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Initiating flight across tower bridge")
        task = await drone.move_by_heading_async(
            heading=math.radians(263), speed=15, v_down=-0.75, duration=19
        )
        await task

        # ------------------------------------------------------------------------------

        # Disable fixed-wing flight and switch back to multirotor flight mode
        projectairsim_log().info("Disabling fixed-wing flight")
        disable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.Multirotor)
        await disable_fw_task

        transition_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0, v_down=0.5, duration=8.0
        )
        await transition_task

        # ------------------------------------------------------------------------------

        prediction_display.display_bb = True # turn on model inference

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Autonomously Landing the Drone")
        task = await land_drone(prediction_display)
        await task

        # ------------------------------------------------------------------------------

        prediction_display.stop()
        image_display.stop()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Stop any in-progress flight command and shut down the drone
        if drone is not None:
            drone.cancel_last_task()
            drone.disarm()
            drone.disable_api_control()

        client.disconnect()


def load_private_assets():
    global world, private_assets, landing_pad_z

    # Asset definition
    pad_asset_path: str = "BasicLandingPad"
    pad_enable_physics: bool = False
    pad_rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
    pad_scale = [7.0, 7.0, 7.0]

    # TakeOffPad
    pad_name = "TakeOffPad"
    pad_translation = Vector3({"x": 5, "y": 123, "z": -20})
    pad_pose: Pose = Pose(
        {
            "translation": pad_translation,
            "rotation": pad_rotation,
            "frame_id": "DEFAULT_ID",
        }
    )
    world.spawn_object(pad_name, pad_asset_path, pad_pose, pad_scale, pad_enable_physics)
    private_assets.append(pad_name)

    # LandingPad
    pad_name = "LandingPad"
    landing_pad_z = -42
    pad_translation = Vector3({"x": 1387, "y": 397, "z": landing_pad_z})
    pad_pose: Pose = Pose(
        {
            "translation": pad_translation,
            "rotation": pad_rotation,
            "frame_id": "DEFAULT_ID",
        }
    )
    world.spawn_object(pad_name, pad_asset_path, pad_pose, pad_scale, pad_enable_physics)
    private_assets.append(pad_name)


async def land_drone(prediction_display: PredictionsDisplayAsync):
    global landing_pad_z, drone

    # Align the drone to the desired orientation
    projectairsim_log().info("Rotating to yaw - 0")
    task = await drone.rotate_to_yaw_async(yaw=0)
    await task

    await asyncio.sleep(2)

    # Get image attributes
    images = drone.get_images(
        camera_id="DownCamera",
        image_type_ids=[ImageType.SCENE],
    )

    image = images[ImageType.SCENE]
    width = image["width"]
    height = image["height"]

    fov = 90
    meter_per_pixel = lambda alt, resolution: (
        alt * math.tan(math.radians(fov / 2))
    ) / (resolution / 2)

    kin = drone.get_ground_truth_kinematics()
    drone_z = kin["pose"]["position"]["z"]
    prev_time = kin["time_stamp"] / 1e9

    # PID control constants
    kp = 1.0
    kd = 0.0
    ki = 0.0

    # Initial error var
    e_prev_x = None
    e_prev_y = None
    e_sum_x = 0
    e_sum_y = 0

    projectairsim_log().info("Starting descent")
    task: asyncio.Task
    target_z_above_pad = 2  # meters
    while abs(drone_z - landing_pad_z) > target_z_above_pad:

        prediction_bbox = prediction_display.get_predicted_bb()

        if prediction_bbox == []:
            projectairsim_log().info("Landing Pad not in view.")
            pass
        x_c, y_c, w, h = prediction_bbox
        kin = drone.get_ground_truth_kinematics()
        pose = kin["pose"]["position"]
        cur_time = kin["time_stamp"] / 1e9
        dt_raw = cur_time - prev_time
        dt = max(dt_raw, 1e-9)
        prev_time = cur_time

        drone_z = pose["z"]
        alt_delta = abs(pose["z"] - landing_pad_z)
        x_m = meter_per_pixel(alt_delta, width)
        y_m = meter_per_pixel(alt_delta, height)
        x_delta = width // 2 - x_c
        y_delta = height // 2 - y_c

        e_sum_x += x_delta * dt
        e_sum_y += y_delta * dt

        if e_prev_x is None:
            e_prev_x = x_delta
            e_prev_y = y_delta

        pid_x = x_delta * kp + kd * (x_delta - e_prev_x) / dt + ki * e_sum_x
        pid_y = y_delta * kp + kd * (y_delta - e_prev_y) / dt + ki * e_sum_y

        e_prev_x = x_delta
        e_prev_y = y_delta

        pose_n = y_m * (pid_y) + pose["x"]
        pose_e = -x_m * (pid_x) + pose["y"]

        # Target centering the pad before getting all the way down to it
        landing_tgt_z = min(landing_pad_z, drone_z + 15)
        task = await drone.move_to_position_async(
            north=pose_n, east=pose_e, down=landing_tgt_z, velocity=2.0
        )

    # Do final touch down at a lower descent speed for a softer landing
    task = await drone.land_async()
    return task


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="UAM Demo.")
    parser.add_argument(
        "--model-endpoint",
        help=("Path to Autonomy Blocks server endpoint."),
        default="http://127.0.0.1:8000/perception/landingpad/"
    )
    args = parser.parse_args()
    model_endpoint = args.model_endpoint

    asyncio.run(main(model_endpoint))  # Runner for async main function

    projectairsim_log().info(f"Press 'Ctrl+C' to exit the inference server")
    input("Press Enter to exit this script")
