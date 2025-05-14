"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Pytest end-end test script for Camera sensor frames-per-second benchmarking.
"""

import asyncio
import time
import numpy as np
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import unpack_image
from projectairsim.types import ImageType
import warnings
import cv2


class CameraBenchmarker:
    def __init__(self, image_mode: str, tgt_num_images: int):
        self.tgt_num_images = tgt_num_images
        self.image_mode = image_mode

    def initialize(self):
        self.projectairsim_client = ProjectAirSimClient()
        self.projectairsim_client.connect()
        self.projectairsim_world = World(
            self.projectairsim_client, "scene_test_drone_camera.jsonc", 1
        )
        self.projectairsim_drone = Drone(
            self.projectairsim_client, self.projectairsim_world, "Drone1"
        )

        self.num_images = 0
        self.run_time = 0
        self.start_time = time.time()

        self.projectairsim_drone.enable_api_control()
        self.projectairsim_drone.arm()

    def safe_division(self, x, y):
        return 0 if y == 0 else (x / y)

    def process_image(self, image):
        if image is not None and self.num_images < self.tgt_num_images:
            self.num_images += 1
            img_np = unpack_image(image)
            assert len(img_np) > 0

            self.run_time = time.time() - self.start_time
            self.avg_fps = self.safe_division(self.num_images, self.run_time)
            print(
                f"result: {self.avg_fps:.2f} avg_fps for "
                f"{self.num_images} {self.image_mode} images"
            )

    async def run_benchmark_pubsub(self):
        # Subscribe to images
        if self.image_mode == "rgb":
            image_topic = self.projectairsim_drone.sensors["DownCamera"]["scene_camera"]
        elif self.image_mode == "depth":
            image_topic = self.projectairsim_drone.sensors["DownCamera"]["depth_camera"]
        else:
            warnings.warn("Invalid image mode.")
            return

        self.projectairsim_client.subscribe(
            image_topic,
            lambda _, rgb: self.process_image(rgb),
        )

        # Command the Drone to move "Up" in NED coordinate system for a long time
        move_task = await self.projectairsim_drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=300
        )

        while self.num_images < self.tgt_num_images:
            await asyncio.sleep(0.01)

        # Unsubscribe to images
        self.projectairsim_client.unsubscribe(image_topic)

        self.projectairsim_drone.cancel_last_task()
        await move_task  # join awaited move_by_velocity_async Task now that it's cancelled

    async def run_benchmark_reqrep(self):
        # Command the Drone to move "Up" in NED coordinate system for a long time
        move_task = await self.projectairsim_drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=300
        )

        while self.num_images < self.tgt_num_images:
            images = self.projectairsim_drone.get_images("DownCamera", [ImageType.SCENE])
            self.process_image(images[ImageType.SCENE])

        self.projectairsim_drone.cancel_last_task()
        await move_task  # join awaited move_by_velocity_async Task now that it's cancelled

    def cleanup(self):
        self.projectairsim_drone.disarm()
        self.projectairsim_drone.disable_api_control()

        self.projectairsim_client.disconnect()


async def image_benchmark_main(image_mode, transport_mode, tgt_num_images):
    benchmarker = CameraBenchmarker(
        image_mode=image_mode, tgt_num_images=tgt_num_images
    )
    benchmarker.initialize()

    if transport_mode == "pubsub":
        await benchmarker.run_benchmark_pubsub()
    elif transport_mode == "reqrep":
        await benchmarker.run_benchmark_reqrep()
    else:
        warnings.warn("Invalid transport mode.")
        return

    benchmarker.cleanup()
    return benchmarker


def print_benchmark_result(benchmarker, image_mode, transport_mode):
    # Use a custom warning to print out the benchmark results for visibility since
    # pytest captures the console output
    warnings.warn(
        f"\n[INFO] {image_mode} camera {transport_mode} benchmark result"
        f" FPS={benchmarker.avg_fps:.2f},"
        f" run time={benchmarker.run_time:.3f}"
    )


def test_rgb_benchmarker_pubsub():
    image_mode = "rgb"
    transport_mode = "pubsub"
    tgt_num_images = 200

    benchmarker = asyncio.run(
        image_benchmark_main(image_mode, transport_mode, tgt_num_images)
    )

    print_benchmark_result(benchmarker, image_mode, transport_mode)
    # Temporarily reduce required FPS until roboLinux1 performance drop is investigated
    # assert benchmarker.avg_fps >= 20
    assert benchmarker.avg_fps >= 15


def test_depth_benchmarker_pubsub():
    image_mode = "depth"
    transport_mode = "pubsub"
    tgt_num_images = 200

    benchmarker = asyncio.run(
        image_benchmark_main(image_mode, transport_mode, tgt_num_images)
    )

    print_benchmark_result(benchmarker, image_mode, transport_mode)
    # Temporarily reduce required FPS until roboLinux1 performance drop is investigated
    # assert benchmarker.avg_fps >= 20
    assert benchmarker.avg_fps >= 15


def test_rgb_benchmarker_reqrep():
    image_mode = "rgb"
    transport_mode = "reqrep"
    tgt_num_images = 50

    benchmarker = asyncio.run(
        image_benchmark_main(image_mode, transport_mode, tgt_num_images)
    )

    print_benchmark_result(benchmarker, image_mode, transport_mode)
    assert benchmarker.avg_fps >= 1


def test_depth_benchmarker_reqrep():
    image_mode = "depth"
    transport_mode = "reqrep"
    tgt_num_images = 50

    benchmarker = asyncio.run(
        image_benchmark_main(image_mode, transport_mode, tgt_num_images)
    )

    print_benchmark_result(benchmarker, image_mode, transport_mode)
    assert benchmarker.avg_fps >= 1
