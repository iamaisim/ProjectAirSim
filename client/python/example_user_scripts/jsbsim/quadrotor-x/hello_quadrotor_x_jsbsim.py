"""
Copyright (C) IAMAI CONSULTING CORP
MIT License.

Demonstrates flying the Auterion quadrotor_x JSBSim model with PX4 and cameras.

The model is distributed under the BSD 3-Clause License. See LICENSE.txt in
this directory for its attribution and license terms.
"""

import asyncio
from pathlib import Path

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.image_utils import ImageDisplay
from projectairsim.utils import projectairsim_log


async def main():
    client = ProjectAirSimClient()
    image_display = ImageDisplay()

    try:
        client.connect()

        sim_config_path = str(Path(__file__).resolve().parent / "sim_config")
        world = World(
            client,
            "scene_quadrotor_x_jsbsim_px4_sitl.jsonc",
            delay_after_load_sec=2,
            sim_config_path=f"{sim_config_path}/",
        )
        drone = Drone(client, world, "Drone1")

        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=2)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        image_display.start()

        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("takeoff_async: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("takeoff_async: completed")

        await asyncio.sleep(3)

        projectairsim_log().info("move_up_async: starting")
        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        await move_up_task
        projectairsim_log().info("move_up_async: completed")

        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("land_async: completed")

        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()
        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())
