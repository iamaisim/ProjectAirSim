"""
Creates the world and saves its configuration to shared volume.
"""

import asyncio
import os
from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log

CONFIG_PATH = "./world_config.json"

async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
        projectairsim_log().info(f"✅ World created")
    except Exception as err:
        projectairsim_log().error(f"❌ Exception: {err}", exc_info=True)
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())
