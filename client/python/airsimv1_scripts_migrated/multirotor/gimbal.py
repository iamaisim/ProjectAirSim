import asyncio
import time

from projectairsim import ProjectAirSimClient, World, Drone
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.types import Vector3, Pose, Quaternion
from projectairsim.image_utils import ImageDisplay

async def main():
    try:
        # connect to the AirSim simulator
        client = ProjectAirSimClient()
        image_display = ImageDisplay()
        client.connect()
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")
        drone.enable_api_control()
        drone.arm()

        #set up a camera view so we can see the gimbal changes
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["front_center"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        image_display.start()

        projectairsim_log().info("Taking off")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("Ready")

        for i in range(5):
            move_task = await drone.move_to_position_async(-50.00,  50.26,  -20.58, 3.5)
            await move_task
            time.sleep(6)
            quaternion_values = rpy_to_quaternion(0.5, 0.5, 0.1)
            quat = Quaternion({"w": quaternion_values[0], "x": quaternion_values[1],"y": quaternion_values[2],"z": quaternion_values[3]})
            camera_pose = Pose({"translation":Vector3({"x":0, "y":0, "z":0}), "rotation":quat})
            drone.set_camera_pose("front_center", camera_pose)
            move_task = await drone.move_to_position_async(50.00, -50.26, -10.58, 3.5)
            await move_task
            time.sleep(6)
            quaternion_values = rpy_to_quaternion(-0.5, -0.5, -0.1)
            quat = Quaternion({"w": quaternion_values[0], "x": quaternion_values[1],"y": quaternion_values[2],"z": quaternion_values[3]})
            camera_pose = Pose({"translation":Vector3({"x":0, "y":0, "z":0}), "rotation":quat})
            drone.set_camera_pose("front_center", camera_pose)
    
    finally:
        image_display.stop()
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())