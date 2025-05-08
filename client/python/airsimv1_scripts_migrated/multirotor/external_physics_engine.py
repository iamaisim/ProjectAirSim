import time
from projectairsim import ProjectAirSimClient, World, Drone
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.types import ImageType

# This example shows how to use the Non-Physics mode to move a drone through the API
# It allows you to control the drone through SetPose and obtain collision information.
# It is especially useful for injecting your own flight dynamics model to the AirSim drone.

# Use Blocks environment to see the drone colliding and seeing the collision information 
# in the command prompt.

def collision_callback(collision_info):
    # info is only sent if there's a collision
    projectairsim_log().info(collision_info)

try:
    client = ProjectAirSimClient()
    client.connect()
    world = World(client, "scene_nonphysics_drone.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")
    pose = drone.get_ground_truth_pose()

    quat = rpy_to_quaternion(0.1, 0.1, 0.1)
    pose["rotation"]["w"] = quat[0]
    pose["rotation"]["x"] = quat[1]
    pose["rotation"]["y"] = quat[2]
    pose["rotation"]["z"] = quat[3]

    drone.set_pose( pose, False )

    client.subscribe(
            drone.robot_info["collision_info"],
            lambda _, collision_info: collision_callback(collision_info),
        )

    # Enough iterations to make sure we hit something
    for i in range(3000):
        projectairsim_log().info(i)
        pose = drone.get_ground_truth_pose()
        pose["translation"]["x"] += 0.03
        quat = rpy_to_quaternion(0.1, 0.1, 0.1)
        pose["rotation"]["w"] = quat[0]
        pose["rotation"]["x"] = quat[1]
        pose["rotation"]["y"] = quat[2]
        pose["rotation"]["z"] = quat[3]
        drone.set_pose( pose, False )
        time.sleep(0.003)
finally:
    client.disconnect()