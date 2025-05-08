"""
Copyright (C) Microsoft Corporation. All rights reserved.

Example script that showcases using PAS client to execute .plan mission files
"""
from projectairsim.utils import projectairsim_log
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.mission_planner import MissionPlanner

import asyncio
import argparse
import pathlib


async def execute_mission__plan(plan_path: pathlib.Path):
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------
        mission_planner = MissionPlanner(plan_path)

        projectairsim_log().info(f"Executing Mission Plan...")
        await mission_planner.execute_mission(world, drone)
        # ------------------------------------------------------------------------------

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

        # ------------------------------------------------------------------------------

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Execute a given mission plan file")

    parser.add_argument(
        "--plan-path",
        help="Path to the .plan file",
        default="./sim_config/mission.plan",
    )
    args = parser.parse_args()

    plan_path = args.plan_path

    asyncio.run(execute_mission__plan(plan_path))
