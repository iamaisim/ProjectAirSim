"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
Pytest end-end test script for drone movement and matching pose data
"""

import asyncio
import pytest
import time
import math
import numpy as np

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import quaternion_to_rpy


@pytest.fixture(scope="class")
def robo():
    class ProjectAirSimTestObject:
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")
        actual_pose_data = None
        rgb_image_data = None
        imu_data = None

        def actual_pose_data_callback(self, topic, pose_msg):
            self.actual_pose_data = pose_msg  # keep a copy of the latest data

            """Basic data validation for actual pose"""
            assert (
                pose_msg["orientation"]["w"] >= -1.0
                and pose_msg["orientation"]["w"] <= 1.0
            )
            assert (
                pose_msg["orientation"]["x"] >= -1.0
                and pose_msg["orientation"]["x"] <= 1.0
            )
            assert (
                pose_msg["orientation"]["y"] >= -1.0
                and pose_msg["orientation"]["y"] <= 1.0
            )
            assert (
                pose_msg["orientation"]["z"] >= -1.0
                and pose_msg["orientation"]["x"] <= 1.0
            )

        def image_callback(self, topic, image_msg):
            self.rgb_image_data = image_msg  # keep a copy of the latest data
            """Basic data validation for images"""
            image_nparr = np.frombuffer(image_msg["data"], dtype="uint8")
            assert np.sum(image_nparr) > 0

        def imu_callback(self, topic, imu_msg):
            self.imu_data = imu_msg  # keep a copy of the latest data
            """Basic data validation for IMU"""
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

        def check_pose_matches(self, is_landed: bool = False):
            actual_pose = self.actual_pose_data

            # TODO For landing, improve collision response to deterministically
            # stabilize the sim pose and UnrealRobot pose with minimal gap to be able
            # to apply a tighter pose_tol tolerance check for landed states.
            # pose_tol = 1e-6 if is_landed else 1e-3
            pose_tol = 1e-3 if is_landed else 1e-3

            # Check pose vs Unreal actor's pose
            actor_pose = self.world.get_object_pose("Drone1")
            assert actor_pose["translation"] == pytest.approx(
                actual_pose["position"], rel=pose_tol, abs=pose_tol
            )
            assert actor_pose["rotation"] == pytest.approx(
                actual_pose["orientation"], rel=pose_tol, abs=pose_tol
            )

            # Check position vs camera image's position
            assert self.rgb_image_data["pos_x"] == pytest.approx(
                actual_pose["position"]["x"], rel=1e-3, abs=1e-3
            )
            assert self.rgb_image_data["pos_y"] == pytest.approx(
                actual_pose["position"]["y"], rel=1e-3, abs=1e-3
            )
            assert self.rgb_image_data["pos_z"] == pytest.approx(
                actual_pose["position"]["z"], rel=1e-3, abs=1e-3
            )

            # Check orientation vs IMU sensor's pose
            actual_rpy = quaternion_to_rpy(
                actual_pose["orientation"]["w"],
                actual_pose["orientation"]["x"],
                actual_pose["orientation"]["y"],
                actual_pose["orientation"]["z"],
            )
            imu_rpy = quaternion_to_rpy(
                self.imu_data["orientation"]["w"],
                self.imu_data["orientation"]["x"],
                self.imu_data["orientation"]["y"],
                self.imu_data["orientation"]["z"],
            )
            assert actual_rpy == pytest.approx(imu_rpy, rel=1e-3, abs=1e-3)

    robo_obj = ProjectAirSimTestObject()
    yield robo_obj

    print("\nTeardown client...")
    robo_obj.client.disconnect()


class TestClientBase:
    async def main(self, robo):
        # Set up client for pytest
        print("Starting test...")
        client = robo.client
        drone = robo.drone

        # Subscribe to sensor data topics
        client.subscribe(
            drone.robot_info["actual_pose"], robo.actual_pose_data_callback
        )
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"], robo.image_callback
        )
        client.subscribe(drone.sensors["IMU1"]["imu_kinematics"], robo.imu_callback)

        # Wait for first pose message update
        timeout_sec = 5
        timeout = time.time() + timeout_sec
        print("Waiting for actual_pose_data and image update...")
        robo.actual_pose_data = None
        robo.rgb_image_data = None
        while robo.actual_pose_data is None or robo.rgb_image_data is None:
            if time.time() > timeout:
                pytest.fail("Timeout waiting for a pose and image message update")
            await asyncio.sleep(0.1)

        # Check pose matching for robot and sensor data before flying
        robo.check_pose_matches(is_landed=True)

        # Prepare drone to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------
        # Command the Drone to move up in NED coordinate system
        prev_pose = robo.actual_pose_data
        move_up = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-2.0, duration=1.0
        )
        print("Move-Up invoked")
        await move_up
        await asyncio.sleep(2.0)  # wait for hover position to settle
        print("Move-Up completed")

        # Check that robot moved to a higher relative position
        assert robo.actual_pose_data["position"]["z"] < prev_pose["position"]["z"]

        # Check pose matching for robot and sensor data at new position
        robo.check_pose_matches(is_landed=False)

        # ------------------------------------------------------------------------------
        # Command the Drone to move North-West in NED coordinate system
        prev_pose = robo.actual_pose_data
        move_north_west = await drone.move_by_velocity_async(
            v_north=2.0, v_east=-2.0, v_down=0.0, duration=6.0
        )
        print("Move-North-West invoked")
        await move_north_west
        await asyncio.sleep(10.0)  # wait for hover position to settle
        print("Move-North-West completed")

        # Check that robot moved North-East
        assert robo.actual_pose_data["position"]["x"] > prev_pose["position"]["x"]
        assert robo.actual_pose_data["position"]["y"] < prev_pose["position"]["y"]

        # Check pose matching for robot and sensor data at new position
        robo.check_pose_matches(is_landed=False)

        # ------------------------------------------------------------------------------
        # Command the Drone to rotate yaw clock-wise
        prev_pose = robo.actual_pose_data
        move_yaw = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=0.0, duration=1.0, yaw=math.radians(45.0)
        )
        print("Move-Yaw invoked")
        await move_yaw
        await asyncio.sleep(3.0)  # wait for hover position to settle
        print("Move-Yaw completed")

        # Check that robot rotated clock-wise
        prev_rpy = quaternion_to_rpy(
            prev_pose["orientation"]["w"],
            prev_pose["orientation"]["x"],
            prev_pose["orientation"]["y"],
            prev_pose["orientation"]["z"],
        )
        cur_rpy = quaternion_to_rpy(
            robo.actual_pose_data["orientation"]["w"],
            robo.actual_pose_data["orientation"]["x"],
            robo.actual_pose_data["orientation"]["y"],
            robo.actual_pose_data["orientation"]["z"],
        )
        assert cur_rpy[2] > prev_rpy[2]

        # Check pose matching for robot and sensor data at new position
        robo.check_pose_matches(is_landed=False)

        # ------------------------------------------------------------------------------
        # Command the Drone to move down in NED coordinate system
        prev_pose = robo.actual_pose_data
        move_down = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=2.0, duration=4.0
        )
        print("Move-Down invoked")
        await move_down
        print("Move-Down completed")

        # Check that robot moved to a lower relative position
        assert robo.actual_pose_data["position"]["z"] > prev_pose["position"]["z"]

        # Check pose matching for robot and sensor data at new position
        robo.check_pose_matches(is_landed=True)

        # ------------------------------------------------------------------------------
        drone.disarm()
        drone.disable_api_control()

        client.disconnect()

    def test_drone_pose(self, robo):
        asyncio.run(self.main(robo))
