"""
Copyright (C) 2026 IAMAI CONSULTING CORP
MIT License.

Load the JSBSim Rascal 110 + PX4 SITL scene and hold the simulator connection.

PX4 SITL must already be running before or shortly after this script starts.
The matching PX4 launcher is run_px4_rascal_takeoff.py in this directory.
"""

import argparse
import asyncio
from pathlib import Path

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log


DEFAULT_SCENE = (
    Path(__file__).resolve().parent
    / "sim_config"
    / "scene_rascal110_px4_sitl.jsonc"
)


async def main(scene: str) -> None:
    scene_path = Path(scene).resolve()
    client = ProjectAirSimClient()
    try:
        client.connect()
        world = World(
            client,
            scene_path.name,
            delay_after_load_sec=2,
            sim_config_path=str(scene_path.parent),
        )
        Drone(client, world, "rascal110_px4")
        projectairsim_log().info(
            "Loaded scene '%s'; holding connection for PX4 SITL.",
            scene_path.name,
        )
        while True:
            await asyncio.sleep(1)
    finally:
        client.disconnect()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Load the JSBSim Rascal 110 + PX4 SITL scene."
    )
    parser.add_argument("--scene", default=str(DEFAULT_SCENE))
    args = parser.parse_args()
    try:
        asyncio.run(main(args.scene))
    except KeyboardInterrupt:
        pass
