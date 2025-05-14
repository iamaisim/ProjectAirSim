"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates asset and object listing functions
"""

import asyncio

from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log

# 
async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()

        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # We can use the list_assets function to find sim assets that can potentially be added to the world
        projectairsim_log().info("Available assets including the word 'LandingPad': %s", world.list_assets(".*LandingPad.*"))

        # The list_actors function provides a list of actors, such as drones
        projectairsim_log().info("Actors in world: %s", world.list_actors())

        # The list_objects function is more general, including all objects in the world
        projectairsim_log().info("Cylinders in world: %s", world.list_objects("Cylinder.*"))

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
