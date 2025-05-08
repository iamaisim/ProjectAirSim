import logging
import traceback
import asyncio
from typing import Dict, List

import std_srvs.srv as rosstdsrv
import std_msgs.msg as rosstdmsg
from projectairsim_ros.srv import *

from projectairsim_rosbridge import utils
from projectairsim_rosbridge.node import ROSNode
from projectairsim_rosbridge.tf_helpers import TFBroadcaster

from projectairsim.image_utils import SEGMENTATION_PALLETE, segmentation_id_to_color, segmentation_color_to_id
from pynng import NNGException

from std_msgs.msg import Header
from projectairsim import Drone, ProjectAirSimClient, World

from abc import ABC, abstractmethod
from projectairsim.types import Vector3, Color, Quaternion, Pose
from rclpy.clock import Clock
from .topic_helpers import ROSTopicsManager
from rclpy.qos import QoSProfile, QoSHistoryPolicy, QoSReliabilityPolicy, QoSDurabilityPolicy
from nav_msgs.msg import OccupancyGrid as OccupancyGridMsg
import numpy as np


class ServiceManager:
    def __init__(self, ros_node: ROSNode, projectairsim_client: ProjectAirSimClient, projectairsim_world, logger: logging.Logger, bridge  ):
        self.ros_node = ros_node
        self.projectairsim_client = projectairsim_client
        self.projectairsim_world = projectairsim_world
        if self.projectairsim_world is not None:
            self.world_parent_topic = self.projectairsim_world.parent_topic
        self.persistent_services = set()
        self.scene_services = set()
        self.logger = logger # Logging object
        self.topics_manager = ROSTopicsManager(ros_node, logger)
        self.bridge = bridge

    def SetWorldReference(self, projectairsim_world):
        self.projectairsim_world = projectairsim_world
        self.world_parent_topic = self.projectairsim_world.parent_topic
    
    def ClearPersistentServices(self):
        for serviceObj in self.persistent_services:
            serviceObj.destroy()

    def ClearSceneServices(self):
        for serviceObj in self.scene_services:   
            serviceObj.destroy()
            
    def CreatePersistentServices(self): #Scene Dependent services
        self.persistent_services.add(ServiceManager.TakeoffGroupService(self, TakeoffGroup))
        self.persistent_services.add(ServiceManager.LandGroupService(self, LandGroup))
        self.persistent_services.add(ServiceManager.CreateVoxelGridService(self, CreateVoxelGrid))
        self.persistent_services.add(ServiceManager.CreateSegmentedVoxelGridService(self, CreateVoxelGrid))
        self.persistent_services.add(ServiceManager.CreateOccupancyGridService(self, OccupancyGrid))
        self.persistent_services.add(ServiceManager.GetSegIdFromMeshService(self, GetSegIdFromMeshId))
        self.persistent_services.add(ServiceManager.GetColorFromMeshService(self, GetColorFromMeshId))
        self.persistent_services.add(ServiceManager.GetObjIdFromSegIdService(self, GetMeshIdsFromSegId))
        self.persistent_services.add(ServiceManager.GetSegIdFromColorService(self, GetSegIdFromColor))
        self.persistent_services.add(ServiceManager.GetColorFromSegIdService(self, GetColorFromSegId))
        self.persistent_services.add(ServiceManager.GetOriginGeoPointService(self, GetOriginGeoPoint))
        self.persistent_services.add(ServiceManager.GetClock(self, GetClock))
        self.persistent_services.add(ServiceManager.ArmGroupService(self, ArmGroup))
        self.persistent_services.add(ServiceManager.DisarmGroupService(self, DisarmGroup))
    
    def CreateDroneServices(self, vehicle_name):
        self.scene_services.add(ServiceManager.TakeoffService(self, Takeoff, vehicle_name))
        self.scene_services.add(ServiceManager.LandService(self, Land, vehicle_name))
        self.scene_services.add(ServiceManager.MoveOnPathService(self, MoveOnPath, vehicle_name))
        self.scene_services.add(ServiceManager.MoveToPositionService(self, MoveToPosition, vehicle_name))
        self.scene_services.add(ServiceManager.ArmService(self, Arm, vehicle_name))
        self.scene_services.add(ServiceManager.DisarmService(self, Disarm, vehicle_name))

    class BasicService():
        def __init__(self, service_manager, service_name, service_type):
            """
            Constructor.

            Arguments:
                projectairsim_client - Client connection to Project AirSim
            """
            self.service_manager = service_manager
            self.ros_node = service_manager.ros_node
            self.projectairsim_client = service_manager.projectairsim_client
            self.projectairsim_world = service_manager.projectairsim_world
            self.service_name = service_name
            self.ros_node.create_service(self.service_name, service_type, self.callback)
            self.logger = service_manager.logger   
            self.topics_manager = service_manager.topics_manager
            

        def __hash__(self):
            return hash(self.service_name)

        def __eq__(self, other):
            return isinstance(other, ServiceManager.BasicService) and self.service_name == other.service_name
        
        def destroy(self):
            self.ros_node.remove_service(self.service_name)
        
        def callback(self, request, response):
            return response

    class TakeoffService(BasicService):
        def __init__(self, service_manager, service_type, vehicle_name):
            super().__init__(service_manager, "airsim_node/" + vehicle_name + "/takeoff", service_type)
            self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)

        def callback(self, request, response):   
            self.logger.debug(f"TakeoffService entry")
            asyncio.get_event_loop().run_until_complete(self.takeoff_async(request, response))
            #response.success = True
            self.logger.debug(f"TakeoffService return response {response.success}")
            return response
        
        async def takeoff_async(self, request, response):
            return_value = False

            if(self.drone is not None):
                try:
                    self.drone.enable_api_control()
                    self.drone.arm()
                    takeoff_task = await asyncio.wait_for(self.drone.takeoff_async(), timeout=10.0)
                    if request.wait_on_last_task == True:
                        await asyncio.wait_for(takeoff_task, timeout=10.0)
                    return_value = True
                except asyncio.TimeoutError:
                    pass
                except Exception as e:
                    pass

            response.success = return_value
            return return_value

    class TakeoffGroupService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/takeoff_group", service_type)

        def callback(self, request, response):   
            self.logger.debug(f"TakeoffGroupService entry")
            asyncio.get_event_loop().run_until_complete(self.takeoff_async(request, response))
            self.logger.debug(f"TakeoffGroupService return response {response.success}")
            return response
        
        async def takeoff_async(self, request, response):
            response.success = False

            takeoff_tasks = []
            vehicle_names = request.vehicle_names
            for vehicle_name in vehicle_names:
                try:
                    drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
                except Exception as  e:
                    self.logger.info(e)
                    continue

                if(drone is not None):
                    drone.enable_api_control()
                    drone.arm()
                    takeoff_tasks.append(drone.takeoff_async())

            try:
                self.logger.debug(f"TakeoffGroupService asyncio gather entry")
                await asyncio.gather(*takeoff_tasks)
            except Exception as e:
                self.logger.debug(f"TakeoffGroupService asyncio timeout {e}")
                return False
            
            response.success = True
            return True
        
    class LandService(BasicService):
        def __init__(self, service_manager, service_type, vehicle_name):
            super().__init__(service_manager, "airsim_node/" + vehicle_name + "/land", service_type)
            self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)

        def callback(self, request, response):            
            self.logger.debug(f"LandService entry")
            asyncio.get_event_loop().run_until_complete(self.land_async(request, response))
            self.logger.debug(f"LandService return response")
            return response
        
        async def land_async(self, request, response):
            return_value = False

            try:
                if(self.drone is not None):
                    land_task = await asyncio.wait_for(self.drone.land_async(), timeout=30.0)

                    if request.wait_on_last_task == True:
                        await asyncio.wait_for(land_task, timeout=30.0)
                    return_value = True
            except asyncio.TimeoutError:
                self.logger.debug(f"LandService land_async asyncio timeout")
            except Exception as e:
                self.logger.debug(f"LandService land_async asyncio exception {e}")

            response.success = return_value
            return return_value


    class LandGroupService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/land_group", service_type)

        def callback(self, request, response):            
            asyncio.get_event_loop().run_until_complete(self.land_async(request, response))
            return response
        
        async def land_async(self, request, response):
            response.success = False

            land_tasks = []
            vehicle_names = request.vehicle_names
            for vehicle_name in vehicle_names:
                try:
                    drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
                except Exception as  e:
                    self.logger.info(e)
                    continue

                if(drone is not None):
                    land_tasks.append(drone.land_async())

            try:
                await asyncio.gather(*land_tasks)
            except Exception as e:
                return False
            
            response.success = True
            return True


    class ArmService(BasicService):
        def __init__(self, service_manager, service_type, vehicle_name):
            super().__init__(service_manager, "airsim_node/" + vehicle_name + "/arm", service_type)
            self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
            self.vehicle_name = vehicle_name

        def callback(self, request, response):   
            self.logger.info(f"Arming drone")
            asyncio.get_event_loop().run_until_complete(self.arm_async(request))
            response.success = True
            return response
        
        async def arm_async(self, request):
            if(self.drone is not None):
                self.drone.enable_api_control()
                self.drone.arm()    


    class DisarmService(BasicService):
        def __init__(self, service_manager, service_type, vehicle_name):
            super().__init__(service_manager, "airsim_node/" + vehicle_name + "/disarm", service_type)
            self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
            self.vehicle_name = vehicle_name

        def callback(self, request, response):   
            self.logger.info(f"Disarming drone")
            asyncio.get_event_loop().run_until_complete(self.disarm_async(request))
            response.success = True
            return response
        
        async def disarm_async(self, request):
            if(self.drone is not None):
                self.drone.disable_api_control()
                self.drone.disarm()                             


    class ArmGroupService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/arm_group", service_type)

        def callback(self, request, response):   
            self.logger.info(f"Arming drones")
            asyncio.get_event_loop().run_until_complete(self.arm_async(request))
            response.success = True
            return response

        async def arm_async(self, request):
            vehicle_names = request.vehicle_names
            for vehicle_name in vehicle_names:
                try:
                    drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
                except Exception as  e:
                    self.logger.info(f"An exception occurred: {e}")
                    continue

                if(drone is not None):
                    drone.enable_api_control()
                    drone.arm() 


    class DisarmGroupService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/disarm_group", service_type)

        def callback(self, request, response):  
            self.logger.info(f"Disarming drones") 
            asyncio.get_event_loop().run_until_complete(self.disarm_async(request))
            response.success = True
            return response

        async def disarm_async(self, request):
            vehicle_names = request.vehicle_names
            for vehicle_name in vehicle_names:
                try:
                    drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)
                except Exception as  e:
                    self.logger.info(f"An exception occurred: {e}")
                    continue

                if(drone is not None):
                    drone.disable_api_control()
                    drone.disarm()


    class MoveOnPathService(BasicService):
        def __init__(self, service_manager, service_type, vehicle_name):
            super().__init__(service_manager, "airsim_node/" + vehicle_name + "/move_on_path", service_type)
            self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)

        def callback(self, request, response):            
            asyncio.get_event_loop().run_until_complete(self.move_on_path_async(request, response))
            return response

        async def move_on_path_async(self, request, response):
            return_value = False

            self.logger.info(f"move_on_path_async entry")

            try:
                if(self.drone is not None):
                    airsim_path = [[pose_stamped.pose.position.x, 
                                    pose_stamped.pose.position.y, 
                                    pose_stamped.pose.position.z] for pose_stamped in request.path]     
                            
                    self.logger.info(f"self.drone.move_on_path_async entry")
                    move_on_path_task = await self.drone.move_on_path_async(airsim_path, request.velocity, request.timeout_sec, 
                                                                            request.drive_train_type, request.yaw_is_rate, request.yaw, 
                                                                            request.lookahead, request.adaptive_lookahead)
                    if request.wait_on_last_task == True:
                        await asyncio.wait_for(move_on_path_task, timeout=request.timeout_sec+10.0)

                    
            except asyncio.TimeoutError:
                self.logger.info(f"MoveOnPathService asyncio.TimeoutError")
            except Exception as e:
               self.logger.info(f"MoveOnPathService Exception {e}")

            response.success = return_value
            return return_value
 

    class MoveToPositionService(BasicService):
            def __init__(self, service_manager, service_type, vehicle_name):
                """
                Constructor.

                Arguments:
                    projectairsim_client - Client connection to Project AirSim
                """
                super().__init__(service_manager, "airsim_node/" + vehicle_name + "/move_to_position", service_type)
                self.drone = Drone(self.projectairsim_client, self.projectairsim_world, vehicle_name)

            def callback(self, request, response):
                
                airsim_path = [[request.x, request.y, request.z]]

                asyncio.get_event_loop().run_until_complete(self.move_to_position_async(request, response, airsim_path))
                return response
            
            async def move_to_position_async(
                    self,
                    request,
                    response,
                    path
                ):
                response.return_value = False

                self.logger.info(f"move_to_position_async entry")

                try:
                    if(self.drone is not None):
                        move_to_position_task = await self.drone.move_on_path_async(path, request.velocity, lookahead=request.lookahead, adaptive_lookahead=request.adaptive_lookahead)

                        if request.wait_on_last_task == True:
                            await asyncio.wait_for(move_to_position_task, timeout=request.timeout_sec+10.0)
                        return_value = True

                except asyncio.TimeoutError:
                    self.logger.info(f"MoveOnPositionService asyncio.TimeoutError")
                except Exception as e:
                    self.logger.info(f"MoveOnPositionService Exception {e}")

                response.success = return_value
                return return_value

    class CreateVoxelGridService(BasicService):
            def __init__(self, service_manager, service_type):
                super().__init__(service_manager, "airsim_node/create_voxel_grid", service_type)

            def callback(self, request, response):
                self.logger.info(f"Generating Map")
                center_pose = Pose({
                    "translation": Vector3({"x": request.position_x, "y": request.position_y, "z": request.position_z}),
                    "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
                    "frame_id": "DEFAULT_ID",
                })
                
                resolution = request.resolution
                n_z_resolution = request.n_z_resolution
                voxel_grid = self.projectairsim_world.create_voxel_grid(center_pose, resolution*request.ncells_x, resolution*request.ncells_y, resolution*request.ncells_z, resolution, n_z_resolution, use_segmentation = False, write_file = True, file_path = request.output_file)                 
                
                response.success = (len(voxel_grid) != 0)
                return response
            

    class CreateSegmentedVoxelGridService(BasicService):
                def __init__(self, service_manager, service_type):
                    super().__init__(service_manager, "airsim_node/create_segmented_voxel_grid", service_type)

                def callback(self, request, response):
                    self.logger.info(f"Generating Map")
                    center_pose = Pose({
                        "translation": Vector3({"x": request.position_x, "y": request.position_y, "z": request.position_z}),
                        "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
                        "frame_id": "DEFAULT_ID",
                    })
                    
                    resolution = request.resolution
                    n_z_resolution = request.n_z_resolution
                    voxel_grid = self.projectairsim_world.create_voxel_grid(center_pose, resolution*request.ncells_x, resolution*request.ncells_y, resolution*request.ncells_z, resolution, n_z_resolution, use_segmentation = True, write_file = True, file_path = request.output_file)                 
                    
                    response.success = (len(voxel_grid) != 0)
                    return response            
                
           
    class CreateOccupancyGridService(BasicService):
            def __init__(self, service_manager, service_type):
                self.parent_topic = f"{service_manager.world_parent_topic}"
                super().__init__(service_manager, "airsim_node/create_occupancy_grid", service_type)
                #initialize qos profile
                self.qos_profile = self._create_qos_profile()
                self.occupancy_grid_publisher = "/airsim_node/occupancy_grid"
                #initialize occupancy grid publisher
                self._initialize_publisher()

                
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
                    topic_name=self.occupancy_grid_publisher,
                    ros_message_type=OccupancyGridMsg,
                    peer_change_callback=None,
                    is_latching=True,
                    ros_qos_profile=self.qos_profile
                )            

            def callback(self, request, response):
                self.logger.info(f"Generating Map")
                
                center_pose = Pose({
                    "translation": Vector3({"x": request.position_x, "y": request.position_y, "z": request.position_z}),
                    "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
                    "frame_id": "DEFAULT_ID",
                })
                
                resolution = request.res
                n_z_resolution = request.n_z_resolution
                voxel_grid = self.projectairsim_world.create_voxel_grid(center_pose, resolution*request.ncells_x, resolution*request.ncells_y, 
                                                                        resolution * n_z_resolution, resolution, n_z_resolution, use_segmentation = False, 
                                                                        write_file = False, file_path ="")         
                
                voxel_grid_3d = self._reshape_voxel_grid(voxel_grid, request.ncells_x, request.ncells_y, 1)
                occupancy_grid = self._create_2d_occupancy_from_3d_array(voxel_grid_3d)
                flattened_occupancy_grid = occupancy_grid.flatten().tolist()


                response.map.header = Header()
                response.map.header.frame_id = "map"
                
                response.map.info.resolution = request.res  # 5 cm per cell
                response.map.info.width = request.ncells_x
                response.map.info.height = request.ncells_y
                response.map.info.origin.position.x = request.position_x
                response.map.info.origin.position.y = request.position_y
                response.map.info.origin.position.z = -request.position_z
                response.map.info.origin.orientation.x = 0.0
                response.map.info.origin.orientation.y = 0.0
                response.map.info.origin.orientation.z = 0.0
                response.map.info.origin.orientation.w = 1.0
                
                # Set occupancy data to voxel grid
                response.map.data =  [127 if cell else 0 for cell in flattened_occupancy_grid]

                occupancy_grid_msg = OccupancyGridMsg()
                occupancy_grid_msg.data = response.map.data
                occupancy_grid_msg.header = response.map.header
                occupancy_grid_msg.info = response.map.info
        
                response.success = (len(voxel_grid) != 0)
                if response.success:
                    self.topics_manager.publish(self.occupancy_grid_publisher, occupancy_grid_msg)

                return response   

            def _reshape_voxel_grid(self, voxel_grid, ncells_x, ncells_y, ncells_z):
                """Reshape the voxel grid to a 3d array."""
                np_voxel_grid = np.array(voxel_grid)
                return np_voxel_grid.reshape((ncells_x, ncells_y, ncells_z))
            
            def _create_2d_occupancy_from_3d_array(self, voxel_grid):
                """Create a 2D occupancy grid from a 3D array."""
                return np.max(voxel_grid, axis=2) > 0
                          
    class GetSegIdFromMeshService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/get_seg_id_from_mesh", service_type)

        def callback(self, request, response):
            self.logger.debug(f"Getting segmentation id from mesh id")
            seg_id = self.projectairsim_world.get_segmentation_id_by_name(request.object_id, True)
            response.success = (seg_id != -1)
            response.segmentation_id = seg_id
            self.logger.debug(f"Getting segmentation id from mesh id return response")
            return response        

    class GetColorFromMeshService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/get_color_from_mesh", service_type)

        def callback(self, request, response):
            self.logger.debug(f"Getting color from mesh ID")
            seg_id = self.projectairsim_world.get_segmentation_id_by_name(request.object_id, True)

            if seg_id == -1:
                self.logger.error(f"Error retrieving segmentation ID: Given mesh ID is invalid: {request.object_id}")
                response.success = False
                return response
            
            if not 0 <= seg_id <= 255:
                self.logger.error(f"Error retrieving color: Segmentation ID must be between 0 and 255)")
                response.success = False
                return response

            self.logger.debug(f"Calling segmentation_id_to_color")
            color = segmentation_id_to_color(seg_id)
            self.logger.debug(f"Returned segmentation_id_to_color")
            response.r, response.g, response.b = color.r, color.g, color.b
            response.success = True
            self.logger.debug(f"Getting color from mesh ID return response")
            return response        

    class GetObjIdFromSegIdService(BasicService):
        def __init__(self, service_manager, service_type):

            self.parent_topic = f"{service_manager.world_parent_topic}"
            super().__init__(service_manager, "airsim_node/get_mesh_from_seg_id", service_type)
            seg_map = self.projectairsim_world.get_segmentation_id_map()
            self.my_map = {}

            # Populate the map
            for id_, value in seg_map.items():
                if value not in self.my_map:
                  self.my_map[value] = []
                self.my_map[value].append(id_)

        def callback(self, request, response):
            self.logger.debug(f"GetObjIdFromSegIdService callback entry")
            segmentation_id = request.segmentation_id

            if segmentation_id in self.my_map:
                response.object_ids = self.my_map[segmentation_id]
                response.success = True
                self.logger.debug(f"Successfully retrieved objects IDs for segmentation ID {segmentation_id}")
            else:
                response.object_ids = []
                response.success = False
                self.logger.error(f"No objects IDs found for segmentation ID {segmentation_id}")
           
            self.logger.debug(f"GetObjIdFromSegIdService return response")
            return response

    class GetSegIdFromColorService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/get_seg_id_from_color", service_type)

        def callback(self, request, response):
            self.logger.debug(f"GetSegIdFromColorService callback entry")
            color_to_find = [request.r, request.g, request.b]
            color_index = -1
            response.success = False
            for index, color in enumerate(SEGMENTATION_PALLETE):
                if color == color_to_find:
                    color_index = index
                    response.success = True
                    break
                
            response.segmentation_id = color_index
            response.success = True
            self.logger.debug(f"GetSegIdFromColorService return response")
            return response                                                   
 
    class GetColorFromSegIdService(BasicService):
            def __init__(self, service_manager, service_type):
                super().__init__(service_manager, "airsim_node/get_color_from_seg_id", service_type)

            def callback(self, request, response):
                self.logger.debug(f"GetColorFromSegIdService callback entry")
                try:
                    segmentation_id = request.segmentation_id
                    if not 0 <= segmentation_id <= 255:
                        raise ValueError("Segmentaiton ID must be between 0 and 255")
                    
                    color = segmentation_id_to_color(segmentation_id)  
                    response.r, response.g, response.b = color.r, color.g, color.b
                    response.success = True
                except Exception as e:
                    response.success = False
                    self.logger.error(f"Error processing segmentation ID: {e}")
                     
                self.logger.debug(f"GetColorFromSegIdService return response")
                return response
            
    class GetOriginGeoPointService(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/origin_geo_point", service_type)

        def callback(self, request, response):            
            if self.projectairsim_world is not None:
                home_geo_point = self.projectairsim_world.home_geo_point
                response.latitude = home_geo_point["latitude"]
                response.longitude = home_geo_point["longitude"]
                response.altitude = home_geo_point["altitude"]
            
            return response
        
    class GetClock(BasicService):
        def __init__(self, service_manager, service_type):
            super().__init__(service_manager, "airsim_node/clock", service_type)

        def callback(self, request, response):            
            if self.projectairsim_world is not None:
                sim_time_nano = self.projectairsim_world.get_sim_time()
                response.nanosec = sim_time_nano
            
            return response
