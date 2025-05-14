"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates the use of environment actors with trajectories imported and set via
client API. A trajectory can be used by multiple actors with some global offset.
"""

import asyncio

from projectairsim import ProjectAirSimClient, World, EnvActor
from projectairsim.utils import projectairsim_log
from typing import List
from math import cos, sin, radians


def get_looptheloop_coords(radius, center_coords, num_points):
    angle_step = radians(360) / num_points
    angle_start = radians(-180)
    angles = [angle_start + angle_step * idx for idx in range(num_points + 1)]

    # now do other half to complete 540 deg loop
    angle_start = angles[-1]
    angles.extend(
        [angle_start + angle_step * idx for idx in range(1, int(num_points / 2) + 1)]
    )

    y_c, z_c = center_coords

    y_coords = [y_c + radius * cos(angle) for angle in angles]
    z_coords = [z_c + radius * sin(angle) for angle in angles]
    return y_coords, z_coords


async def env_actor_motion_plan(env_actors: List[EnvActor], world: World):
    """Move env_actors in a descending and loop pattern.

    Arguments:
        env_actor {List[EnvActor]} -- List of ProjectAirSim EnvActors
        world {World} -- ProjectAirSim World
    """

    env_actor2 = env_actors[1]
    env_actor3 = env_actors[2]
    env_actor4 = env_actors[3]

    # ----------------------------------------------------------------------------------

    # Set env_actor2's trajectory to the same as env_actor1's with a time and
    # position offset
    projectairsim_log().info(
        "Setting " + env_actor2.name + "'s trajectory to 'right_and_descend_config'."
    )
    # when setting traj through API, note there is a delay
    traj_name = "right_and_descend_config"

    # rotate the tilt rotors to point rotors forward
    env_actor2.set_link_rotation_angles(
        {"Shroud_FL": 90, "Shroud_RL": 90, "Shroud_FR": 90, "Shroud_RR": 90}
    )
    env_actor2.set_trajectory(
        traj_name, to_loop=True, time_offset=3, x_offset=2, yaw_offset=1.57
    )

    projectairsim_log().info("Executing...")

    # ----------------------------------------------------------------------------------

    # Create a loop trajectory for env_actor3 and env_actor4
    traj_name = "looptheloop"
    time_sec = [4]
    pose_x = [1]
    pose_y = [0]
    pose_z = [-6]

    # Circle coords, exists in YZ plane
    num_points = 20  # discretization
    radius = 3
    center_coords = [10, -6]  # (y, z)

    # The looptheloop begins at y = 10 - 3 = 7 and z = -6, computed from the center
    # coords and the radius. It exits the looptheloop at (13, -6).
    y_looptheloop, z_looptheloop = get_looptheloop_coords(
        radius, center_coords, num_points
    )

    step_size = 0.2  # controls how fast the drone completes the looptheloop
    looptheloop_start = time_sec[-1] + 2
    looptheloop_times = [
        looptheloop_start + step_size * i for i in range(1, len(y_looptheloop) + 1)
    ]
    time_sec.extend(looptheloop_times)
    pose_x.extend([pose_x[-1]] * len(y_looptheloop))  # x coord is fixed
    pose_y.extend(y_looptheloop)
    pose_z.extend(z_looptheloop)

    # final segment after the looptheloop
    time_sec.append(time_sec[-1] + 2)  # slow down over 2 seconds
    pose_x.append(pose_x[-1])
    pose_y.append(pose_y[-1] + 3)  # travel 3 more units
    pose_z.append(pose_z[-1])

    world.import_ned_trajectory(traj_name, time_sec, pose_x, pose_y, pose_z)
    projectairsim_log().info("Setting '" + traj_name + "' to env actors.")
    env_actor3.set_trajectory(traj_name, to_loop=True)
    env_actor4.set_trajectory(traj_name, to_loop=True, time_offset=2)
    projectairsim_log().info("Executing...")

    # ----------------------------------------------------------------------------------


if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_env_actor.jsonc", delay_after_load_sec=2)

        # Create EnvActor objects to interact with those in the loaded world
        env_actor1 = EnvActor(client, world, "ActorWithConfigTraj")
        env_actor2 = EnvActor(client, world, "TiltrotorWithConfigTrajOffset")
        env_actor3 = EnvActor(client, world, "ActorWithApiTraj")
        env_actor4 = EnvActor(client, world, "ActorWithApiTrajAndOffset")

        env_actor_list = []
        env_actor_list.append(env_actor1)
        env_actor_list.append(env_actor2)
        env_actor_list.append(env_actor3)
        env_actor_list.append(env_actor4)

        # Run the async function to execute the async drone commands
        asyncio.run(env_actor_motion_plan(env_actor_list, world))

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
