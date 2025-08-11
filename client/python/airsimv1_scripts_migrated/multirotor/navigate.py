import cv2
import time
import math
import sys
import numpy as np
import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, quaternion_to_rpy, unpack_image
from projectairsim.drone import YawControlMode
from projectairsim.types import ImageType
from projectairsim.image_utils import ImageDisplay

async def main():
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()
    try:
        client.connect()
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        # set up the imageDisplay
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["front_center"]["depth_planar_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        image_display.start()

        drone.enable_api_control()
        drone.arm()

        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        yaw = 0
        pi = 3.14159265483
        vx = 0
        vy = 0

        while True:
            # this will return png width= 256, height= 144
            result = drone.get_images("front_center", [ImageType.DEPTH_PLANAR])
            depth_image = unpack_image(result[ImageType.DEPTH_PLANAR])

            # slice the image so we only check what we are headed into (and not what is down on the ground below us).

            top = np.vsplit(depth_image, 2)[0]

            # now look at 4 horizontal bands (far left, left, right, far right) and see which is most open.
            # the depth map uses black for very close (0) and white for far away (255), so we use that
            # to get an estimate of distance.
            bands = np.hsplit(top, [50,100,150,200])
            mins = [np.min(x) for x in bands]
            print(mins)
            max = np.argmax(mins)    
            distance = mins[max]

            # sanity check on what is directly in front of us (slot 2 in our hsplit)
            current = mins[2]

            if (current < 20):
                hover_task = await drone.hover_async()
                await hover_task
                projectairsim_log().info("whoops - we are about to crash, so stopping!")
                input()
        
            orientation = drone.get_ground_truth_kinematics()["pose"]["orientation"]
            pitch, roll, yaw = quaternion_to_rpy(orientation["w"], orientation["x"], orientation["y"], orientation["z"])

            if (distance > current + 300):
            
                # we have a 90 degree field of view (pi/2), we've sliced that into 5 chunks, each chunk then represents
                # an angular delta of the following pi/10.
                change = 0
                if (max == 0):
                    change = -2 * pi / 10
                elif (max == 1):
                    change = -pi / 10
                elif (max == 2):
                    change = 0 # center strip, go straight
                elif (max == 3):
                    change = pi / 10
                else:
                    change = 2*pi/10
        
                yaw = (yaw + change)
                vx = math.cos(yaw)
                vy = math.sin(yaw)
                projectairsim_log().info("switching angle yaw=%f vx=%f vy=%f max=%f distance=%d current=%d", math.degrees(yaw), vx, vy, max, distance, current)
        
            if (vx == 0 and vy == 0):
                vx = math.cos(yaw)
                vy = math.sin(yaw)

            projectairsim_log().info("distance=%d", current)
            move_task = await drone.move_by_velocity_z_async(vx, vy,-6, 1, YawControlMode.ForwardOnly, yaw=0, yaw_is_rate=False)
            await move_task

            key = cv2.waitKey(1) & 0xFF
            if (key == 27 or key == ord('q') or key == ord('x')):
                break
    finally:
        image_display.stop()
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())
