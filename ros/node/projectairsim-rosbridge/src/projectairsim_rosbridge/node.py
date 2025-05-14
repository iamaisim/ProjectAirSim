"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ROS bridge for Project AirSim: ROS version abstractions
"""
from abc import ABC, abstractmethod
from rclpy.qos import QoSProfile
from rclpy.callback_groups import CallbackGroup
import tf2_ros


class ROSNode(ABC):
    """
    Abstract base for class that abstracts the differences (we care about) between ROS1 and ROS2.
    """

    class Publisher(ABC):
        """
        Abstract base class for ROS topic publisher.
        """

        @property
        @abstractmethod
        def topic_name(self):
            """
            Return the topic name.
            """
            return None

        def __init__(self):
            """
            Constructor.
            """
            pass

        def __del__(self):
            """
            Destructor.
            """
            self.destroy()

        @abstractmethod
        def destroy(self):
            """
            Stop the publisher and free any resources.
            """
            pass

        @abstractmethod
        def publish(self, msg):
            """
            Publish the message to the topic.
            
            Arguments:
                msg - Message to publish
            """
            pass

    class SensorHelper(ABC):
        """
        Abstract base class for helper for ROS version-specific sensor messages
        """

        def __init__(self):
            pass

        @abstractmethod
        def set_camera_info(
            self,
            camera_info,
            distortion_params,
            intrinsic_camera_matrix,
            rectification_matrix,
            projection_matrix,
        ):
            pass

    class Subscriber(ABC):
        """
        Abstract base class for ROS topic subscriber
        """

        @property
        @abstractmethod
        def topic_name(self):
            """
            Return the topic name.
            """
            return self.subscription.topic_name

        def __init__(self):
            """
            Constructor.
            """
            pass

        @abstractmethod
        def destroy(self):
            """
            Stop the subscriber and free any resources.
            """
            pass

    # PointCloud2 abstracted class
    # PointCloud2 = None

    # ROS interrupted exception class
    # ROSInterruptException = None

    @property
    def name(self):
        """
        Returns the name of the node
        """
        return self.node_name

    @property
    @abstractmethod
    def ros_is_running(self):
        """
        Returns whether ROS is running
        """
        raise NotImplementedError()

    def __init__(self, name: str):
        """
        Constructor.

        Arguments:
            name - Name of this node
        """
        self.node_name = name

    def __del__(self):
        """
        Destructor.
        """
        self.destroy()

    @abstractmethod
    def create_publisher(
        self,
        topic: str,
        msg_type,
        subscriber_listener=None,
        latch: bool = False,
        queue_size: int = 10,
        qos_profile: QoSProfile = None,
    ):
        """
        Creates a ROS topic publisher.

        Arguments:
            topic - Name of the topic
            msg_type - Data type of the topic messages
            subscriber_listener - Callback to invoke when a peer subscribes to the topic
            latch - If true, the last topic message is automatically sent to new subscribers
            queue_size - Maximum number of messages to allow in queue for sending before dropping messages

        Returns:
            (Return) - New publisher object
        """
        raise NotImplementedError()

    @abstractmethod
    def create_rate(self, frequency: float):
        """
        Creates a rate object.

        Arguments:
            frequency - The desired cycle rate

        Returns:
            (Return) - New rate object
        """
        raise NotImplementedError()

    @abstractmethod
    def create_sensor_helper(self):
        """
        Create a sensor helper object that abstracts various ROS1/2 sensor message differences.
        """
        raise NotImplementedError()

    @abstractmethod
    def create_static_transform_broadcaster(self):
        """
        Creates a TF2 static transform broadcaster.

        Returns:
            (Return) - New static transform broadcaster object
        """
        raise NotImplementedError()

    @abstractmethod
    def create_subscriber(self, topic: str, msg_type, callback=None):
        """
        Creates a ROS topic subscriber.

        Arguments:
            topic - Name of the topic
            msg_type - Data type of the topic messages
            callback - Callback to invoke when a peer publishes to the topic

        Returns:
            (Return) - New subscriber object
        """
        raise NotImplementedError()
    
    @abstractmethod
    def create_timer(self, period_sec: float, callback=None):
        """
        Creates a ROS topic timer.

        Arguments:
            period_sec - Rate to call callback
            callback - Callback to at timer

        Returns:
            (Return) - New subscriber object
        """
        raise NotImplementedError()

    @abstractmethod
    def create_transform_broadcaster(self):
        """
        Creates a TF2  transform broadcaster.

        Returns:
            (Return) - New transform broadcaster object
        """
        raise NotImplementedError()
    
    @abstractmethod
    def create_transform_buffer(self):
        """
        Creates a TF2  transform buffer.

        Returns:
            (Return) - New transform buffer object
        """
        raise NotImplementedError()    
    
    @abstractmethod
    def create_transform_listener(self, buffer):
        """
        Creates a TF2  transform listener.

        Returns:
            (Return) - New transform listener object
        """
        raise NotImplementedError()    
    
    @abstractmethod
    def create_service(self, service_name, service_type, callback=None):
        """
        Creates and publishes a ROS service.

        Args:
        service_name: Name of the service (e.g., /my_service)
        service_type: Service message type (e.g., MyServiceName.srv)
        callback: Function to handle service requests

        Returns:
        None
        """
        raise NotImplementedError()
    
    @abstractmethod
    def create_action(self, action_name, action_type, execute_callback=None, callback_group=None, goal_callback=None, cancel_callback=None):
        """
        Creates and publishes a ROS action.

        Args:
        action_name: Name of the action (e.g., /my_action)
        action_type: Action message type (e.g., MyActionName.action)
        callback: Function to handle action requests

        Returns:
        None
        """
        raise NotImplementedError()
    
    @abstractmethod
    def remove_service(self, service_name):
        """
        Removes a service from the node.

        Args:
        service_name: Name of the service to remove

        Returns:
        None
        """
        raise NotImplementedError()

    @abstractmethod
    def clear_services(self):
        """
        Removes all services from the node.

        Returns:
        None
        """
        raise NotImplementedError()
    
    @abstractmethod
    def declare_parameter(self, name: str, value, descriptor):
        raise NotImplementedError()


    def destroy(self):
        """
        Stops this node and frees any resources
        """
        pass

    @abstractmethod
    def get_parameter(self, name: str, default):
        """
        Get the value of a declared parameter.

        Arguments:
        name - The name of the parameter

        Returns:
        The value of the parameter
        """
        raise NotImplementedError()   

    @abstractmethod
    def get_time_from_msg(self, header_stamp):
        """
        Returns the time value from the topic message header timestamp.

        Returns:
            (Return) - Time value from message header timestamp
        """
        raise NotImplementedError()

    @abstractmethod
    def get_time_now(self):
        """
        Returns the current node time value.

        Returns:
            (Return) - Current node time value
        """
        raise NotImplementedError()

    def get_time_now_msg(self):
        """
        Returns the current node time for a topic message header timestamp.

        Returns:
            (Return) - Current node time value
        """
        return self.get_time_to_msg(self.get_time_now())

    @abstractmethod
    def get_time_to_msg(self, timestamp):
        """
        Returns the topic message header timestamp from the time value.

        Returns:
            (Return) - Message header timestamp from time value
        """
        raise NotImplementedError()

    @abstractmethod
    def on_shutdown(self, callback):
        """
        Register a callback for when the node is shutdown.

        Arguments:
            callback - Callback to invoke when node is shutdown
        """
        raise NotImplementedError()

    @abstractmethod
    def spin(self):
        """
        Process ROS messages until the node is shutdown.
        """
        raise NotImplementedError()

    @abstractmethod
    def spin_once(self) -> bool:
        """
        Process any pending ROS messages and return.

        Returns:
            (Return) - True if ROS is running, False if ROS is shutdown
        """
        raise NotImplementedError()
