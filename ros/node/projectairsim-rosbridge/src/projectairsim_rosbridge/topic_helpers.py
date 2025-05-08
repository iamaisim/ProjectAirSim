"""
Copyright (C) Microsoft Corporation. All rights reserved.
ROS bridge for Project AirSim: Topic management, handlers, and helpers
"""

import copy
import logging
import traceback
from typing import Dict
import traceback

import geometry_msgs.msg as rosgeommsg
import sensor_msgs.msg as rossensmsg

from . import utils
from .node import ROSNode
from .tf_helpers import TFBroadcaster, TFListener

import projectairsim.types
import projectairsim
from projectairsim import ProjectAirSimClient, World

import open3d as o3d
import numpy as np
from sensor_msgs.msg import Image, PointCloud2, PointField
from sensor_msgs_py import point_cloud2
import std_msgs.msg
from image_geometry import PinholeCameraModel
import cv2
from cv_bridge import CvBridge, CvBridgeError
from std_msgs.msg import Header
import rclpy
from rclpy.node import Node
#import time
from rosgraph_msgs.msg import Clock
from rclpy.qos import QoSProfile, QoSHistoryPolicy, QoSReliabilityPolicy, QoSDurabilityPolicy
from scipy.spatial.transform import Rotation
from builtin_interfaces.msg import Time

#--------------------------------------------------------------------------
# from tf_transformations import quaternion_from_euler
#
#  From ros-humble-tf-transformations, which is causing numpy version incompatibilties
#
import transforms3d

def _reorder_output_quaternion(quaternion):
    """Reorder quaternion to have w term last."""
    w, x, y, z = quaternion
    return x, y, z, w


def quaternion_from_euler(ai, aj, ak, axes='sxyz'):
    """
    Return quaternion from Euler angles and axis sequence.

    ai, aj, ak : Euler's roll, pitch and yaw angles
    axes : One of 24 axis sequences as string or encoded tuple

    >>> q = quaternion_from_euler(1, 2, 3, 'ryxz')
    >>> numpy.allclose(q, [0.310622, -0.718287, 0.444435, 0.435953])
    True

    """
    return _reorder_output_quaternion(
        transforms3d.euler.euler2quat(ai, aj, ak, axes=axes)
    )

def generate_ros_topic(ros_topic_name, projectairsim_topic_name):
    """
    Generates a ROS topic name based on the provided ros_topic_name and projectairsim_topic_name.

    Parameters:
        ros_topic_name (str): The initial ROS topic name. Can be None.
        projectairsim_topic_name (str): The topic name used in the Project AirSim context.

    Returns:
        str: The generated ROS topic name.
    """
    if ros_topic_name is not None and not ros_topic_name.startswith('/airsim_node/'):
        # Split the topic into segments
        segments = projectairsim_topic_name.strip('/').split('/')

        actor_name = 'default_actor'
        sensor_name = ''
        remaining_topic = ''

        # Find the actor name after "robots" or "env_actors"
        if 'robots' in segments or 'env_actors' in segments:
            actors_index = segments.index('robots') if 'robots' in segments else segments.index('env_actors')
            if len(segments) > actors_index + 1:
                actor_name = segments[actors_index + 1]

        # Find the sensor name after "sensors"
        if 'sensors' in segments:
            sensors_index = segments.index('sensors')
            if len(segments) > sensors_index + 1:
                sensor_name = segments[sensors_index + 1]
                # Capture the remaining topic after the sensor
                remaining_segments = segments[sensors_index + 2:]

                if len(remaining_segments) > 1:
                    remaining_topic = f"{remaining_segments[0]}/{ros_topic_name.strip('/')}"
                elif remaining_segments:
                    remaining_topic = ros_topic_name.strip('/')
            else:
                remaining_topic = ros_topic_name.strip('/')

        # Build the new topic
        if sensor_name and remaining_topic:
            ros_topic_name = f"/airsim_node/{actor_name}/{sensor_name}/{remaining_topic}"
        elif sensor_name:
            ros_topic_name = f"/airsim_node/{actor_name}/{sensor_name}/{ros_topic_name.strip('/')}"
        else:
            ros_topic_name = f"/airsim_node/{actor_name}/{ros_topic_name.strip('/')}"

    elif ros_topic_name is not None and ros_topic_name.startswith('/airsim_node/'):
        ros_topic_name = ros_topic_name

    else:
        # If ros_topic_name is None, use the default behavior
        segments = projectairsim_topic_name.strip('/').split('/')

        actor_name = 'default_actor'
        sensor_name = ''
        remaining_topic = ''

        # Find the actor name after "robots" or "env_actors"
        if 'robots' in segments or 'env_actors' in segments:
            actors_index = segments.index('robots') if 'robots' in segments else segments.index('env_actors')
            if len(segments) > actors_index + 1:
                actor_name = segments[actors_index + 1]

        # Find the sensor name after "sensors"
        if 'sensors' in segments:
            sensors_index = segments.index('sensors')
            if len(segments) > sensors_index + 1:
                sensor_name = segments[sensors_index + 1]
                remaining_topic = '/'.join(segments[sensors_index + 2:])
            else:
                remaining_topic = ''

        # Build the final topic
        if sensor_name and remaining_topic:
            ros_topic_name = f"/airsim_node/{actor_name}/{sensor_name}/{remaining_topic}"
        elif sensor_name:
            ros_topic_name = f"/airsim_node/{actor_name}/{sensor_name}"
        else:
            ros_topic_name = f"/airsim_node/{actor_name}/{remaining_topic}"

    return ros_topic_name

# --------------------------------------------------------------------------
class TopicCallbacks(utils.Callbacks):
    """
    Callback list for (ROS or Project AirSim) topic changes.
    """

    def __init__(self):
        """
        Constructor.
        """
        super().__init__()

    def __call__(self, topic_name, message):
        """
        Invoke all topic change callback functions.


        Arguments:
            topic_name - The name of the topic
            message - The message received from the topic
        """
        for callback in self.callbacks:
            callback(topic_name, message)


# --------------------------------------------------------------------------
class ROSPeerChangeCallbacks(utils.Callbacks):
    """
    Callback list for changes to the number of (external) subscribers to a
    ROS topic.
    """

    def __init__(self):
        """
        Constructor.
        """
        super().__init__()

    def __call__(self, topic_name: str, num_peers: int):
        """
        Invoke all peer change callback functions.

        Arguments:
            topic_name - The name of the ROS topic
            num_peers - The new number of external subscribers to the topic
        """
        for callback in self.callbacks:
            callback(topic_name, num_peers)


# --------------------------------------------------------------------------
class ProjectAirSimTopicsManager:
    """
    Manages access to Project AirSim topics.

    Project AirSim topics are subscribed to when there is at least one
    subscriber and unsubscribed when there are no subscribers.  Multiple
    subscribers to an Project AirSim topic are supported.
    """

    def __init__(
        self, projectairsim_client: ProjectAirSimClient, logger: logging.Logger
    ):
        """
        Constructor.

        Arguments:
            projectairsim_client - Project AirSim client object
            logger - Log message handler
        """
        self.projectairsim_client = projectairsim_client  # Project AirSim client object
        self.logger = logger  # Logging object
        self.sub_topic_callbacks = (
            {}
        )  # Mapping from topic name to topic callback functions

    def add_subscriber(self, topic_name, topic_callback):
        """
        Adds a subscriber to an Project AirSim topic.

        Arguments:
            topic_name - Name of the Project AirSim topic
            topic_callback - The topic callback function to be invoked when a message is published to the topic
        """
        if topic_name not in self.sub_topic_callbacks:
            callbacks = TopicCallbacks()
            self.sub_topic_callbacks[topic_name] = callbacks
        else:
            callbacks = self.sub_topic_callbacks[topic_name]

        callback_added = callbacks.add(topic_callback)

        # If this is the first subscriber, subscribe to the Project AirSim topic
        if callback_added and (len(callbacks) == 1):
            self._subscribe_to_topic(topic_name)

    def clear(self):
        """
        Stop handling all topics and clear all resources.
        """
        if self.projectairsim_client is not None:
            for topic_name in self.sub_topic_callbacks:
                try:
                    self._unsubscribe_from_topic(topic_name)
                except:
                    # Most likely scene has been changed externally invalidating our subscriptions
                    self.logger.debug(
                        f"Caught exception unsubscribing from Project AirSim topic {topic_name}: {traceback.format_exc()}"
                    )

    def publish(self, topic_name, message):
        """
        'Publish a message to topic.

        Arguments:
            topic_name - The name of the topic
            message - The data to publish to the topic
        """
        if self.projectairsim_client is not None:
            self.projectairsim_client.publish(topic_name, message)

    def refresh(self):
        """
        Refresh subscriptions if necessary.

        Subscriptions to Project AirSim topics may need to recreated if the
        simulation scene has been loaded.
        """
        if self.projectairsim_client is not None:
            for topic_name in self.sub_topic_callbacks:
                # Make sure we're still not subscribed
                try:
                    self._unsubscribe_from_topic(topic_name)
                except:
                    # Most likely scene has been changed externally invalidating our subscriptions
                    self.logger.debug(
                        f"Caught exception unsubscribing from Project AirSim topic {topic_name}: {traceback.format_exc()}"
                    )

                # Resubscribe
                self._subscribe_to_topic(topic_name)

    def remove_subscriber(self, topic_name, topic_callback):
        """
        Remove a subscriber from an Project AirSim topic.

        Arguments:
            topic_name - Name of the topic
            topic_callback - The topic callback function to remove
        """
        if topic_name in self.sub_topic_callbacks:
            try:
                callbacks = self.sub_topic_callbacks[topic_name]
                callbacks.remove(topic_callback)
                if not callbacks:
                    # We're removed the last callback--unsubscribe from the Project AirSim topic
                    del self.sub_topic_callbacks[topic_name]
                    self._unsubscribe_from_topic(topic_name)
            except:
                self.logger.warning(
                    f"caught exception removing subscriber for Project AirSim topic {topic_name}: {traceback.format_exc()}"
                )

    def _subscribe_to_topic(self, topic_name):
        """
        Subscribe to an Project AirSim topic.

        Arguments:
            topic_name - The name of the topic to which to subscribe
        """
        if self.projectairsim_client is not None:
            self.logger.debug(f"Subscribing to AirSim topic {topic_name}")
            self.projectairsim_client.subscribe(topic_name, self._topic_update_cb)

    def _topic_update_cb(self, topic, message):
        """
        Callback to handle a message from the Project AirSim topic.
        The topic callbacks are invoked to handle the message.

        Arguments:
            topic - Project AirSim topic information
            message - The data received from the Project AirSim topic
        """
        # self.logger.debug(f"Update on AirSim topic {topic.path}")
        if topic.path in self.sub_topic_callbacks:
            try:
                self.sub_topic_callbacks[topic.path](topic, message)
            except:
                self.logger.error(
                    f"caught exception invoking callbacks for Project AirSim topic {topic.path}: {traceback.format_exc()}"
                )

    def _unsubscribe_from_topic(self, topic_name):
        """
        Unsubscribe from an Project AirSim topic.

        Arguments:
            topic_name - The name of the topic from which to unsubscribe
        """
        if self.projectairsim_client is not None:
            self.logger.debug(f"Unsubscribing from AirSim topic {topic_name}")
            try:
                self.projectairsim_client.unsubscribe(topic_name)
            # except BrokenPipeError:
            #     # Ignore error unsubscribing
            #     pass
            except:
                self.logger.warning(
                    f"caught exception calling projectairsim_client.unsubscribe for Project AirSim topic {topic_name}: {traceback.format_exc()}"
                )


