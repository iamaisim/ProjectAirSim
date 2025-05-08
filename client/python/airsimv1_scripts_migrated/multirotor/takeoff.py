import sys
import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

async def main():
    z = 5
    if len(sys.argv) > 1:
        z = float(sys.argv[1])

    client = ProjectAirSimClient()
    client.connect()

    try:
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")
        drone.enable_api_control()

        drone.arm()

        projectairsim_log().info("taking off...")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        projectairsim_log().info("make sure we are hovering at {} meters...".format(z))

        if z > 5:
            # AirSim uses NED coordinates so negative axis is up.
            # z of -5 is 5 meters above the original launch point.
            move_task = await drone.move_to_position_async(0,0,-5,1)
            await move_task
            hover_task = await drone.hover_async()
            await hover_task
    finally:
        client.disconnect()
        projectairsim_log().info("done.")

if __name__ == "__main__":
    asyncio.run(main())