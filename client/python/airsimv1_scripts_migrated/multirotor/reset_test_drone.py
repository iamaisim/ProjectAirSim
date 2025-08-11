import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

import time

async def main():
    client = ProjectAirSimClient()
    client.connect()
    try:
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("fly")
        move_task = await drone.move_to_position_async(0, 0, -10, 5)
        await move_task

        # Reset the scene by re-instantiating World
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")
        drone.enable_api_control()
        drone.arm()
        time.sleep(5)
        projectairsim_log().info("done")

        projectairsim_log().info("fly")
        move_task = await drone.move_to_position_async(0, 0, -10, 5)
        await move_task
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())