import rclpy
from rclpy.node import Node
from projectairsim_ros.msg import Kinematics
import pytest
import time
from std_msgs.msg import String

from conftest import launch_description

from projectairsim_ros.srv import LoadScene
from load_scene_client import LoadSceneClientAsync, ServiceClient

# Create a ROS 2 node for testing
@pytest.fixture
def test_node():
    time.sleep(15)
    node = rclpy.create_node('test_env_actor_spawn_position_node')
    yield node
    node.destroy_node()

# Test case to verify that the env_actor spawns correctly in the scene
@pytest.mark.launch(fixture=launch_description)
def test_env_actor_spawn_position(test_node):
    # Step 1: Call the /load_scene service to load the scene with the env_actor
    scene_file = 'scene_env_actor_car.jsonc'
    retries = 0
    while retries < 5:
        load_scene_client = LoadSceneClientAsync('test_env_actor_spawn_position_node')
        load_scene_client.get_logger().info('Attempting to load scene %s' % scene_file)
        response = load_scene_client.send_request(scene_file)
        load_scene_client.get_logger().info('Result of load scene %d' % response.success)

        if response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()

    # Step 2: Wait for the scene to fully load
    test_node.get_logger().info("Waiting for scene load...")
    time.sleep(1)
    test_node.get_logger().info("Scene loaded.")

    # Step 3: Verify that the new topics have appeared, especially the one ending with /actual_kinematics
    retries = 0
    actual_kinematics_topic = None

    while retries < 5:
        topics = test_node.get_topic_names_and_types()
        time.sleep(2)

        for topic, _ in topics:
            if topic.endswith('/actual_kinematics'):
                actual_kinematics_topic = topic
                break

        if actual_kinematics_topic is not None:
            break

        retries += 1

    assert actual_kinematics_topic is not None, "The /actual_kinematics topic was not found."

    # Step 4: Subscribe to the /actual_kinematics topic and wait for a message
    received_msg = None

    def callback(msg):
        nonlocal received_msg
        received_msg = msg

    sub = test_node.create_subscription(
        Kinematics, actual_kinematics_topic, callback, 10
    )

    test_node.get_logger().info(f"Subscribed to {actual_kinematics_topic}")
    # Step 5: Wait for the message to be received with a timeout
    start_time = time.time()
    while received_msg is None and time.time() - start_time < 20:  # Timeout after 20 seconds
        rclpy.spin_once(test_node)
        time.sleep(0.1)

    assert received_msg is not None, "No message was received on the /actual_kinematics topic."

    # Step 6: Verify the x and y positions are within expected ranges
    position = received_msg.pose.position
    assert abs(position.x - 3.0) < 0.1, f"Expected x position: 3.0, but received: {position.x}"
    assert abs(position.y - 0.0) < 0.1, f"Expected y position: 0.0, but received: {position.y}"

    # Clean up: Destroy publisher and subscriber
    test_node.destroy_subscription(sub)
