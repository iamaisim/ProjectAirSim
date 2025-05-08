import rclpy
from rclpy.node import Node
from projectairsim_ros.msg import Kinematics

from std_msgs.msg import *
from geometry_msgs.msg import *

import pytest
import time
from std_msgs.msg import String

from conftest import launch_description
from load_scene_client import LoadSceneClientAsync, ServiceClient

from projectairsim_ros.srv import *

# Create a ROS 2 node for testing
@pytest.fixture
def test_node():
    time.sleep(15)
    node = rclpy.create_node('load_scene_two_drones_node')
    yield node
    node.destroy_node()

# Test case to verify that the multiple drones operate correctly in the scene
@pytest.mark.launch(fixture=launch_description)
def test_two_drones(test_node):
    max_retries = 5

    # Step 1: Call the /load_scene service to load the scene with the env_actor
    scene_file = 'scene_two_drones.jsonc'
    retries = 0
    while retries < max_retries:
        load_scene_client = LoadSceneClientAsync('load_scene_two_drones')
        test_node.get_logger().info('Attempting to load scene %s' % scene_file)
        response = load_scene_client.send_request(scene_file)
        test_node.get_logger().info('Result of load scene %d' % response.success)

        if response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()

    # Step 2: Wait for the scene to fully load
    test_node.get_logger().info("Waiting for scene load...")
    time.sleep(5)
    test_node.get_logger().info("Scene loaded.")

    # Step 3: Drone takeoff
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/takeoff_group", TakeoffGroup)

        service_client.req.vehicle_names = ["Drone1", "Drone2"]
        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        test_node.get_logger().warning( f"Takeoff Group failure {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True

    time.sleep(1)


    # Step 4: Drones move on path
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=30.0, y=0.0, z=-30.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose2 = PoseStamped()
        pose2.header = Header()
        pose2.header.frame_id = 'world'
        pose2.pose = Pose()
        pose2.pose.position = Point(x=-15.0, y=15.0, z=-50.0)
        pose2.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose3 = PoseStamped()
        pose3.header = Header()
        pose3.header.frame_id = 'world'
        pose3.pose = Pose()
        pose3.pose.position = Point(x=10.0, y=0.0, z=-25.0)
        pose3.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose4 = PoseStamped()
        pose4.header = Header()
        pose4.header.frame_id = 'world'
        pose4.pose = Pose()
        pose4.pose.position = Point(x=0.0, y=0.0, z=-8.0)
        pose4.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1, pose2, pose3, pose4]
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 45.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = False

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        test_node.get_logger().warning( f"MoveOnPath Drone1 {response.success} attempt {retries}" )

        time.sleep(0.5)
        retries += 1

    assert response != None
#    assert response.success == True

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone2/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=30.0, y=10.0, z=-30.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose2 = PoseStamped()
        pose2.header = Header()
        pose2.header.frame_id = 'world'
        pose2.pose = Pose()
        pose2.pose.position = Point(x=-15.0, y=25.0, z=-50.0)
        pose2.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose3 = PoseStamped()
        pose3.header = Header()
        pose3.header.frame_id = 'world'
        pose3.pose = Pose()
        pose3.pose.position = Point(x=10.0, y=10.0, z=-25.0)
        pose3.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        pose4 = PoseStamped()
        pose4.header = Header()
        pose4.header.frame_id = 'world'
        pose4.pose = Pose()
        pose4.pose.position = Point(x=0.0, y=4.0, z=-8.0)
        pose4.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1, pose2, pose3, pose4]
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 45.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        test_node.get_logger().warning( f"MoveOnPath Drone2 {response.success} attempt {retries}" )

        time.sleep(0.5)
        retries += 1

    assert response != None
#    assert response.success == True
    
    # Step 5: Drones Land
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/land_group", LandGroup)

        service_client.req.vehicle_names = ["Drone1", "Drone2"]
        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        test_node.get_logger().warning( f"Land Group failure {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True
