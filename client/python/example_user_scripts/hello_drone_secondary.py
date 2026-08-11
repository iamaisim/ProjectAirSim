"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI Consulting Corp.
MIT License. All rights reserved.

Secondary controller: flies the drone using only service calls (no topic subscriptions).
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log


async def main():
    client = ProjectAirSimClient()

    try:
        # 1) Connect to the running simulation (primary already loaded the scene)
        client.connect()

        # 2) Attach to current world without loading a new scene
        world = World(client)

        # 3) Attach to the existing drone
        drone = Drone(client, world, "Drone1")

        # ----------------- FLIGHT SEQUENCE (control) -----------------
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("takeoff_async: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("takeoff_async: completed")

        # Move up 1 m/s for 4 s
        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up_task
        projectairsim_log().info("Move-Up completed")

        # Move down 1 m/s for 4 s
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
        )
        projectairsim_log().info("Move-Down invoked")
        await move_down_task
        projectairsim_log().info("Move-Down completed")

        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("land_async: completed")

        # Disarm/disable at the end
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
