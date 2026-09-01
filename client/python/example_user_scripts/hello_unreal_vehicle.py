"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Project AirSim Unreal vehicle motion and sensor example.

Loads scene_unreal_vehicle.jsonc, sends throttle/brake/steering actuator
commands, and prints ground-truth kinematics to confirm that the Unreal/Chaos
vehicle is moving, synchronizing state back into Project AirSim, and publishing
its configured sensor streams.
"""

import asyncio
from pathlib import Path

from projectairsim import ProjectAirSimClient, World
from projectairsim.unreal_vehicle import UnrealVehicle
from projectairsim.utils import projectairsim_log


sample_counts = {}


def set_throttle(vehicle: UnrealVehicle, value: float):
    vehicle.set_parameter(0, value)


def set_brake(vehicle: UnrealVehicle, value: float):
    vehicle.set_parameter(1, value)


def set_steering(vehicle: UnrealVehicle, value: float):
    vehicle.set_parameter(2, value)


def summarize_sensor_sample(sample):
    if not isinstance(sample, dict):
        return sample

    summary = {}
    for key, value in sample.items():
        if key in ("data", "point_cloud"):
            summary[key] = f"<{len(value)} values>"
        else:
            summary[key] = value
    return summary


def make_sensor_logger(sensor_id: str, stream_name: str, every: int = 20):
    key = f"{sensor_id}.{stream_name}"
    sample_counts[key] = 0

    def log_sample(_, sample):
        sample_counts[key] += 1
        if sample_counts[key] % every == 0:
            projectairsim_log().info(f"{key}: {summarize_sensor_sample(sample)}")

    return log_sample


def subscribe_sensors(client: ProjectAirSimClient, vehicle: UnrealVehicle):
    for sensor_id, streams in vehicle.sensors.items():
        for stream_name, topic in streams.items():
            if stream_name.endswith("_info"):
                continue

            every = 60 if stream_name.endswith("_camera") else 20
            client.subscribe(topic, make_sensor_logger(sensor_id, stream_name, every))
            projectairsim_log().info(f"Subscribed to {sensor_id}.{stream_name}")


def set_controls(
    vehicle: UnrealVehicle,
    throttle: float = 0.0,
    brake: float = 0.0,
    steering: float = 0.0,
):
    set_throttle(vehicle, throttle)
    set_brake(vehicle, brake)
    set_steering(vehicle, steering)


def stop_vehicle(vehicle: UnrealVehicle):
    set_controls(vehicle, throttle=0.0, brake=0.0, steering=0.0)


def get_vector(data: dict, *path: str) -> dict:
    value = data
    for key in path:
        if not isinstance(value, dict):
            return {}
        value = value.get(key, {})
    return value if isinstance(value, dict) else {}


async def log_kinematics(vehicle: UnrealVehicle, label: str):
    kin = vehicle.get_kinematics()
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


async def run_motion_test(vehicle: UnrealVehicle):
    await log_kinematics(vehicle, "Initial")

    projectairsim_log().info("Forward")
    set_controls(vehicle, throttle=0.7)
    await asyncio.sleep(3.0)
    await log_kinematics(vehicle, "After forward")

    projectairsim_log().info("Right curve")
    set_controls(vehicle, throttle=0.35, steering=0.75)
    await asyncio.sleep(3.0)
    await log_kinematics(vehicle, "After right curve")

    projectairsim_log().info("Left curve")
    set_controls(vehicle, throttle=0.35, steering=-0.75)
    await asyncio.sleep(3.0)
    await log_kinematics(vehicle, "After left curve")

    projectairsim_log().info("Straighten")
    set_controls(vehicle, throttle=0.5)
    await asyncio.sleep(2.0)
    await log_kinematics(vehicle, "After straighten")

    projectairsim_log().info("Brake")
    set_controls(vehicle, brake=1.0)
    await asyncio.sleep(2.0)
    await log_kinematics(vehicle, "After brake")

    stop_vehicle(vehicle)


async def main():
    client = ProjectAirSimClient()
    sim_config_path = str(Path(__file__).resolve().parent / "sim_config")

    try:
        client.connect()
        projectairsim_log().info("Connected to ProjectAirSim")

        world = World(
            client,
            "scene_unreal_vehicle.jsonc",
            delay_after_load_sec=2,
            sim_config_path=sim_config_path,
        )
        if world.switch_streaming_view():
            projectairsim_log().info("Switched simulator viewport to vehicle camera")

        vehicle = UnrealVehicle(client, world, "UnrealVehicle")
        subscribe_sensors(client, vehicle)

        await run_motion_test(vehicle)
        projectairsim_log().info("Done")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
