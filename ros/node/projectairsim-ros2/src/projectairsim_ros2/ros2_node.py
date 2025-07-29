"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
ROS bridge for Project AirSim: ROS 2 abstractions
"""
import rclpy
import rclpy.node
import rclpy.publisher
import rclpy.qos
import rclpy.subscription
import rclpy.time
import sensor_msgs_py.point_cloud2
import tf2_ros

from projectairsim_rosbridge import node

from . import ros1_compatibility


class ROS2Node(node.ROSNode):
    """
    Wrapper for all ROS2-specific functionality in the Project AirSim ROS Bridge
    """

    class Publisher(node.ROSNode.Publisher):
        """
        Implements Publisher class using a rclpy Publisher.
        """

        @property
        def native_publisher(self):
            return self.rclpy_publisher

        @property
        def topic_name(self):
            return self.rclpy_publisher.topic_name

        def __init__(self, native_publisher: rclpy.publisher.Publisher, rosnode):
            self.rclpy_publisher = native_publisher
            self.rosnode = rosnode  # Object of ROSNode class, not rclpy.node.Node

        def __del__(self):
            self.destroy()

        def destroy(self):
            if self.rosnode:
                self.rosnode._handle_publisher_destroy(self)

                self.rclpy_publisher = None
                self.rosnode = None

        def publish(self, msg):
            if rclpy.ok():
                self.rclpy_publisher.publish(msg)

    class SensorHelper(node.ROSNode.SensorHelper):
        """
        Implements helper for ROS2 version-specific sensor messages
        """

        def __init__(self):
            pass

        def set_camera_info(
            self,
            camera_info,
            distortion_params,
            intrinsic_camera_matrix,
            rectification_matrix,
            projection_matrix,
        ):
            camera_info.d = distortion_params
            camera_info.k = intrinsic_camera_matrix
            camera_info.r = rectification_matrix
            camera_info.p = projection_matrix

    class Subscriber(node.ROSNode.Subscriber):
        """
        Implements Subscriber class using a rclpy Subscription.
        """

        @property
        def native_subscription(self):
            return self.rclpy_subscription

        @property
        def topic_name(self):
            return self.rclpy_subscription.topic_name

        def __init__(
            self, native_subscription: rclpy.subscription.Subscription, rosnode
        ):
            self.rclpy_subscription = native_subscription
            self.rosnode = rosnode

        def destroy(self):
            if self.rosnode:
                self.rosnode._handle_subscriber_destroy(self)

                self.rclpy_subscription = None
                self.rosnode = None

    class SubscriberMonitor:
        """
        Unlike ROS1 and rospy, ROS2 and rclpy don't generate callbacks when a
        peer subscribes or unsubscribes from a ROS topic.  We'll simulate it
        by polling the number of subscribers to each topic we publish and invoking
        the SubscriberListener callback when we notice a change.
        """

        class _SubscriberListenerEntry:
            def __init__(self, subscriber_listener):
                self.count_subscribers = 0
                self.subscriber_listener = subscriber_listener

        def __init__(self, rclpy_node: rclpy.node.Node):
            self.rclpy_node = rclpy_node
            self.publisher_subscriber_listeners = {}
            self.sec_poll_interval = 0.5
            self.timer = None

        def __del__(self):
            self.destroy()

        def add(self, publisher: node.ROSNode.Publisher, subscriber_listener):
            if publisher not in self.publisher_subscriber_listeners:
                self.publisher_subscriber_listeners[
                    publisher
                ] = self._SubscriberListenerEntry(subscriber_listener)

            if not self.timer:
                self.timer = self.rclpy_node.create_timer(
                    self.sec_poll_interval, self._timer_do_poll
                )

        def destroy(self):
            self._stop()
            self.publisher_subscriber_listeners = None
            self.rclpy_node = None

        def remove(self, publisher: node.ROSNode.Publisher):
            if publisher in self.publisher_subscriber_listeners:
                del self.publisher_subscriber_listeners[publisher]

            if len(self.publisher_subscriber_listeners) == 0:
                self._stop()

        def _stop(self):
            if self.timer:
                self.timer.cancel()
                self.timer = None

        def _timer_do_poll(self):
            for (
                publisher,
                subscriber_listener_entry,
            ) in self.publisher_subscriber_listeners.items():
                # Get current number of subscribers to the topic
                count_subscribers_now = (
                    publisher.rclpy_publisher.get_subscription_count()
                )

                # If we have more subscribers now, tell subscriber listeners of the new subscribers
                if count_subscribers_now > subscriber_listener_entry.count_subscribers:
                    topic_name = publisher.topic_name
                    for isubscriber in range(
                        0,
                        count_subscribers_now
                        - subscriber_listener_entry.count_subscribers,
                    ):
                        subscriber_listener_entry.subscriber_listener.peer_subscribe(
                            topic_name
                        )
                    subscriber_listener_entry.count_subscribers = count_subscribers_now

                # If we have less subscribers now, tell subscriber listeners of the lost subscribers
                elif (
                    count_subscribers_now < subscriber_listener_entry.count_subscribers
                ):
                    topic_name = publisher.topic_name
                    subscriber_listener_entry.subscriber_listener.peer_unsubscribe(
                        topic_name, count_subscribers_now
                    )
                    subscriber_listener_entry.count_subscribers = count_subscribers_now

    # PointCloud2 class for ROS2
    PointCloud2 = sensor_msgs_py.point_cloud2

    # ROSInterruptException class for ROS2
    ROSInterruptException = rclpy.exceptions.ROSInterruptException

    @property
    def native_node(self):
        return self.rclpy_node

    @property
    def ros_is_running(self):
        return rclpy.ok()

    def __init__(
        self, name: str, anonymous: bool = False, native_node: rclpy.node.Node = None
    ):
        """
        Constructor.

        Arguments:
            name - (Base) name of the node
            anonymous - If True, the node name is randomized by appending a random 
            native_node - rclpy node
        """
        # If an anonymous name is requested, apply ROS1 algorithm
        if native_node is None and anonymous:
            name = ros1_compatibility.anonymous_name(name)

        super().__init__(name)

        if native_node:
            self.rclpy_node = native_node
            self.rclpy_node_is_ours = False
        else:
            self.rclpy_node = rclpy.node.Node(node_name=name)
            self.rclpy_node_is_ours = True

        self.subscriber_monitor = ROS2Node.SubscriberMonitor(self.rclpy_node)

    def create_publisher(
        self,
        topic: str,
        msg_type,
        subscriber_listener=None,
        latch: bool = False,
        queue_size: int = 10,
    ):
        qos_profile = rclpy.qos.QoSProfile(
            durability=rclpy.qos.DurabilityPolicy.TRANSIENT_LOCAL
            if latch
            else rclpy.qos.DurabilityPolicy.VOLATILE,
            depth=queue_size,
        )
        rclpy_publisher = self.rclpy_node.create_publisher(
            msg_type=msg_type, topic=topic, qos_profile=qos_profile
        )
        publisher = ROS2Node.Publisher(native_publisher=rclpy_publisher, rosnode=self)
        if subscriber_listener:
            self.subscriber_monitor.add(publisher, subscriber_listener)

        return publisher

    def create_rate(self, frequency: float):
        return self.rclpy_node.create_rate(frequency)

    def create_sensor_helper(self):
        return ROS2Node.SensorHelper()

    def create_static_transform_broadcaster(self):
        return tf2_ros.StaticTransformBroadcaster(self.native_node)

    def create_subscriber(self, topic: str, msg_type, callback=None):
        rclpy_subscription = self.rclpy_node.create_subscription(
            msg_type=msg_type,
            topic=topic,
            callback=callback,
            qos_profile=rclpy.qos.QoSPresetProfiles.SYSTEM_DEFAULT.value,
        )
        return ROS2Node.Subscriber(native_subscription=rclpy_subscription, rosnode=self)

    def create_transform_broadcaster(self):
        return tf2_ros.TransformBroadcaster(self.native_node)

    def destroy(self):
        if self.subscriber_monitor:
            self.subscriber_monitor.destroy()
            self.subscriber_monitor = None

        if self.rclpy_node_is_ours and self.rclpy_node:
            self.rclpy_node.destroy_node()
        self.rclpy_node = None

    def get_time_now(self):
        return self.rclpy_node.get_clock().now()

    def get_time_from_msg(self, header_stamp):
        return rclpy.time.Time.from_msg(header_stamp)

    def get_time_to_msg(self, timestamp):
        return rclpy.time.Time.to_msg(timestamp)

    def on_shutdown(self, callback):
        self.rclpy_node.context.on_shutdown(callback)

    def spin(self):
        rclpy.spin(self.native_node)

    def spin_once(self) -> bool:
        if not rclpy.ok():
            return False

        rclpy.spin_once(self.native_node, timeout_sec=0)
        return rclpy.ok()

    def _handle_publisher_destroy(self, publisher: Publisher):
        if self.subscriber_monitor:
            self.subscriber_monitor.remove(publisher)
        if self.native_node:
            self.native_node.destroy_publisher(publisher.native_publisher)

    def _handle_subscriber_destroy(self, subscriber: Subscriber):
        if self.native_node:
            self.native_node.destroy_subscription(subscriber.native_subscription)
