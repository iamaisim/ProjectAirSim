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

        projectairsim_log().info("Taking off")
        takeoff_task = await drone.move_by_velocity_z_async(0, 0, -20, 8)
        await takeoff_task
        time.sleep(3)    

        for i in range(1, 6):
            projectairsim_log().info("Starting command to run for 15sec")
            move_task = await drone.move_by_velocity_z_async(-1*i, -1*i, -20-i, 15)
            await move_task
            time.sleep(5) #run
            projectairsim_log().info("Pausing after 5sec")
            world.pause()
            time.sleep(5) #paused
            projectairsim_log().info("Restarting command to run for 7.5sec")
            world.continue_for_sim_time(7500000000) 
            time.sleep(10)
            projectairsim_log().info("Finishing rest of the command")
            world.resume()
            time.sleep(15)
            projectairsim_log().info("Finished cycle")
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())