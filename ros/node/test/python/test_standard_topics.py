import rclpy
from rclpy.node import Node
from rclpy.executors import SingleThreadedExecutor
from std_msgs.msg import String
from nav_msgs.msg import Odometry
from projectairsim_ros.srv import TakeoffGroup
import pytest
import time

from conftest import launch_description

from projectairsim_ros.srv import LoadScene
from load_scene_client import LoadSceneClientAsync, ServiceClient

topicsDict = { 
    '/airsim_node/Drone1/actual_pose': 'geometry_msgs/msg/PoseStamped',
    '/airsim_node/Drone1/cmd_vel': 'geometry_msgs/msg/Twist',
    '/airsim_node/Drone1/collision_state': 'projectairsim_ros/msg/CollisionInfo',
    '/airsim_node/Drone1/desired_pose': 'geometry_msgs/msg/PoseStamped',
    '/airsim_node/Drone1/odom_local_ned': 'nav_msgs/msg/Odometry',
    '/airsim_node/Drone1/Barometer/barometer': 'sensor_msgs/msg/FluidPressure',
    '/airsim_node/Drone1/Chase/desired_pose': 'geometry_msgs/msg/PoseStamped',
    '/airsim_node/Drone1/Chase/scene_camera/camera_info': 'sensor_msgs/msg/CameraInfo',
    '/airsim_node/Drone1/Chase/scene_camera/image': 'sensor_msgs/msg/Image',
    '/airsim_node/Drone1/DownCamera/depth_camera/camera_info': 'sensor_msgs/msg/CameraInfo',
    '/airsim_node/Drone1/DownCamera/depth_camera/image': 'sensor_msgs/msg/Image',
    '/airsim_node/Drone1/DownCamera/desired_pose': 'geometry_msgs/msg/PoseStamped',
    '/airsim_node/Drone1/DownCamera/scene_camera/camera_info': 'sensor_msgs/msg/CameraInfo',
    '/airsim_node/Drone1/DownCamera/scene_camera/image': 'sensor_msgs/msg/Image',
    '/airsim_node/Drone1/GPS/global_gps': 'sensor_msgs/msg/NavSatFix',
    '/airsim_node/Drone1/IMU1/imu': 'sensor_msgs/msg/Imu',
    '/airsim_node/Drone1/Magnetometer/magnetometer': 'sensor_msgs/msg/MagneticField',
    '/airsim_node/Drone1/waypoint': 'projectairsim_ros/msg/Waypoint',
    '/airsim_node/Drone1/DownCamera/depth_planar_camera/points': 'sensor_msgs/msg/PointCloud2',
    '/airsim_node/Drone1/waypoint': 'projectairsim_ros/msg/Waypoint',
    '/airsim_node/occupancy_grid': 'nav_msgs/msg/OccupancyGrid',
#    '/airsim_node/Drone1/move_on_path/_action/feedback': 'projectairsim_ros/action/MoveOnPath_FeedbackMessage',
#    '/airsim_node/Drone1/move_on_path/_action/status': 'action_msgs/msg/GoalStatusArray',
    '/bond': 'bond/msg/Status',
    '/clock': 'rosgraph_msgs/msg/Clock',
    '/diagnostics': 'diagnostic_msgs/msg/DiagnosticArray',
    '/map_saver/transition_event': 'lifecycle_msgs/msg/TransitionEvent',
    '/parameter_events': 'rcl_interfaces/msg/ParameterEvent',
    '/rosout': 'rcl_interfaces/msg/Log',
    '/tf': 'tf2_msgs/msg/TFMessage',
    '/tf_static': 'tf2_msgs/msg/TFMessage',
}

@pytest.fixture
def test_node():
    time.sleep(15.0)
    node = Node('test_standard_topics_node')
    executor = SingleThreadedExecutor()
    executor.add_node(node)
    yield node
    node.destroy_node()

def find_topic(topics, topic_name):
    for topic, _ in topics:
        if topic_name == topic:
            return True
        
    return False

def find_topic_type(topics, topic_name, topic_type):
    for topic, type in topics:
        if topic_name == topic:
            for aType in type:
                if aType == topic_type:
                    return True
        
    return False

@pytest.mark.launch(fixture=launch_description)
def test_load_scene_topic_exists_and_publish(test_node):
    # Step 2: Publish to the /load_scene topic
    scene_file = 'scene_basic_drone.jsonc'
    retries = 0
    while retries < 5:
        load_scene_client = LoadSceneClientAsync('test_standard_topics_node')
        load_scene_client.get_logger().info('Attempting to load scene %s' % scene_file)
        response = load_scene_client.send_request(scene_file)
        load_scene_client.get_logger().info('Result of load scene %d' % response.success)

        if response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()

    # Give some time for the message to be processed
    rclpy.spin_once(test_node, timeout_sec=10)

    # Step 3: Find the /odom_local_ned topic
    retries = 0

    while retries < 5:
        new_topics = test_node.get_topic_names_and_types()
        new_topics_length = len(new_topics)
        time.sleep(2)
        
        test_node.get_logger().info(f"Number of topics is {new_topics_length}")

        retries += 1

    assert new_topics_length >= 25, "Insufficient topic count after loading scene."

    for aTopic in new_topics:
        test_node.get_logger().info(f"Topics is {aTopic}")

    for aTopic in topicsDict.keys():
        assert find_topic(new_topics, aTopic), f"Could not find {aTopic} topic."
        assert find_topic_type(new_topics, aTopic, topicsDict[aTopic]), f"Could not find {aTopic} topic type {topicsDict[aTopic]}."
