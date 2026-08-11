"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Control script for Drone1.
This script loads the two-drone scene and controls only Drone1.
"""

import asyncio

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.image_utils import ImageDisplay
from projectairsim.utils import projectairsim_log


def imu_callback_drone1(_, imu_msg):
    print(f"[Drone1][IMU] {imu_msg}")


async def fly_box_pattern(drone: Drone, label: str):
    """Fly a simple box-like path for one drone."""
    cmd_duration_sim_sec = 3
    velocity_mps = 1

    projectairsim_log().info(f"[{label}] Move Up")
    task = await drone.move_by_velocity_async(0, 0, -velocity_mps, cmd_duration_sim_sec)
    await task

    projectairsim_log().info(f"[{label}] Move North")
    task = await drone.move_by_velocity_async(velocity_mps, 0, 0, cmd_duration_sim_sec)
    await task

    projectairsim_log().info(f"[{label}] Move West")
    task = await drone.move_by_velocity_async(0, -velocity_mps, 0, cmd_duration_sim_sec)
    await task

    projectairsim_log().info(f"[{label}] Move South")
    task = await drone.move_by_velocity_async(-velocity_mps, 0, 0, cmd_duration_sim_sec)
    await task

    projectairsim_log().info(f"[{label}] Move East")
    task = await drone.move_by_velocity_async(0, velocity_mps, 0, cmd_duration_sim_sec)
    await task

    projectairsim_log().info(f"[{label}] Move Down")
    task = await drone.move_by_velocity_async(0, 0, velocity_mps, cmd_duration_sim_sec)
    await task


async def main():
    client = ProjectAirSimClient()
    image_display = ImageDisplay()

    try:
        client.connect()

        # Drone1 script is the one that loads the scene.
        world = World(client, "scene_two_drones.jsonc", delay_after_load_sec=2)
        drone1 = Drone(client, world, "Drone1")

        chase_cam_window = "Drone1-ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone1.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )
        client.subscribe(drone1.sensors["IMU1"]["imu_kinematics"], imu_callback_drone1)
        image_display.start()

        drone1.enable_api_control()
        drone1.arm()

        await fly_box_pattern(drone1, "Drone1")

        drone1.disarm()
        drone1.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()
        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())
