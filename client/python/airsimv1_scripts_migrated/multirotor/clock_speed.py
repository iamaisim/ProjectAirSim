import asyncio
import time

from projectairsim import ProjectAirSimClient, World, Drone

# Run this script with step_ns: 3000000 and real-time-update-rate: 3000000 in scene_basic_drone.jsonc
# then run it again with step_ns: 1500000

async def main():
    client = ProjectAirSimClient()
    client.connect()
    try:
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        drone.arm()

        vx = vy = 0
        z = -20
        duration = 3

        # with step_ns: 1500000 you will see that this takes 6s (system time) and not 3s
        move_task = drone.move_by_velocity_async(vx, vy, z, duration)
        await move_task

        while True:
            vx = vy = 5
            z = -20
            duration = 5
            # with step_ns: 1500000 you will see that this takes 10s (system time)
            # and not 5s in each iteration
            move_task = drone.move_by_velocity_async(vx, vy, z, duration)
            await move_task
            time.sleep(10)
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())