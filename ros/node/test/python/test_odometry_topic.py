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

@pytest.fixture
def node():
    time.sleep(15.0)
    node = Node('test_odometry_topic_node')
    executor = SingleThreadedExecutor()
    executor.add_node(node)
    yield node
    node.destroy_node()

def capture_odometry(node, odom_topic):
    """Capture an odometry message using a temporary subscription."""
    received_odom = []

    def odom_callback(msg):
        received_odom.append(msg)
        node.get_logger().info(f"Received odometry: {msg.pose.pose.position.z}")

    subscription = node.create_subscription(Odometry, odom_topic, odom_callback, 10)

    # Wait for an odometry message to arrive
    for _ in range(10):
        rclpy.spin_once(node, timeout_sec=1)
        if received_odom:
            break

    node.destroy_subscription(subscription)
    
    assert received_odom, f"No odometry message received on {odom_topic}"
    return received_odom[0]

@pytest.mark.launch(fixture=launch_description)
def test_load_scene_topic_exists_and_publish(node):
   # Step 1: List topics and find the /load_scene topic
    #load_scene_topic = "/ProjectAirSim/node/IntegrationTestNode/load_scene_topic"
    #assert load_scene_topic is not None, "Could not find /load_scene topic."

    # Step 2: Publish to the /load_scene topic
    scene_file = 'scene_basic_drone.jsonc'
    retries = 0
    while retries < 5:
        load_scene_client = LoadSceneClientAsync('test_odometry_topic_node')
        load_scene_client.get_logger().info('Attempting to load scene %s' % scene_file)
        response = load_scene_client.send_request(scene_file)
        load_scene_client.get_logger().info('Result of load scene %d' % response.success)

        if response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()

    # Give some time for the message to be processed
    rclpy.spin_once(node, timeout_sec=10)

    # Step 3: Find the /odom_local_ned topic
    retries = 0
    odom_topic = None

    while retries < 5:
        new_topics = node.get_topic_names_and_types()
        new_topics_length = len(new_topics)
        time.sleep(2)
        
        node.get_logger().info(f"Number of topics is {new_topics_length}")

        for topic_name, _ in new_topics:
            if topic_name.endswith('/odom_local_ned'):
                odom_topic = topic_name
                break

        if odom_topic is not None:
            break

        retries += 1

    assert odom_topic is not None, "Could not find /odom_local_ned topic."

    # Step 4: Capture odometry before takeoff
    odom_before = capture_odometry(node, odom_topic)
    z_before = odom_before.pose.pose.position.z

    # Step 5: Call the takeoff service
    takeoff_service = '/airsim_node/takeoff_group'
    client = node.create_client(TakeoffGroup, takeoff_service)

    if not client.wait_for_service(timeout_sec=10.0):
        node.get_logger().error(f'Service {takeoff_service} not available.')
        assert False, f'Service {takeoff_service} not available.'

    request = TakeoffGroup.Request()
    request.vehicle_names = ['Drone1']
    request.wait_on_last_task = False
    future = client.call_async(request)

    rclpy.spin_until_future_complete(node, future)
    response = future.result()

    assert response is not None, f"Failed to call service {takeoff_service}"
    assert response.success, f"Takeoff failed: {response}"
    node.get_logger().info(f"Service call {takeoff_service} completed successfully")

    # Step 6: Capture odometry after takeoff
    time.sleep(5)  # Wait for the drone to take off
    odom_after = capture_odometry(node, odom_topic)
    z_after = odom_after.pose.pose.position.z

    # Step 7: Compare positions
    assert z_after != z_before, f"Expected the drone to move, but z position did not change (z_before={z_before}, z_after={z_after})."
    assert z_after < 0, f"Expected the drone to be in the air with negative z position, but got z_after={z_after}."

    # Cleanup
