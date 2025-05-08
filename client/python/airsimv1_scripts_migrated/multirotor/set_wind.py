import time
import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

async def main():
    client = ProjectAirSimClient()
    client.connect()

    try:
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("Setting wind to 10m/s in forward direction") # NED
        world.set_wind_velocity(10, 0, 0)

        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        time.sleep(5)

        projectairsim_log().info("Setting wind to 15m/s towards right") # NED
        world.set_wind_velocity(0, 15, 0)

        time.sleep(5)

        # Set wind to 0
        projectairsim_log().info("Resetting wind to 0")
        world.set_wind_velocity(0, 0, 0)
    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())