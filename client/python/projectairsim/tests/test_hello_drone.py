"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Pytest end-end test script for hello_drone.py functionality
"""

import asyncio
import pytest
import time
import numpy as np

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log


def check_image(img_msg):
    img_nparr = np.frombuffer(img_msg["data"], dtype="uint8")
    if img_nparr.size == 0:
        return
    if np.sum(img_nparr) == 0:
        return


def check_imu(imu_msg):
    assert len(imu_msg) > 0
    orientation = imu_msg["orientation"]
    lin_accel = imu_msg["linear_acceleration"]
    ang_vel = imu_msg["angular_velocity"]
    assert -1.0 <= orientation["w"] <= 1.0
    assert -1.0 <= orientation["x"] <= 1.0
    assert -1.0 <= orientation["y"] <= 1.0
    assert -1.0 <= orientation["z"] <= 1.0
    assert -20.0 <= lin_accel["x"] <= 20.0
    assert -20.0 <= lin_accel["y"] <= 20.0
    assert -20.0 <= lin_accel["z"] <= 20.0
    assert -5.0 <= ang_vel["x"] <= 5.0
    assert -5.0 <= ang_vel["y"] <= 5.0
    assert -5.0 <= ang_vel["z"] <= 5.0


async def wait_for_pose_change(multirotor, prev_pose, timeout=2.0):
    start = time.time()
    while True:
        pose = multirotor.robot_actual_pose
        if pose is not None and pose != prev_pose:
            return pose
        if time.time() - start > timeout:
            pytest.fail("Timeout waiting for pose update")
        await asyncio.sleep(0.05)


@pytest.fixture(scope="class")
def multirotor():
    class ProjectAirSimTestObject:
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")
        robot_actual_pose = None

        def robot_actual_pose_callback(self, topic, message):
            self.robot_actual_pose = message

    multirotor_obj = ProjectAirSimTestObject()
    yield multirotor_obj

    print("\nTeardown client...")
    multirotor_obj.client.disconnect()


class TestClientBase:
    async def main(self, multirotor):
        print("start")
        drone = multirotor.drone
        client = multirotor.client

        client.subscribe(
            drone.robot_info["actual_pose"], multirotor.robot_actual_pose_callback
        )

        timeout = time.time() + 5
        multirotor.robot_actual_pose = None
        while multirotor.robot_actual_pose is None:
            if time.time() > timeout:
                pytest.fail("Timeout waiting for a pose message update")
            await asyncio.sleep(0.1)

        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: check_image(rgb),
        )
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: check_image(depth),
        )
        client.subscribe(
            drone.sensors["IMU1"]["imu_kinematics"],
            lambda _, imu: check_imu(imu),
        )

        drone.enable_api_control()
        drone.arm()

        prev_pose = multirotor.robot_actual_pose
        move_up = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-2.0, duration=2.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up
        projectairsim_log().info("Move-Up completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["z"] < prev_pose["position"]["z"]

        prev_pose = new_pose
        move_north = await drone.move_by_velocity_async(
            v_north=2.0, v_east=0.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-North invoked")
        await move_north
        projectairsim_log().info("Move-North completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["x"] > prev_pose["position"]["x"]

        prev_pose = new_pose
        move_west = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-2.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-West invoked")
        await move_west
        projectairsim_log().info("Move-West completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["y"] < prev_pose["position"]["y"]

        prev_pose = new_pose
        move_south = await drone.move_by_velocity_async(
            v_north=-2.0, v_east=0.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-South invoked")
        await move_south
        projectairsim_log().info("Move-South completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["x"] < prev_pose["position"]["x"]

        prev_pose = new_pose
        move_east = await drone.move_by_velocity_async(
            v_north=0.0, v_east=2.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-East invoked")
        await move_east
        projectairsim_log().info("Move-East completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["y"] > prev_pose["position"]["y"]

        prev_pose = new_pose
        move_down = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=2.0, duration=4.0
        )
        projectairsim_log().info("Move-Down invoked")
        await move_down
        projectairsim_log().info("Move-Down completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["z"] > prev_pose["position"]["z"]

        drone.disarm()
        drone.disable_api_control()

        client.disconnect()

    def test_hello_drone(self, multirotor):
        asyncio.run(self.main(multirotor))