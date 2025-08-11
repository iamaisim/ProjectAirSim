from datetime import datetime
import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.types import ImageType

async def main():
    try:
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_drone_highres_camera.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        framecounter = 1

        prevtimestamp = datetime.now()

        while(framecounter <= 500):
            if framecounter%150 == 0:
                drone.get_images("high_res", [ImageType.SCENE])
                projectairsim_log().info("High resolution image captured.")

            if framecounter%30 == 0:
                now = datetime.now()
                projectairsim_log().info(f"Time spent for 30 frames: {now-prevtimestamp}")
                prevtimestamp = now

            drone.get_images("low_res", [ImageType.SCENE])
            framecounter += 1
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())