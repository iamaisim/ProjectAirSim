"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
Pytest end-end test script for PX4 SITL integration
"""

import asyncio
import pytest
import time

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log


@pytest.fixture(scope="class")
def robo_fixture():
    """Pytest fixture to setup/tear down client connection and API objects"""

    class TestObject:
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_test_px4_sitl.jsonc", 1)
        time.sleep(10)  # Temporary work-around for bug 22323
        drone = PX4Drone(client, world, "Drone1")

    robo_obj = TestObject()
    yield robo_obj

    print("\nTeardown client...")
    robo_obj.client.disconnect()


class PX4Drone(Drone):
    """Custom data and callbacks to store on the drone object"""

    actual_pose = None

    def actual_pose_callback(self, _, message) -> None:
        self.actual_pose = message


class TestPX4:
    def test_px4_hello_drone(self, robo_fixture):
        """Pytest test function entry point"""
        asyncio.run(
            self.fly_px4_drone(
                robo_fixture.client, robo_fixture.world, robo_fixture.drone
            )
        )

    async def fly_px4_drone(
        self, client: ProjectAirSimClient, world: World, drone: PX4Drone
    ):
        """Main test logic to fly/test the PX4 drone"""

        # ------------------------------------------------------------------------------

        # Set up data for test assertions
        client.subscribe(drone.robot_info["actual_pose"], drone.actual_pose_callback)
        timeout_sec = 5
        timeout = time.time() + timeout_sec
        print("\n  Waiting for actual_pose update...")
        drone.actual_pose = None
        while drone.actual_pose is None:
            if time.time() > timeout:
                pytest.fail("Timeout waiting for a pose message update")
            await asyncio.sleep(0.1)

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------

        prev_pose = drone.actual_pose

        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up_task
        projectairsim_log().info("Move-Up completed")

        assert drone.actual_pose["position"]["z"] < prev_pose["position"]["z"]

        # ------------------------------------------------------------------------------

        prev_pose = drone.actual_pose

        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
        )
        projectairsim_log().info("Move-Down invoked")
        await move_down_task
        projectairsim_log().info("Move-Down completed")

        assert drone.actual_pose["position"]["z"] > prev_pose["position"]["z"]

        # ------------------------------------------------------------------------------

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()
