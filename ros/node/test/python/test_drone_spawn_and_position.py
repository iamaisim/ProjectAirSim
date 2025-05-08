import rclpy
from rclpy.node import Node
from rclpy.executors import SingleThreadedExecutor
from std_msgs.msg import String
from geometry_msgs.msg import PoseStamped
import pytest

import time
from conftest import launch_description

from projectairsim_ros.srv import LoadScene
from load_scene_client import LoadSceneClientAsync, ServiceClient

@pytest.fixture
def node():
    #
    #  The launch file fixture uses a launch file having delays to ensure that the nodes are up and running
    #
    time.sleep(15.0)
    node = Node('test_drone_spawn_and_position_node')
    executor = SingleThreadedExecutor()
    executor.add_node(node)
    yield node
    node.destroy_node()


@pytest.mark.launch(fixture=launch_description)
def test_drone_spawn_and_position(node):
    # Step 1: Load a new scene file 
    scene_file = 'scene_basic_drone.jsonc'
    retries = 0
    while retries < 5:
        load_scene_client = LoadSceneClientAsync('test_drone_spawn_and_position_node')
        load_scene_client.get_logger().info('Attempting to load scene %s' % scene_file)
        response = load_scene_client.send_request(scene_file)
        load_scene_client.get_logger().info('Result of load scene %d' % response.success)

        if response is not None and response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()

    # Give some time for the message to be processed
    rclpy.spin_once(node, timeout_sec=5)

    # Step 2: List topics to confirm
    retries = 0
    while retries < 5:
        new_topics = node.get_topic_names_and_types()
        if len(new_topics) >= 28:
            break

        time.sleep(1)
        retries += 1

    assert len(new_topics) >= 28, "No new topics appeared after loading scene."

    # Step 3: Subscribe to /actual_pose topic
    actual_pose_topic = None
    for topic_name, _ in new_topics:
        if topic_name.endswith('/actual_pose'):
            actual_pose_topic = topic_name
            break

    assert actual_pose_topic is not None, "Could not find /actual_pose topic."

    received_poses = []

    def pose_callback(msg):
        received_poses.append(msg)

    subscription = node.create_subscription(PoseStamped, actual_pose_topic, pose_callback, 10)
    node.get_logger().info(f"Subscribed to {actual_pose_topic}")

    # Wait to receive some pose data
    for _ in range(15):
        rclpy.spin_once(node, timeout_sec=3)

        if len(received_poses) > 0:
            break

    assert len(received_poses) > 0, "No pose messages received."

    # Step 4: Verify position is near (0, 0) within a margin of 0.01
    for pose_msg in received_poses:
        x = pose_msg.pose.position.x
        y = pose_msg.pose.position.y
        node.get_logger().info(f"Received pose: x={x}, y={y}")
        assert abs(x) <= 0.01, f"x position is out of bounds: {x}"
        assert abs(y) <= 0.01, f"y position is out of bounds: {y}"

    # Cleanup
    node.destroy_subscription(subscription)
