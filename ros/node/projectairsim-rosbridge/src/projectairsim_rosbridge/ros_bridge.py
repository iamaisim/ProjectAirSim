"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
ROS bridge for Project AirSim: Main bridge module
"""

import logging
import re

import geometry_msgs.msg as rosgeommsg
import radar_msgs.msg as rosradarmsg
import sensor_msgs.msg as rossensmsg
import std_msgs.msg as rosstdmsg

import projectairsim
from projectairsim import ProjectAirSimClient
from projectairsim.utils import projectairsim_log

from . import utils
from .msg_converter import MsgConverter
from .node import ROSNode
from .topic_helpers import (
    BasicBridgeToROS,
    BasicROSSubscriber,
    CameraBridgeToROS,
    SensorBridgeToROS,
    RobotPoseBridgeFromROS,
    RobotPoseBridgeToROS,
    TopicsManagers,
)


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
                ros_topic_is_latching=False,
            ),
            self.TopicEntry(
                self.TopicEntry.MatchType.ENDS_WITH,
                "/actual_pose",
                rosgeommsg.PoseStamped,
                topic_handler_type=RobotPoseBridgeToROS,
                message_callback=self.msg_converter.convert_actual_pose_to_ros,
                frame_id_parent="map",
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
        ]

        # Initialize data members
        self.projectairsim_client = client  # Connection to Project AirSim
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

        # Register ROS shutdown hook
        self.ros_node.on_shutdown(self._on_ros_shutdown)

        # Add persistent topics not dictated by the scene
        self.topic_handlers[
            self.HANDLER_PREFIX_PERSISTENT + "load_scene"
        ] = BasicROSSubscriber(
            ros_topic_name=f"/ProjectAirSim/node/{self.ros_node.name}/load_scene",
            ros_message_type=rosstdmsg.String,
            topics_managers=self.topics_managers,
            message_callback=self._load_scene_message_cb,
        )

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

    def update_topics(self):
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
                            )

                        topic_handlers_new[topic_name] = topic_handler
                        break

        # Clear handlers that are no longer needed and save new handlers
        self._clear_handlers()  # Skip setting empty dictionaries
        self.topic_handlers = topic_handlers_new
        self.robot_paths = robot_paths_new
        self.msg_converter.set_robot_base_frame_ids(robot_base_frame_ids_new)

        # Refresh topic handlers

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

    def _load_scene_message_cb(self, ros_topic_name, ros_message):
        """
        Handle a message received from the ROS load_scene topic which loads
        the simulation server with a new scene.  This causes us to stop
        publishing ROS topics that aren't in the new scene and start
        publishing ROS topics that are.

        Arguments:
            ros_topic_name - ROS topic name
            ros_message - Message received from the ROS topic
        """
        scene_config = ros_message.data
        self.logger.info(f"Got request to load scene config file: {scene_config}")
        if self.projectairsim_client is not None:
            try:
                # Clear existing scene-based handlers
                self._drop_handlers_for_scene()

                # Load scene and update ROS topics to match the new scene
                projectairsim.World(
                    client=self.projectairsim_client,
                    scene_config_name=scene_config,
                    sim_config_path=self.sim_config_path,
                )
                if self.ros_is_started:
                    self.update_topics()

                self.logger.info(f"Successfully load scene config file: {scene_config}")
            except Exception as e:
                self.logger.error(
                    f'Failed to load scene config file "{scene_config}": {e}'
                )

    def _on_ros_shutdown(self):
        """
        This function is called when ROS is shutdown so we can shutdown our node.
        """
        self.logger.info("ROS is shutting down--closing down node...")
        self.clear()
