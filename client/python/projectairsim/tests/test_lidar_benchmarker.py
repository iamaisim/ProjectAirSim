"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
Pytest end-end test script for lidar sensor.
"""

import asyncio
import time
import numpy as np
from projectairsim import ProjectAirSimClient, Drone, World
import threading
import pytest
import warnings


class LidarTester:
    def __init__(self, mode: str = "standard", duration: float = 10):
        self.duration = duration
        self.mode = mode

        self.num_points = 0
        self.is_start_time_initialized = False
        self.start_time = 0
        self.start_simtime = 0
        self.last_realtime = 0
        self.last_simtime = 0
        self.real_time_elapsed = 0
        self.sim_time_elapsed = 0
        self.is_timer_thread_active = True
        self.timer_thread = threading.Thread(
            target=self.repeat_timer_callback, args=(self.timer_callback, 0.1)
        )

    async def start(self):
        self.projectairsim_client = ProjectAirSimClient()
        self.projectairsim_client.connect()
        self.projectairsim_world = World(
            self.projectairsim_client, "scene_test_drone_lidar.jsonc", 1
        )
        self.projectairsim_drone = Drone(
            self.projectairsim_client, self.projectairsim_world, "Drone1"
        )

        if self.mode == "standard":
            self.projectairsim_client.subscribe(
                self.projectairsim_drone.sensors["lidar1"]["lidar"],
                lambda _, lidar_msg: self.lidar_callback(lidar_msg),
            )

        self.projectairsim_drone.enable_api_control()
        self.projectairsim_drone.arm()

        self.timer_thread.start()
        await self.move_drone()

    def safe_division(self, x, y):
        if y == 0:
            return 0
        return x / y

    def lidar_callback(self, lidar_data):
        if lidar_data is not None:
            if not self.is_start_time_initialized:
                self.start_time = time.time()
                self.start_simtime = lidar_data["time_stamp"]
                self.is_start_time_initialized = True
            else:
                points = np.array(lidar_data["point_cloud"], dtype=np.dtype("f4"))
                points = np.reshape(points, (int(points.shape[0] / 3), 3))
                self.num_points += points.shape[0]
                self.avg_fps = self.safe_division(
                    self.num_points, time.time() - self.start_time
                )
                # print(
                #     f"Received {points.shape[0]} points after"
                #     f" {time.time() - self.start_time} seconds"
                # )
            self.last_realtime = time.time()
            self.last_simtime = lidar_data["time_stamp"]

    # move the drone in the environment for realistic benchmarking
    async def move_drone(self):
        # Command the Drone to move "Up" in NED coordinate system for 4 seconds
        move_up = await self.projectairsim_drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        await move_up

        # Command the Drone to move "Down" in NED coordinate system for 4 seconds
        move_down = await self.projectairsim_drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
        )
        await move_down

    def timer_callback(self):
        if (time.time() - self.start_time) > self.duration:
            self.is_timer_thread_active = False

    # call task() method every "period" seconds.
    def repeat_timer_callback(self, task, period):
        while self.is_timer_thread_active:
            task()
            time.sleep(period)

    def end(self):
        self.real_time_elapsed = self.last_realtime - self.start_time
        self.sim_time_elapsed = (self.last_simtime - self.start_simtime) / 1.0e9

        self.projectairsim_drone.disarm()
        self.projectairsim_drone.disable_api_control()

        self.projectairsim_client.disconnect()
        self.timer_thread.join()


async def lidar_test_main():
    lidar_tester = LidarTester(mode="standard", duration=10)
    await lidar_tester.start()
    while lidar_tester.is_timer_thread_active:
        await asyncio.sleep(0.1)
    lidar_tester.end()
    print_benchmark_result(lidar_tester)
    return lidar_tester


def print_benchmark_result(benchmarker):
    # Use a custom warning to print out the benchmark results for visibility since
    # pytest captures the console output
    time_ratio = benchmarker.sim_time_elapsed / benchmarker.real_time_elapsed
    warnings.warn(
        f"\n[INFO] Lidar benchmark real time elapsed {benchmarker.real_time_elapsed}"
        f" sec, sim time elapsed {benchmarker.sim_time_elapsed} sec,"
        f" sim/real time ratio = {time_ratio:.3f}"
    )
    warnings.warn(
        f"\n[INFO] Lidar benchmark result ="
        f" {round(benchmarker.avg_fps)} avg points/second"
        f" for {benchmarker.num_points} points"
    )


@pytest.fixture()
def lidar_test():
    return asyncio.run(lidar_test_main())


class TestClientBase:
    def test_lidar_benchmarker(self, lidar_test):
        assert lidar_test.avg_fps > 10000