# --------------------------------------------------------------------------
class ROSTopicsManager:
    """
    Manages access to ROS topics.

    ROS topics are subscribed to when there is at least one subscriber and
    unsubscribed when there are no subscribers.  Multiple subscribers to a
    ROS topic are supported.
    """

    def __init__(self, ros_node: ROSNode, logger: logging.Logger):
        """
        Constructor.

        Arguments:
            logger - Logging object
        """
        self.logger = logger
        self.ros_node = ros_node
        self.ros_publishers = {}
        self.ros_subscribers = {}
        self.ros_timers = {}

    def clear(self):
        """
        Stop publishing and unsubscribe from all topics and free resources.
        """
        for pair in self.ros_subscribers.items():
            pair[1]["subscriber"].unregister()
        self.ros_subscribers = {}

        for pair in self.ros_publishers.items():
            pair[1]["publisher"].unregister()
        self.ros_publishers = {}

        for pair in self.ros_timers.items():
            pair[1]["timer"].unregister()
        self.ros_timers = {}

    def add_publisher(
        self,
        topic_name: str,
        ros_message_type: type,
        peer_change_callback=None,
        is_latching: bool = True,
        ros_queue_size: int = 1,
        ros_qos_profile: QoSProfile = None,
    ):
        """
        Add a publisher for a ROS topic.  If this is the first publisher,
        the ROS topic is immediately advertised.  The message type and
        message latching parameters are only used if the topic is not yet
        advertised by us.

        The peer change callback function is invoked when another ROS
        node subscribes to or unsubscribes from the topic.

        Arguments:
            topic_name - The name of the ROS topic
            ros_message_type - The data type of messages published to the
                ROS topic
            peer_change_callback - The peer change callback function
            is_latching - If true, the last topic message is sent to new subscribers
            ros_queue_size - The number of topic messages that may be queued while
                waiting to be sent to subscribers
        """
        
        if topic_name in self.ros_publishers:
            ros_peer_change_callbacks = self.ros_publishers[topic_name]["callbacks"]
        else:
            self.logger.info(f'Advertising ROS topic "{topic_name}"')
            ros_peer_change_callbacks = ROSPeerChangeCallbacks()
            self.ros_publishers[topic_name] = {
                "publisher": self.ros_node.create_publisher(
                    topic=topic_name,
                    msg_type=ros_message_type,
                    queue_size=ros_queue_size,
                    latch=is_latching,
                    subscriber_listener=self,
                    pub_qos_profile=ros_qos_profile,
                ),
                "callbacks": ros_peer_change_callbacks,
                "num_peers": 0,
            }

        if peer_change_callback is not None:
            ros_peer_change_callbacks.add(peer_change_callback)

    def add_subscriber(self, topic_name: str, ros_message_type: type, topic_callback):
        """
        Add a subscriber to a ROS topic.  Each topic may have multiple
        subscribers.

        Arguments:
            topic_name - The name of the topic
            ros_message_type - The expected data type of messages received
                from the topic
            topic_callback - The callback function to invoke when a message
                is received from the topic
        """
        if topic_name in self.ros_subscribers:
            topic_callbacks = self.ros_subscribers[topic_name]["callbacks"]
        else:
            self.logger.info(f'Subscribing to ROS topic "{topic_name}"')
            topic_callbacks = TopicCallbacks()
            self.ros_subscribers[topic_name] = {
                "subscriber": self.ros_node.create_subscriber(
                    msg_type=ros_message_type,
                    topic=topic_name,
                    callback=self._topic_cb_decorator(
                        self._topic_update_cb, topic_name
                    ),
                ),
                "callbacks": topic_callbacks,
            }

        topic_callbacks.add(topic_callback)

    def add_timer(self, period_sec: float, topic_callback):
        """
        Add a publisher for a timer.

        Arguments:
            topic_frequency - The frequency at which the publisher should publish
            topic_callback - The callback function to invoke at timer
        """

        if period_sec not in self.ros_timers:
            topic_callbacks = TopicCallbacks()
            self.ros_timers[period_sec] = {
                "timer": self.ros_node.create_timer(
                    period_sec=period_sec,
                    callback=topic_callback
                ),
                "callbacks": topic_callbacks,
            }
        else:
            topic_callbacks = self.ros_timers[period_sec]["callbacks"]

        topic_callbacks.add(topic_callback)      

    def peer_subscribe(self, topic_name):
        """
        Implementation of SubscribeListener method.

        Callback to handle when someone subscribes to our ROS topic.

        Arguments:
            topic_name - The (resolved/remapped) ROS topic name
        """
        if topic_name in self.ros_publishers:
            self.logger.debug(f"Peer subscribed to {topic_name}")
            num_peers = self.ros_publishers[topic_name]["num_peers"] + 1
            self.ros_publishers[topic_name]["num_peers"] = num_peers
            self.ros_publishers[topic_name]["callbacks"](topic_name, num_peers)

    def peer_unsubscribe(self, topic_name, num_peers):
        """
        Implementation of SubscribeListener method.

        Callback to handle when someone unsubscribes from our ROS topic.

        Arguments:
            topic_name - The (resolved/remapped) ROS topic name
            num_peers - Number of remaining subscribed peers
        """
        if topic_name in self.ros_publishers:
            self.logger.debug(f"Peer unsubscribed from {topic_name}")
            self.ros_publishers[topic_name]["num_peers"] = num_peers
            self.ros_publishers[topic_name]["callbacks"](topic_name, num_peers)

    def publish(self, topic_name: str, message):
        """
        Publish a message to topic.  The topic must be registered first
        with add_publisher().

        Arguments:
            topic_name - The name of the topic
            message - The data to publish to the topic
        """
        if topic_name in self.ros_publishers:
            self.ros_publishers[topic_name]["publisher"].publish(message)

    def remove_publisher(self, topic_name: str, ros_peer_callback):
        """
        Remove a publisher for a topic.  When all publishers are removed,
        the ROS topic is no longer advertised.

        Arguments:
            topic_name - The name fo the topic
            ros_peer_callback - The ROS peer change callback function
                registered via add_publisher()
        """
        if topic_name in self.ros_publishers:
            ros_peer_callbacks = self.ros_publishers[topic_name]["callbacks"]
            ros_peer_callbacks.remove(ros_peer_callback)

            # If no more callbacks, stop publishing the topic
            if not ros_peer_callbacks:
                self.logger.info(f'Unadvertising ROS topic "{topic_name}"')
                self.ros_publishers[topic_name]["publisher"].destroy()
                del self.ros_publishers[topic_name]

    def remove_subscriber(self, topic_name: str, topic_callback):
        """
        Remove a subscriber from a topic.  When all subscribers are removed,
        the ROS topic is no longer subscribed to.

        Arguments:
            topic_name - The name of the topic
            topic_callback - The topic callback function registered via
                add_subscriber()
        """
        if topic_name in self.ros_subscribers:
            ros_topic_callbacks = self.ros_subscribers[topic_name]["callbacks"]
            ros_topic_callbacks.remove(topic_callback)

            # If no more callbacks, stop subscribing to the topic
            if not ros_topic_callbacks:
                self.logger.info(f'Unsubscribing from ROS topic "{topic_name}"')
                self.ros_subscribers[topic_name]["subscriber"].destroy()
                del self.ros_subscribers[topic_name]

    def remove_timer(self, period_sec: float, topic_callback):
        """
        Remove a callback from a timer.  When all callbacks are removed,
        the ROS timer is no longer subscribed to.

        Arguments:
            publish_rate_hz - The publishing rate of timer
            topic_callback - The topic callback function registered via
                add_timer()
        """
        if period_sec in self.ros_timers:
            ros_topic_callbacks = self.ros_timers[period_sec]["callbacks"]
            ros_topic_callbacks.remove(topic_callback)

            # If no more callbacks, stop subscribing to the topic
            if not ros_topic_callbacks:
                self.ros_timers[period_sec]["timer"].destroy()
                del self.ros_timers[period_sec]


    @staticmethod
    def _topic_cb_decorator(fn_cb_original, topic_name: str):
        """
        Decorator to add topic name to topic callback function since the ROS
        topic callback only passes the message from the topic.

        Arguments:
            fn_cb_original - Original topic callback function
            topic_name - Topic name to pass to cb_original
        """

        def cb_wrapper(message):
            return fn_cb_original(topic_name, message)

        return cb_wrapper

    def _topic_update_cb(self, topic_name, message):
        """
        Callback to handle a message from the ROS topic.
        The topic callbacks are invoked to handle the message.

        Arguments:
            topic_name - The name of the topic
            message - The data received from the Project AirSim topic
        """
        # self.logger.debug(f"Update from ROS topic {topic_name}")
        if topic_name in self.ros_subscribers:
            try:
                self.ros_subscribers[topic_name]["callbacks"](topic_name, message)
            except:
                self.logger.error(
                    f"caught exception invoking callbacks for ROS topic {topic_name}: {traceback.format_exc()}"
                )


# --------------------------------------------------------------------------
class TopicsManagers:
    """
    The managers for ROS and Project AirSim topics and ROS transforms
    """

    def __init__(
        self,
        projectairsim_client: ProjectAirSimClient,
        ros_node: ROSNode,
        logger: logging.Logger,
        projectairsim_world = None
    ):
        """
        Constructor.
        """
        self.logger = logger
        self.projectairsim_topics_manager = ProjectAirSimTopicsManager(
            projectairsim_client, logger
        )
        self.ros_node = ros_node
        self.ros_topics_manager = ROSTopicsManager(ros_node, logger)
        self.tf_broadcaster = TFBroadcaster(ros_node, logger)
        self.tf_listener = TFListener(ros_node, logger)

    def set_world(self, world):
        self.tf_broadcaster.set_world(world)

