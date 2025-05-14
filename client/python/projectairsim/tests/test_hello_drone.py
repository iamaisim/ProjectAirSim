"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

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
    """Basic data validation for images"""
    img_nparr = np.frombuffer(img_msg["data"], dtype="uint8")
    assert np.sum(img_nparr) > 0


def check_imu(imu_msg):
    """Basic data validation for IMU"""
    assert len(imu_msg) > 0
    orientation = imu_msg["orientation"]
    lin_accel = imu_msg["linear_acceleration"]
    ang_vel = imu_msg["angular_velocity"]
    assert orientation["w"] >= -1.0 and orientation["w"] <= 1.0
    assert orientation["x"] >= -1.0 and orientation["x"] <= 1.0
    assert orientation["y"] >= -1.0 and orientation["y"] <= 1.0
    assert orientation["z"] >= -1.0 and orientation["x"] <= 1.0
    assert lin_accel["x"] >= -20.0 and lin_accel["x"] <= 20.0
    assert lin_accel["y"] >= -20.0 and lin_accel["y"] <= 20.0
    assert lin_accel["z"] >= -20.0 and lin_accel["z"] <= 20.0
    assert ang_vel["x"] >= -5.0 and ang_vel["x"] <= 5.0
    assert ang_vel["y"] >= -5.0 and ang_vel["y"] <= 5.0
    assert ang_vel["z"] >= -5.0 and ang_vel["z"] <= 5.0


@pytest.fixture(scope="class")
def robo():
    class ProjectAirSimTestObject:
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")
        robot_actual_pose = None

        def robot_actual_pose_callback(self, topic, message):
            # print("Got new pose:", message)
            self.robot_actual_pose = message

    robo_obj = ProjectAirSimTestObject()
    yield robo_obj

    print("\nTeardown client...")
    robo_obj.client.disconnect()


class TestClientBase:
    async def main(self, robo):
        # Set up client for pytest
        print("start")
        drone = robo.drone
        client = robo.client

        client.subscribe(
            drone.robot_info["actual_pose"], robo.robot_actual_pose_callback
        )
        timeout_sec = 5
        timeout = time.time() + timeout_sec
        print("\n  Waiting for robot_actual_pose update...")
        robo.robot_actual_pose = None
        while robo.robot_actual_pose is None:
            if time.time() > timeout:
                pytest.fail("Timeout waiting for a pose message update")
            time.sleep(0.1)

        # Test hello_drone.py pattern below

        # Susbscribe to Drone's sensors with a callback to receive the data
        # RGB Camera
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"], lambda _, rgb: check_image(rgb)
        )
        # Depth Camera
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: check_image(depth),
        )
        # IMU
        client.subscribe(
            drone.sensors["IMU1"]["imu_kinematics"], lambda _, imu: check_imu(imu)
        )

        drone.enable_api_control()
        drone.arm()

        prev_pose = robo.robot_actual_pose

        # Command the Drone to move "Up" in NED coordinate system for 2 seconds
        move_up = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-2.0, duration=2.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up
        projectairsim_log().info("Move-Up completed")
        assert robo.robot_actual_pose["position"]["z"] < prev_pose["position"]["z"]

        prev_pose = robo.robot_actual_pose
        # Command the Drone to move "North" in NED coordinate system for 2 seconds
        move_north = await drone.move_by_velocity_async(
            v_north=2.0, v_east=0.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-North invoked")
        await move_north
        projectairsim_log().info("Move-North completed")
        assert robo.robot_actual_pose["position"]["x"] > prev_pose["position"]["x"]

        prev_pose = robo.robot_actual_pose
        # Command the Drone to move "West" in NED coordinate system for 2 seconds
        move_west = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-2.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-West invoked")
        await move_west
        projectairsim_log().info("Move-West completed")
        assert robo.robot_actual_pose["position"]["y"] < prev_pose["position"]["y"]

        prev_pose = robo.robot_actual_pose
        # Command the Drone to move "South" in NED coordinate system for 2 seconds
        move_south = await drone.move_by_velocity_async(
            v_north=-2.0, v_east=0.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-South invoked")
        await move_south
        projectairsim_log().info("Move-South completed")
        assert robo.robot_actual_pose["position"]["x"] < prev_pose["position"]["x"]

        prev_pose = robo.robot_actual_pose
        # Command the Drone to move "East" in NED coordinate system for 4 seconds
        move_east = await drone.move_by_velocity_async(
            v_north=0.0, v_east=2.0, v_down=0.0, duration=2.0
        )
        projectairsim_log().info("Move-East invoked")
        await move_east
        projectairsim_log().info("Move-East completed")
        assert robo.robot_actual_pose["position"]["y"] > prev_pose["position"]["y"]

        prev_pose = robo.robot_actual_pose
        # Command the Drone to move "Down" in NED coordinate system for 4 seconds
        move_down = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=2.0, duration=4.0
        )
        projectairsim_log().info("Move-Down invoked")
        await move_down
        projectairsim_log().info("Move-Down completed")
        assert robo.robot_actual_pose["position"]["z"] > prev_pose["position"]["z"]

        drone.disarm()
        drone.disable_api_control()

        client.disconnect()

    def test_hello_drone(self, robo):
        asyncio.run(self.main(robo))
