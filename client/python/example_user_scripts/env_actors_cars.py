"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates the use of environment actors with trajectories imported and set via
client API. A trajectory can be used by multiple actors with some global offset.
"""

from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log
import time

def set_env_car_traj(traj_name, csv_file):
    """Load trajectory from a CSV file and assign it to the environment actor."""
    time_list = []
    pose_x = []
    pose_y = []
    pose_z = []
    pose_yaw = []

    with open(csv_file, 'r') as f:
        next(f)
        for line in f:
            line = line.split(',')
            pose_x.append(float(line[0]))
            pose_y.append(float(line[1]))
            pose_z.append(float(line[2]) * -1)
            pose_yaw.append(float(line[3]))
            time_list.append(float(line[4]))

    pose_roll = [0] * len(time_list)
    pose_pitch = [0] * len(time_list)

    print(f"Trajectory {traj_name} imported successfully from {csv_file}")

    world.import_ned_trajectory(traj_name, time_list, pose_x, pose_y, pose_z, pose_roll, pose_pitch, pose_yaw)

    env_actor.set_trajectory(traj_name, to_loop=False)

if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_env_actor_car.jsonc", delay_after_load_sec=2)

        # Create an EnvActor object to interact with the sim world
        env_actor = EnvActor(client, world, "car1")

        # Load first trajectory
        set_env_car_traj("traj1", "car_path_2.csv")

        # Wait 10 seconds before loading the new trajectory
        print("Waiting 10 seconds before loading new trajectory...")
        time.sleep(6)

        # Load second trajectory
        set_env_car_traj("traj2", "car_path.csv")
        print("Trajectory loaded successfully")
    except Exception as e:
        projectairsim_log("Error occurred: {}".format(e))
        client.disconnect()
        exit(1)

    finally:
        # Disconnect from the simulation environment
        client.disconnect()
        projectairsim_log("Disconnected from the simulation environment")
