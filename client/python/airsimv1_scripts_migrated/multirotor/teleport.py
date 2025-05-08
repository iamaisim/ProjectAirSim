import time
from projectairsim import ProjectAirSimClient, Drone, World

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")

    pose = drone.get_ground_truth_pose()
    print(pose)

    # teleport the drone + 10 meters in x-direction
    pose["translation"]["x"] += 10

    drone.set_pose(pose)

    time.sleep(2)

    # teleport the drone back
    pose["translation"]["x"] -= 10
    drone.set_pose(pose)

finally:
    client.disconnect()