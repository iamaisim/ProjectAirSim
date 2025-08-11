from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.types import Pose

import pprint

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_computer_vision.jsonc", delay_after_load_sec=2)
    # below works in Blocks environment

    #------------------------------------ Get current pose ------------------------------------------------

    # search object by name: 
    pose1 = world.get_object_pose("OrangeBall")
    projectairsim_log().info("OrangeBall - Position: %s, Orientation: %s" % (pprint.pformat(pose1["translation"]),
        pprint.pformat(pose1.rotation)))

    # search another object by tag
    pose2 = world.get_object_pose("Cone_5");
    projectairsim_log().info("Cone - Position: %s, Orientation: %s" % (pprint.pformat(pose2["translation"]),
        pprint.pformat(pose2.rotation)))

    # search non-existent object
    pose3 = world.get_object_pose("Non-Existent"); # should return nan pose
    projectairsim_log().info("Non-Existent - Position: %s, Orientation: %s" % (pprint.pformat(pose3["translation"]),
        pprint.pformat(pose3.rotation)))


    #------------------------------------ Set new pose ------------------------------------------------

    # here we move with teleport enabled so collisions are ignored
    pose1["translation"]["x"] -= 2
    pose1["translation"]["y"] -= 2
    pose1["translation"]["z"] -= 2
    success = world.set_object_pose("OrangeBall", pose1, True);
    projectairsim_log().info("OrangeBall moved. Success: %i (press enter to continue)" % (success))
    input()

    # here we move with teleport enabled so collisions are not ignored
    pose2["translation"]["x"] += 3
    pose2["translation"]["y"] += 3
    pose2["translation"]["z"] -= 2
    success = world.set_object_pose("Cone_5", pose2, False);
    projectairsim_log().info("Cone moved. Success: %i (press enter to continue)" % (success))
    input()

    # move non-existent object - will throw exception
    success = False
    try:
        world.set_object_pose("Non-Existent", pose2, False); # should return nan pose
        success = True
    except:
        pass
    projectairsim_log().info("Non-Existent moved. Success: %i (press enter to continue)" % (success))
    input()

    #------------------------------------ Get new pose ------------------------------------------------


    pose1 = world.get_object_pose("OrangeBall");
    projectairsim_log().info("OrangeBall - Position: %s, Orientation: %s" % (pprint.pformat(pose1.translation),
        pprint.pformat(pose1.rotation)))

    # search another object by tag
    pose2 = world.get_object_pose("Cone_5");
    projectairsim_log().info("Cone_5 - Position: %s, Orientation: %s" % (pprint.pformat(pose2.translation),
        pprint.pformat(pose2.rotation)))

    # search non-existent object
    pose3 = world.get_object_pose("Non-Existent"); # should return nan pose
    projectairsim_log().info("Non-Existent - Position: %s, Orientation: %s" % (pprint.pformat(pose3.translation),
        pprint.pformat(pose3.rotation)))
finally:
    client.disconnect()