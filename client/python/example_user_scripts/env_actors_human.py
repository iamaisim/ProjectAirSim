"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates the use of environment actors with trajectories imported and set via
client API. A trajectory can be used by multiple actors with some global offset.
"""

from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log

import asyncio
import time

def set_env_car_traj():
    # read csv file and import the arguments of simAddNEDTrajectory()
    time = []
    pose_x = []
    pose_y = []
    pose_z = []
    pose_yaw = []
    with open('car_path.csv', 'r') as f:
        next(f)
        for line in f:
            line = line.split(',')
            pose_x.append(float(line[0]))
            pose_y.append(float(line[1]))
            pose_z.append(float(line[2]) * -1)
            pose_yaw.append(float(line[3])-(3.14159/2))
            time.append(float(line[4]))

    pose_roll = [0] * len(time)
    pose_pitch = [0] * len(time)

    print("Trajectory imported successfully")

    world.import_ned_trajectory("traj1", time, pose_x, pose_y, pose_z, pose_roll, pose_pitch, pose_yaw)

    # def simSetEnvCarTrajectory(self, vehicle_name="", traj_name="", to_loop=False, time_offset=0, x_offset=0, y_offset=0, z_offset=0, roll_offset=0, pitch_offset=0, yaw_offset=0):
    # use z_offset= 1
    env_actor.set_trajectory("traj1", to_loop=True)

if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_env_actor_human.jsonc", delay_after_load_sec=2)

        # Create an EnvActor object to interact with the sim world
        env_actor = EnvActor(client, world, "pedestrian")


        # Run the async method to set the trajectory for the actor
        set_env_car_traj()

    except Exception as e:
        projectairsim_log("Error occurred: {}".format(e))
        client.disconnect()
        exit(1)
    
    finally:
        # Disconnect from the simulation environment
        client.disconnect()
        projectairsim_log("Disconnected from the simulation environment")