from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, quaternion_to_rpy

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_computer_vision.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")
    pose = drone.get_ground_truth_pose()
    projectairsim_log().info("x={}, y={}, z={}".format(pose["translation"]["x"], pose["translation"]["y"], pose["translation"]["z"]))

    angles = quaternion_to_rpy(pose["rotation"]["w"],pose["rotation"]["x"],pose["rotation"]["y"],pose["rotation"]["z"])
    projectairsim_log().info("pitch={}, roll={}, yaw={}".format(angles[0], angles[1], angles[2]))

    pose["translation"]["x"] = pose["translation"]["x"] + 1
    drone.set_pose(pose, True)

finally:
    client.disconnect()