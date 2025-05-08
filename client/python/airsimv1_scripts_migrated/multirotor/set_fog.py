import time
import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.types import WeatherParameter

async def main():
    client = ProjectAirSimClient()
    client.connect()
    world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")
    world.enable_weather_visual_effects()

    drone.enable_api_control()
    drone.arm()

    projectairsim_log().info("Setting fog to 25%")
    world.set_weather_visual_effects_param(WeatherParameter.FOG, 0.25)

    takeoff_task = await drone.takeoff_async()
    await takeoff_task

    time.sleep(5)

    projectairsim_log().info("Setting fog to 50%")
    world.set_weather_visual_effects_param(WeatherParameter.FOG, 0.5)

    time.sleep(5)

    # Set fog to 0
    projectairsim_log().info("Resetting fog to 0%")
    world.set_weather_visual_effects_param(WeatherParameter.FOG, 0.0)

    client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())