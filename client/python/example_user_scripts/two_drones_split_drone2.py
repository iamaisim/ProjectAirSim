"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Control script for Drone2.
This script does NOT reload the scene; it attaches to the current world with World(client).
Run this after the Drone1 script has already loaded the scene.
"""

import asyncio

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log


def imu_callback_drone2(_, imu_msg):
    print(f"[Drone2][IMU] {imu_msg}")


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

    try:
        client.connect()

        # Drone2 script attaches to the already-loaded scene.
        world = World(client)
        drone2 = Drone(client, world, "Drone2")

        if not client.is_service_only_mode():
            imu_topic = drone2.sensors.get("IMU1", {}).get("imu_kinematics")
            if imu_topic:
                client.subscribe(imu_topic, imu_callback_drone2)
            else:
                projectairsim_log().warning(
                    "[Drone2] IMU topic was not found; continuing without topic subscription."
                )
        else:
            projectairsim_log().warning(
                "[Drone2] Client is running in service-only mode; skipping topic subscription."
            )

        drone2.enable_api_control()
        drone2.arm()

        await fly_box_pattern(drone2, "Drone2")

        drone2.disarm()
        drone2.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
