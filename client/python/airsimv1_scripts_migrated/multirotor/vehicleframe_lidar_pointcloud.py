from time import sleep
import numpy as np
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

class LidarTest:

    def __init__(self):

        # connect to the Project AirSim simulator
        self.client = ProjectAirSimClient()
        self.client.connect()
        self.world = World(self.client, "scene_drone_double_lidar.jsonc", delay_after_load_sec=2)
        self.drone = Drone(self.client, self.world, "Drone1")
        projectairsim_log().info('Connected!\n')

    def execute(self,vehicle_name,lidar_names):
        self.vehicle_name = vehicle_name
        projectairsim_log().info('Scanning Has Started\n')
        projectairsim_log().info('Use Keyboard Interrupt \'CTRL + C\' to Stop Scanning\n')
        self.existing_data_cleared = dict()
        try:
            for i, lidar_name in enumerate(lidar_names):
                self.existing_data_cleared[lidar_name] = False #change to true to superimpose new scans onto existing .asc files
                self.client.subscribe(
                    self.drone.sensors[lidar_names[i]]["lidar"],
                    lambda _, lidar, name=lidar_names[i]: self.handle_lidar(name, lidar),
                )
            while True:
                #spin while the subscribers do all the work
                sleep(1)
                pass
        except KeyboardInterrupt:
            projectairsim_log().info('Press enter to stop running this script')
            input()
            projectairsim_log().info("Done!\n")
        finally:
            self.client.disconnect()

    def handle_lidar(self, lidar_name, lidar_data):
        filename = f"{self.vehicle_name}_{lidar_name}_pointcloud.asc"
        if not self.existing_data_cleared[lidar_name]:
            f = open(filename,'w')
            projectairsim_log().info(filename)
        else:
            f = open(filename,'a')

        for i in range(0, len(lidar_data["point_cloud"]), 3):
            xyz = lidar_data["point_cloud"][i:i+3]
            f.write("%f %f %f %d %d %d \n" % (xyz[0],xyz[1],xyz[2],255,255,0))
        f.close()
        self.existing_data_cleared[lidar_name] = True

# main
if __name__ == "__main__":
    lidarTest = LidarTest()
    lidarTest.execute('Drone1',['LidarSensor1','LidarSensor2'])
