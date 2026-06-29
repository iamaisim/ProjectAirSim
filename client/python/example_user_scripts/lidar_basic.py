"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Demonstrates using a basic lidar sensor with a cylindrical scan pattern.
"""

import asyncio
import math
import time
from pathlib import Path

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.lidar_utils import LidarTopDownRenderer
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay


class LidarStats:
    def __init__(self):
        self.message_count = 0
        self.last_report_wall = 0.0

    def receive(self, lidar_data):
        if lidar_data is None:
            return

        point_count = int(len(lidar_data.get("point_cloud", [])) / 3)
        if point_count == 0:
            return

        self.message_count += 1
        now = time.time()
        if self.message_count == 1 or now - self.last_report_wall >= 1.0:
            projectairsim_log().info(
                f"lidar1: msg={self.message_count} points={point_count}"
            )
            self.last_report_wall = now


def receive_lidar(lidar_data, lidar_stats, lidar_renderer, image_display, image_name):
    lidar_stats.receive(lidar_data)
    image_display.receive(lidar_renderer.render_image_msg(lidar_data), image_name)


# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()
    lidar_stats = LidarStats()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    # ----------------------------------------------------------------------------------

    # Initialize a top-down LIDAR image renderer.
    lidar_renderer = LidarTopDownRenderer(
        width=640,
        height=640,
        range_m=60.0,
    )

    # ----------------------------------------------------------------------------------

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        sim_config_path = str(Path(__file__).resolve().parent / "sim_config")
        world = World(
            client,
            "scene_lidar_drone.jsonc",
            delay_after_load_sec=2,
            sim_config_path=sim_config_path,
        )

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=1)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        lidar_name = "LIDAR"
        image_display.add_image(lidar_name, resize_x=640, resize_y=640)

        image_display.start()

        # ------------------------------------------------------------------------------

        client.subscribe(
            drone.sensors["lidar1"]["lidar"],
            lambda _, lidar: receive_lidar(
                lidar, lidar_stats, lidar_renderer, image_display, lidar_name
            ),
        )

        # client.subscribe(
        #     drone.sensors["lidar1"]["lidar"],
        #     lambda _, lidar: print(lidar),
        # )
        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Fly the drone around the scene
        projectairsim_log().info("Move up")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-3.0, duration=4.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=12.0
        )
        await move_task

        projectairsim_log().info("Yaw 720 degrees")
        yaw_task = await drone.rotate_by_yaw_rate_async(
            yaw_rate=math.radians(12.0),
            duration=20.0,
        )
        await yaw_task

        projectairsim_log().info("Move north-east")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=4.0, v_down=0.0, duration=8.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=3.0
        )
        await move_task

        projectairsim_log().info("Move down")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=3.0, duration=4.0
        )
        await move_task

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()

        # ------------------------------------------------------------------------------


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
