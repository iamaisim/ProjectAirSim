"""
Copyright (C) Microsoft Corporation. All rights reserved.
Demo client script to demonstrate the use of environment
actors with a trajectory imported from a JSBSim script
and set via client API. The trajectory can be used by 
multiple actors with some global offset.

Usage:
    cd jsbsim                             
    python jsbsim_trajectory_generator.py \
        --root="{JSBSim root folder}"     \
        --script={JSBSim script path}     \
        --out=JSBSimTrajectory.csv
    cd ..
    python jsbsim_env_actor.py
"""
from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log
import asyncio


async def env_actor_motion_plan(env_actor: EnvActor, world: World):
    """Assign JSBSim trajectory to the env_actor.

    Arguments:
        env_actor -- EnvActor object
        world {World} -- ProjectAirSim World
    """

    traj_name = "JSBSimTrajectory"
    world.import_ned_trajectory_from_csv(traj_name, "./jsbsim/JSBSimTrajectory.csv")
    env_actor.set_trajectory(traj_name, x_offset= 20, y_offset= -10,z_offset= - 10)
    projectairsim_log().info("Executing...")


if __name__ == "__main__":

    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a world object to interact with the ProjectAirSim world and load a scene
        # Set scene file to scene_env_actor.jsonc
        world = World(client, "scene_jsbsim_env_actor.jsonc", delay_after_load_sec=2)

        # Create env actor objects to interact with those in the loaded world
        env_actor1 = EnvActor(client, world, "env_actor")

        # Run the async function to execute the async drone commands
        asyncio.run(env_actor_motion_plan(env_actor1, world))

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
