from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, unpack_image
from projectairsim.types import ImageType

import cv2
import numpy as np

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_computer_vision.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")
    projectairsim_log().info('Press enter to set all object IDs to 0')
    input()
    found = world.set_segmentation_id_by_name("[\w]*", 0, True, True);
    projectairsim_log().info("Done: %r" % (found))

    #for block environment

    projectairsim_log().info('Press enter to change one ground object ID')
    input()
    found = world.set_segmentation_id_by_name("Ground", 20, True, True);
    projectairsim_log().info("Done: %r" % (found))

    #regex are case insensitive
    projectairsim_log().info('Press enter to change all ground object ID')
    input()
    found = world.set_segmentation_id_by_name("ground[\w]*", 22, True, True);
    projectairsim_log().info("Done: %r" % (found))

    ##for neighborhood environment

    #set object ID for sky
    found = world.set_segmentation_id_by_name("SkySphere", 42, True, True);
    projectairsim_log().info("Done: %r" % (found))

    #below doesn't work yet. You must set CustomDepthStencilValue in Unreal Editor for now
    projectairsim_log().info('Press enter to set Landscape object ID to 128')
    input()
    found = world.set_segmentation_id_by_name("[\w]*", 128, True, True);
    projectairsim_log().info("Done: %r" % (found))

    responses = drone.get_images("front_center", [ImageType.SEGMENTATION])
    projectairsim_log().info('Retrieved images: %d', len(responses))

    #save segmentation images in various formats
    for idx, response in enumerate(responses.values()):
        filename = 'c:/temp/py_seg_' + str(idx)

        projectairsim_log().info("Type %s, size %d", response["encoding"], len(response["data"]))
        img_rgb = unpack_image(response)

        #find unique colors
        projectairsim_log().info(np.unique(img_rgb[:,:,0], return_counts=True)) #red
        projectairsim_log().info(np.unique(img_rgb[:,:,1], return_counts=True)) #green
        projectairsim_log().info(np.unique(img_rgb[:,:,2], return_counts=True)) #blue 
finally:
    client.disconnect()