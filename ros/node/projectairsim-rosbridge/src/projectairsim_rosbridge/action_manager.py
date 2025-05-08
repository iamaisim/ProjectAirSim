import logging
import traceback
import asyncio
from typing import Dict, List

import std_srvs.srv as rosstdsrv
from projectairsim_ros.action import MoveOnPath

from projectairsim_rosbridge import utils
from projectairsim_rosbridge.node import ROSNode
from projectairsim_rosbridge.tf_helpers import TFBroadcaster
from projectairsim_ros.msg import Waypoint

from pynng import NNGException

from std_msgs.msg import Header
from projectairsim import Drone, ProjectAirSimClient, World

from abc import ABC, abstractmethod
from projectairsim.types import Vector3, Color, Quaternion, Pose

import rclpy
from rclpy.action import ActionClient, ActionServer, GoalResponse, CancelResponse
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.node import Node
import asyncio
import math
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, quaternion_to_rpy
from projectairsim.image_utils import ImageDisplay
from .topic_helpers import ROSTopicsManager
from rclpy.qos import QoSProfile, QoSHistoryPolicy, QoSReliabilityPolicy, QoSDurabilityPolicy
from geometry_msgs.msg import Point


class ActionManager:
    def __init__(self, ros_node: ROSNode, projectairsim_client: ProjectAirSimClient, projectairsim_world, logger: logging.Logger):
        self.ros_node = ros_node
        self.projectairsim_client = projectairsim_client
        self.projectairsim_world = projectairsim_world
        if self.projectairsim_world is not None:
            self.world_parent_topic = self.projectairsim_world.parent_topic
        self.persistent_actions = set()
        self.scene_actions = set()
        self.logger = logger # Logging object
        self.topics_manager = ROSTopicsManager(ros_node, logger)

    def SetWorldReference(self, projectairsim_world):
        self.projectairsim_world = projectairsim_world
        self.world_parent_topic = self.projectairsim_world.parent_topic
    
    def ClearPersistentActions(self):
        for actionObj in self.persistent_actions:
            actionObj.destroy()

    def ClearSceneActions(self):
        for actionObj in self.scene_actions:   
            actionObj.destroy()
    
    def CreateDroneActions(self, vehicle_name):
        self.scene_actions.add(ActionManager.MoveOnPathAction(self, MoveOnPath, vehicle_name))

    class BasicAction():
        def __init__(self, action_manager, action_name, action_type):
            """
            Constructor.

            Arguments:
                projectairsim_client - Client connection to Project AirSim
            """
            self.action_manager = action_manager
            self.ros_node = action_manager.ros_node
            self.projectairsim_client = action_manager.projectairsim_client
            self.projectairsim_world = action_manager.projectairsim_world
            self.action_name = action_name
            self.ros_node.create_action(self.action_name, action_type, execute_callback=self.execute_callback, callback_group=rclpy.callback_groups.ReentrantCallbackGroup(), goal_callback=self.goal_callback, cancel_callback=self.cancel_callback)
            self.logger = action_manager.logger
            self.topics_manager = action_manager.topics_manager
            

        def __hash__(self):
            return hash(self.action_name)

        def __eq__(self, other):
            return isinstance(other, ActionManager.BasicAction) and self.action_name == other.action_name
        
        def destroy(self):
            self.ros_node.remove_action(self.action_name)
        
        def execute_callback(self, goal_handle):
            pass
        
        def goal_callback(self, goal_request):
            pass
        
        def cancel_callback(self, goal_handle):
            pass


    class MoveOnPathAction(BasicAction):
        def __init__(self, action_manager, action_type, vehicle_name):
            super().__init__(action_manager, "/airsim_node/" + vehicle_name + "/move_on_path", action_type)
            self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
            self.path_number = 0
            self.waypoint_map = {}
            self.waypoint_subscribed_flag = False
            self.qos_profile = self._create_qos_profile()
            self.vehicle_name = vehicle_name
            self.waypoint_publisher = self.create_publisher_name()
            self._initialize_publisher()
            self.current_subscription = None
            

        def _create_qos_profile(self):
            """Create a QoS profile with the required settings."""
            qos_profile = QoSProfile(
                history=QoSHistoryPolicy.KEEP_LAST,
                depth=1,  # Keep the last 1 message
                reliability=QoSReliabilityPolicy.RELIABLE
            )
            qos_profile.durability = QoSDurabilityPolicy.TRANSIENT_LOCAL
            return qos_profile    

        def _initialize_publisher(self):
            """Add a publisher to the topics manager."""
            self.topics_manager.add_publisher(
                topic_name=self.waypoint_publisher,
                ros_message_type=Waypoint,
                peer_change_callback=None,
                is_latching=True,
                ros_qos_profile=self.qos_profile
            )             

        def execute_callback(self, goal_handle):     
            self.moveonpath_flag = True
            loop = asyncio.get_event_loop()
            loop.run_until_complete(self.move_on_path_async(goal_handle))
            return MoveOnPath.Result(success=True)
        
        def goal_callback(self, goal_request):
            return GoalResponse.ACCEPT

        def cancel_callback(self, goal_handle):
            return CancelResponse.ACCEPT
        
        def handle_current_waypoint_number(self, _, msg):
            waypoint_num = msg['value']
            
            if waypoint_num == 0:
                waypoint = self.waypoint_map[waypoint_num + 1]
                self.logger.info(f"Upcoming waypoint: {waypoint}, Index: {waypoint_num}")
            elif waypoint_num == -1:
                self.projectairsim_client.unsubscribe(self.drone.robot_info["current_waypoint_number"])
                self.waypoint_subscribed_flag = False
            elif waypoint_num == len(self.waypoint_map):
                waypoint = self.waypoint_map[waypoint_num]
                self.logger.info(f"Last Traversed Waypoint: {waypoint}, Index: {waypoint_num}: end of path reached")
                waypoint_msg = self.convert_waypoint_to_point_msg(waypoint, waypoint_num)
                self.topics_manager.publish(self.waypoint_publisher, waypoint_msg)
            else:
                waypoint = self.waypoint_map[waypoint_num]
                next_waypoint = self.waypoint_map[waypoint_num + 1]
                self.logger.info(f"Last Traversed Waypoint: {waypoint}, Index: {waypoint_num}; Upcoming Waypoint: {next_waypoint}, Index: {waypoint_num}")
                waypoint_msg = self.convert_waypoint_to_point_msg(waypoint, waypoint_num)
                self.topics_manager.publish(self.waypoint_publisher, waypoint_msg)

        def convert_waypoint_to_point_msg(self, waypoint, waypoint_num):
            #timestamp not currently included, include as later change?
            point = Waypoint()
            point.point.x = waypoint[0]
            point.point.y = waypoint[1]
            point.point.z = waypoint[2]
            point.index = waypoint_num
            return point
        
        def create_publisher_name(self):
            topic_name = '/airsim_node/' + self.vehicle_name + '/waypoint'
            return topic_name
        
        def _unsubscribe_from_waypoint_topic(self):
            if self.current_subscription is not None:
                self.projectairsim_client.unsubscribe(self.current_subscription)
                self.current_subscription = None
                self.waypoint_subscribed_flag = False
                    
        async def move_on_path_async(self, goal_handle):
            goal = goal_handle.request
        
            if self.drone is not None:
                airsim_path = [[pose_stamped.pose.position.x, 
                                pose_stamped.pose.position.y, 
                                pose_stamped.pose.position.z] for pose_stamped in goal.path]     
                
            self.waypoint_map = {index: point for index, point in enumerate(airsim_path, start=1)}
            self.path_number = self.path_number + 1
 
            try:
                move_on_path_task = await self.drone.move_on_path_async(airsim_path, goal.velocity, goal.timeout_sec, 
                                                                        goal.drive_train_type, goal.yaw_is_rate, goal.yaw, 
                                                                        goal.lookahead, goal.adaptive_lookahead)

                if not self.waypoint_subscribed_flag:
                    self._unsubscribe_from_waypoint_topic()
                    self.current_subscription = self.projectairsim_client.subscribe(self.drone.robot_info["current_waypoint_number"], self.handle_current_waypoint_number)
                    self.waypoint_subscribed_flag = True
                    #self.projectairsim_client.subscribe(self.drone.robot_info["current_waypoint_number"], self.handle_current_waypoint_number)
                    
                #if goal.wait_on_last_task == True:
                    #await move_on_path_task
                goal_handle.succeed()
            except Exception as e:
                self.logger.error(f"Exception during move_on_path_async: {e}")
                goal_handle.abort()