# --------------------------------------------------------------------------
class AutoSubscriber:
    """
    This class handles subscribing to an Project AirSim topic automatically
    when a peer subscribes to the corresponding ROS topic.

    The caller call the instance method peer_change_cb() when the ROS
    publisher's peer change callback is invoked.  This class won't
    subscribe to the Project AirSim topic until peer_change_cb() is called
    with a non-zero num_peer argument.
    """

    def __init__(
        self,
        projectairsim_topic_name,
        projectairsim_topic_callback,
        airsim_topics_manager: ProjectAirSimTopicsManager,
    ):
        """
        Constructor.

        Arguments:
            projectairsim_topic_name - Name of the Project AirSim topic
            projectairsim_topic_callback - The callback function to invoke
                when a message is published to the Project AirSim topic
            airsim_topics_manager - Project AirSim topics manager
        """
        if not callable(projectairsim_topic_callback):
            raise TypeError(
                f"projectairsim_topic_callback is not callable: {projectairsim_topic_callback}"
            )

        self.is_subscribed = (
            False  # Whether we're currenly subscribed to the Project AirSim topic
        )
        self.projectairsim_topic_name = (
            projectairsim_topic_name  # Name of the Project AirSim topic
        )
        self.projectairsim_topic_callback = (
            projectairsim_topic_callback
        )  # Callback to invoke when a message is published to the Project AirSim topic
        self.projectairsim_topics_manager = (
            airsim_topics_manager  # Project AirSim topics manager
        )

    def __del__(self):
        """
        Destructor.
        """
        self.clear()

    def clear(self):
        """
        Unsubscribe from the Project AirSim topic is necessary and free resources.
        """
        if self.is_subscribed:
            self.unsubscribe()
        self.projectairsim_topic_callback = None
        self.projectairsim_topics_manager = None

    def peer_change_cb(self, ros_topic_name: str, num_peers: int):
        """
        Handles a change to the number of subscribers to the ROS topic.

        We subscribe to the Project AirSim topic when have someone subscribing
        to the ROS topic and unsubscribe when we don't.

        Typically the caller passes this method to ROSTopicManager.add_publisher()
        as the peer_change_callback argument.  If needed, the caller may
        instead register their only callback function and then explicitly
        call this method.

        Arguments:
            ros_topic_name - Name of the ROS topic
            num_peers - New number of subscribers to the ROS topic
        """
        if num_peers == 0:
            if self.is_subscribed:
                self.unsubscribe()
        else:
            if not self.is_subscribed:
                self.subscribe()

    def subscribe(self):
        self.projectairsim_topics_manager.add_subscriber(
            self.projectairsim_topic_name, self.projectairsim_topic_callback
        )
        self.is_subscribed = True

    def unsubscribe(self):
        self.is_subscribed = False
        self.projectairsim_topics_manager.remove_subscriber(
            self.projectairsim_topic_name, self.projectairsim_topic_callback
        )


