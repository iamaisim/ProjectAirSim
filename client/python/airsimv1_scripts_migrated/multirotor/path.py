import sys
import time
import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.drone import YawControlMode

async def main():
    client = ProjectAirSimClient()
    client.connect()
    try:
        # this script was originally designed to fly down the streets of AirSim OSS's Neighborhood environment
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        projectairsim_log().info("arming the drone...")
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("taking off...")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        time.sleep(1)

        # Project AirSim uses NED coordinates so negative axis is up.
        # z of -30 is 30 meters above the original launch point.
        # we need a high height to avoid colliding with the objects in the blocks environment
        z = -30
        projectairsim_log().info("make sure we are hovering at {} meters...".format(-z))
        move_task = await drone.move_to_position_async(0,0,z,1)
        await move_task

        projectairsim_log().info("flying on path...")
        path_task = await drone.move_on_path_async([[125,0,z],
                                        [125,-130,z],
                                        [0,-130,z],
                                        [0,0,z]],
                                12, 120,
                                YawControlMode.ForwardOnly, False, 0.0, 20, 1)
        await path_task

        # drone will over-shoot so we bring it back to the start point before landing.
        move_task = await drone.move_to_position_async(0,0,z,1)
        await move_task
        projectairsim_log().info("landing...")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("disarming...")
        drone.disarm()
        drone.disable_api_control()
        projectairsim_log().info("done.")
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())
