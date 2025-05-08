import time
import os
from projectairsim import ProjectAirSimClient, World

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
    path = "file:///" + os.getcwd() + "/sample_texture.jpg"
    print(path)
    world.set_object_texture_from_url("OrangeBall", path)
finally:
    client.disconnect()