# --------------------------------------------------------------------------
class BasicBridgeToROS:
    """
    Topic handler that transforms data from an Project AirSim topic and
    publishes it to a ROS topic.  The ROS topic usually has the same name as
    the Project AirSim topic.  A message handler callback function is used to
    convert from the Project AirSim to ROS message type.

    ros_topic_is_latching controls whether a new ROS topic subscriber will get
    the last message published when they connect.  This is suitable,
    for instance, for "level action" topics that report the last known
    state of something and not suitable for "pulse action" topics
    intended to indicate an event has just occurred, triggering an
    action once per message.

    The message handler callback function usually transforms a received
    Project AirSim message and returns an appropriate ROS message which is
    published to the ROS topic.  It is not restricted in doing so and can
    do whatever processing it wishes (including publishing messages to
    to ROS topics directly) and return None.
    """

    def __init__(
        self,
        projectairsim_topic_name: str,
        ros_message_type,
        topics_managers: TopicsManagers,
        message_callback,
        ros_topic_name: str = None,
        append_ros_topic_name: bool = True,
        ros_topic_is_latching: bool = True,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        Standard Arguments:
            projectairsim_topic_name - Name of source Project AirSim topic
            ros_message_type - The data type of the ROS topic messages
            topics_managers - Topic and transform managers

        Class-Specific Arguments:
            message_callback - Message handler callback function
            ros_topic_name - Name of the ROS topic if different from projectairsim_topic_name
            append_ros_topic_name - Automatically append ros_topic_name to projectairsim
            ros_topic_is_latching - If true, new subscribers of the ROS topic receive the last message published
        """
        if not callable(message_callback):
            raise TypeError(f"message_callback is not callable: {message_callback}")

        ros_topic_name = generate_ros_topic(ros_topic_name, projectairsim_topic_name)

        self.projectairsim_topic_name = projectairsim_topic_name
        self.message_callback = message_callback  # Message handler callback
        self.ros_topic_name = ros_topic_name
        self.topics_managers = topics_managers  # Topics managers
        self.logger = logger

        # Create auto-subscriber to manage our subscription to the Project AirSim topic
        self._auto_subscriber = AutoSubscriber(
            projectairsim_topic_name,
            self._projectairsim_topic_update_cb,
            topics_managers.projectairsim_topics_manager,
        )

        # Advertise the ROS topic
        topics_managers.ros_topics_manager.add_publisher(
            topic_name=ros_topic_name,
            ros_message_type=ros_message_type,
            is_latching=ros_topic_is_latching,
            peer_change_callback=self._auto_subscriber.peer_change_cb,
        )

    def __del__(self):
        """
        Destructor.
        """
        self.clear()

    def clear(self):
        """
        Stop handling messages and free resources.
        """

        # Unsubscribe from the Project AirSim topic first
        if self._auto_subscriber is not None:
            self._auto_subscriber.clear()

        # Stop publishing the ROS topic
        if self.ros_topic_name is not None:
            ros_topic_name = self.ros_topic_name
            self.ros_topic_name = None
            self.topics_managers.ros_topics_manager.remove_publisher(
                ros_topic_name, self._auto_subscriber.peer_change_cb
            )

        self._auto_subscriber = None
        self.topics_manager = None
        self.message_callback = None

    def _projectairsim_topic_update_cb(
        self, projectairsim_topic, projectairsim_message_data
    ):
        """
        Handle a new message published to the Project AirSim topic.

        The message callback function is invoked with the new message.  If
        the callback functions returns an object, the object is published
        to the ROS topic.  The callback function is free to publish
        any messages on its own (or not) and return None.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            ros_message = self.message_callback(
                projectairsim_topic.path, projectairsim_message_data
            )
            if ros_message is not None:
                self.topics_managers.ros_topics_manager.publish(
                    self.ros_topic_name, ros_message
                )


# --------------------------------------------------------------------------
class BasicROSSubscriber:
    """
    Topic handler that accepts data a ROS topic and forwards it to the
    message callback.  It does not forward the message to Project AirSim
    (see the BasicBridgeFromROS class.)
    """

    def __init__(
        self,
        ros_topic_name: str,
        ros_message_type,
        topics_managers: TopicsManagers,
        message_callback,
        projectairsim_topic_name: str = None,
    ):
        """
        Constructor.

        Standard Arguments:
            projectairsim_topic_name - Saved but not used if specified
            ros_message_type - The data type of the ROS topic messages
            topics_managers - Topic and transform managers

        Class-Specific Arguments:
            ros_topic_name - Name of the ROS topic if different from projectairsim_topic_name
            message_callback - Message handler callback function
        """
        if not callable(message_callback):
            raise TypeError(f"message_callback is not callable: {message_callback}")

        if ros_topic_name is None:
            ros_topic_name = generate_ros_topic(ros_topic_name, projectairsim_topic_name)

        self.message_callback = message_callback
        self.projectairsim_topic_name = projectairsim_topic_name
        self.ros_topic_name = ros_topic_name
        self.topics_managers = topics_managers

        topics_managers.ros_topics_manager.add_subscriber(
            ros_topic_name, ros_message_type, self._ros_topic_update_cb
        )

    def __del__(self):
        """
        Destructor.
        """
        self.clear()

    def clear(self):
        """
        Stop handling messages and free resources.
        """
        # Stop subscribing to the ROS topic
        if self.ros_topic_name:
            ros_topic_name = self.ros_topic_name
            self.ros_topic_name = None
            self.topics_managers.ros_topics_manager.remove_subscriber(
                ros_topic_name, self._ros_topic_update_cb
            )
 
        self.topic_manager = None
        self.message_callback = None

    def _ros_topic_update_cb(self, ros_topic_name, ros_message_data):
        """
        Handle a new message published to the ROS topic by forwarding to
        the message callback function.

        Arguments:
            ros_topic_name - Name of the Project AirSim topic
            ros_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            self.message_callback(ros_topic_name, ros_message_data)


# --------------------------------------------------------------------------
class BasicBridgeFromROS(BasicROSSubscriber):
    """
    This class extends BasicROSSubscriber by also transforming the data from
    the ROS topic and publishing it to an Project AirSim topic.  The ROS topic
    name is derived from the Project AirSim topic name and is usually the same.
    The message handler callback function is used to convert from the ROS to
    the Project AirSim message type.

    The message handler callback function usually transforms a received ROS
    message and returns an appropriate Project AirSim message which is
    published to the Project AirSim topic.  It is not restricted in doing so
    and can do whatever processing it wishes (including publishing messages
    to Project AirSim topics directly) and return None.
    """

    def __init__(
        self,
        projectairsim_topic_name: str,
        ros_message_type,
        topics_managers: TopicsManagers,
        message_callback,
        ros_topic_name: str = None,
    ):
        """
        Constructor.

        Standard Arguments:
            projectairsim_topic_name - Name of target destination Project AirSim topic
            ros_message_type - The data type of the ROS topic messages
            topics_managers - Topic and transform managers

        Class-Specific Arguments:
            message_callback - Message handler callback function
            ros_topic_name - Name of the ROS topic if different from projectairsim_topic_name
        """
        if ros_topic_name is None:
            segments = projectairsim_topic_name.strip('/').split('/')
            
            actor_name = 'default_actor'
            remaining_topic = ''
            
            # Buscar el nombre del actor después de "robots"
            if 'robots' in segments:
                robots_index = segments.index('robots')
                if len(segments) > robots_index + 1:
                    actor_name = segments[robots_index + 1]
                    remaining_topic = '/'.join(segments[robots_index + 2:])
                else:
                    remaining_topic = ''
            
            ros_topic_name = f"/airsim_node/{actor_name}/{remaining_topic}"

        super().__init__(
            projectairsim_topic_name=projectairsim_topic_name,
            ros_topic_name=ros_topic_name,
            ros_message_type=ros_message_type,
            topics_managers=topics_managers,
            message_callback=message_callback,
        )

    def _ros_topic_update_cb(self, ros_topic_name, ros_message_data):
        """
        Handle a new message published to the ROS topic.

        The message callback function is invoked with the new message.  If
        the callback functions returns an object, the object is published
        to the Project AirSim topic.  The callback function is free to
        publish any messages on its own (or not) and return None.

        Arguments:
            ros_topic_name - Name of the Project AirSim topic
            ros_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            projectairsim_message = self.message_callback(
                ros_topic_name, ros_message_data
            )
            if projectairsim_message is not None:
                self.topics_managers.projectairsim_topics_manager.publish(
                    self.projectairsim_topic_name, projectairsim_message
                )


# --------------------------------------------------------------------------
class SensorBridgeToROS(BasicBridgeToROS):
    """
    Based on BasicBridgeToROS, this class handles publishing ROS topics
    for sensor data received from Project AirSim with a transform frame
    corresponding to the sensor's orientation with respect to the robot.
    The external message handler callback function converts from the Project
    AirSim sensor data message to the ROS message and extracts the sensor's
    relative transform.
    """

    # -------------------------------------------------------------------------
    # SensorBridgeToROS Methods
    # -------------------------------------------------------------------------
    def __init__(
        self,
        projectairsim_topic_name: str,
        ros_message_type,
        topics_managers: TopicsManagers,
        message_callback,
        transform_message_callback,
        ros_topic_name: str = None,
        ros_topic_is_latching: bool = False,
        frame_id: str = None,
        frame_id_parent: str = "map",
        transform_params: dict = None,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        "message_callback" converts the sensor data from the Project AirSim topic
        message to an ROS message.  It can return None
        it handles the Project AirSim sensor data message entirely by itself.

        "transform_message_callback" extracts the sensor orientation data from the Project
        AirSim topic message and returns a pose suitable for the ROS transform frame.
        It can return None it handles the transform update entirely by itself.

        Standard Arguments:
            projectairsim_topic_name - Name of source Project AirSim topic
            ros_message_type - The data type of the ROS topic messages
            topics_managers - Topic and transform managers
            transform_message_callback - Transform extraction callback function

        Class-Specific Arguments:
            message_callback - Image message handler callback function (inherited from BasicBridgeToROS)
            ros_topic_is_latching - If true, new subscribers of the ROS topic
                receive the last message published  (inherited from BasicBridgeToROS)
            frame_id - The name of the transform frame to broadcast; if None,
                the robot name is used, derived from the Project AirSim topic name
            frame_id_parent - The name of the parent frame
        """
        if not callable(message_callback):
            raise TypeError(f"message_callback is not callable: {message_callback}")
        if not callable(transform_message_callback):
            raise TypeError(
                f"transform_message_callback is not callable: {transform_message_callback}"
            )
        self.transform_message_callback = transform_message_callback

        # Setup the sensor transform frame
        if frame_id is not None:
            if not frame_id:
                raise ValueError("frame_id can't be an empty string")
            self.frame_id = frame_id
        else:
            self.frame_id = utils.get_sensor_name_id(projectairsim_topic_name)

        self.frame_id_parent = frame_id_parent
        self.transform = (
            rosgeommsg.Transform()
        )  # Cache of sensor's last known transform for ROS
        #topics_managers.tf_broadcaster.add_frame(self.frame_id, self.frame_id_parent)
        self.set_static_transforms(topics_managers, projectairsim_topic_name, transform_params)

        # Initialize the super class with the sensor topic
        super().__init__(
            projectairsim_topic_name=projectairsim_topic_name,
            ros_message_type=ros_message_type,
            topics_managers=topics_managers,
            message_callback=message_callback,
            ros_topic_is_latching=ros_topic_is_latching,
            ros_topic_name=ros_topic_name,
            logger=logger,
        )


    def set_static_transforms(
            self,
            topics_managers,
            projectairsim_topic_name,
            transform_params: dict = None 
        ):
            self.static_parent_frame_id = utils.get_robot_frame_id(projectairsim_topic_name)    

            self.static_transform = (
                rosgeommsg.Transform()
            )  # Cache of camera's last known transform for ROS        

            self.optical_static_transform = (
                rosgeommsg.Transform()
            )  # Cache of camera's last known transform for ROS     

            #set camera _body/static frame
            self.static_frame_id = self.frame_id + "_body/static"
            self.optical_static_frame_id = self.frame_id + "_optical/static"

            #topics_managers.tf_broadcaster.add_frame(self.static_frame_id, self.static_parent_frame_id)
            #topics_managers.tf_broadcaster.add_frame(self.optical_static_frame_id, self.static_parent_frame_id)

            if transform_params is None:
                transform_params = {
                    'x_coord': 0,
                    'y_coord': 0,
                    'z_coord': 0,
                    'roll': 0,
                    'pitch': 0,
                    'yaw': 0,
                }
            (
                self.static_transform.translation.x,
                self.static_transform.translation.y,
                self.static_transform.translation.z,
            ) = utils.to_ros_position_list2list(
                (
                    (float)(transform_params.get('x_coord', 0)),
                    (float)(transform_params.get('y_coord', 0)),
                    (float)(transform_params.get('z_coord', 0)),
                )
            )

            q = quaternion_from_euler((float)(transform_params.get('roll', 0)), (float)(transform_params.get('pitch', 0)), (float)(transform_params.get('yaw', 0)))
            (
                self.static_transform.rotation.x,
                self.static_transform.rotation.y,
                self.static_transform.rotation.z,
                self.static_transform.rotation.w,
            ) = utils.to_ros_quaternion_list2list(
                (
                    (float)(q[0]),
                    (float)(q[1]),
                    (float)(q[2]),
                    (float)(q[3]),
                )
            )

            self.optical_static_transform = utils.get_camera_optical_tf_from_body_tf(self.static_transform)

            #topics_managers.tf_broadcaster.set_frame(self.static_frame_id, self.static_transform)
            #topics_managers.tf_broadcaster.set_frame(self.optical_static_frame_id, self.optical_static_transform)        

    def clear(self):
        """
        Stop handling messages and free resources.
        """
        super().clear()

        if self.frame_id is not None:
            #self.topics_managers.tf_broadcaster.remove_frame(self.frame_id)
            self.frame_id = None

    def _projectairsim_topic_update_cb(
        self, projectairsim_topic, projectairsim_message_data
    ):
        """
        Handle a new sensor data message published to the Project AirSim topic.

        We convert the message to the equivalent ROS message and update the
        sensor transform from the sensor pose indicated in the message.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_message_data - Message published to the Project AirSim topic
        """

        if self.ros_topic_name:
            # Attach transform frame ID to message for message callback
            projectairsim_message_data["frame_id"] = self.frame_id

            # Calculate sensor transform frame
            ros_transform = self.transform_message_callback(
                projectairsim_topic.path, projectairsim_message_data
            )

            # Publish the sensor transform frame
            '''
            if ros_transform is not None:
                self.transform = ros_transform
                self.topics_managers.tf_broadcaster.set_frame(
                    self.frame_id, ros_transform
                )
            '''

            # Generate the ROS topic message
            ros_sensor_message = self.message_callback(
                projectairsim_topic.path, projectairsim_message_data
            )

            # Publish the ROS topic data
            self.topics_managers.ros_topics_manager.publish(
                self.ros_topic_name, ros_sensor_message
            )


# --------------------------------------------------------------------------
class CameraBridgeToROS(BasicBridgeToROS):
    """
    Based on BasicBridgeToROS, this class handles publishing ROS topics
    for a camera image type received from Project AirSim.  The external
    message handler callback function converts from the Project AirSim image
    message to the ROS geometry_msgs.Image message.

    This class subscribes to two Project AirSim topics, subscribes to a ROS
    topic for the camera pose, publishes three ROS topics for the camera info,
    camera image, and compressed camera image, and broadcasts a transform
    frame for the camera.  Create this class for the Project AirSim "*_camera"
    topic and it will automatically subscribe to "*_camera_info".  Do not
    create this class for the "*_camera_info" topic.

    We unify all of the Project AirSim camera topics for a camera image type
    into a single instance because they're published independently by Project
    AirSim but we need to publish all three together in sychrony when the
    image is updated with logic to handle when only one of the ROS topics
    have subscribers.
    """

    # -------------------------------------------------------------------------
    # CameraBridgeToROS Types
    # -------------------------------------------------------------------------

    # --------------------------------------------------------------------------
    class DesiredPoseBridgeFromROS:
        """
        Helper class for bridging from a ROS camera desired_pose topic to
        a Project AirSim camera pose.

        This class is designed to be a singleton that ensures we create only one
        "desired_pose" topic for a camera sensor even though we create an
        instance of CameraBridgeToROS() for each image subtype published by the
        camera (scene_camera, depth_camera, depth_planar_camera, etc.)  Each
        CameraBridgetToROS() instance calls add_camera() and this singleton
        maps duplicate calls for the same camera to a single camera entry.
        """

        class CameraEntry:
            def __init__(
                self,
                camera_path: str,
                desired_pose_message_callback,
                projectairsim_client: projectairsim.ProjectAirSimClient,
                ros_topics_manager: ROSTopicsManager,
            ):
                """
                Constructor.

                "camera_path" is the part of the camera sensors's Project AirSim
                topic name up to but not including the forward-slash ("/")
                terminating the sensor name.

                "desired_pose_message_callback" converts the ROS topic message
                into a Project AirSim PoseMessage structure like that for the
                robot's "desired_pose" topic.

                Arguments:
                    camera_path - Project AirSim camera path
                    desired_pose_message_callback - Callback function to
                        process the ROS message into an Project AirSim message
                    projectairsim_client - Client connection to Project AirSim
                    ros_topics_manager - ROS topics manager
                """
                
                ros_topic_name_ = generate_ros_topic(None, camera_path)
                
                self.refs = 1  # Number of "references" to this entry
                self.projectairsim_client = (
                    projectairsim_client
                )  # Project AirSim client
                self.desired_pose_message_callback = (
                    desired_pose_message_callback  # ROS message conversion callback
                )
                self.ros_topic_name = (
                    ros_topic_name_ + "/desired_pose"
                )  # ROS topic name we subscribe to
                self.ros_topics_manager = ros_topics_manager  # ROS topics manager
                self.service_method_set_pose = (
                    utils.get_sensor_path(camera_path) + "/SetPose"
                )  # Project Airsim service method to set camera pose

                # Subscribe to the ROS topic
                self.ros_topics_manager.add_subscriber(
                    self.ros_topic_name,
                    rosgeommsg.PoseStamped,
                    self._ros_topic_update_cb,
                )

            def __del__(self):
                """
                Destructor.
                """
                self.clear()

            def clear(self):
                """
                Stop processing and free resources.
                """
                if self.ros_topic_name is not None:
                    self.ros_topics_manager.remove_subscriber(
                        self.ros_topic_name, self._ros_topic_update_cb
                    )
                    self.ros_topic_name = None

                self.projectairsim_client = None
                self.desired_pose_message_callback = None
                self.ros_topics_manager = None

            def _ros_topic_update_cb(self, ros_topic_name, ros_message_data):
                """
                Handle the ROS message to set the camera pose.


                Arguments:
                    ros_topic_name - ROS topic name
                    ros_message_data - ROS topic message
                """
                if self.ros_topic_name:
                    pose = self.desired_pose_message_callback(
                        ros_topic_name, ros_message_data
                    )
                    if pose is not None:
                        # The Project AirSim service method expects a Pose/Transform
                        # object which uses different field names than the Pose
                        # message used by Project AirSim topics like the robot's
                        # "desired_pose".  We don't bother popping the old keys since
                        # they're ignored by Pose().
                        pose["translation"] = pose["position"]
                        pose["rotation"] = pose["orientation"]
                        pose = projectairsim.types.Pose(pose)

                        # Call set_pose service method directly so we don't need World and Drone objects
                        self.projectairsim_client.request(
                            {
                                "method": self.service_method_set_pose,
                                "params": {"pose": pose},
                                "version": 1.0,
                            }
                        )

        # -------------------------------------------------------------------------
        # CameraBridgeToROS Methods
        # -------------------------------------------------------------------------
        def __init__(self):
            """
            Constructor.
            """
            self.cameras = {}

        def add_camera(
            self,
            camera_path: str,
            desired_pose_message_callback,
            topics_managers: TopicsManagers,
        ):
            """
            Adds handling for the specified camera.  If we're already handling
            the specified camera, we just increment the ref count.

            Arguments:
                camera_path - Project AirSim topic path for the camera
                desired_pose_message_callback - ROS message converter
                topics_managers - Topics and transform managers
            """
            if camera_path in self.cameras:
                # Entry already exists, just increment the ref count
                self.cameras[camera_path].refs += 1
            else:
                # New camera path, new entry
                self.cameras[camera_path] = self.CameraEntry(
                    camera_path,
                    desired_pose_message_callback,
                    topics_managers.projectairsim_topics_manager.projectairsim_client,
                    topics_managers.ros_topics_manager,
                )

        def remove_camera(self, camera_path: str):
            """
            Decrement ref count for the specified camera.  We stop handling
            the camera when the ref count drops to zero.

            Arguments:
                camera_path - Project AirSim topic path for the camera
            """
            if camera_path in self.cameras:
                camera_entry = self.cameras[camera_path]
                camera_entry.refs -= 1

                # If no more refs, delete the entry
                if camera_entry.refs <= 0:
                    camera_entry.clear()
                    del self.cameras[camera_path]

    # -------------------------------------------------------------------------
    # CameraBridgeToROS Class-Wide Data
    # -------------------------------------------------------------------------

    # Singleton to manage the "desired_pose" topic for camera sensors
    _desired_pose_bridge_from_ros = DesiredPoseBridgeFromROS()

    # -------------------------------------------------------------------------
    # CameraBridgeToROS Methods
    # -------------------------------------------------------------------------
    def __init__(
        self,
        projectairsim_topic_name: str,
        ros_message_type,
        topics_managers: TopicsManagers,
        image_message_callback,
        desired_pose_message_callback,
        ros_topic_is_latching: bool = False,
        frame_id: str = None,
        frame_id_parent: str = "map",
        transform_params: dict = None,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        "image_message_callback" converts the image from the Project AirSim topic
        message to an ROS image message.  It can return None if it handles the
        the Project AirSim message entirely by itself.

        "desired_pose_message_callback" converts the ROS topic message into an
        Project AirSim PoseMessage structure like that for the robot's
        "desired_pose" topic.  It can return None if it handles the ROS
        message entirely by itself.

        Standard Arguments:
            projectairsim_topic_name - Name of source Project AirSim topic
            ros_message_type - The data type of the ROS topic messages
            topics_managers - Topic and transform managers

        Class-Specific Arguments:
            image_message_callback - Image message handler callback function (inherited from BasicBridgeToROS)
            desired_pose_message_callback - Pose message handler callback function
            ros_topic_is_latching - If true, new subscribers of the ROS topic
                receive the last message published  (inherited from BasicBridgeToROS)
            frame_id - The name of the transform frame to broadcast; if None,
                the robot name is used, derived from the Project AirSim topic name
            frame_id_parent - The name of the parent frame
        """
        if not callable(image_message_callback):
            raise TypeError(
                f"image_message_callback is not callable: {image_message_callback}"
            )
        if not callable(desired_pose_message_callback):
            raise TypeError(
                f"desired_pose_message_callback is not callable: {desired_pose_message_callback}"
            )

        if not projectairsim_topic_name.endswith("_camera"):
            raise ValueError(
                f'{__class__} can only be used for *_camera topics only, not "{projectairsim_topic_name}"'
            )
        
        ros_topic_name_camera_image_ = generate_ros_topic(None, projectairsim_topic_name)

        # ROS version-specific sensor message helper
        self.sensor_helper = topics_managers.ros_node.create_sensor_helper()

        self.ros_topic_name_camera_image = (
            ros_topic_name_camera_image_ + "/image"
        )  # ROS topic name for camera image

        # Setup the camera transform frame
        if frame_id is not None:
            if not frame_id:
                raise ValueError("frame_id can't be an empty string")
            self.frame_id = frame_id
        else:
            sensor_name = utils.get_sensor_name_id(projectairsim_topic_name)
            self.frame_id = utils.get_robot_frame_id(projectairsim_topic_name) + "/" + sensor_name + "_body"

        
        if frame_id_parent is not None:
            if not frame_id_parent:
                raise ValueError("frame_id can't be an empty string")
            self.frame_id_parent = frame_id_parent
        else:
            frame_id_parent = utils.get_robot_frame_id(projectairsim_topic_name)   
            
        self.frame_id_parent = frame_id_parent
        self.transform = (
            rosgeommsg.Transform()
        )  # Cache of camera's last known transform for ROS
        topics_managers.tf_broadcaster.add_frame(self.frame_id, self.frame_id_parent)
        self.set_static_transforms(topics_managers, projectairsim_topic_name, transform_params)

        # Initialize the super class with the image topic
        super().__init__(
            projectairsim_topic_name=projectairsim_topic_name,
            ros_message_type=ros_message_type,
            topics_managers=topics_managers,
            message_callback=image_message_callback,
            ros_topic_is_latching=ros_topic_is_latching,
            ros_topic_name=self.ros_topic_name_camera_image,
            append_ros_topic_name=False,
            logger = logger,
        )

        # Setup the camera info topic info
        self.ros_topic_name_camera_info = (
            ros_topic_name_camera_image_ + "/camera_info"
        )  # ROS topic name for camera info
        self.camera_info = (
            rossensmsg.CameraInfo()
        )

        self.camera_info.width =  transform_params.get('width', 0)
        self.camera_info.height =  transform_params.get('height', 0)

          # Cache of last known camera info for ROS
        self.auto_subscriber_camera_info = AutoSubscriber(
            projectairsim_topic_name + "_info",
            self._projectairsim_topic_update_camera_info_cb,
            topics_managers.projectairsim_topics_manager,
        )

        # Advertise the camera info topic with peer change linked to camera info auto-subscriber
        topics_managers.ros_topics_manager.add_publisher(
            self.ros_topic_name_camera_info,
            rossensmsg.CameraInfo,
            self.auto_subscriber_camera_info.peer_change_cb,
            True,
        )

        self.ros_image = Image()

        # Setup the camera pointcloud topic info
        depth_camera_list = ["depth_planar_camera"]#, "depth_vis_camera"]#, "disparity_normalized_camera", "depth_camera", "depth_perspective"]

        for word in depth_camera_list:
            if word in projectairsim_topic_name:
                self.ros_topic_name_camera_points = (
                    ros_topic_name_camera_image_ + "/points"
                )  # ROS topic name for camera pointcloud

                self.ros_topic_name_camera_points_worldview = (
                    self.ros_topic_name_camera_points + "_worldview"
                )  # ROS topic name for camera pointcloud

                self.camera_points = (
                    rossensmsg.PointCloud2()
                )  # Cache of last known camera pointcloud for ROS

                self._basic_ros_subscriber_points  = ROSAutoSubscriber(
                    self.ros_topic_name_camera_points,
                    self._projectairsim_topic_update_camera_points_cb,
                    self.ros_camera_info_callback,
                    topics_managers.ros_topics_manager,
                )

                qos_profile = self._create_qos_profile()
                # Advertise the camera pointcloud topic with peer change linked to camera info auto-subscriber
                topics_managers.ros_topics_manager.add_publisher(
                    self.ros_topic_name_camera_points,
                    rossensmsg.PointCloud2,
                    self._basic_ros_subscriber_points.peer_change_cb,
                    is_latching=True,
                    ros_qos_profile=qos_profile
                ) 

                topics_managers.ros_topics_manager.add_publisher(
                    self.ros_topic_name_camera_points_worldview,
                    rossensmsg.PointCloud2,
                    self._basic_ros_subscriber_points.peer_change_cb,
                    is_latching=True,
                    ros_qos_profile=qos_profile
                ) 

        # Subscribe to ROS camera desired pose topic
        self._camera_path = utils.get_sensor_path(projectairsim_topic_name)
        self._desired_pose_bridge_from_ros.add_camera(
            self._camera_path, desired_pose_message_callback, self.topics_managers
        )

    def set_static_transforms(
            self,
            topics_managers,
            projectairsim_topic_name,
            transform_params: dict = None 
        ):
            #self.static_parent_frame_id = utils.get_robot_frame_id(projectairsim_topic_name)    
            self.static_parent_frame_id = utils.get_robot_frame_id(projectairsim_topic_name) + "/odom_local_ned"

            self.static_transform = (
                rosgeommsg.Transform()
            )  # Cache of camera's last known transform for ROS        

            self.optical_static_transform = (
                rosgeommsg.Transform()
            )  # Cache of camera's last known transform for ROS     

            #set camera _body/static frame
            sensor_name = utils.get_sensor_name_id(projectairsim_topic_name)
            self.static_frame_id = utils.get_robot_frame_id(projectairsim_topic_name) + "/" + sensor_name + "_body/static"
            self.optical_static_frame_id = utils.get_robot_frame_id(projectairsim_topic_name) + "/" + sensor_name + "_optical/static"

            topics_managers.tf_broadcaster.add_frame(self.static_frame_id, self.static_parent_frame_id)
            topics_managers.tf_broadcaster.add_frame(self.optical_static_frame_id, self.static_parent_frame_id)

            if transform_params is None:
                transform_params = {
                    'x_coord': 0,
                    'y_coord': 0,
                    'z_coord': 0,
                    'roll': 0,
                    'pitch': 0,
                    'yaw': 0,
                }
            (
                self.static_transform.translation.x,
                self.static_transform.translation.y,
                self.static_transform.translation.z,
            ) = utils.to_ros_position_list2list(
                (
                    (float)(transform_params.get('x_coord', 0)),
                    (float)(transform_params.get('y_coord', 0)),
                    (float)(transform_params.get('z_coord', 0)),
                )
            )

            q = quaternion_from_euler((float)(transform_params.get('roll', 0)), (float)(transform_params.get('pitch', 0)), (float)(transform_params.get('yaw', 0)))
            (
                self.static_transform.rotation.x,
                self.static_transform.rotation.y,
                self.static_transform.rotation.z,
                self.static_transform.rotation.w,
            ) = utils.to_ros_quaternion_list2list(
                (
                    (float)(q[0]),
                    (float)(q[1]),
                    (float)(q[2]),
                    (float)(q[3]),
                )
            )

            self.optical_static_transform = utils.get_camera_optical_tf_from_body_tf(self.static_transform)

            topics_managers.tf_broadcaster.set_frame(self.static_frame_id, self.static_transform, topics_managers.ros_node.get_time_now())
            topics_managers.tf_broadcaster.set_frame(self.optical_static_frame_id, self.optical_static_transform, topics_managers.ros_node.get_time_now())        

    def _create_qos_profile(self):
        """Create a QoS profile with the required settings."""
        qos_profile = QoSProfile(
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=50,  # Keep the last 1 message
            reliability=QoSReliabilityPolicy.RELIABLE
        )
        qos_profile.durability = QoSDurabilityPolicy.TRANSIENT_LOCAL
        return qos_profile          

    def clear(self):
        """
        Stop handling messages and free resources.
        """
        super().clear()

        if self.frame_id is not None:
            self.topics_managers.tf_broadcaster.remove_frame(self.frame_id)
            self.frame_id = None

        # Stop subscribing to Project AirSim topic first
        if self.auto_subscriber_camera_info is not None:
            self.auto_subscriber_camera_info.unsubscribe()

        # Stop subscribing to the camera's "desired_pose" topic
        if self._camera_path is not None:
            self._desired_pose_bridge_from_ros.remove_camera(self._camera_path)
            self._camera_path = None

        # Stop publishing the ROS topic (and unregister the auto-subscriber)
        if self.ros_topic_name_camera_info is not None:
            self.topics_managers.ros_topics_manager.remove_publisher(
                self.ros_topic_name_camera_info,
                self.auto_subscriber_camera_info.peer_change_cb,
            )
            self.ros_topic_name_camera_info = None

        # Drop the camera info auto-subscriber
        self.auto_subscriber_camera_info = None

    def _projectairsim_topic_update_cb(
        self, projectairsim_topic, projectairsim_message_data
    ):
        """
        Handle a new camera image message published to the Project AirSim topic.

        We convert the message to the equivalent ROS image message and if
        someone is subscribed to the ROS camera info topic, publish the camera
        info at the same time with the same timestamp.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            ros_image = self.message_callback(
                projectairsim_topic.path, projectairsim_message_data
            )
            ros_image.header.frame_id = self.frame_id

            (
                self.transform.translation.x,
                self.transform.translation.y,
                self.transform.translation.z,
            ) = utils.to_ros_position_list2list(
                (
                    (float)(projectairsim_message_data["pos_x"]),
                    (float)(projectairsim_message_data["pos_y"]),
                    (float)(projectairsim_message_data["pos_z"]),
                )
            )

            (
                self.transform.rotation.x,
                self.transform.rotation.y,
                self.transform.rotation.z,
                self.transform.rotation.w,
            ) = utils.to_ros_quaternion_list2list(
                (
                    (float)(projectairsim_message_data["rot_x"]),
                    (float)(projectairsim_message_data["rot_y"]),
                    (float)(projectairsim_message_data["rot_z"]),
                    (float)(projectairsim_message_data["rot_w"]),
                )
            )
            timestamp = self.topics_managers.ros_node.get_time_from_msg(ros_image.header.stamp)
            self.topics_managers.tf_broadcaster.set_frame(self.frame_id, self.transform, timestamp)

            self.topics_managers.ros_topics_manager.publish(
                self.ros_topic_name_camera_image, ros_image
            )
            if (
                self.auto_subscriber_camera_info
                and self.auto_subscriber_camera_info.is_subscribed
            ):
                # Need to publish the camera info too using the same header
                self.camera_info.header = ros_image.header
                self.topics_managers.ros_topics_manager.publish(
                    self.ros_topic_name_camera_info, self.camera_info
                )

    def _projectairsim_topic_update_camera_info_cb(
        self, projectairsim_topic, projectairsim_message_data
    ):
        """
        Handle a new camera info message published to the Project AirSim topic.

        We convert the message to the equivalent ROS camera info but if
        someone is subscribed to any of the ROS image topics, we don't publish
        immediately until we receive an image update.  That way the camera
        info and the images will all have the same timestamp.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            self._convert_projectairsim_camera_info_to_ros(projectairsim_message_data)

            if not self._auto_subscriber.is_subscribed:
                # No subscribers to image topic, update the ROS camera info topic now
                self.topics_managers.ros_topics_manager.publish(
                    self.ros_topic_name_camera_info, self.camera_info
                )

    def _convert_projectairsim_camera_info_to_ros(self, projectairsim_camera_info):
        """
        Convert the Project AirSim camera info message to the equivalent ROS camera
        info.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_camera_info - Project AirSim camera info topic message
        """
        self.camera_info.header.stamp = self.topics_managers.ros_node.get_time_now_msg()
        self.camera_info.header.frame_id = self.frame_id

        self.camera_info.width = projectairsim_camera_info["width"]
        self.camera_info.height = projectairsim_camera_info["height"]
        self.camera_info.distortion_model = projectairsim_camera_info[
            "distortion_model"
        ]
        self.sensor_helper.set_camera_info(
            self.camera_info,
            projectairsim_camera_info["distortion_params"],
            projectairsim_camera_info["intrinsic_camera_matrix"],
            projectairsim_camera_info["rectification_matrix"],
            projectairsim_camera_info["projection_matrix"],
        )

    def _projectairsim_topic_update_camera_points_cb(
            self, projectairsim_topic, projectairsim_message_data
        ):
            if self.camera_info is None or self.ros_image is None:
                return
            if self.ros_topic_name:
                self._convert_projectairsim_camera_points_to_ros(projectairsim_message_data)

            if (
                hasattr(self, '_basic_ros_subscriber_points') and self.camera_points.width > 0
            ):
                # Need to publish the camera info too using the same header
                #self.camera_points.header = ros_image.header
                self.topics_managers.ros_topics_manager.publish(
                    self.ros_topic_name_camera_points, self.camera_points
                )
                transformed_points_map = self._transform_points_to_frame(self.camera_points, "map")
                self.topics_managers.ros_topics_manager.publish(
                    self.ros_topic_name_camera_points_worldview, transformed_points_map
                ) 

    def _convert_projectairsim_camera_points_to_ros(self, msg):
        if self.camera_info.width == 0:
            return

        #set pointcloud parameters
        pc_msg = PointCloud2()
        pc_msg.width = self.camera_info.width
        pc_msg.height = self.camera_info.height
        header = msg.header       

        #set intrinsic parameters
        width = self.camera_info.width
        height = self.camera_info.height
        fx = self.camera_info.k[0]
        fy = self.camera_info.k[4]
        cx = self.camera_info.k[2]
        cy = self.camera_info.k[5]
        intrinsic = o3d.camera.PinholeCameraIntrinsic(width, height, fx, fy, cx, cy)

        self.logger.debug(
            f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge intrinsic {width} {height} {fx} {fy} {cx} {cy}"
        )

        self.logger.debug(
            f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge intrinsic {intrinsic.intrinsic_matrix}"
        )

        #determines density of samples/points. In this case, we use every point from the depth cloud
        stride = 1

        self.logger.debug(
            f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge"
        )
 
        try:
            bridge = CvBridge()
            cv_image = bridge.imgmsg_to_cv2(msg, desired_encoding="passthrough")
        except CvBridgeError as e:
            self.logger.debug(
                f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge {e}"
            )
        except Exception as e:
            self.logger.debug(
                f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge generic {e}"
            )
        finally:
            self.logger.debug(
                f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge cv init completed for type {msg.encoding}"
            )

        try:
            #convert depth_vis_camera to pointcloud
            if msg.encoding == "bgr8":
                cv_image = bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
                depth_raw = o3d.geometry.Image(cv_image)

                cv_image_rgb = cv2.cvtColor(cv_image, cv2.COLOR_BGR2RGB)
                color_raw = o3d.geometry.Image(cv_image_rgb)

                rgbd_image = o3d.geometry.RGBDImage.create_from_color_and_depth(
                    color_raw,
                    depth_raw,
                    depth_scale=1.0,  # Adjust based on your depth image format
                    depth_trunc=255.0,  # Adjust as needed
                    convert_rgb_to_intensity=False
                )

                pcd = o3d.geometry.PointCloud.create_from_rgbd_image(
                    rgbd_image,
                    intrinsic
                )

            #convert depth_planar to pointcloud
            elif msg.encoding == "mono8":    
                self.logger.debug(
                    f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge mono8"
                )

                try:
                    cv_image = cv2.imread("/home/jnovak/testimage_edited.png", cv2.IMREAD_GRAYSCALE)
                except Exception as e:
                    self.logger.debug(
                        f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros read image {e}"
                    )
                finally:
                    self.logger.debug(
                        f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros read successful"
                    )
                        
                depth_image = o3d.geometry.Image(cv_image.astype(np.uint16))

                self.logger.debug(
                    f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge mono8 after o3d image constructor"
                )

                #Create point cloud from depth image
                pcd = o3d.geometry.PointCloud.create_from_depth_image(
                    depth_image,
                    intrinsic,
                    depth_scale=1.0,  # Adjust based on your depth image format
                    depth_trunc=255.0,  # Adjust as needed
                    stride=1,
                )

            elif msg.encoding == "32FC1":    
                depth_image = o3d.geometry.Image(cv_image.astype(np.float32))

                #Create point cloud from depth image
                pcd = o3d.geometry.PointCloud.create_from_depth_image(
                    depth_image,
                    intrinsic,
                    depth_scale=1.0,  # Adjust based on your depth image format
                    depth_trunc=65504.0,  # Adjust as needed
                    stride=1,
                )

            # Apply rotation to align the point cloud
            rotation_matrix = self.get_rotation_matrix_90_degrees()
            self.apply_rotation_to_point_cloud(pcd, rotation_matrix)

        except CvBridgeError as e:
            self.logger.debug(
                f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge {e}"
            )
        except Exception as e:
            self.logger.debug(
                f"CameraBridgeToROS._convert_projectairsim_camera_points_to_ros cvbridge generic {e}"
            )

        self.camera_points = self.convert_depth_to_points(pcd, header)  

    def get_rotation_matrix_90_degrees(self):
        return np.array([
            [0, 0, 1],
            [-1, 0, 0],
            [0, -1, 0]
        ])        

    #rotate pointcloud 90 degrees normal to z dimension
    def apply_rotation_to_point_cloud(self, pcd, rotation_matrix):
        points = np.asarray(pcd.points)
        points_rotated = np.dot(points, rotation_matrix.T)
        pcd.points = o3d.utility.Vector3dVector(points_rotated)    

    def convert_depth_to_points(self, pointcloud, header):
        points = np.asarray(pointcloud.points)
        if len(points) == 0:
            return PointCloud2()

        fields = [
            PointField(name='x', offset=0, datatype=PointField.FLOAT32, count=1),
            PointField(name='y', offset=4, datatype=PointField.FLOAT32, count=1),
            PointField(name='z', offset=8, datatype=PointField.FLOAT32, count=1),
        ]

        # Create PointCloud2 message
        pc2_msg = point_cloud2.create_cloud(header, fields, points)

        pc2_msg.is_dense = True
        pc2_msg.height = self.camera_info.height
        pc2_msg.width = self.camera_info.width
        pc2_msg.row_step = pc2_msg.point_step * pc2_msg.width

        return pc2_msg
    
    def _transform_points_to_frame(self, point_msg, transform_type):
        """Transforms a PointCloud2 message from the camera frame to the map frame using PoseStamped."""
        
        transform_stamped = self.topics_managers.tf_listener.get_latest_transform(point_msg.header.frame_id, transform_type, rclpy.time.Time())

        if transform_stamped is None:
            self.get_logger().warn(f'No transform available from {point_msg.header.frame_id} to {transform_type}')
            return point_msg # No transform available

        # Extract position and orientation from PoseStamped
        translation = np.array([
            transform_stamped.transform.translation.x,
            transform_stamped.transform.translation.y,
            transform_stamped.transform.translation.z
        ])

        quaternion = np.array([
            transform_stamped.transform.rotation.x,
            transform_stamped.transform.rotation.y,
            transform_stamped.transform.rotation.z,
            transform_stamped.transform.rotation.w
        ])

        # Convert quaternion to rotation matrix using scipy
        rotation_matrix = Rotation.from_quat(quaternion).as_matrix()

        # Convert PointCloud2 to a NumPy structured array
        points_structured = np.array(
            list(point_cloud2.read_points(point_msg, field_names=("x", "y", "z"), skip_nans=True))
        )

        # Extract x, y, z values and convert them to float32
        points = np.stack(
            [points_structured['x'], points_structured['y'], points_structured['z']], axis=-1
        ).astype(np.float32)

        if points.shape[0] == 0:
            return None  # No points to transform

        points = points.reshape(-1, 3)  # Ensure points are in the shape (N, 3)

        # Ensure that the point cloud has the expected shape
        if points.shape[1] != 3:
            raise ValueError(f"PointCloud has an unexpected shape: {points.shape}")

        if points.size == 0:
            return point_msg
        
        # Transform the points to the target frame by applying inverse rotation and translation
        transformed_points = np.dot(points, rotation_matrix.T) + translation

        # Convert the transformed points back to PointCloud2 format
        fields = [
            PointField(name="x", offset=0, datatype=PointField.FLOAT32, count=1),
            PointField(name="y", offset=4, datatype=PointField.FLOAT32, count=1),
            PointField(name="z", offset=8, datatype=PointField.FLOAT32, count=1),
        ]

        # Create PointCloud2 message
        transformed_cloud = PointCloud2()
        transformed_cloud.header.frame_id = transform_type
        transformed_cloud.header.stamp = point_msg.header.stamp
        transformed_cloud = point_cloud2.create_cloud(transformed_cloud.header, fields, transformed_points)

        transformed_cloud.is_dense = True
        transformed_cloud.height = self.camera_info.height
        transformed_cloud.width = self.camera_info.width
        transformed_cloud.row_step = transformed_cloud.point_step * transformed_cloud.width

        return transformed_cloud

    #ros camera_info topic needs to be subscribed in order for pointcloud to work
    #callback is required as part of subscription
    def ros_camera_info_callback(self, ros_topic, camera_info):
        self.camera_info = camera_info
        return

# --------------------------------------------------------------------------
class RobotPoseBridgeFromROS:
    class Drone(projectairsim.Drone):
        """
        Subclass of projectairsim.Drone when a World object is not available.

        Methods that take a sensor name must not be called but methods for
        the camera sensor and others such as flight methods may be used.
        """

        def __init__(self, client: ProjectAirSimClient, robot_path: str):
            self.parent_topic = robot_path
            super().__init__(
                client=client, world=None, robot_name=utils.get_robot_name(robot_path)
            )

        def log_topics(self):
            pass

        def set_topics(self, world: projectairsim.World):
            self.sensors_topic = f"{self.parent_topic}"
            self.set_robot_info_topics()

        def get_barometer_data(self, sensor_name) -> Dict:
            raise NotImplementedError()

        def get_gps_data(self, sensor_name) -> Dict:
            raise NotImplementedError()

        def get_imu_data(self, sensor_name) -> Dict:
            raise NotImplementedError()

        def get_magnetometer_data(self, sensor_name) -> Dict:
            raise NotImplementedError()

    """
    This class uses the BasicBridgeFromROS class to handle the desired_pose
    and cmd_vel ROS pose topics and sends messages to the appropriate Project
    AirSim topics or makes the appropriate Project AirSim service calls.

    This class must be instantiated for the ".../desired_pose" Project AirSim
    topic.
    """

    def __init__(
        self,
        projectairsim_topic_name: str,
        ros_message_type,
        topics_managers: TopicsManagers,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        Standard Arguments:
            projectairsim_topic_name - Name of source Project AirSim topic
            ros_message_type - Ignored
            topics_managers - Topic and transform managers
        """
        if not projectairsim_topic_name.endswith("/desired_pose"):
            raise ValueError(
                f'{__class__} can only be used for ".../desired_pose" topics only, not "{projectairsim_topic_name}"'
            )

        robot_path = utils.get_robot_path(projectairsim_topic_name)
        self.method_path_move_by_velocity = robot_path + "/MoveByVelocity"

        self.topics_managers = topics_managers

        # Create bridge from ROS to Project AirSim desired_pose topics
        self._basic_bridge_from_ros_desired_pose = BasicBridgeFromROS(
            projectairsim_topic_name,
            rosgeommsg.PoseStamped,
            topics_managers,
            self.convert_desired_pose_from_ros,
        )

        # Create handler for ROS cmd_vel topic
        self._basic_ros_subscriber_cmd_vel = BasicROSSubscriber(
            ros_topic_name=robot_path + "/cmd_vel",
            ros_message_type=rosgeommsg.Twist,
            topics_managers=topics_managers,
            message_callback=self.handle_ros_cmd_vel,
        )

    def clear(self):
        """
        Stop handling messages and free resources.
        """
        self._basic_bridge_from_ros_desired_pose.clear()
        self._basic_ros_subscriber_cmd_vel.clear()

    def convert_desired_pose_from_ros(self, ros_topic_name, ros_posestamped):
        """
        Handle a new ROS robot pose request.  This handler converts the pose from
        the ROS Pose message into a Project AirSim pose message.

        Arguments:
            topic_sub_handler - Topic handler for this topic
            ros_topic - The ROS topic name
            ros_pose - The vehicle pose received from the ROS topic

        Return:
            (return) - Corresponding Project AirSim Pose object
        """
        projectairsim_pose = {
            "position": utils.to_projectairsim_position(ros_posestamped.pose.position),
            "orientation": utils.to_projectairsim_quaternion(
                ros_posestamped.pose.orientation
            ),
        }
        return projectairsim_pose

    def handle_ros_cmd_vel(self, ros_topic_name: str, ros_twist: rosgeommsg.Twist):
        projectairsim_client = (
            self.topics_managers.projectairsim_topics_manager.projectairsim_client
        )
        projectairsim_velocity = utils.to_projectairsim_position(ros_twist.linear)
        projectairsim_angular_rotation = utils.to_projectairsim_angular_rotation(
            ros_twist.angular
        )

        # Call MoveByVelocity service method directly so we don't need World and Drone objects
        self.topics_managers.logger.info(f"Setting velocity {projectairsim_velocity}")
        projectairsim_client.request(
            {
                "method": self.method_path_move_by_velocity,
                "params": {
                    "vx": projectairsim_velocity["x"],
                    "vy": projectairsim_velocity["y"],
                    "vz": projectairsim_velocity["z"],
                    "duration": 1.0,  # Timeout to failsafe--updates must be received more frequent than this
                    "drivetrain": projectairsim.drone.YawControlMode.MaxDegreeOfFreedom,
                    "yaw_is_rate": True,
                    "yaw": projectairsim_angular_rotation["z"],
                },
                "version": 1.0,
            }
        )


# --------------------------------------------------------------------------
class RobotPoseBridgeToROS(BasicBridgeToROS):
    """
    Based on BasicBridgeToROS, this class handles publishing a pose received
    from an Project AirSim pose topic and also broadcasts a matching transform
    frame.  A message handler callback function is still required to perform
    the message conversion.
    """

    def __init__(
        self,
        projectairsim_topic_name: str,
        ros_message_type,
        transform_message_callback,
        topics_managers: TopicsManagers,
        message_callback,
        frame_id_parent: str,
        ros_topic_name: str = None,
        ros_topic_is_latching: bool = True,
        frame_id: str = None,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        Standard Arguments:
            projectairsim_topic_name - Name of source Project AirSim topic
            ros_message_type - The data type of the ROS topic messages
            topics_managers - Topic and transform managers

        Class-Specific Arguments:
            ros_topic_is_latching - If true, new subscribers of the ROS topic
                receive the last message published  (inherited from BasicBridgeToROS)
            message_callback - Message handler callback function (inherited from BasicBridgeToROS)
            frame_id - The name of the transform frame to broadcast; if None,
                the robot name is used, derived from the Project AirSim topic name
            frame_id_parent - The name of the parent frame
        """
        super().__init__(
            projectairsim_topic_name=projectairsim_topic_name,
            ros_message_type=ros_message_type,
            topics_managers=topics_managers,
            message_callback=message_callback,
            ros_topic_name=ros_topic_name,
            ros_topic_is_latching=ros_topic_is_latching,
            logger = logger,
        )

        if not callable(transform_message_callback):
            raise TypeError(
                f"transform_message_callback is not callable: {transform_message_callback}"
            )
        self.transform_message_callback = transform_message_callback

        self.is_subscribed_to_projectairsim_topic = False
        self.frame_id_parent = frame_id_parent
        self.transform = rosgeommsg.Transform()

        # Get ID of transform frame corresponding to the pose and add the frame to the broadcaster
        if frame_id:
            self.frame_id = frame_id
        else:
            self.frame_id = utils.get_robot_frame_id(projectairsim_topic_name)

            if "gt_kinematics" in projectairsim_topic_name:
                self.frame_id = utils.get_robot_frame_id(projectairsim_topic_name) + "/odom_local_ned"
            if not self.frame_id:
                raise RuntimeError(
                    "Unable to derive the robot name from the Project AirSim topic name--unexpected format?"
                )

        # Add the transform frame
        topics_managers.tf_broadcaster.add_frame(self.frame_id, self.frame_id_parent)

        # Subscribe to Project AirSim topic now since we need to constantly update the transform frame
        self._auto_subscriber.subscribe()

    def clear(self):
        """
        Stop handling messages and free resources.
        """
        super().clear()
        if self.frame_id is not None:
            self.topics_managers.tf_broadcaster.remove_frame(self.frame_id)
            self.frame_id = None

    def _projectairsim_topic_update_cb(
        self, projectairsim_topic, projectairsim_message_data
    ):
        """
        Handle a new message published to the Project AirSim topic.

        The message callback function is invoked with the new message.  The
        the callback function must return a ROS PoseStamped object which is
        published to the ROS topic and frame transform.

        The callback function must not return None.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            posestamped = self.message_callback(
                projectairsim_topic.path, projectairsim_message_data
            )

            # If not specified, this pose is relative to the parent transform frame
            if (posestamped.header.frame_id is None) or (
                not posestamped.header.frame_id
            ):
                posestamped.header.frame_id = self.frame_id_parent

            # Publish to the ROS topic
            self.topics_managers.ros_topics_manager.publish(
                self.ros_topic_name, posestamped
            )

            # Update transform frame
            # Calculate sensor transform frame
            self.transform = self.transform_message_callback(
                projectairsim_message_data
            )
            
            self.topics_managers.tf_broadcaster.set_frame(
                frame_id=self.frame_id,
                transform=self.transform,
                timevalue=self.topics_managers.ros_node.get_time_from_msg(
                    posestamped.header.stamp
                ),
            )

    def _ros_topic_peer_change_cb(self, ros_topic_name, num_peer):
        """
        Handle a peer subscribing to or unsubscribing from our ROS topic.

        Since we want to be always subscribed to the Project AirSim topic,
        we want to ignore the change in number of peers.

        Arguments:
            ros_topic_name - Name of the ROS topic
            num_peer - Number of peers now subscribed to the ROS topic
        """
        pass

class RobotKinematicsBridgeToROS(BasicBridgeToROS):
    """
    Handles publishing a robot's kinematic state received
    from a Project AirSim kinematics topic and also broadcasts a matching transform
    frame.  A message handler callback function is still required to perform
    the message conversion.
    """

    def _projectairsim_topic_update_cb(
        self, projectairsim_topic, projectairsim_message_data
    ):
        """
        Handle a new message published to the Project AirSim topic.

        The message callback function is invoked with the new message.  The
        the callback function must return a ROS PoseStamped object which is
        published to the ROS topic and frame transform.

        The callback function must not return None.

        Arguments:
            projectairsim_topic - Project AirSim topic info
            projectairsim_message_data - Message published to the Project AirSim topic
        """
        if self.ros_topic_name:
            kinematics = self.message_callback(
                projectairsim_topic.path, projectairsim_message_data
            )

            # If not specified, this pose is relative to the parent transform frame
            # if (kinematics.header.frame_id is None) or (
            #     not kinematics.header.frame_id
            # ):
            #     kinematics.header.frame_id = self.frame_id_parent
            match kinematics.header.frame_id:
                case None:
                    kinematics.header.frame_id = utils.get_robot_frame_id(self.projectairsim_topic_name)
                case _:
                    kinematics.header.frame_id = self.frame_id_parent

            # Publish to the ROS topic
            self.topics_managers.ros_topics_manager.publish(
                self.ros_topic_name, kinematics
            )

            # Update transform frame
            self.transform.translation = rosgeommsg.Vector3(
                x=kinematics.pose.pose.position.x,
                y=kinematics.pose.pose.position.y,
                z=kinematics.pose.pose.position.z,
            )
            self.transform.rotation = kinematics.pose.pose.orientation
            self.topics_managers.tf_broadcaster.set_frame(
                frame_id=self.frame_id,
                transform=self.transform,
                timevalue=self.topics_managers.ros_node.get_time_from_msg(
                    kinematics.header.stamp
                ),
            )

class BasicROSTimer:
    """
        This class handles callbacks associated with a specific timer. For a 
        list of timers, frequency is used as the dict key, and callback is/are the values
    """

    def __init__(
        self,
        topics_managers: TopicsManagers,
        publish_rate_hz: float = 300.0,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        Arguments:
        ros_topic_name - Name of the ROS topic to publish the time to.
        publish_rate_hz - The rate at which to publish the time (in Hz).
        """
        
        #initialize the publisher
        self.topics_managers = topics_managers
        self.publish_rate_hz = publish_rate_hz
        self.timer = None

        self.add_timer()

        # Set up a timer to call the publish function at the desired rate
        #self.timer = self.create_timer(1.0 / publish_rate_hz, self.timer_callback)
        
    def add_timer(self):
        self.timer = self.topics_managers.ros_topics_manager.add_timer(
            period_sec=1.0 / self.publish_rate_hz,
            topic_callback=self._timer_callback,
        )

    def __del__(self):
        """
        Destructor.
        """
        self.clear()        

    #finish this up better
    def clear(self):
        """
        Stop handling messages and free resources.
        """
        if self.timer:
            if self.publish_rate_hz:
                self.topics_managers.ros_topics_manager.remove_timer(
                    period_sec = 1.0 / self.publish_rate_hz,
                    topic_callback = self._timer_callback,
                )
                self.timer = None
        self.topics_managers = None

    def _timer_callback(self):
        """
        Timer callback function that is called at frequency of publish_rate_hz
        """
        pass

# --------------------------------------------------------------------------
class SimClockBridgeToROS(BasicROSTimer):
    """
        This class handles publishing the current time to the /clock topic at a specified rate.
    """

    def __init__(
        self,
        projectairsim_world,
        topics_managers: TopicsManagers,
        ros_topic_name: str = '/clock',
        publish_rate_hz: float = 300.0, 
        ros_topic_is_latching: bool = True,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        Arguments:
        ros_topic_name - Name of the ROS topic to publish the time to.
        publish_rate_hz - The rate at which to publish the time (in Hz).
        """
        
        # Initialize the super class with the clock frequency
        super().__init__(
            topics_managers=topics_managers,
            publish_rate_hz=publish_rate_hz, 
        )
        
        self.ros_topic_name = ros_topic_name
        self.publish_rate_hz = publish_rate_hz

        topics_managers.ros_topics_manager.add_publisher(
            topic_name=ros_topic_name,
            ros_message_type=Clock,
            is_latching=ros_topic_is_latching,
        )
        
        self.timer = None
        self.add_timer()

    # Set up a timer to call the publish function at the desired rate
    def add_timer(self):
        period = 1.0 / self.publish_rate_hz
        self.timer = self.topics_managers.ros_topics_manager.add_timer(
            period_sec=period,
            topic_callback=self._timer_callback,
        )

    def clear(self):
        # Stop publishing the ROS topi
        if self.timer:
            self.topics_managers.ros_topics_manager.remove_timer(
                period_sec = 1.0 / self.publish_rate_hz, topic_callback = self._timer_callback
            )
            self.publish_rate_hz = None
            self.timer = None

        #self.topics_managers.ros_topics_manager.remove_publisher(self.ros_topic_name)
        super().clear()

    def _timer_callback(
        self
    ):
        """
        Timer callback function that publishes the current time to the /clock topic.
        """
        # Publish to the clock topic

        try:
            now = self.projectairsim_world.get_sim_time()
            sim_time = utils.to_ros_timestamp(now)
        
            clock_msg = Clock()
            clock_msg.clock = sim_time
        
            if sim_time is not None:
                        self.topics_managers.ros_topics_manager.publish(
                            self.ros_topic_name, clock_msg
                        )
        except Exception as e:
            sim_time = utils.to_ros_timestamp(0)

class ROSAutoSubscriber:
    """
    This class handles subscribing to an Project AirSim topic automatically
    when a peer subscribes to the corresponding ROS topic.
    The caller call the instance method peer_change_cb() when the ROS
    publisher's peer change callback is invoked.  This class won't
    subscribe to the Project AirSim topic until peer_change_cb() is called
    with a non-zero num_peer argument.
    """

    def __init__(
        self,
        ros_topic_name,
        ros_topic_callback,
        ros_camera_info_callback,
        ros_topics_manager: ROSTopicsManager,
    ):
        """
        Constructor.
        Arguments:
            projectairsim_topic_name - Name of the Project AirSim topic
            projectairsim_topic_callback - The callback function to invoke
                when a message is published to the Project AirSim topic
            airsim_topics_manager - Project AirSim topics manager
        """
        if not callable(ros_topic_callback):
            raise TypeError(
                f"projectairsim_topic_callback is not callable: {ros_topic_callback}"
            )

        self.is_subscribed = (
            False  # Whether we're currenly subscribed to the Project AirSim topic
        )
        self.ros_topic_name = (
            ros_topic_name # Name of the ROS topic
        )
        self.ros_topic_callback = (
            ros_topic_callback
        )  # Callback to invoke when a message is published to the Project AirSim topic
        
        self.ros_camera_info_callback = (
            ros_camera_info_callback
        )  # Callback to invoke when a message is published to the Project AirSim topic


        self.ros_topics_manager = (
            ros_topics_manager  # Project AirSim topics manager
        )

    def __del__(self):
        """
        Destructor.
        """
        self.clear()

    def clear(self):
        """
        Unsubscribe from the Project AirSim topic is necessary and free resources.
        """
        if self.is_subscribed:
            self.unsubscribe()
        self.ros_topic_callback = None
        self.projectairsim_topics_manager = None

    def peer_change_cb(self, ros_topic_name: str, num_peers: int):
        """
        Handles a change to the number of subscribers to the ROS topic.
        We subscribe to the Project AirSim topic when have someone subscribing
        to the ROS topic and unsubscribe when we don't.
        Typically the caller passes this method to ROSTopicManager.add_publisher()
        as the peer_change_callback argument.  If needed, the caller may
        instead register their only callback function and then explicitly
        call this method.
        Arguments:
            ros_topic_name - Name of the ROS topic
            num_peers - New number of subscribers to the ROS topic
        """
        if num_peers == 0:
            if self.is_subscribed:
                self.unsubscribe()
        else:
            if not self.is_subscribed:
                self.subscribe()

    def subscribe(self):
        ros_image_topic = self.ros_topic_name.replace("points", "image")
        self.ros_topics_manager.add_subscriber(
            ros_image_topic,
            rossensmsg.Image,
            self.ros_topic_callback,
        )

        self.is_subscribed = True

        ros_camera_info_topic = self.ros_topic_name.replace("points", "camera_info")
        self.ros_topics_manager.add_subscriber(
            ros_camera_info_topic,
            rossensmsg.CameraInfo,
            self.ros_camera_info_callback,
        )

    def unsubscribe(self):
        self.is_subscribed = False
        ros_image_topic = self.ros_topic_name.replace("points", "image")
        self.ros_topics_manager.remove_subscriber(
            ros_image_topic, self.ros_topic_callback
        )
        ros_camera_info_topic = self.ros_topic_name.replace("points", "camera_info")
        self.ros_topics_manager.remove_subscriber(
            ros_camera_info_topic, self.ros_camera_info_callback
        )
