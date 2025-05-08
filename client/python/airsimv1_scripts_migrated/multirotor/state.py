import pprint
import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

# There's no true equivalent to getMultirotorState, so just print the kinematics
def print_state():
    projectairsim_log().info("===============================================================")
    state = drone.get_ground_truth_kinematics()
    projectairsim_log().info("state: %s" % pprint.pformat(state))
    return state


client = ProjectAirSimClient()
client.connect()
world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
drone = Drone(client, world, "Drone1")
state = print_state()
client.disconnect()