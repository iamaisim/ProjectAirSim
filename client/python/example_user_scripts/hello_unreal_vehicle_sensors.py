"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Minimal sensor validation for the Unreal/Chaos vehicle.
"""

import asyncio

from projectairsim import ProjectAirSimClient, World
from projectairsim.unreal_vehicle import UnrealVehicle
from projectairsim.utils import projectairsim_log


sample_counts = {}


def set_controls(vehicle: UnrealVehicle, throttle=0.0, brake=0.0, steering=0.0):
    vehicle.set_parameter("throttle", throttle)
    vehicle.set_parameter("brake", brake)
    vehicle.set_parameter("steering", steering)


def summarize(sample):
    if not isinstance(sample, dict):
        return sample

    summary = {}
    for key, value in sample.items():
        if key in ("data", "point_cloud"):
            summary[key] = f"<{len(value)} values>"
        else:
            summary[key] = value
    return summary


def make_logger(sensor_id, stream_name, every=20):
    key = f"{sensor_id}.{stream_name}"
    sample_counts[key] = 0

    def log_sample(_, sample):
        sample_counts[key] += 1
        if sample_counts[key] % every == 0:
            projectairsim_log().info(f"{key}: {summarize(sample)}")

    return log_sample


def subscribe_sensors(client: ProjectAirSimClient, vehicle: UnrealVehicle):
    for sensor_id, streams in vehicle.sensors.items():
        for stream_name, topic in streams.items():
            if stream_name.endswith("_info"):
                continue

            every = 60 if stream_name.endswith("_camera") else 20
            client.subscribe(topic, make_logger(sensor_id, stream_name, every))
            projectairsim_log().info(f"Subscribed to {sensor_id}.{stream_name}")


async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        world = World(client, "scene_unreal_vehicle.jsonc", delay_after_load_sec=2)
        vehicle = UnrealVehicle(client, world, "UnrealVehicle")
        subscribe_sensors(client, vehicle)

        set_controls(vehicle, throttle=0.5, steering=0.4)
        await asyncio.sleep(6.0)

        set_controls(vehicle, throttle=0.0, brake=1.0)
        await asyncio.sleep(2.0)

        set_controls(vehicle)
        projectairsim_log().info("Sensor validation complete")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
