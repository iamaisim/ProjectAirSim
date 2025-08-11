import time
import asyncio
from projectairsim import ProjectAirSimClient, World, Drone
from projectairsim.utils import projectairsim_log
from projectairsim.types import WeatherParameter

async def main():
    client = ProjectAirSimClient()
    client.connect()
    try:
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        world.enable_weather_visual_effects()

        projectairsim_log().info('Press enter to enable rain at 25%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.25)

        projectairsim_log().info('Press enter to enable rain at 75%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.75)

        projectairsim_log().info('Press enter to enable snow at 50%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.SNOW, 0.50)

        projectairsim_log().info('Press enter to enable maple leaves at 50%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.MAPLE_LEAF, 0.50)

        projectairsim_log().info('Press enter to set all effects to 0%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.0)
        world.set_weather_visual_effects_param(WeatherParameter.SNOW, 0.0)
        world.set_weather_visual_effects_param(WeatherParameter.MAPLE_LEAF, 0.0)

        projectairsim_log().info('Press enter to enable dust at 50%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.DUST, 0.50)

        projectairsim_log().info('Press enter to enable fog at 50%')
        input()
        world.set_weather_visual_effects_param(WeatherParameter.FOG, 0.50)

        projectairsim_log().info('Press enter to disable all weather effects')
        input()

        world.disable_weather_visual_effects()
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())