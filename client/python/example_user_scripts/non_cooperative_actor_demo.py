"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates how to use the client apis to create a non-cooperative actor
"""

import asyncio
from projectairsim import ProjectAirSimClient, Drone, World, EnvActor
from projectairsim.utils import projectairsim_log
from projectairsim.types import Pose
from projectairsim.non_cooperative_actor import NonCooperativeActor

# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        client.connect()

        world = World(
            client, "scene_non_cooperative_actor.jsonc", delay_after_load_sec=2
        )

        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        drone.arm()

        path = [[1, 8, -40], [21, -28, -40], [41, -48, -40]]

        (
            spawn_point,
            intercept_point,
            leg_start,
        ) = world.get_random_free_position_near_path(path)

        nco = NonCooperativeActor(
            client,
            world,
            "NonCooperativeActor",
            spawn_point,
            4.0,
            1.0
        )

        projectairsim_log().info(
            f"Placing non-cooperative actor at {spawn_point['x']}, {spawn_point['y']}, {spawn_point['z']}"
        )

        projectairsim_log().info(
            f"Planning to intercept the drone at point {intercept_point[0]}, {intercept_point[1]}, {intercept_point[2]}"
        )

        nco.set_pose(
            Pose(
                {
                    "translation": spawn_point,
                    "rotation": {"w": 0, "x": 0, "y": 0, "z": 0},
                    "frame_id": "DEFAULT_ID",
                }
            ),
            world,
        )

        await asyncio.sleep(0.5)

        drone_velocity = 5

        
        nco.setup_drone_intercept(
            intercept_point, drone, path, [drone_velocity for _ in path], leg_start
        )
        nco.set_trigger_drone_distance(20.0)

        move_on_path_task = await drone.move_on_path_async(
            path=path, velocity=drone_velocity
        )
        await move_on_path_task

        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
