"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Pytest end-end test script for Livox Mid-70 lidar sensor.
"""

import asyncio
import pytest

from typing import Deque

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log


class LIDARAnalysis:
    def __init__(self):
        self.sec_report_last = 0.0
        self.dqdata = Deque()
        self.cpoints_sum = 0
        self.pts_per_second_avg = 0.0
        self.sec_report_interval_avg = 0

    def receive_lidar(self, lidar_data):
        """Callback for receiving new LIDAR data. Keep this processing as short as
        possible to avoid blocking the callback for receiving data from NNG.
        """
        if lidar_data is not None:
            secCur = float(lidar_data["time_stamp"]) / (1000 * 1000 * 1000)

            lidar_points = lidar_data["point_cloud"]
            cpoints = len(lidar_points) / 3  # three coordinate values per point

            # Add latest report time and size to queue
            self.dqdata.append([secCur, cpoints])
            self.cpoints_sum += cpoints

            # Purge older data to limit queue size
            while len(self.dqdata) > 10:
                self.cpoints_sum -= self.dqdata.popleft()[1]

            # Calculate time since last report and update average number of points reported per second
            if len(self.dqdata) < 2:
                self.pts_per_second_avg = cpoints
                dsec_report = 0
            else:
                entry_first = self.dqdata[0]
                dsec_data = secCur - entry_first[0]

                # Ignore point count from oldest entry since the count is for before the time period measured
                self.pts_per_second_avg = (
                    self.cpoints_sum - entry_first[1]
                ) / dsec_data
                dsec_report = secCur - self.sec_report_last

            # Update average report interval
            self.sec_report_interval_avg = (
                self.sec_report_interval_avg * 0.9 + dsec_report * 0.1
            )

            self.sec_report_last = secCur


# Async main function to wrap async drone commands
async def main(
    scene_config_file: str,
    sim_config_path: str,
    drone_name: str,
    lidar_sensor_name: str,
):
    lidaranalysis = LIDARAnalysis()

    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(
            client,
            scene_config_file,
            delay_after_load_sec=2,
            sim_config_path=sim_config_path,
        )

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, drone_name)

        # ------------------------------------------------------------------------------

        client.subscribe(
            drone.sensors[lidar_sensor_name]["lidar"],
            lambda _, lidar: lidaranalysis.receive_lidar(lidar),
        )

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------

        # Display the LIDAR reporting statistics
        projectairsim_log().info("Displaying LIDAR statistics for 5 seconds")
        for i in range(0, 5):
            await asyncio.sleep(1)
            pts_per_second_avg = lidaranalysis.pts_per_second_avg
            sec_report_interval_avg = lidaranalysis.sec_report_interval_avg
            hz_report_avg = (
                0 if sec_report_interval_avg == 0 else 1.0 / sec_report_interval_avg
            )
            projectairsim_log().info(
                f"{lidar_sensor_name}: data rate = {pts_per_second_avg:.2f} points/s avg., report frequency = {hz_report_avg:.2f} Hz avg."
            )

        # ------------------------------------------------------------------------------

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

    return lidaranalysis


@pytest.fixture()
def lidar_test():
    return asyncio.run(
        main(
            scene_config_file="scene_test_lidar_mid70.jsonc",
            sim_config_path="sim_config",
            drone_name="Drone1",
            lidar_sensor_name="lidar1",
        )
    )


class TestClientBase:
    def test_lidar_mid70(self, lidar_test):
        # points-per-second setting, Mid-70 defaults to 100,000
        assert lidar_test.pts_per_second_avg == pytest.approx(100000, abs=5000)

        # Interval of report-frequency setting, Mid-70 defaults to 10 Hz
        assert lidar_test.sec_report_interval_avg == pytest.approx(0.1, rel=0.1)
