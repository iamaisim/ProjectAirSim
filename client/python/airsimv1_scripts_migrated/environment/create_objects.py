import random
import time
from projectairsim import ProjectAirSimClient, World
from projectairsim.types import Vector3, Quaternion, Pose

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
    
    scale = [1.0, 1.0, 1.0]

    asset_name = '1M_Cube_Chamfer'

    desired_name = f"{asset_name}_spawn_{random.randint(0, 100)}"
    pose = Pose({"translation":Vector3({"x":5.0, "y":0.0, "z":-10.0}), "rotation":Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})})

    obj_name = world.spawn_object(desired_name, asset_name, pose, scale, True)

    print(f"Created object {obj_name} from asset {asset_name} "
        f"at pose {pose}, scale {scale}")

    all_objects = world.list_objects(".*")
    if obj_name not in all_objects:
        print(f"Object {obj_name} not present!")

    time.sleep(10.0)

    print(f"Destroying {obj_name}")
    world.destroy_object(obj_name)
finally:
    client.disconnect()
