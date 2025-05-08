"""
Copyright (C) Microsoft Corporation. All rights reserved.
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

from std_msgs.msg import *
from builtin_interfaces.msg import *
from geometry_msgs.msg import *
from projectairsim_ros.srv import *
import rclpy
from rclpy.executors import SingleThreadedExecutor
from rclpy.node import Node
from sensor_msgs.msg import Image

from collections import defaultdict

from conftest import launch_description

from projectairsim_ros.srv import LoadScene
from load_scene_client import LoadSceneClientAsync, ServiceClient

@pytest.fixture
def node():
    time.sleep(15.0)
    node = Node('test_image_topics_node')
    executor = SingleThreadedExecutor()
    executor.add_node(node)
    yield node
    node.destroy_node()

@pytest.mark.launch(fixture=launch_description)
def test_image_topics(node):
    # Step 1: Set the /load_scene topic
    load_scene_topic = "/airsim_node/load_scene"
    assert load_scene_topic is not None, "Could not find /load_scene topic."

    # Step 2: Publish to the /load_scene topic to load the scene
    scene_file = 'scene_drone_sensors.jsonc'
    retries = 0
    while retries < 5:
        load_scene_client = LoadSceneClientAsync('test_image_topics_node')
        node.get_logger().info('Attempting to load scene %s' % scene_file)
        response = load_scene_client.send_request(scene_file)
        node.get_logger().info('Result of load scene %d' % response.success)

        if response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()

    # Give some time for the message to be processed
    rclpy.spin_once(node, timeout_sec=10)

    # Define the specific subpaths to look for in topic names
    target_subpaths = [
        'DownCamera/depth_camera/image',
        'DownCamera/scene_camera/image',
        'DownCamera/segmentation_camera/image'
    ]

    # Step 3: Filter topics that contain the target subpaths

    retries = 0
    image_topics = []

    while retries < 5:
        new_topics = node.get_topic_names_and_types()
        time.sleep(1)

        image_topics = [
            topic for topic in new_topics
            if any(subpath in topic[0] for subpath in target_subpaths)
        ]

        if len(image_topics) >= 3:
            break

        retries += 1
        time.sleep(3)
        node.get_logger().info('Retrying image topics search %d, found %d' % retries, len(image_topics))

        for aTopic in new_topics:
            node.get_logger().info('Found topic %s' % aTopic)

        image_topics = []

    assert len(image_topics) >= 3, f"Expected at least 3 topics containing the specified subpaths but found {len(image_topics)}."

    # Use a dictionary to collect timestamps per topic
    received_timestamps = defaultdict(list)

    def image_callback(msg, topic):
        # Extract and store the timestamp of each received message
        timestamp = msg.header.stamp.sec * 1e9 + msg.header.stamp.nanosec
        node.get_logger().info(f"Received image from {topic} at {timestamp} nanoseconds")
        received_timestamps[topic].append(timestamp)

    # Subscribe to each identified topic
    subscriptions = []
    for topic, _ in image_topics:
        subscription = node.create_subscription(
            Image, topic, lambda msg, topic=topic: image_callback(msg, topic), 10)
        subscriptions.append(subscription)
        node.get_logger().info(f"Subscribed to {topic}")

    # Wait to receive sufficient image data
    for _ in range(90):  # Increase iteration limit if necessary
        rclpy.spin_once(node, timeout_sec=3)
        # Ensure we have at least some data from all topics
        if all(len(timestamps) >= 5 for timestamps in received_timestamps.values()):
            break

    # Ensure that all topics have received messages
    assert all(len(timestamps) > 0 for timestamps in received_timestamps.values()), \
        "Did not receive enough messages from all specified image topics."

    # New logic: Ensure that for every message in any topic, there are corresponding messages in the other topics
    tolerance_ms = 30.0
    tolerance_ns = tolerance_ms * 1e6  # Convert milliseconds to nanoseconds

    for topic in received_timestamps:
        for timestamp in received_timestamps[topic]:
            matching_sets = []

            for other_topic in received_timestamps:
                if other_topic == topic:
                    continue

                close_timestamps = []

                # Find timestamps in other topics that are close to the current timestamp within the tolerance
                for ts in received_timestamps[other_topic]:
                    ts_diff = abs(ts - timestamp)
                    node.get_logger().info(f"Diff of {topic} and {other_topic} is {ts_diff}")

                    if abs(ts - timestamp) <= tolerance_ns:
                        close_timestamps.append(ts)

                if len( close_timestamps ) > 0:
                    matching_sets.append(close_timestamps[0])  # Take the first matching timestamp

            # Ensure we found corresponding messages in the other two topics
            assert len(matching_sets) == 2, \
                f"Did not find corresponding messages within {tolerance_ms}ms for all topics for timestamp {timestamp} from topic {topic}"

            node.get_logger().info(f"Found matching timestamps for message at {timestamp} from topic {topic}")

    # Cleanup: Unsubscribe from all topics
    for subscription in subscriptions:
        node.destroy_subscription(subscription)
