"""
Loads the existing world from saved config and flies a drone.
"""

import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay

CONFIG_PATH = "./world_config.json"

async def main():
    client = ProjectAirSimClient()
    image_display = ImageDisplay()

    try:
        client.connect()
        world = World(client)  # sin argumentos
        drone = Drone(client, world, "Drone1")

        # --- Set up image windows ---
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=2)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        image_display.start()

        # --- Flight demo ---
        drone.enable_api_control()
        drone.arm()

        await (await drone.takeoff_async())
        await (await drone.move_by_velocity_async(v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0))
        await (await drone.move_by_velocity_async(v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0))
        await (await drone.land_async())

        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"❌ Exception: {err}", exc_info=True)
    finally:
        client.disconnect()
        image_display.stop()

if __name__ == "__main__":
    asyncio.run(main())