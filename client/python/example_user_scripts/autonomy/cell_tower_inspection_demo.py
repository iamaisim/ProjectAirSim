"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Seattle Cell Tower Inspection Demo Script.
Uses 3D Aerial Obstacle Detection Autonomy Block for the Detect and Avoid Sequence.
"""

import argparse
import asyncio
import sys
import math

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay
from projectairsim.lidar_utils import LidarDisplay
from projectairsim.types import Vector3, Quaternion, Pose
from projectairsim.autonomy.utils import PredictionsDisplayAsync


client: ProjectAirSimClient = None  # AirSim client API object
world: World = None  # AirSim simulation API object
drone: Drone = None  # AirSim drone API object

cell_tower_positions = [
    Vector3({"x": 250, "y": -210, "z": 20}),
    Vector3({"x": 598, "y": 45, "z": -182}),
]


def load_private_assets():
    global world

    # spawn landing pad
    takeoff_pad_name: str = "TakeoffPad"
    pad_asset_path: str = "BasicLandingPad"
    pad_enable_physics: bool = False
    pad_translation = Vector3({"x": 0.0, "y": 0.0, "z": -6.2})
    pad_rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
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

    # spawn cell tower
    cell_tower_name: str = "CellTower1"
    cell_tower_asset_path: str = "CellTowerLargeNest"
    cell_tower_enable_physics: bool = False
    cell_tower_translation = cell_tower_positions[0]
    cell_tower_rotation = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
    cell_tower_scale = [2, 2, 2]
    cell_tower_pose: Pose = Pose(
        {
            "translation": cell_tower_translation,
            "rotation": cell_tower_rotation,
            "frame_id": "DEFAULT_ID",
        }
    )
    world.spawn_object(
        cell_tower_name,
        cell_tower_asset_path,
        cell_tower_pose,
        cell_tower_scale,
        cell_tower_enable_physics,
    )

    # spawn small cell tower 2
    cell_tower_name: str = "CellTower2"
    cell_tower_asset_path: str = "CellTowerSmall"
    cell_tower_enable_physics: bool = False
    cell_tower_translation = cell_tower_positions[1]
    cell_tower_rotation = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
    cell_tower_scale = [3, 3, 3]
    cell_tower_pose: Pose = Pose(
        {
            "translation": cell_tower_translation,
            "rotation": cell_tower_rotation,
            "frame_id": "DEFAULT_ID",
        }
    )
    world.spawn_object(
        cell_tower_name,
        cell_tower_asset_path,
        cell_tower_pose,
        cell_tower_scale,
        cell_tower_enable_physics,
    )


def connect_to_sim_server():
    global client, world, drone

    # Create a ProjectAirSim Client and connect
    server = "127.0.0.1"
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a world object to interact with the sim World
    world = World(
        client,
        "scene_gis_seattle.jsonc",
        delay_after_load_sec=0,
        sim_config_path=r"./configs",
    )
    projectairsim_log().info("**** Loaded Seattle scene ****")

    load_private_assets()
    projectairsim_log().info("**** Spawned private assets ****")

    # Create a drone object to interact with a Drone in the ProjectAirSim World
    drone = Drone(client, world, "Drone1")


async def main(model_endpoint):
    global client, world, drone

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()
    lidar_display = None
    prediction_display = PredictionsDisplayAsync(
        model_endpoint=model_endpoint, bb_3D=True
    )

    try:
        # Connect to simulation environment
        connect_to_sim_server()

        # Add down-facing cam display
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)

        # Add prediction display
        perception_window_name = "AutonomyBlocks::Perception:3DAOD-DAA"
        prediction_display.add_image(perception_window_name, subwin_idx=0)

        # Subscribe to rgb cam and pipe to image/prediction display
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, image: prediction_display.receive(image, perception_window_name),
        )

        # Subscribe to depth cam
        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=2)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        # Initialize a LidarDisplay object for a point cloud visualization sub-window
        # with a fixed 10x10x10 meter view boundry-box
        lidar_subwin = image_display.get_subwin_info(1)
        lidar_display = LidarDisplay(
            x=lidar_subwin["x"],
            y=lidar_subwin["y"] + 30,
            view=LidarDisplay.VIEW_FORWARD,
            view_bounds=[-5, -5, -5, 5, 5, 5],
            coordinate_axes_size=0.5,
        )  # add 30 y-pix for window title bar

        client.subscribe(
            drone.sensors["lidar1"]["lidar"],
            lambda _, lidar: lidar_display.receive(lidar),
        )

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        image_display.start()
        lidar_display.start()
        prediction_display.start()
        prediction_display.display_bb = False

        await asyncio.sleep(2)

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Taking Off")
        task = await drone.takeoff_async()
        await task

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Moving to Cell Tower 1")
        task = await drone.move_to_position_async(
            north=220, east=-200, down=-30, velocity=6
        )
        await task

        await asyncio.sleep(5)

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Moving Up")
        task = await drone.move_to_position_async(
            north=220,
            east=-200,
            down=-205,
            velocity=6,
            yaw_is_rate=False,
            yaw=math.radians(30),
        )
        await task

        await asyncio.sleep(5)

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Moving to Cell Tower 2")
        task = await drone.move_to_position_async(
            north=310, east=-125, down=-205, velocity=6
        )
        await task

        # ------------------------------------------------------------------------------

        prediction_display.display_bb = True

        projectairsim_log().info(f"Avoiding Obstacle")

        await asyncio.sleep(10)

        prediction_display.display_bb = False

        projectairsim_log().info(f"Moving to Cell Tower 2")
        task = await drone.move_to_position_async(
            north=565, east=35, down=-205, velocity=6
        )
        await task

        # ------------------------------------------------------------------------------

        await asyncio.sleep(5)

        image_display.stop()
        lidar_display.stop()
        prediction_display.stop()

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    try:

        parser = argparse.ArgumentParser(description="UAM Demo.")
        parser.add_argument(
            "--model-endpoint",
            help=("Path to Autonomy Blocks server endpoint."),
            default="http://127.0.0.1:8000/perception/aerialobstacle/",
        )
        args = parser.parse_args()
        model_endpoint = args.model_endpoint

        asyncio.run(main(model_endpoint))  # Runner for async main function

    except KeyboardInterrupt:
        pass  # Exit normally

    except:
        print(f"main caught exception {sys.exc_info[0]}")

    projectairsim_log().info(f"Press 'Ctrl+C' to exit the inference server")
    input("Press Enter to exit this script")
