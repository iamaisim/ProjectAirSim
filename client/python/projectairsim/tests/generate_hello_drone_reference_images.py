"""
Capture reference images for test_hello_drone.py correlation checks.

Usage:
    python generate_hello_drone_reference_images.py --output-dir ./refs

Outputs:
- hello_drone_<step>_<camera>_ref.png: Camera RGB scene reference

Where <step> is one of: initial, move_up, move_north, move_west,
move_south, move_east, move_down, and <camera> is one of: down_camera, front_camera.
"""

from __future__ import annotations

import asyncio
import argparse
from pathlib import Path
from datetime import datetime

import cv2
import numpy as np

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.types import ImageType


import math

STEP_SEQUENCE = [
    ("initial", None),
    ("move_up", {"v_north": 0.0, "v_east": 0.0, "v_down": -2.0, "duration": 5.0}),
    ("move_north1", {"v_north": 2.0, "v_east": 0.0, "v_down": 0.0, "duration": 5.0}),
    ("rotate_z_60", {"rotate_z_rate": math.radians(30.0), "seconds": 2.0}),
    ("move_north2", {"v_north": 2.0, "v_east": 0.0, "v_down": 0.0, "duration": 5.0}),
    ("rotate_z_45a", {"rotate_z_rate": math.radians(22.5), "seconds": 2.0}),
    ("move_west", {"v_north": 0.0, "v_east": -2.0, "v_down": 0.0, "duration": 5.0}),
    ("rotate_z_45b", {"rotate_z_rate": math.radians(22.5), "seconds": 2.0}),
]


def _decode_scene_bgr(scene_msg: dict) -> np.ndarray:
    width = int(scene_msg["width"])
    height = int(scene_msg["height"])
    raw = scene_msg["data"]
    if isinstance(raw, list):
        buf = np.array(raw, dtype=np.uint8)
    else:
        buf = np.frombuffer(raw, dtype=np.uint8)

    expected = width * height * 3
    if buf.size < expected:
        raise ValueError(f"scene buffer too small: {buf.size} < {expected}")
    return buf[:expected].reshape((height, width, 3))


async def _get_images_with_retry(drone: Drone, camera_id: str, retries: int = 6) -> dict:
    last_error: Exception | None = None
    for attempt in range(1, retries + 1):
        try:
            return drone.get_images(camera_id, [ImageType.SCENE])
        except RuntimeError as error:
            last_error = error
            if "Timeout waiting for new captured_images" not in str(error):
                raise
            if attempt < retries:
                await asyncio.sleep(0.35)
    raise RuntimeError(f"failed to capture from {camera_id} after {retries} attempts") from last_error


async def _capture_step_references(
    world: World,
    drone: Drone,
    output_dir: Path,
    step: str,
) -> None:
    await asyncio.sleep(5.0)

    down_images = await _get_images_with_retry(drone, "DownCamera")
    front_images = await _get_images_with_retry(drone, "FrontCamera")

    # Process DownCamera images
    down_scene_png = output_dir / f"hello_drone_{step}_down_camera_ref.png"

    down_scene_bgr = _decode_scene_bgr(down_images[ImageType.SCENE])

    if not cv2.imwrite(str(down_scene_png), down_scene_bgr):
        raise RuntimeError(f"failed to write scene reference: {down_scene_png}")

    print(f"saved {step}:")
    print(f"  down_camera scene: {down_scene_png}")

    front_scene_png = output_dir / f"hello_drone_{step}_front_camera_ref.png"

    front_scene_bgr = _decode_scene_bgr(front_images[ImageType.SCENE])

    if not cv2.imwrite(str(front_scene_png), front_scene_bgr):
        raise RuntimeError(f"failed to write scene reference: {front_scene_png}")

    print(f"  front_camera scene: {front_scene_png}")


async def _run(args: argparse.Namespace) -> None:
    base_output_dir = Path(args.output_dir).resolve()
    base_output_dir.mkdir(parents=True, exist_ok=True)
    run_id = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = base_output_dir / f"hello_drone_refs_{run_id}"
    output_dir.mkdir(parents=True, exist_ok=False)

    client = ProjectAirSimClient()
    client.connect()

    try:
        world = World(client, args.scene_name, 1)
        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        drone.arm()

        for step, cmd in STEP_SEQUENCE:
            if cmd is not None:
                if "rotate_z_rate" in cmd and "seconds" in cmd:
                    await drone.rotate_by_yaw_rate_async(cmd["rotate_z_rate"], cmd["seconds"])
                else:
                    move = await drone.move_by_velocity_async(**cmd)
                    await move
            await _capture_step_references(world, drone, output_dir, step)

        drone.disarm()
        drone.disable_api_control()

        print("\nUse these env vars when running the test:")
        print(f"  PROJECTAIRSIM_TEST_HELLO_DRONE_REF_IMAGE={output_dir}")
        print(f"  Generated files folder: {output_dir}")
    finally:
        client.disconnect()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        default=".",
        help="Directory to save generated reference files",
    )
    parser.add_argument(
        "--scene-name",
        default="scene_test_drone_hello_drone_refs.jsonc",
        help="Scene config used by test_hello_drone.py",
    )
    args = parser.parse_args()
    asyncio.run(_run(args))


if __name__ == "__main__":
    main()
