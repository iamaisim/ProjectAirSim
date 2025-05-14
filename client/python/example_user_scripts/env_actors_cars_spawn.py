"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates the use of environment actors with trajectories imported and set via
client API. A trajectory can be used by multiple actors with some global offset.
"""

from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log

import asyncio
import time

if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_env_actor_car_new_example.jsonc", delay_after_load_sec=2)

    except Exception as e:
        projectairsim_log("Error occurred: {}".format(e))
        client.disconnect()
        exit(1)
    
    finally:
        # Disconnect from the simulation environment
        client.disconnect()
        projectairsim_log("Disconnected from the simulation environment")