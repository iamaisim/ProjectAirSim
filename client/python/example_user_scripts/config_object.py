"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates loading a sim config as a Python Dict object, modifying it, and loading it
into the sim server scene.
"""

import asyncio

from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import load_scene_config_as_dict, projectairsim_log

# Async main function to wrap async drone commands
async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        # Constructing the World object without specifying a config does not load a
        # scene into the sim server
        world = World(client)

        config_dict, config_paths = load_scene_config_as_dict("scene_basic_drone.jsonc")

        projectairsim_log().info(
            "Loaded scene config from "
            + config_paths[0]
            + ", robot configs from "
            + str(config_paths[1])
            + ", and env actor configs from "
            + str(config_paths[2])
        )

        # Load the base config into the sim server
        projectairsim_log().info("Loading base scene config to sim server...")
        world.load_scene(config_dict, delay_after_load_sec=2)

        config_dict["id"] = "SceneBasicDroneModified"
        # Change the 1st actor's robot configuration physics type from fast-physics
        # to non-physics
        config_dict["actors"][0]["robot-config"]["physics-type"] = "non-physics"

        # Load the modified config into the sim server
        projectairsim_log().info("Loading modified scene config to sim server...")
        world.load_scene(config_dict, delay_after_load_sec=0)

        # Uncomment to print the loaded configuration data
        # print(world.get_configuration())
    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
