import asyncio
import pytest
import time
import numpy as np
from scipy.spatial.transform import Rotation as R

from projectairsim import EnvActor, ProjectAirSimClient, World

#To test with the shelby env car add the shelby assets to the Blocks/Content file
#Also add this lines inside the "environment-actors" list at the scene_test_env_actor.jsonc file
'''{
    "type": "env_car",
    "name": "shelby",
    "origin": {
        "xyz": "23.0 5.0 0.0",
        "rpy-deg": "0 0 0"
    },
    "env-actor-config": "robot_test_env_actor_car_shelby.jsonc"
}'''


class ProjectAirSimTestObject:
    def __init__(self):
        self.client = ProjectAirSimClient()
        self.client.connect()
        self.world = World(self.client, "scene_test_env_actor.jsonc", 1)
        self.robot_position = None
        self.robot_orientation = None

    def suv_actual_data_callback(self, topic, kine_msg):
        low_ground_level = -1.4
        up_ground_level = -1.3
        self.robot_position = kine_msg['kinematics']['pose']['position'] 
        self.robot_orientation = kine_msg['kinematics']['pose']['orientation']

        #The values 0, 0, 30 refer to the rpy values of the orientation of the suv car, defined in the scene_test_env_actor.jsonc file
        orientation = self.convertOrientationToQuaternion(0, 0, 30)
        self.validate_orientation(orientation[3], orientation[0], orientation[1], orientation[2])

        #The values 23, -2 refer to the x and y position of the suv car, defined in the scene_test_env_actor.jsonc file
        self.validate_position(23, -2)

        #Validate the z position of the suv car is between -1.4 and -1, this values corresponde to a correct ground level
        self.validate_ground_level(low_ground_level, up_ground_level)

    def human_actual_data_callback(self, topic, kine_msg):
        low_ground_level = -1.1
        up_ground_level = 0
        self.robot_position = kine_msg['kinematics']['pose']['position'] 
        self.robot_orientation = kine_msg['kinematics']['pose']['orientation']
        
        #The values 15, 5 refer to the x and y position of the pedestrian, defined in the scene_test_env_actor.jsonc file
        self.validate_position(15, 5)

        #The values 0, 0, 90 refer to the rpy values of the orientation of the suv car, defined in the scene_test_env_actor.jsonc file
        orientation = self.convertOrientationToQuaternion(0, 0, 90)
        self.validate_orientation(orientation[3], orientation[0], orientation[1], orientation[2])

        #Validate the z position of the suv car is between 0 and -1, this values corresponde to a correct ground level
        self.validate_ground_level(low_ground_level, up_ground_level)
    

    #Uncomment the following line to test the shelby car
    def shelby_actual_data_callback(self, topic, kine_msg):
        low_ground_level = -1.6
        up_ground_level = -1.5
        self.robot_position = kine_msg['kinematics']['pose']['position'] 
        self.robot_orientation = kine_msg['kinematics']['pose']['orientation']
        
        #The values 23, 5 refer to the x and y position of the shelby car, defined in the scene_test_env_actor.jsonc file
        self.validate_position(23, 5)

        #The values 0, 0, 0 refer to the rpy values of the orientation of the shelby car, defined in the scene_test_env_actor.jsonc file
        orientation = self.convertOrientationToQuaternion(0, 0, 0)
        self.validate_orientation(orientation[3], orientation[0], orientation[1], orientation[2])

        #Validate the z position of the shelby car is between -1.4 and -1.5, this values corresponde to a correct ground level
        self.validate_ground_level(low_ground_level, up_ground_level)

    def validate_position(self, expected_x, expected_y):
        assert self.robot_position['x'] == expected_x 
        assert self.robot_position['y'] == expected_y
        assert self.robot_position['z'] <= 0

    def validate_orientation(self, expected_w, expected_x, expected_y, expected_z):
        x_kine_orientation = self.robot_orientation["x"]
        y_kine_orientation = self.robot_orientation["y"]
        z_kine_orientation = self.robot_orientation["z"]
        w_kine_orientation = self.robot_orientation["w"]

        assert expected_w == pytest.approx(w_kine_orientation, rel=1e-1), \
                f"The value of {expected_w} it is not close enough to {w_kine_orientation}"
        assert expected_x == pytest.approx(x_kine_orientation, rel=1e-1), \
                f"The value of {expected_x} it is not close enough to {x_kine_orientation}"
        assert expected_y == pytest.approx(y_kine_orientation, rel=1e-1), \
                f"The value of {expected_y} it is not close enough to {y_kine_orientation}"
        assert expected_z == pytest.approx(z_kine_orientation, rel=1e-1), \
                f"The value of {expected_z} it is not close enough to {z_kine_orientation}"
        
    def validate_ground_level(self, low_ground_level, up_ground_level):
        time.sleep(2)
        result = self.robot_position['z']
        assert low_ground_level < result < up_ground_level, \
                f"Result {result} is not within the expected range between {low_ground_level} and {up_ground_level}"

    def convertOrientationToQuaternion(self, roll, pitch, yaw):
        roll_rad = np.deg2rad(roll)
        pitch_rad = np.deg2rad(pitch)
        yaw_rad = np.deg2rad(yaw)

        # Create the rotation and convert it to a quaternion
        r = R.from_euler('xyz', [roll_rad, pitch_rad, yaw_rad])
        quaternion = r.as_quat()

        # Round the values to a similar precision
        quaternion_rounded = np.round(quaternion, decimals=16)

        return quaternion_rounded

@pytest.fixture(scope="class")
def robo():
    robo_obj = ProjectAirSimTestObject()
    yield robo_obj

    print("\nTeardown client...")
    robo_obj.client.disconnect()

class TestClientBase:
    async def main(self, robo):
        print("Starting test...")
        client = robo.client

        client.subscribe(
            "/Sim/SceneBasicDrone/env_actors/car1/actual_kinematics", robo.suv_actual_data_callback
        )

        client.subscribe(
            "/Sim/SceneBasicDrone/env_actors/pedestrian/actual_kinematics", robo.human_actual_data_callback
        )

        #Uncomment the following line to test the shelby car
        '''client.subscribe(
            "/Sim/SceneBasicDrone/env_actors/shelby/actual_kinematics", robo.shelby_actual_data_callback
        )'''
        


    def test_env_car(self, robo):
        asyncio.run(self.main(robo))
