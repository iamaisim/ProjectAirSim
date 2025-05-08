# Python client example to get Lidar data from a drone
import sys
import time
import argparse
import pprint
import numpy
import asyncio

from projectairsim import ProjectAirSimClient, World, Drone
from projectairsim.utils import projectairsim_log

# Makes the drone fly and get Lidar data
class LidarTest:

    def __init__(self):

        # connect to the AirSim simulator
        self.client = ProjectAirSimClient()
        self.client.connect()
        self.world = World(self.client, "scene_drone_sensors.jsonc", delay_after_load_sec=2)
        self.drone = Drone(self.client, self.world, "Drone1")
        self.drone.enable_api_control()

    async def execute(self):

        projectairsim_log().info("Arming the drone...")
        self.drone.arm()

        state = self.drone.get_ground_truth_kinematics()
        s = pprint.pformat(state)
        #airsim_vnext_log().info("state: %s" % s)

        projectairsim_log().info('Press enter to takeoff')
        input()
        takeoff_task = await self.drone.takeoff_async()
        await takeoff_task


        state = self.drone.get_ground_truth_kinematics()
        #airsim_vnext_log().info("state: %s" % pprint.pformat(state))

        projectairsim_log().info('Press enter to move vehicle to (-10, 10, -10) at 5 m/s')
        input()
        move_task = await self.drone.move_to_position_async(-10, 10, -10, 5)
        await move_task

        hover_task = await self.drone.hover_async()
        await hover_task

        projectairsim_log().info('Press enter to get Lidar readings')
        input()

        self.reading_number = 1
        self.last_time_stamp = 0

        self.client.subscribe(
            self.drone.sensors["lidar1"]["lidar"],
            lambda _, lidar: self.handle_lidar(lidar),
        )

        time.sleep(25)

    def handle_lidar(self, lidar_data):
        time = float(lidar_data["time_stamp"]) / (1000 * 1000 * 1000)
        if time > self.last_time_stamp + 5:
            if (len(lidar_data["point_cloud"]) < 3):
                projectairsim_log().info("\tNo points received from Lidar data")
            else:
                points = self.parse_lidarData(lidar_data)
                projectairsim_log().info("\tReading %d: time_stamp: %f number_of_points: %d" % (self.reading_number, time, len(points)))
                projectairsim_log().info("\t\tlidar position: %s" % (pprint.pformat(lidar_data["pose"]["position"])))
                projectairsim_log().info("\t\tlidar orientation: %s" % (pprint.pformat(lidar_data["pose"]["orientation"])))
            self.reading_number += 1
            self.last_time_stamp = time
        if self.reading_number == 6:
            self.client.unsubscribe(self.drone.sensors["lidar1"]["lidar"])

    def parse_lidarData(self, data):

        # reshape array of floats to array of [X,Y,Z]
        points = numpy.array(data["point_cloud"], dtype=numpy.dtype('f4'))
        points = numpy.reshape(points, (int(points.shape[0]/3), 3))
       
        return points

    def stop(self):

        projectairsim_log().info('Press enter to disconnect')
        input()
        projectairsim_log().info("Done!\n")
        self.client.disconnect()

# main
if __name__ == "__main__":
    lidarTest = LidarTest()
    try:
        asyncio.run(lidarTest.execute())
    finally:
        lidarTest.stop()
