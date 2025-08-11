
from http.client import responses
import cv2
import numpy as np 
import pprint

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, unpack_image
from projectairsim.types import ImageType

client = ProjectAirSimClient()
client.connect()

try:
    world = World(client, "scene_detection.jsonc", delay_after_load_sec=2)
    drone = Drone(client, world, "Drone1")

    # set camera name and image type to request images and detections
    camera_name = "front_center"
    image_type = ImageType.SCENE

    # the detection filter is set up in the camera configuration
    # distance filtering is currently not supported

    while True:
        images = drone.get_images(camera_name, [image_type])
        if not images[0]:
            continue
        png = unpack_image(images[0])
        cylinders = images[0]["annotations"]
        if cylinders:
            for cylinder in cylinders:
                s = pprint.pformat(cylinder)
                print("Cylinder: %s" % s)

                xmin = int(cylinder["bbox2d"]["center"]["x"] - cylinder["bbox2d"]["size"]["x"]/2.0)
                width = int(cylinder["bbox2d"]["size"]["x"])
                ymin = int(cylinder["bbox2d"]["center"]["y"] - cylinder["bbox2d"]["size"]["y"]/2.0)
                height = int(cylinder["bbox2d"]["size"]["y"])
                cv2.rectangle(png,(xmin,ymin,width,height),(255,0,0),2)
                cv2.putText(png, cylinder["object_id"], (xmin,ymin - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (36,255,12))

        
        cv2.imshow("AirSim", png)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cv2.destroyAllWindows() 
finally:
    client.disconnect()
