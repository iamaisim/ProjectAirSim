"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
"""

import os
import time

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, TimerAction, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration
import launch_ros.actions
from launch.launch_description_sources import PythonLaunchDescriptionSource

import launch_pytest
import launch_testing
import launch_testing.actions

from ament_index_python.packages import get_package_share_directory

import pytest
import logging

LOGGER = logging.getLogger(__name__)

from std_msgs.msg import *
from builtin_interfaces.msg import *
from geometry_msgs.msg import *
from projectairsim_ros.srv import *
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image

from collections import defaultdict

from conftest import launch_description
from load_scene_client import LoadSceneClientAsync, ServiceClient

def pytest_addoption(parser):
    parser.addoption(
        "--voxelgrid",
        action="store",
        default="/home/jnovak/Projects/ProjectAirSim_main/test_output/voxelgrid.binvox",
        help="Path to store voxel grid files",
    )
    parser.addoption(
        "--occupancygrid",
        action="store",
        default="/home/jnovak/Projects/ProjectAirSim_main/test_output/occupancygrid.binvox",
        help="Path to the store occupancy grid files",
    )
    parser.addoption(
        "--unrealenvscript",
        action="store",
        default="/home/airsim/packages/Blocks/Development/Linux/Blocks.sh",
        help="Path to the store occupancy grid files",
    )

#  Define test fixtures and test cases
@pytest.fixture(scope='module')
def order():
    return []

@pytest.fixture
def voxelgrid(request):
    return request.config.getoption("--voxelgrid")

@pytest.fixture
def occupancygrid(request):
    return request.config.getoption("--occupancygrid")


@pytest.fixture(scope="function", autouse=True)
def delay_between_tests():
    # Add a delay of 1 second between each test. Services are called too quickly for node to recognize.
    time.sleep(1)

@pytest.mark.launch(fixture=launch_description)
def test_case_getColorFromMeshId(order, max_retries=5):
    order.append('test_case_getColorFromMeshId')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/get_logging_directory", GetLoggingDirectory)

        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"GetColorFromMeshId response is None" )
        else:
            LOGGER.warning( f"GetColorFromMeshId {response.success}" )

        retries += 1
        
    LOGGER.info( "GetColorFromMeshId logging directory", response.logging_directory )

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/get_color_from_mesh", GetColorFromMeshId)

        service_client.req.object_id = "TemplateCube_Rounded_6"

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"GetColorFromMeshId response is None" )
        else:
            LOGGER.warning( f"GetColorFromMeshId {response.success}")

        retries += 1
    
    assert response != None
    assert response.r == 44
    assert response.g == 216
    assert response.b == 103

@pytest.mark.launch(fixture=launch_description)
def test_case_getColorFromSegId(order, max_retries=5):
    order.append('test_case_getColorFromSegId')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/get_color_from_seg_id", GetColorFromSegId)

        service_client.req.segmentation_id = 241

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
             
        if response is None:
            LOGGER.warning( f"getColorFromSegId response is None" )
        else:
            LOGGER.warning( f"getColorFromSegId {response.success}" )

        retries += 1
     
    assert response != None
    assert response.r == 44
    assert response.g == 216
    assert response.b == 103

@pytest.mark.launch(fixture=launch_description)
def test_case_getMeshIdsFromSegId(order, max_retries=5):
    order.append('test_case_getMeshIdsFromSegId')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/get_mesh_from_seg_id", GetMeshIdsFromSegId)

        service_client.req.segmentation_id = 241

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"getMeshIdsFromSegId response is None" )
        else:
            LOGGER.warning( f"getMeshIdsFromSegId {response.success}" )

        retries += 1
     
    assert response != None
    assert len(response.object_ids) == 156 # Returns a list of 156 object ids

@pytest.mark.launch(fixture=launch_description)
def test_case_getSegIdFromMeshId(order, max_retries=5):
    order.append('test_case_getSegIdFromMeshId')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/get_seg_id_from_mesh", GetSegIdFromMeshId)

        service_client.req.object_id = "TemplateCube_Rounded_6"

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"getSegIdFromMeshId response is None" )
        else:
            LOGGER.warning( f"getSegIdFromMeshId {response.success}" )

        retries += 1
     
    assert response != None
    assert response.segmentation_id == 241

@pytest.mark.launch(fixture=launch_description)
def test_case_getSegIdFromColor(order, max_retries=5):
    order.append('test_getSegIdFromColor')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/get_seg_id_from_color", GetSegIdFromColor)

        service_client.req.r = 44
        service_client.req.g = 216
        service_client.req.b = 103

        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"getSegIdFromColor response is None" )
        else:
            LOGGER.warning( f"getSegIdFromColor {response.success}" )

        retries += 1
     
    assert response != None
    assert response.segmentation_id == 241

@pytest.mark.launch(fixture=launch_description)
def test_case_takeoff(order, max_retries=5):
    order.append('test_case_takeoff')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/takeoff", Takeoff)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"Takeoff response is None" )
        else:
            LOGGER.warning( f"Takeoff {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True

@pytest.mark.launch(fixture=launch_description)
def test_case_land1(order, max_retries=5):
    order.append('test_case_land1')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/land", Land)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"Land1 response is None" )
        else:
            LOGGER.warning( f"Land1 {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True

@pytest.mark.launch(fixture=launch_description)
def test_case_takeoff_group(order, max_retries=5):
    order.append('test_case_takeoff_group')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/takeoff_group", TakeoffGroup)

        service_client.req.vehicle_names = ["Drone1"]
        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"TakeoffGroup response is None" )
        else:
            LOGGER.warning( f"TakeoffGroup {response.success}")

        retries += 1
    
    assert response != None
    assert response.success == True

@pytest.mark.launch(fixture=launch_description)
def test_case_land(order, max_retries=5):
    order.append('test_case_land')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/land", Land)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"Land2 response is None" )
        else:
            LOGGER.warning( f"Land2 {response.success}" )

        retries += 1
    
    assert response != None
    assert response.success == True

@pytest.mark.launch(fixture=launch_description)
def test_case_move_on_path(order, max_retries=5):
    order.append('test_case_move_on_path')

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
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MoveOnPath response is None" )
        else:
            LOGGER.warning( f"MoveOnPath {response.success}" )

        retries += 1

    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/land", Land)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MoveOnPath Land response is None" )
        else:
            LOGGER.warning( f"MoveOnPath Land {response.success}" )

        retries += 1
    
    assert response != None
    assert response.success == True


@pytest.mark.launch(fixture=launch_description)
def test_case_takeoff_and_multiple_moves(order, max_retries=5):
    order.append('test_case_takeoff_and_multiple_moves')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/takeoff", Takeoff)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPositiontakeoff response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPositiontakeoff {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_to_position", MoveToPosition)

        service_client.req.x = 0.0
        service_client.req.y = 0.0
        service_client.req.z = -15.0
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 30.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition1 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition1 {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_to_position", MoveToPosition)

        service_client.req.x = 5.0
        service_client.req.y = -5.0
        service_client.req.z = -10.0
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 30.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition2 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition2 {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_to_position", MoveToPosition)

        service_client.req.x = 10.0
        service_client.req.y = 10.0
        service_client.req.z = -15.0
        service_client.req.velocity = 8.0
        service_client.req.timeout_sec = 60.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()
        
        if response is not None:
            break

        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition3 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition3 {response.success}" )

        retries += 1
  
    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_to_position", MoveToPosition)

        service_client.req.x = 0.0
        service_client.req.y = 0.0
        service_client.req.z = -7.0
        service_client.req.velocity = 8.0
        service_client.req.timeout_sec = 60.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()
        
        if response is not None:
            break

        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition4 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition4 {response.success}" )

        retries += 1

    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/land", Land)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPositionLand response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPositionLand {response.success}" )

        retries += 1
 
    assert response != None
    assert response.success == True

@pytest.mark.launch(fixture=launch_description)
def test_case_takeoff_and_multiple_moves_path(order, max_retries=5):
    order.append('test_case_takeoff_and_multiple_moves_path')

    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/takeoff", Takeoff)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None and response.success == True:
            break
        
        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPositiontakeoff response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPositiontakeoff {response.success}" )

        retries += 1
     
    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=0.0, y=0.0, z=-15.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1]
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 60.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()
        
        if response is not None:
            break

        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition1 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition1 {response.success}" )

        retries += 1

    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=15.0, y=-15.0, z=-10.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1]
        service_client.req.velocity = 10.0
        service_client.req.timeout_sec = 60.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()
        
        if response is not None:
            break

        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition2 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition2 {response.success}" )

        retries += 1    

    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=10.0, y=10.0, z=-15.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1]
        service_client.req.velocity = 8.0
        service_client.req.timeout_sec = 60.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()
        
        if response is not None:
            break

        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition3 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition3 {response.success}" )

        retries += 1
 
    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/move_on_path", MoveOnPath)

        pose1 = PoseStamped()
        pose1.header = Header()
        pose1.header.frame_id = 'world'
        pose1.pose = Pose()
        pose1.pose.position = Point(x=0.0, y=0.0, z=-7.0)
        pose1.pose.orientation = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)

        service_client.req.path = [pose1]
        service_client.req.velocity = 8.0
        service_client.req.timeout_sec = 60.0
        service_client.req.drive_train_type = 0
        service_client.req.yaw_is_rate = True
        service_client.req.yaw = 0.0
        service_client.req.lookahead = -1.0
        service_client.req.adaptive_lookahead = 1.0
        service_client.req.wait_on_last_task = True

        response = service_client.call()
        service_client.destroy_node()
        
        if response is not None:
            break

        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPosition4 response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPosition4 {response.success}" )

        retries += 1

    assert response != None
    assert response.success == True
    retries = 0

    while retries < max_retries:
        service_client = ServiceClient("/airsim_node/Drone1/land", Land)

        service_client.req.wait_on_last_task = True
        response = service_client.call()
        service_client.destroy_node()

        if response is not None:
            break
        
        if response is None:
            LOGGER.warning( f"MultiMoveMoveoPositionLand response is None" )
        else:
            LOGGER.warning( f"MultiMoveMoveoPositionLand {response.success}" )

        retries += 1

    assert response != None
    assert response.success == True
