"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ROS bridge for Project AirSim: ROS 1 abstractions
"""
from projectairsim_rosbridge import node
import rospy
import sensor_msgs.point_cloud2
import tf2_ros


class ROS1Node(node.ROSNode):
    """
    Wrapper for all ROS1-specific functionality in the Project AirSim ROS Bridge
    """

    class Publisher(node.ROSNode.Publisher):
        """
        Implements Publisher class using a rospy Publisher.
        """

        @property
        def topic_name(self):
            return self.rclpy_publisher.topic_name

        def __init__(
            self,
            topic: str,
            msg_type,
            subscriber_listener,
            latch: bool,
            queue_size: int,
        ):
            self.rospy_publisher = rospy.Publisher(
                name=topic,
                data_class=msg_type,
                latch=latch,
                queue_size=queue_size,
                subscriber_listener=self,
            )
            self.subscriber_listener_target = subscriber_listener

        def __del__(self):
            self.destroy()

        def destroy(self):
            if self.rospy_publisher:
                self.rospy_publisher.unregister()
                self.rospy_publisher = None

        def publish(self, msg):
            self.rospy_publisher.publish(msg)

        def peer_subscribe(self, topic_name, publish_topic, publish_subscribers):
            """
            Map ROS1's peer_subscribe callback to our callback prototype that drops
            the two publish methods
            """
            if self.subscriber_listener_target:
                self.subscriber_listener_target.peer_subscribe(topic_name)

        def peer_unsubscribe(self, topic_name, num_peers):
            if self.subscriber_listener_target:
                self.subscriber_listener_target.peer_unsubscribe(topic_name, num_peers)

    class SensorHelper(node.ROSNode.SensorHelper):
        """
        Implements helper for ROS1 version-specific sensor messages
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
            camera_info.D = distortion_params
            camera_info.K = intrinsic_camera_matrix
            camera_info.R = rectification_matrix
            camera_info.P = projection_matrix

    class Subscriber(node.ROSNode.Subscriber):
        """
        Implements Publisher class using a rospy Subscriber.
        """

        @property
        def topic_name(self):
            return self.subscriber.topic_name

        def __init__(self, topic: str, msg_type, callback):
            self.rospy_subscriber = rospy.Subscriber(
                name=topic, data_class=msg_type, callback=callback
            )

        def destroy(self):
            if self.rospy_subscriber:
                self.rospy_subscriber.unregister()
                self.rospy_subscriber = None

    # PointCloud2 class for ROS1
    PointCloud2 = sensor_msgs.point_cloud2

    # ROSInterruptException class for ROS1
    ROSInterruptException = rospy.exceptions.ROSInterruptException

    @property
    def ros_is_running(self):
        return not rospy.is_shutdown()

    def __init__(self, name: str, anonymous: bool = False):
        """
        Constructor.

        Arguments:
            name - (Base) name of the node
            anonymous - If True, the node name is randomized by appending a random 
        """
        rospy.init_node(name=name, anonymous=anonymous)
        super().__init__(rospy.get_name()[1:])

    def create_publisher(
        self,
        topic: str,
        msg_type,
        subscriber_listener=None,
        latch: bool = False,
        queue_size: int = 10,
    ):
        return ROS1Node.Publisher(
            topic=topic,
            msg_type=msg_type,
            subscriber_listener=subscriber_listener,
            latch=latch,
            queue_size=queue_size,
        )

    def create_rate(self, frequency: float):
        return rospy.Rate(frequency)

    def create_static_transform_broadcaster(self):
        return tf2_ros.StaticTransformBroadcaster()

    def create_sensor_helper(self):
        return ROS1Node.SensorHelper()

    def create_subscriber(self, topic: str, msg_type, callback=None):
        return ROS1Node.Subscriber(topic=topic, msg_type=msg_type, callback=callback)

    def create_transform_broadcaster(self):
        return tf2_ros.TransformBroadcaster()

    def destroy(self):
        pass

    def get_time_from_msg(self, header_stamp):
        return header_stamp

    def get_time_now(self):
        return rospy.Time.now()

    def get_time_to_msg(self, timestamp):
        return timestamp

    def on_shutdown(self, callback):
        rospy.on_shutdown(callback)

    def spin(self):
        rospy.spin()

    def spin_once(self) -> bool:
        return not rospy.is_shutdown()
