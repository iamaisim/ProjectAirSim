"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Project AirSim Unreal vehicle example driven by SimpleDrive.

Loads scene_unreal_vehicle_simpledrive.jsonc (unreal-physics vehicle +
simple-drive-api controller), arms the vehicle, and commands a 3-point
MoveOnPath trajectory.
"""

import asyncio

from projectairsim import ProjectAirSimClient, Rover, World
from projectairsim.utils import projectairsim_log


def get_vector(data: dict, *path: str) -> dict:
    value = data
    for key in path:
        if not isinstance(value, dict):
            return {}
        value = value.get(key, {})
    return value if isinstance(value, dict) else {}


async def log_kinematics(rover: Rover, label: str):
    kin = rover.get_ground_truth_kinematics()
    if not isinstance(kin, dict):
        projectairsim_log().info(f"{label}: {kin}")
        return

    pos = get_vector(kin, "pose", "position") or get_vector(kin, "position")
    vel = get_vector(kin, "twist", "linear") or get_vector(kin, "linear_velocity")

    projectairsim_log().info(
        f"{label}: "
        f"pos=({pos.get('x', 0):.2f}, {pos.get('y', 0):.2f}, {pos.get('z', 0):.2f}) "
        f"vel=({vel.get('x', 0):.2f}, {vel.get('y', 0):.2f}, {vel.get('z', 0):.2f})"
    )


async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        projectairsim_log().info("Connected to Project AirSim")

        world = World(
            client, "scene_unreal_vehicle_simpledrive.jsonc", delay_after_load_sec=2
        )
        # Rover client exposes SimpleDrive APIs; physics remains unreal-vehicle.
        rover = Rover(client, world, "UnrealVehicle")

        projectairsim_log().info("Enabling API control")
        assert rover.enable_api_control()

        projectairsim_log().info("Arming vehicle")
        assert rover.arm()

        await log_kinematics(rover, "Initial")

        # 3-point NED path near spawn origin (-500, 0, -4)
        path = [[-480.0, 0.0], [-480.0, 20.0], [-460.0, 20.0]]
        projectairsim_log().info(f"MoveOnPath with {len(path)} waypoints: {path}")
        move_task = await rover.move_on_path_async(path=path, velocity=4.0, timeout_sec=60.0)
        await move_task

        await log_kinematics(rover, "After path")

        projectairsim_log().info("Braking")
        brake_task = await rover.set_rover_controls(
            engine=0.0, steering_angle=0.0, brake=1.0
        )
        await brake_task
        await rover.wait_until_stopped_async(timeout_sec=10.0)

        await log_kinematics(rover, "Final")

        rover.disarm()
        rover.disable_api_control()
        projectairsim_log().info("Done")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
