"""
Copyright (C) Microsoft Corporation. All rights reserved.
ROS bridge for Project AirSim: ROS 2 abstractions
"""
import rclpy
import rclpy.node
import rclpy.publisher
import rclpy.qos
import rclpy.subscription
import rclpy.time
import rclpy.logging
import sensor_msgs_py.point_cloud2
from rosgraph_msgs.msg import Clock
import tf2_ros
from rclpy.action import ActionClient, ActionServer
from rclpy.parameter import Parameter
import logging
from projectairsim.utils import projectairsim_log
from rclpy.callback_groups import ReentrantCallbackGroup

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
            camera_info.d = tuple(float(x) for x in distortion_params)
            camera_info.k = tuple(float(x) for x in intrinsic_camera_matrix)
            camera_info.r = tuple(float(x) for x in rectification_matrix)
            camera_info.p = tuple(float(x) for x in projection_matrix)

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

    class ROSServiceManager:
        """
        Class for managing ROS2 services: creation and removal.
        """

        def __init__(self, rclpy_node: rclpy.node.Node):
            """
            Constructor.

            Args:
            rclpy_node: An instance of the ROS2Node class.
            """
            self.rclpy_node = rclpy_node
            self.services = {}  # Dictionary to store service

        def __del__(self):
            self.destroy()

        def destroy(self):
            self.clear_services()
            self.rclpy_node = None

        def create_service(self, service_name, service_type, callback):
            """
            Creates and publishes a ROS service.

            Args:
            service_name: Name of the service
            service_type: Service message type
            callbackObj: Function to handle service requests

            Returns:
            None
            """
            service = self.rclpy_node.create_service(
                srv_type=service_type,
                srv_name=service_name,
                callback=callback,
                qos_profile=rclpy.qos.QoSPresetProfiles.SYSTEM_DEFAULT.value,
            )

            self.services[service_name] = service

        def remove_service(self, service_name):
            """
            Removes a service from the node.

            Args:
            service_name: Name of the service to remove

            Returns:
            None
            """
            if service_name in self.services:
                self.rclpy_node.destroy_service(self.services[service_name])
                del self.services[service_name]
            else:
                raise ValueError(f"Service '{service_name}' not found")
        
        def clear_services(self):
            """
            Removes all services from the node.

            Returns:
            None
            """
            for service_name in self.services:
                self.rclpy_node.destroy_service(self.services[service_name])
                del self.services[service_name]
        
    class ROSActionManager:
        """
        Class for managing ROS2 actions: creation and removal.
        """

        def __init__(self, rclpy_node: rclpy.node.Node):
            """
            Constructor.

            Args:
            rclpy_node: An instance of the ROS2Node class.
            """
            self.rclpy_node = rclpy_node
            self.actions = {}  # Dictionary to store service

        def __del__(self):
            self.destroy()

        def destroy(self):
            self.clear_actions()
            self.rclpy_node = None

        def create_action(self, action_name, action_type, execute_callback, callback_group=None, goal_callback=None, cancel_callback=None):
            """
            Creates and publishes a ROS action.

            Args:
            action_name: Name of the action
            action_type: Action message type
            callbackObj: Function to handle action requests

            Returns:
            None
            """
            action_server = ActionServer(
                self.rclpy_node,
                action_type,
                action_name,
                execute_callback=execute_callback,
                goal_callback=goal_callback,
                cancel_callback=cancel_callback,
                callback_group=callback_group
        )
            self.actions[action_name] = action_server

        def remove_action(self, action_name):
            """
            Removes a action from the node.

            Args:
            action_name: Name of the action to remove

            Returns:
            None
            """
            if action_name in self.actions:
                self.actions[action_name].destroy()
                del self.actions[action_name]
            else:
                raise ValueError(f"Action '{action_name}' not found")
        
        def clear_actions(self):
            """
            Removes all action from the node.

            Returns:
            None
            """
            for action_name in self.actions:
                self.actions[action_name].destroy()
                del self.actions[action_name]


    # PointCloud2 class for ROS2
    PointCloud2 = sensor_msgs_py.point_cloud2

    # ROSInterruptException class for ROS2
    ROSInterruptException = rclpy.exceptions.ROSInterruptException

    
    '''
    def __init__(self, node_name, namespace=None):
        """
        Initialize the ROS2 node and declare some parameters
        """
        super().__init__(node_name=node_name, namespace=namespace)
        self._rclpy_node = rclpy.node.Node(node_name=node_name, namespace=namespace)
        self.declared_parameters = {}  # Dictionary to store declared parameters
    '''

    def declare_parameter(self, name: str, value=None, descriptor=None):

        if self.rclpy_node:
            #check if parameter already exists
            if not self.rclpy_node.has_parameter(name):
                if descriptor is not None:
                    self.rclpy_node.declare_parameter(name, value, descriptor)
                else:
                    self.rclpy_node.declare_parameter(name, value)             

    def get_parameter(self, name: str, default=None):

        if self.rclpy_node:
            # Get the parameter and its value
            try:
                param = self.rclpy_node.get_parameter(name)
            except rclpy.exceptions.ParameterNotDeclaredException:
                self.rclpy_node.get_logger().info("Parameter not declared")
            except Exception as e:
                self.get_logger().error(f"Failed to get parameter '{name}': {str(e)}")

            param_value = param.get_parameter_value()

        paramtype = param_value.type
    
        # Check the type of the parameter and return the appropriate value
        #bool
        if param_value.type == 1:#rclpy.Parameter.Type.BOOL:
            return param_value.bool_value if param_value.bool_value is not None else default
        #string
        elif param_value.type == 4:
            return param_value.string_value if param_value.string_value is not None else default
        #integer
        elif param_value.type == 2:
            return param_value.integer_value if param_value.integer_value is not None else default
        #double
        elif param_value.type == 3:#rclpy.Parameter.Type.DOUBLE:
            return param_value.double_value if param_value.double_value is not None else default
        #bool array
        elif param_value.type == 6:
            return param_value.bool_array_value if param_value.bool_array_value is not None else default
        #string array
        elif param_value.type == 9:
            return param_value.string_array_value if param_value.string_array_value is not None else default
        #integer array
        elif param_value.type == 7:
            return param_value.integer_array_value if param_value.integer_array_value is not None else default
        #double array
        elif param_value.type == 8:
            return param_value.double_array_value if param_value.double_array_value is not None else default
        else:
            # Handle unknown parameter types or raise an exception if necessary
            return default

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

        self.callback_group = ReentrantCallbackGroup()  
        self.subscriber_monitor = ROS2Node.SubscriberMonitor(self.rclpy_node)
        self.service_manager = ROS2Node.ROSServiceManager(self.rclpy_node)
        self.action_manager = ROS2Node.ROSActionManager(self.rclpy_node)
        self.current_time = rclpy.time.Time()
        self.create_subscriber("/clock", Clock, self.clock_callback)

    def clock_callback(self, msg):
        """
        Callback function for the /clock topic. Updates the current_time with the latest timestamp.
        """
        self.current_time = rclpy.time.Time.from_msg(msg.clock)
        

    def get_current_time(self):
        """
        Returns the latest timestamp received from /clock topic.
        """
        return self.current_time

    def create_publisher(
        self,
        topic: str,
        msg_type,
        subscriber_listener=None,
        latch: bool = False,
        queue_size: int = 10,
        pub_qos_profile: rclpy.qos.QoSProfile = None 
    ):
        if pub_qos_profile is None:
            #create default qos profile
            pub_qos_profile = rclpy.qos.QoSProfile(
                durability=rclpy.qos.DurabilityPolicy.TRANSIENT_LOCAL
                if latch
                else rclpy.qos.DurabilityPolicy.VOLATILE,
                depth=queue_size,
            )
        
        rclpy_publisher = self.rclpy_node.create_publisher(
            msg_type=msg_type, topic=topic, qos_profile=pub_qos_profile
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
            callback_group=self.callback_group
        )
        return ROS2Node.Subscriber(native_subscription=rclpy_subscription, rosnode=self)
        
    def create_timer(self, period_sec: float, callback=None):
            rclpy_subscription = self.rclpy_node.create_timer(
                timer_period_sec=period_sec,
                callback=callback,
            )
            return ROS2Node.Subscriber(native_subscription=rclpy_subscription, rosnode=self)    

    def create_transform_broadcaster(self):
        return tf2_ros.TransformBroadcaster(self.native_node)
    
    def create_transform_buffer(self):
        return tf2_ros.Buffer()
    
    def create_transform_listener(self, buffer): 
        return tf2_ros.TransformListener(buffer, self.native_node)
    
    def create_service(self, service_name, service_type, callback=None):
        self.service_manager.create_service(service_name, service_type, callback)

    def create_action(self, action_name, action_type, execute_callback, callback_group=None, goal_callback=None, cancel_callback=None):
        self.action_manager.create_action(action_name, action_type, execute_callback, callback_group, goal_callback, cancel_callback)      

    def remove_service(self, service_name):
        self.service_manager.remove_service(service_name)

    def remove_action(self, action_name):
        self.action_manager.remove_action(action_name)        

    def clear_services(self):
        self.service_manager.clear_services()

    def clear_actions(self):
        self.action_manager.clear_actions()     

    def destroy(self):
        if self.subscriber_monitor:
            self.subscriber_monitor.destroy()
            self.subscriber_monitor = None

        if self.service_manager:
            self.service_manager.destroy()
            self.service_manager = None

        if self.action_manager:
            self.action_manager.destroy()
            self.action_manager = None

        if self.rclpy_node_is_ours and self.rclpy_node:
            self.rclpy_node.destroy_node()
        self.rclpy_node = None

    def get_time_now(self):
        return self.get_current_time()

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

        rclpy.spin_once(self.native_node, timeout_sec=0.01)
        return rclpy.ok()

    def _handle_publisher_destroy(self, publisher: Publisher):
        if self.subscriber_monitor:
            self.subscriber_monitor.remove(publisher)
        if self.native_node:
            self.native_node.destroy_publisher(publisher.native_publisher)

    def _handle_subscriber_destroy(self, subscriber: Subscriber):
        if self.native_node:
            self.native_node.destroy_subscription(subscriber.native_subscription)

    def get_logging_directory(self):
        return rclpy.logging.get_logging_directory()            
