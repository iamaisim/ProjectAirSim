"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Spwans actors in the scene.
"""

from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log


if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_env_objects_fire_and_smoke.jsonc", delay_after_load_sec=2)

    except Exception as e:
        projectairsim_log("Error occurred: {}".format(e))
        client.disconnect()
        exit(1)
    
    finally:
        # Disconnect from the simulation environment
        client.disconnect()
        projectairsim_log("Disconnected from the simulation environment")