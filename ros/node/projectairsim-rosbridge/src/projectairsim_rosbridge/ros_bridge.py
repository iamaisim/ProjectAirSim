"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
ROS bridge for Project AirSim: Main bridge module
"""

import logging
import re

import geometry_msgs.msg as rosgeommsg
import radar_msgs.msg as rosradarmsg
import sensor_msgs.msg as rossensmsg
import std_msgs.msg as rosstdmsg
import std_srvs.srv as rosstdsrv
import nav_msgs.msg as rosnavmsg

from projectairsim_ros.msg import *
from projectairsim_ros.srv import LoadScene, GetLoggingDirectory

import projectairsim
from projectairsim import ProjectAirSimClient
from projectairsim.utils import projectairsim_log
from sensor_msgs.msg import PointCloud2

from . import utils
from .msg_converter import MsgConverter
from .service_manager import ServiceManager
from .action_manager import ActionManager
from .node import ROSNode
from .topic_helpers import (
    BasicBridgeToROS,
    BasicROSSubscriber,
    CameraBridgeToROS,
    SensorBridgeToROS,
    RobotPoseBridgeFromROS,
    RobotPoseBridgeToROS,
    RobotKinematicsBridgeToROS,
    SimClockBridgeToROS,
    TopicsManagers,
)
import traceback


class ProjectAirSimROSBridge:
    """
    This class bridges Project AirSim and ROS by "wrapping" Project AirSim topics
    and presenting them as ROS topics and vice-versa, and publishes
    ROS transform updates for suitable AirSim topics such as robot vehicle
    poses.

    The topic message processing is done by "topic handlers" that act upon
    messages received from a topic (from Project AirSim or ROS.)  Typically
    topic handlers simply convert the message to the data type appropriate for
    the corresponding topic on the "other side" of the bridge and publishes
    the message to the "other side" (i.e., Project AirSim to ROS and ROS to
    Project AirSim.)

    Some messages require more processing such as a robot's actual pose.
    For that, the topic handler converts and publishes the pose
    to a ROS topic and also publishes a ROS transform that is used to
    map robot-relative data (like sensor data) to "map" coordinates.

    Topic managers for the Project AirSim and ROS topics abstract the details
    of topic management away from topic handlers.  Topic handlers need only
    indicate the topics they wish to subscribe to and publish; the topic
    managers handle the details of actually subscribing and publishing
    topics and routing topic messages to subscribing topic handlers.
    """

    # -------------------------------------------------------------------------
    # ProjectAirSimROSBridge Types
    # -------------------------------------------------------------------------

    # --------------------------------------------------------------------------
    class TopicEntry:
        """
        Contains info for a Project AirSim topic name pattern and how we want to
        handle the topic.
        """

        class MatchType:
            """
            How to match a topic name to our name pattern.
            """

            EXACT = 0  # Name matches exactly
            ENDS_WITH = 1  # Name ends with this pattern
            REGEX = 2  # Name matches this regular expression
            REGEX_ANY = 3  # Name matches this regular expression anywhere in the name

        def __init__(
            self,
            match_type: MatchType,
            name_pattern: str,
            ros_message_type,
            topic_handler_type,
            **kwargs,
        ):
            """
            Constructor.

            topic_handler_type is the class that handles the Project AirSim
            topic.
            Typically the topic handler receives the data from a ROS/Project
            AirSim topic, converts the data type and publishes the converted
            data to the corresponding Project AirSim/ROS topic.  A handler is
            created in response to one Project AirSim topic but can publish
            or subscribe to multiple Project AirSim or ROS topics and is not
            otherwise restricted.

            The topic handler class may have additional class-specific
            constructor parameters (see the class documentation.)  Those
            parameters are specified as additional keyword arguments to this
            constructor and store in self.topic_handler_params.  The
            following named arguments are always passed to the topic handler
            class constructor :
                projectairsim_topic_name - The name of the Project Airsim topic
                ros_message_type - The class object for the ROS messages published to or received from the ROS topic
                topics_managers - Project AirSim and ROS topic managers and ROS Transform broadcaster

            Note that the topic handler class must derive the ROS topic name
            from the Project AirSim topic name (usually the same.)

            Arguments:
                match_type - How to match a topic name to our name_pattern
                name_pattern - The string recognition pattern for topics to which this entry applies
                topic_handler_type - The class from which to construct the topic handler
                ros_message_type - The class object for the ROS messages published to or received from the ROS topic
            """
            # ros_message_type must be a class
            if not isinstance(ros_message_type, type):
                raise TypeError(
                    "ros_message_type must be a type or class object, not an instance"
                )

            self.name_pattern = name_pattern
            self.match_type = match_type
            self.topic_handler_type = topic_handler_type
            self.topic_handler_params = kwargs
            self.ros_message_type = ros_message_type
            if (match_type == self.MatchType.REGEX) or (
                match_type == self.MatchType.REGEX_ANY
            ):
                self.regex = re.compile(name_pattern)
            else:
                self.regex = None

        def is_match(self, topic_name):
            """
            Returns whether the specified string matches our topic name pattern.

            Argument:
                topic_name - The string to compare to our topic name pattern

            Returns:
                (Return) - True if the string matches, False otherwise
            """
            if self.match_type == self.MatchType.ENDS_WITH:
                return topic_name.endswith(self.name_pattern)
            elif self.match_type == self.MatchType.REGEX:
                return self.regex.match(topic_name)
            elif self.match_type == self.MatchType.REGEX_ANY:
                return self.regex.search(topic_name)

            return topic_name == self.name_pattern

    # -------------------------------------------------------------------------
    # ProjectAirSimROSBridge Constants
    # -------------------------------------------------------------------------

    # Topic name prefix for persistent (non-scene based) topic handlers
    HANDLER_PREFIX_PERSISTENT = "//./"

    # -------------------------------------------------------------------------
    # ProjectAirSimROSBridge Properties
    # -------------------------------------------------------------------------
    @property
    def is_connected(self):
        """
        Returns whether we're connected to Project AirSim as a client
        """
        return (self.projectairsim_client is not None) and self.is_connected_to_client

    # -------------------------------------------------------------------------
    # ProjectAirSimROSBridge Methods
    # -------------------------------------------------------------------------
    def __init__(
        self,
        ros_node: ROSNode,
        address: str = "127.0.0.1",
        port_topics: int = 8989,
        port_services: int = 8990,
        sim_config_path: str = "sim_config/",
        start_ros: bool = True,
        client: ProjectAirSimClient = None,
        logger: logging.Logger = None,
    ):
        """
        Constructor.

        If start_ros is False, start_ros() must be called to start processing
        topics.

        An Project AirSim client object that's already connected to Project AirSim
        can be provided via the client argument.  Usually it is left as None
        and a client object is automatically created and connected to Project
        AirSim at the IP address and TCP/IP ports specified by the address,
        port_topics, and port_services arguments.  This client can be retrieved
        as the projectairsim_client attribute.

        When ROS is shutdown, an automatically-created client is also disconnected
        and closed.  If an external client object is passed-in, then the client
        is NOT closed and the caller must disconnect and close the client
        themselves.

        Arguments:
            ros_node - Project AirSim ROS node object
            address - The IP address where Project AirSim is found if client is None
            port_topics - The TCP port where Project AirSim is handling the pub-sub Client API if client is None
            port_services - The TCP port where Project AirSim is handling the services Client API if client is None
            sim_config_path - THe directory containing the simulation config files
            start_ros - If true, ROS topic processing is started immediately
            client - Project AirSim client object (already connected to Project AirSim)
            logger - Logger object; if None, the default Project AirSim logger is used
        """
        # TODO: make dynamic limits class or rosparam?

        self.msg_converter = MsgConverter(ros_node)  # Topic message converter
        self.sim_clock_bridge = None  # Sim clock bridge

        # List of Project AirSim topics we'll bridge to ROS.  Each Project AirSim
        # topic name is matched against this list in this order and the first
        # match is used.
        self.topic_entries = [
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/gps",
                rossensmsg.NavSatFix,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_gps_to_ros,
                ros_topic_name="/global_gps",
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/actual_pose",
                rosgeommsg.PoseStamped,
                topic_handler_type=RobotPoseBridgeToROS,
                message_callback=self.msg_converter.convert_actual_pose_to_ros,
                transform_message_callback=self.msg_converter.convert_actual_pose_to_ros_tf,
                ros_topic_name="/actual_pose",
                frame_id_parent="map",
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/gt_kinematics",
                rosnavmsg.Odometry,
                topic_handler_type=RobotPoseBridgeToROS,
                message_callback=self.msg_converter.convert_gt_kinematics_to_ros,
                transform_message_callback=self.msg_converter.convert_gt_kinematics_to_ros_tf,
                ros_topic_name = "/odom_local_ned",
                frame_id_parent = "map"
            ),
            # Env actors
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/actual_kinematics",
                Kinematics,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_actual_kinematics_to_ros,
                ros_topic_name="/actual_kinematics",
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/collision_info",
                CollisionInfo,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_collision_info_to_ros,
                ros_topic_name="/collision_state",
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/barometer",
                rossensmsg.FluidPressure,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_barometer_to_ros,
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/depth_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/depth_planar_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/depth_vis_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/desired_pose",
                rosgeommsg.PoseStamped,
                topic_handler_type=RobotPoseBridgeFromROS,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/disparity_normalized_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/imu_kinematics",
                rossensmsg.Imu,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_imu_to_ros,
                ros_topic_name="/imu",
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/lidar",
                rossensmsg.PointCloud2,
                topic_handler_type=SensorBridgeToROS,
                message_callback=self.msg_converter.convert_lidar_to_ros,
                transform_message_callback=self.msg_converter.convert_lidar_to_ros_transform,
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/magnetometer",
                rossensmsg.MagneticField,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_magnetometer_to_ros,
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/radar_detections",
                rosradarmsg.RadarScan,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_radar_detection_to_ros,
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/radar_tracks",
                rosradarmsg.RadarTracks,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_radar_track_to_ros,
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/scene_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/segmentation_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/surface_normals_camera",
                rossensmsg.Image,
                topic_handler_type=CameraBridgeToROS,
                image_message_callback=self.msg_converter.convert_image_to_ros,
                desired_pose_message_callback=self.msg_converter.convert_desired_pose_from_ros,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/scene_camera",
                rosgeommsg.PoseStamped,
                topic_handler_type=BasicBridgeToROS,
                message_callback=self.msg_converter.convert_camera_pose_to_ros,
                ros_topic_name="/camera_pose",
                ros_topic_is_latching=False,
            ),
        ]

        # Initialize data members
        self.projectairsim_client = client  # Connection to Project AirSim
        self.projectairsim_world = None
        self.sim_config_path = (
            sim_config_path  # Directory containing the simulation configuration files
        )
        self.is_client_ours = (
            False  # Whether we created self.projectairsim_client or it was passed to us
        )
        self.is_connected_to_client = (
            False  # Whether self.projectairsim_client is connected to Project AirSim
        )
        self.topic_handlers = {}  # Handlers for each topic
        self.robot_paths = {}  # Mapping from Project AirSim topic name to robot path
        self.robot_base_frame_ids = (
            {}
        )  # Mapping from Project AirSim topic name to robot's base transform frame ID
        self.ros_node = ros_node  # ROS node
        self.ros_is_running = (
            False  # If true, stop_ros() has not yet been called since a start_ros()
        )
        self.topics_managers = None  # Topic and transform managers
        
        self.topic_params = None

        if logger is None:
            self.logger = projectairsim_log()
        else:
            self.logger = logger

        # Create and connect to Project AirSim client if one wasn't given
        if self.projectairsim_client is not None:
            self.is_client_ours = False
        else:
            self.projectairsim_client = ProjectAirSimClient(
                address, port_topics=port_topics, port_services=port_services
            )
            self.is_client_ours = True
            self.projectairsim_client.connect()
            self.projectairsim_client.get_topic_info()
        self.is_connected_to_client = True

        # Create topic managers
        self.topics_managers = TopicsManagers(
            self.projectairsim_client, self.ros_node, self.logger
        )

        # Create service manager
        self.service_manager = ServiceManager(self.ros_node, self.projectairsim_client, None, self.logger, self)
        self.action_manager = ActionManager(self.ros_node, self.projectairsim_client, None, self.logger)

        # Register ROS shutdown hook
        self.ros_node.on_shutdown(self._on_ros_shutdown)

        # Create service for reloading scene file
        self.load_scene_service = self.ros_node.create_service('airsim_node/load_scene', LoadScene, self.load_scene_service_cb)
        self.get_logging_directory_service = self.ros_node.create_service('airsim_node/get_logging_directory', GetLoggingDirectory, self.get_logging_directory_service_cb)

        # Start ROS processing, if so directed
        if start_ros:
            self.start_ros()

    def __del__(self):
        """
        Destructor.
        """
        self.clear()

    def clear(self):
        """
        Stop processing and free resources
        """
        self.stop_ros()
        if self.projectairsim_client is not None and self.is_connected_to_client:
            self.is_connected_to_client = False
            if self.is_client_ours:
                self.projectairsim_client.disconnect()
            self.projectairsim_client = None
        if (self.topics_managers is not None) and (
            self.topics_managers.tf_broadcaster is not None
        ):
            self.topics_managers.tf_broadcaster.clear()
            self.topics_managers = None

        self.ros_node = None

    def start_ros(self):
        """
        Start ROS topic processing.
        """
        self.ros_is_started = True
        self.update_topics()
        self.topics_managers.tf_broadcaster.start()

    def stop_ros(self):
        """
        Shutdown ROS topics processing
        """
        if self.topics_managers is not None:
            self.topics_managers.tf_broadcaster.stop()
        self._drop_handlers()
        self.ros_is_started = False

    def update_topics(self, params=None):
        """
        Update ROS topics by scanning the available topics from Project AirSim and setting up
        handlers to advertise and subscribe to the corresponding ROS topics.

        Existing handlers are reused if possible.  Existing handlers for topics that no longer
        exist are removed.
        """

        topic_handlers_new = {}
        robot_paths_new = {}
        robot_base_frame_ids_new = {}

        # Save persistent topic handlers
        for pair in self.topic_handlers.items():
            if pair[0].startswith(self.HANDLER_PREFIX_PERSISTENT):
                topic_handlers_new[pair[0]] = pair[1]
        for topic in topic_handlers_new:
            del self.topic_handlers[topic]

        # Construct new scene-dependant topic handlers
        if self.projectairsim_client and self.projectairsim_client.topics:
            for topic_name in self.projectairsim_client.topics:
                for topic_entry in self.topic_entries:
                    if topic_entry.is_match(topic_name):
                        # Get the robot path and transform frame ID for this topic
                        robot_path = utils.get_robot_path(topic_name)
                        if robot_path is not None:
                            robot_paths_new[topic_name] = robot_path
                        robot_base_frame_id = utils.get_robot_frame_id(topic_name)
                        if robot_base_frame_id is not None:
                            robot_base_frame_ids_new[topic_name] = robot_base_frame_id

                        #add parameters to camera or lidar, incl image width & height, and sensor origin
                        if ("camera" in topic_name or "lidar" in topic_name) and not self.is_camera_pose(topic_entry):
                            camera_name = utils.get_sensor_name(topic_name) 
                            drone_name = utils.get_robot_name(topic_name)
                            sensor_params = params[drone_name]['params'][camera_name]
                            topic_entry.topic_handler_params = {**topic_entry.topic_handler_params, 'transform_params': sensor_params}

                        # Get topic handler object
                        if topic_name in self.topic_handlers:
                            # Have an existing handler for the topic--reuse it
                            topic_handler = self.topic_handlers.pop(topic_name)
                        else:
                            # Create a new handler for the topic
                            topic_handler = topic_entry.topic_handler_type(
                                projectairsim_topic_name=topic_name,
                                ros_message_type=topic_entry.ros_message_type,
                                topics_managers=self.topics_managers,
                                **topic_entry.topic_handler_params,
                                logger=self.logger,
                            )

                        topic_handlers_new[topic_name] = topic_handler

        # Clear handlers that are no longer needed and save new handlers
        self._clear_handlers()  # Skip setting empty dictionaries
        self.topic_handlers = topic_handlers_new
        self.robot_paths = robot_paths_new
        self.msg_converter.set_robot_base_frame_ids(robot_base_frame_ids_new)

        # Get a list of actors(drones) in the current scene
        # For services, we will assume all actors(drones) support (takeoff, land, reset) methods
        if self.projectairsim_world != None:
            self.service_manager.ClearPersistentServices()
            self.service_manager.ClearSceneServices()
            self.service_manager.CreatePersistentServices()
            actors = self.projectairsim_world.list_actors()
            for actor in actors:
                self.service_manager.CreateDroneServices(actor)

        if self.projectairsim_world != None:
            self.action_manager.ClearPersistentActions()
            self.action_manager.ClearSceneActions()
            actors = self.projectairsim_world.list_actors()
            for actor in actors:
                self.action_manager.CreateDroneActions(actor)
            
        # Refresh topic handlers
        
    def is_camera_pose(self, topic_entry):
        # Method to check if transform_params is a parameter in the topic handler in update_topics"
        if "transform_params" in topic_entry.topic_handler_type.__init__.__code__.co_varnames:
            return False
        else:
            return True

    def _clear_handlers(self):
        """
        Clear all topic handlers of their resources.
        """
        for pair in self.topic_handlers.items():
            pair[1].clear()

    def _drop_handlers(self):
        """
        Clear all topic handlers of their resources and drop them.
        """
        self._clear_handlers()
        self.topic_handlers = {}

    def _drop_handlers_for_scene(self):
        """
        Clear scene-related topic handlers of their resources and drop
        them.  Persistent handlers are unaffected.
        """
        topic_handlers_new = {}
        for pair in self.topic_handlers.items():
            if pair[0].startswith(self.HANDLER_PREFIX_PERSISTENT):
                topic_handlers_new[pair[0]] = pair[1]
            else:
                pair[1].clear()
                self.topic_handlers[pair[0]] = None

        self.topic_handlers = topic_handlers_new

    def load_scene_message_cb(self, request):
        """
        Handle a message received from the ROS load_scene service which loads
        the simulation server with a new scene.  This causes us to stop
        publishing ROS topics that aren't in the new scene and start
        publishing ROS topics that are.

        Arguments:
            ros_topic_name - ROS topic name
            ros_message - Message received from the ROS topic
        """
        scene_config = request.scene_file
        self.logger.info(f"Got request to load scene config file: {scene_config}")
 
        #get user set parameters from launch file
        self.ros_node.declare_parameter('publish_rate_hz', 333.33)
        self.ros_node.declare_parameter('use_sim_times', True)        

        use_sim_times = self.ros_node.get_parameter("use_sim_times")
        self.publish_rate_hz = self.ros_node.get_parameter("publish_rate_hz")

        if self.projectairsim_client is not None:
            try:
                # Clear existing scene-based handlers
                self._drop_handlers_for_scene()

                if self.sim_clock_bridge is not None:
                    self.sim_clock_bridge.clear()
                    self.sim_clock_bridge = None

                # Load scene and update ROS topics to match the new scene
                self.projectairsim_world = projectairsim.World(
                    client=self.projectairsim_client,
                    scene_config_name=scene_config,
                    sim_config_path=self.sim_config_path,
                    actual_load=request.is_primary_client,
                )
                
                self.service_manager.SetWorldReference(self.projectairsim_world)
                self.action_manager.SetWorldReference(self.projectairsim_world)

                config_parser = utils.config_parser()
                topic_params = config_parser.parse_params(self.projectairsim_world)

                if self.ros_is_started:
                    self.sim_clock_bridge = SimClockBridgeToROS(projectairsim_world=self.projectairsim_world, topics_managers=self.topics_managers, ros_topic_name='/clock', publish_rate_hz=1.0)
                    self.topics_managers.set_world(self.projectairsim_world)
                    self.update_topics(topic_params)

                self.logger.info(f"Successfully load scene config file: {scene_config}")
                return True
            
            except Exception as e:
                self.logger.error(
                    f'Failed to load scene config file "{scene_config}": {e}'
                )
                self.logger.error(traceback.format_exc())
                
        return False

    def load_scene_service_cb(self, request, response):
        response.success = self.load_scene_message_cb(request)
            
        self.logger.debug(f"LoadSceneService return response {response.success}")
        return response

    def get_logging_directory_service_cb(self, request, response):
        logging_directory = self.ros_node.get_logging_directory()
        self.logger.debug(f"get_logging_directory_service return response {logging_directory}") 

        response.logging_directory = str(self.ros_node.get_logging_directory())
            
        self.logger.debug(f"get_logging_directory_service return response {response.logging_directory}")
        return response
    
    def _on_ros_shutdown(self):
        """
        This function is called when ROS is shutdown so we can shutdown our node.
        """
        self.logger.info("ROS is shutting down--closing down node...")
        self.clear()