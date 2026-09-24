"""Drive a native Unreal AWheeledVehiclePawn."""

import asyncio
from pathlib import Path

from projectairsim import ProjectAirSimClient, WheeledVehicle, World


async def main():
    client = ProjectAirSimClient()
    config_dir = Path(__file__).parent / "sim_config"
    vehicle = None
    controls_started = False
    try:
        client.connect()
        world = World(
            client,
            "scene_wheeled_vehicle.jsonc",
            delay_after_load_sec=2,
            sim_config_path=str(config_dir),
        )
        vehicle = WheeledVehicle(client, world, "WheeledVehicle")

        vehicle.set_throttle(0.7)
        controls_started = True
        await asyncio.sleep(3.0)
        vehicle.set_steering(0.6)
        await asyncio.sleep(2.0)
        vehicle.set_throttle(0.0)
        vehicle.set_steering(0.0)
        vehicle.set_brakes(1.0)
        await asyncio.sleep(1.0)
    finally:
        # Do not mask the original startup/RPC error with a second failed stop
        # request when the first control command never reached the simulator.
        if vehicle is not None and controls_started:
            try:
                vehicle.set_throttle(0.0)
                vehicle.set_steering(0.0)
                vehicle.set_brakes(1.0)
            except RuntimeError as stop_error:
                print(f"Warning: could not send final stop controls: {stop_error}")
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
