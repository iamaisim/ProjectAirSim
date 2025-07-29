"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
ROS bridge for Project AirSim: Topic management, handlers, and helpers
"""

import logging
import traceback
from typing import Dict

import geometry_msgs.msg as rosgeommsg
import sensor_msgs.msg as rossensmsg

from . import utils
from .node import ROSNode
from .tf_helpers import TFBroadcaster

import projectairsim.types
import projectairsim
from projectairsim import ProjectAirSimClient


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

    def add_publisher(
        self,
        topic_name: str,
        ros_message_type: type,
        peer_change_callback=None,
        is_latching: bool = True,
        ros_queue_size: int = 1,
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
                ),
                "callbacks": ros_peer_change_callbacks,
                "num_peers": 0,
            }

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
        ros_topic_is_latching: bool = True,
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
            ros_topic_is_latching - If true, new subscribers of the ROS topic receive the last message published
        """
        if not callable(message_callback):
            raise TypeError(f"message_callback is not callable: {message_callback}")

        if ros_topic_name is None:
            ros_topic_name = projectairsim_topic_name

        self.projectairsim_topic_name = projectairsim_topic_name
        self.message_callback = message_callback  # Message handler callback
        self.ros_topic_name = ros_topic_name
        self.topics_managers = topics_managers  # Topics managers

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
            ros_topic_name = projectairsim_topic_name

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
            ros_topic_name = projectairsim_topic_name

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

        if ros_topic_name is None:
            ros_topic_name = projectairsim_topic_name

        # Setup the sensor transform frame
        if frame_id is not None:
            if not frame_id:
                raise ValueError("frame_id can't be an empty string")
            self.frame_id = frame_id
        else:
            self.frame_id = utils.get_sensor_frame_id(projectairsim_topic_name)
        self.frame_id_parent = frame_id_parent
        self.transform = (
            rosgeommsg.Transform()
        )  # Cache of sensor's last known transform for ROS
        topics_managers.tf_broadcaster.add_frame(self.frame_id, self.frame_id_parent)

        # Initialize the super class with the sensor topic
        super().__init__(
            projectairsim_topic_name=projectairsim_topic_name,
            ros_message_type=ros_message_type,
            topics_managers=topics_managers,
            message_callback=message_callback,
            ros_topic_is_latching=ros_topic_is_latching,
            ros_topic_name=ros_topic_name,
        )

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
            if ros_transform is not None:
                self.transform = ros_transform
                self.topics_managers.tf_broadcaster.set_frame(
                    self.frame_id, ros_transform
                )

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
                self.refs = 1  # Number of "references" to this entry
                self.projectairsim_client = (
                    projectairsim_client
                )  # Project AirSim client
                self.desired_pose_message_callback = (
                    desired_pose_message_callback  # ROS message conversion callback
                )
                self.ros_topic_name = (
                    camera_path + "/desired_pose"
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

        # ROS version-specific sensor message helper
        self.sensor_helper = topics_managers.ros_node.create_sensor_helper()

        self.ros_topic_name_camera_image = (
            projectairsim_topic_name + "/image"
        )  # ROS topic name for camera image

        # Setup the camera transform frame
        if frame_id is not None:
            if not frame_id:
                raise ValueError("frame_id can't be an empty string")
            self.frame_id = frame_id
        else:
            self.frame_id = utils.get_sensor_frame_id(projectairsim_topic_name)
        self.frame_id_parent = frame_id_parent
        self.transform = (
            rosgeommsg.Transform()
        )  # Cache of camera's last known transform for ROS
        topics_managers.tf_broadcaster.add_frame(self.frame_id, self.frame_id_parent)

        # Initialize the super class with the image topic
        super().__init__(
            projectairsim_topic_name=projectairsim_topic_name,
            ros_message_type=ros_message_type,
            topics_managers=topics_managers,
            message_callback=image_message_callback,
            ros_topic_is_latching=ros_topic_is_latching,
            ros_topic_name=self.ros_topic_name_camera_image,
        )

        # Setup the camera info topic info
        self.ros_topic_name_camera_info = (
            projectairsim_topic_name + "/camera_info"
        )  # ROS topic name for camera info
        self.camera_info = (
            rossensmsg.CameraInfo()
        )  # Cache of last known camera info for ROS
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

        # Subscribe to ROS camera desired pose topic
        self._camera_path = utils.get_sensor_path(projectairsim_topic_name)
        self._desired_pose_bridge_from_ros.add_camera(
            self._camera_path, desired_pose_message_callback, self.topics_managers
        )

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
                    projectairsim_message_data["pos_x"],
                    projectairsim_message_data["pos_y"],
                    projectairsim_message_data["pos_z"],
                )
            )

            (
                self.transform.rotation.x,
                self.transform.rotation.y,
                self.transform.rotation.z,
                self.transform.rotation.w,
            ) = utils.to_ros_quaternion_list2list(
                (
                    projectairsim_message_data["rot_x"],
                    projectairsim_message_data["rot_y"],
                    projectairsim_message_data["rot_z"],
                    projectairsim_message_data["rot_w"],
                )
            )
            self.topics_managers.tf_broadcaster.set_frame(self.frame_id, self.transform)

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
            self.sensors_topic = f"{self.parent_topic}/sensors"
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
        topics_managers: TopicsManagers,
        message_callback,
        frame_id_parent: str,
        ros_topic_is_latching: bool = True,
        frame_id: str = None,
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
            ros_topic_is_latching=ros_topic_is_latching,
        )
        self.is_subscribed_to_projectairsim_topic = False
        self.frame_id_parent = frame_id_parent
        self.transform = rosgeommsg.Transform()

        # Get ID of transform frame corresponding to the pose and add the frame to the broadcaster
        if frame_id is not None:
            if not frame_id:
                raise ValueError("frame_id can't be an empty string")
            self.frame_id = frame_id
        else:
            self.frame_id = utils.get_robot_frame_id(projectairsim_topic_name)
            if self.frame_id is None:
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
            self.transform.translation = rosgeommsg.Vector3(
                x=posestamped.pose.position.x,
                y=posestamped.pose.position.y,
                z=posestamped.pose.position.z,
            )
            self.transform.rotation = posestamped.pose.orientation
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
