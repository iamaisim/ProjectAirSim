"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
Python API class for ProjectAirSim World.
"""
import commentjson
from typing import Dict, List
import time
from datetime import datetime
import numpy as np
import math
import random

from projectairsim import ProjectAirSimClient
from projectairsim.types import Vector3, Quaternion, Pose, BoxAlignment
from projectairsim.utils import (
    projectairsim_log,
    get_pitch_between_traj_points,
    get_heading_between_traj_points,
    load_scene_config_as_dict,
    get_trajectory_from_kml,
    validate_trajectory,
    convert_string_with_spaces_to_float_list,
    rpy_to_quaternion,
    get_point_to_line_segment_distance,
    generate_perpendicular_unit_vector,
    rotate_vector_about_axis,
    calculate_path_time,
    calculate_path_length,
    get_point_distance_along_path,
    get_voxel_grid_idx,
)
from projectairsim.planners import AStarPlanner


class World(object):
    def __init__(
        self,
        client: ProjectAirSimClient,
        scene_config_name: str = "",
        delay_after_load_sec: int = 0,
        sim_config_path: str = "sim_config/",
        sim_instance_idx: int = -1,
        actual_load: bool = True,
    ):
        """ProjectAirSim World Interface.

        Args:
            client (ProjectAirSimClient): ProjectAirSim client object
            scene_config (str): Name of the scene config JSON file to load in the sim
            delay_after_load_sec (int): Time in seconds to wait after the scene is loaded
            sim_config_path (string): Relative path to search for the scene_config
            sim_instance_idx (int): the instance index of the simulation (for distributed sim only)
        """
        self.client = client
        self.sim_config_path = sim_config_path
        self.sim_instance_idx = sim_instance_idx
        self.parent_topic = "/Sim/SceneBasicDrone"  # default-scene's ID

        self.sim_config = None
        self.home_geo_point = None
        if scene_config_name:
            config_loaded, config_paths = load_scene_config_as_dict(
                scene_config_name,
                sim_config_path,
                sim_instance_idx,
            )
            config_dict = config_loaded
            self.scene_config_path = config_paths[0]
            self.robot_config_paths = config_paths[1]
            self.envactor_config_paths = config_paths[2]
            self.envobject_config_paths = config_paths[3]
            self.load_scene(config_dict, delay_after_load_sec=delay_after_load_sec, actual_load = actual_load)
        random.seed()
        self.import_ned_trajectory(
            "null_trajectory", [0, 1], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]
        )

    def get_configuration(self) -> Dict:
        """Get the current configuration that has been loaded to the sim server

        Returns:
            Dict: the configuration
        """
        return self.sim_config

    def get_sim_clock_type(self) -> str:
        """Get sim clock type

        Returns:
            string: the sim clock type ("steppable" or "real-time")
        """
        sim_clock_type_req: Dict = {
            "method": f"{self.parent_topic}/GetSimClockType",
            "params": {},
            "version": 1.0,
        }
        sim_clock_type_result: str = self.client.request(sim_clock_type_req)
        return sim_clock_type_result

    def get_sim_time(self) -> int:
        """Get current sim time in nanoseconds

        Returns:
            int: the sim time in nanoseconds
        """
        sim_time_req: Dict = {
            "method": f"{self.parent_topic}/GetSimTime",
            "params": {},
            "version": 1.0,
        }
        sim_time: int = self.client.request(sim_time_req)
        return sim_time

    def pause(self) -> str:
        """Pause simulation

        Returns:
            string: a human-readable string with the pause state and time
        """
        pause_req: Dict = {
            "method": f"{self.parent_topic}/Pause",
            "params": {"do_pause": True},
            "version": 1.0,
        }
        pause_resume_state = self.client.request(pause_req)
        return pause_resume_state

    def set_sunlight_intensity(self, intensity) -> bool:
        """Set sun light intensity

        Args:
            intensity (float): the sun light intensity, from 0 to 75000 lux

        Returns:
            bool: true if successful, false otherwise
        """
        set_light: Dict = {
            "method": f"{self.parent_topic}/SetSunLightIntensity",
            "params": {"intensity": intensity},
            "version": 1.0,
        }
        set_light_status = self.client.request(set_light)
        return set_light_status

    def get_sunlight_intensity(self) -> float:
        """Get sun light intensity

        Returns:
            get_light_intensity (float): the sun light intensity from 0-75000 lux
        """
        get_light: Dict = {
            "method": f"{self.parent_topic}/GetSunLightIntensity",
            "params": {},
            "version": 1.0,
        }
        get_light_intensity = self.client.request(get_light)
        return get_light_intensity

    def set_cloud_shadow_strength(self, strength: float) -> bool:
        """Set cloud shadow strength

        Args:
            strength (float): the cloud shadow strength, from 0.0 to 1.0

        Returns:
            bool: true if successful, false otherwise
        """
        set_shadow: Dict = {
            "method": f"{self.parent_topic}/SetCloudShadowStrength",
            "params": {"strength": strength},
            "version": 1.0,
        }
        set_shadow_status = self.client.request(set_shadow)
        return set_shadow_status

    def get_cloud_shadow_strength(self) -> float:
        """Get cloud shadow strength

        Returns:
            float: the cloud shadow strength, from 0.0 to 1.0
        """
        get_shadow: Dict = {
            "method": f"{self.parent_topic}/GetCloudShadowStrength",
            "params": {},
            "version": 1.0,
        }
        get_shadow_strength = self.client.request(get_shadow)
        return get_shadow_strength

    def set_wind_velocity(self, v_x: float, v_y: float, v_z: float) -> bool:
        """Set global wind velocity by specifying velocity vector components

        Args:
            v_x (float): the x-direction wind velocity in m/s
            v_y (float): the y-direction wind velocity in m/s
            v_z (float): the z-direction wind velocity in m/s

        Returns:
            bool: true if successful, false otherwise
        """
        wind_setting: Dict = {
            "method": f"{self.parent_topic}/SetWindVelocity",
            "params": {"v_x": v_x, "v_y": v_y, "v_z": v_z},
            "version": 1.0,
        }
        wind_setting_state = self.client.request(wind_setting)
        return wind_setting_state

    def get_wind_velocity(self) -> tuple:
        """Returns the current global wind velocity as a tuple

        Returns:
            tuple: a tuple (x,y,z) of global wind velocities in m/s
        """
        get_wind: Dict = {
            "method": f"{self.parent_topic}/GetWindVelocity",
            "params": {},
            "version": 1.0,
        }
        wind_vel = self.client.request(get_wind)
        assert len(wind_vel) == 3
        return (wind_vel[0], wind_vel[1], wind_vel[2])

    def resume(self) -> str:
        """Resume simulation

        Returns:
            string: a human-readable string with the pause state and time
        """
        resume_req: Dict = {
            "method": f"{self.parent_topic}/Pause",
            "params": {"do_pause": False},
            "version": 1.0,
        }
        pause_resume_state = self.client.request(resume_req)
        return pause_resume_state

    def is_paused(self) -> bool:
        """Check if sim is paused

        Returns:
            bool: true if paused, false otherwise
        """
        is_paused_req: Dict = {
            "method": f"{self.parent_topic}/IsPaused",
            "params": {},
            "version": 1.0,
        }
        is_paused_state: bool = self.client.request(is_paused_req)
        return is_paused_state

    def continue_for_sim_time(
        self, delta_time_nanos: int, wait_until_complete: bool = True
    ) -> int:
        """Continue sim for delta_time_nanos and then pause

        Args:
            delta_time_nanos (int): the number of nanoseconds to continue for
            wait_until_complete (bool): if true, this function will wait until the time has passed before returning

        Returns:
            int: the sim time when the function returns
        """
        continue_for_simtime_req: Dict = {
            "method": f"{self.parent_topic}/ContinueForSimTime",
            "params": {
                "delta_time": delta_time_nanos,
                "wait_until_complete": wait_until_complete,
            },
            "version": 1.0,
        }
        continue_for_simtime_result: str = self.client.request(continue_for_simtime_req)
        return continue_for_simtime_result

    def continue_until_sim_time(
        self, target_time_nanos: int, wait_until_complete: bool = True
    ) -> int:
        """Continue sim until target_time_nanos and then pause

        Args:
            target_time_nanos (int): the time in nanoseconds to pause at
            wait_until_complete (bool): if true, this function will wait until the time has passed before returning

        Returns:
            int: the sim time when the function returns
        """
        continue_until_simtime_req: Dict = {
            "method": f"{self.parent_topic}/ContinueUntilSimTime",
            "params": {
                "target_time": target_time_nanos,
                "wait_until_complete": wait_until_complete,
            },
            "version": 1.0,
        }
        continue_until_simtime_result: str = self.client.request(
            continue_until_simtime_req
        )
        return continue_until_simtime_result

    def continue_for_n_steps(
        self, n_steps: int, wait_until_complete: bool = True
    ) -> int:
        """Continue sim for N steps and then pause

        Args:
            n_steps (int): the number of steps to continue for
            wait_until_complete (bool): if true, this function will wait until the time has passed before returning

        Returns:
            int: the sim time when the function returns
        """
        continue_for_n_steps_req: Dict = {
            "method": f"{self.parent_topic}/ContinueForNSteps",
            "params": {"n_steps": n_steps, "wait_until_complete": wait_until_complete},
            "version": 1.0,
        }
        continue_for_n_steps_result: str = self.client.request(continue_for_n_steps_req)
        return continue_for_n_steps_result

    def continue_for_single_step(self, wait_until_complete: bool = True) -> int:
        """Continue sim for a single step and then pause

        Args:
            wait_until_complete (bool): if true, this function will wait until the time has passed before returning

        Returns:
            int: the sim time when the function returns
        """
        single_step_req: Dict = {
            "method": f"{self.parent_topic}/ContinueForSingleStep",
            "params": {"wait_until_complete": wait_until_complete},
            "version": 1.0,
        }
        single_step_result: str = self.client.request(single_step_req)
        return single_step_result

    def list_actors(self) -> List[str]:
        """List actor names in the scene

        Returns:
            List[str]: a list of actor names
        """
        list_actors_req: Dict = {
            "method": f"{self.parent_topic}/ListActors",
            "params": {},
            "version": 1.0,
        }
        actor_names = self.client.request(list_actors_req)
        return actor_names

    def list_objects(self, name_regex: str) -> List:
        """List scene object names matching a regex

        Args:
            name_regex (str): the regex to match object names against

        Returns:
            List[str]: a list of object names
        """
        list_obj_req: Dict = {
            "method": f"{self.parent_topic}/ListObjects",
            "params": {"name": name_regex},
            "version": 1.0,
        }
        scene_objects_list = self.client.request(list_obj_req)
        return scene_objects_list

    def list_assets(self, name_regex: str = ".*") -> List:
        """List available static and blueprint assets matching a regex

        Args:
            name_regex {str}: the regex to match asset names against

        Returns:
            {List}: a list of the available asset names
        """
        list_obj_req: Dict = {
            "method": f"{self.parent_topic}/ListAssets",
            "params": {"name": name_regex},
            "version": 1.0,
        }
        scene_objects_list = self.client.request(list_obj_req)
        return scene_objects_list

    def get_object_pose(self, object_name: str) -> Pose:
        """Get a scene object's pose

        Args:
            object_name (str): the object to get information about

        Returns:
            Pose: the object pose
        """
        get_obj_pose_req: Dict = {
            "method": f"{self.parent_topic}/GetObjectPose",
            "params": {"object_name": object_name},
            "version": 1.0,
        }
        object_pose = self.client.request(get_obj_pose_req)
        assert "translation" in object_pose.keys(), "Invalid Pose result"
        assert "rotation" in object_pose.keys(), "Invalid Pose result"
        object_pose = Pose(object_pose)
        return object_pose

    def get_object_poses(self, object_names: List[str]) -> List[Pose]:
        """Get multiple scene objects' poses

        Args:
            object_names (List[str]): the objects to be read

        Returns:
            List[Pose]: the object poses corresponding to the objects in object_names, in the same order
        """
        get_obj_poses_req: Dict = {
            "method": f"{self.parent_topic}/GetObjectPoses",
            "params": {"object_names": object_names},
            "version": 1.0,
        }
        object_poses = self.client.request(get_obj_poses_req)
        for i, obj_pose in enumerate(object_poses):
            assert "translation" in obj_pose.keys(), "Invalid Pose result"
            assert "rotation" in obj_pose.keys(), "Invalid Pose result"
            object_poses[i] = Pose(obj_pose)
        return object_poses

    def set_object_pose(
        self, object_name: str, object_pose: Pose, teleport: bool
    ) -> str:
        """Set a scene object's pose

        Args:
            object_name (str): the object to move
            object_pose (Pose): the new pose
            teleport (bool): whether the object is teleported.

        Returns:
            bool: whether the operation was successful
        """
        set_obj_pose_req: Dict = {
            "method": f"{self.parent_topic}/SetObjectPose",
            "params": {
                "object_name": object_name,
                "pose": object_pose,
                "teleport": teleport,
            },
            "version": 1.0,
        }
        status = self.client.request(set_obj_pose_req)
        return status

    def get_object_scale(self, object_name: str) -> List:
        """Get a scene object's scale (x, y, z)

        Args:
            object_name (str): the object to get information about

        Returns:
            List[float]: the scale as a 3-item list [x,y,z]
        """
        get_obj_scale_req: Dict = {
            "method": f"{self.parent_topic}/GetObjectScale",
            "params": {"object_name": object_name},
            "version": 1.0,
        }
        object_scale = self.client.request(get_obj_scale_req)
        return object_scale

    def set_object_scale(self, object_name: str, object_scale: list) -> str:
        """Set a scene object's scale (x, y, z)

        Args:
            object_name (str): the object to change the scale of
            object_scale (list): the scale (x,y,z)

        Returns:
            bool: true if successful, false otherwise
        """
        set_obj_scale_req: Dict = {
            "method": f"{self.parent_topic}/SetObjectScale",
            "params": {"object_name": object_name, "scale": object_scale},
            "version": 1.0,
        }
        status = self.client.request(set_obj_scale_req)
        return status

    def spawn_object(
        self,
        object_name: str,
        asset_path: str,
        object_pose: Pose,
        object_scale: List[float],
        enable_physics: bool,
    ) -> str:
        """Spawn an object in the scene using an asset packaged into the sim server

        Args:
            object_name (str): name to be given to the spawned object
            asset_path (str): path to the asset in the sim server
            object_pose (Pose): the initial pose of the spawned object
            object_scale (List[float]): the scale (x,y,z) of the spawned object
            enable_physics (bool): whether the spawned object should be affected by physics

        Returns:
            str: the name of the spawned object. The name may have been modified to make it unique
        """
        spawn_obj_req: Dict = {
            "method": f"{self.parent_topic}/SpawnObject",
            "params": {
                "object_name": object_name,
                "asset_path": asset_path,
                "pose": object_pose,
                "scale": object_scale,
                "enable_physics": enable_physics,
            },
            "version": 1.0,
        }
        status = self.client.request(spawn_obj_req)
        return status

    def spawn_object_from_file(
        self,
        object_name: str,
        file_format: str,
        byte_array: bytes,
        is_binary: bool,
        object_pose: Pose,
        object_scale: List[float],
        enable_physics: bool,
    ) -> str:
        """Spawn an object in the scene using a client asset, passing the data to the sim server

        Args:
            object_name (str): name to be given to the spawned object
            file_format (str): file format. Currently only supports "gltf"
            byte_array (bytes): the raw file data
            is_binary (bool): whether the file data is binary
            object_pose (Pose): the pose of the spawned object
            object_scale (List[float]): the scale of the spawned object, as [x,y,z]
            enable_physics (bool): whether the spawned object should be affected by physics

        Returns:
            (str): the name of the spawned object. The name may have been modified to make it unique
        """
        spawn_obj_req: Dict = {
            "method": f"{self.parent_topic}/spawnObjectFromFile",
            "params": {
                "object_name": object_name,
                "file_format": file_format,
                "byte_array": byte_array,
                "is_binary": is_binary,
                "pose": object_pose,
                "scale": object_scale,
                "enable_physics": enable_physics,
            },
            "version": 1.0,
        }
        status = self.client.request(spawn_obj_req)
        return status

    def spawn_object_from_file_at_geo(
        self,
        object_name: str,
        file_format: str,
        byte_array: bytes,
        is_binary: bool,
        latitude: float,
        longitude: float,
        altitude: float,
        rotation: List[float],
        object_scale: List[float],
        enable_physics: bool,
    ) -> str:
        """Spawn an in the scene using a client asset, passing the data to the sim server, at the given geodetic coordinates

        Args:
            object_name (str): name to be given to the spawned object
            file_format (str): file format. Currently only supports "gltf"
            byte_array (bytes): the raw file data
            is_binary (bool): whether the file data is binary
            latitude (float): the latitude to spawn the object at, in degrees
            longitude (float): the longitude to spawn the object at, in degrees
            altitude (float): the altitutde to spawn the object at, in meters
            rotation (List[float]): the rotation, as a [w,x,y,z] quaternion
            object_scale (List[float]): the scale of the spawned object, as [x,y,z]
            enable_physics (bool): whether the spawned object should be affected by physics

        Returns:
            str: the name of the spawned object. The name may have been modified to make it unique
        """
        spawn_obj_req: Dict = {
            "method": f"{self.parent_topic}/spawnObjectFromFileAtGeo",
            "params": {
                "object_name": object_name,
                "file_format": file_format,
                "byte_array": byte_array,
                "is_binary": is_binary,
                "latitude": latitude,
                "longitude": longitude,
                "altitude": altitude,
                "rotation": rotation,
                "scale": object_scale,
                "enable_physics": enable_physics,
            },
            "version": 1.0,
        }
        status = self.client.request(spawn_obj_req)
        return status

    def spawn_object_at_geo(
        self,
        object_name: str,
        asset_path: str,
        latitude: float,
        longitude: float,
        altitude: float,
        rotation: List[float],
        object_scale: List[float],
        enable_physics: bool,
    ) -> str:
        """Spawn an object in the scene using an asset packaged into the sim server, at the given geodetic coordinates

        Args:
            object_name (str): name to be given to the spawned object
            asset_path (str): path to the asset in the sim server
            latitude (float): the latitude to spawn the object at, in degrees
            longitude (float): the longitude to spawn the object at, in degrees
            altitude (float): the altitude to spawn the object at, in meters
            rotation (List[float]): the rotation, as a [w,x,y,z] quaternion
            object_scale (List[float]): the scale (x,y,z) of the spawned object
            enable_physics (bool): whether the spawned object should be affected by physics

        Returns:
            str: the name of the spawned object. The name may have been modified to make it unique
        """
        spawn_obj_req: Dict = {
            "method": f"{self.parent_topic}/spawnObjectAtGeo",
            "params": {
                "object_name": object_name,
                "asset_path": asset_path,
                "latitude": latitude,
                "longitude": longitude,
                "altitude": altitude,
                "rotation": rotation,
                "scale": object_scale,
                "enable_physics": enable_physics,
            },
            "version": 1.0,
        }
        status = self.client.request(spawn_obj_req)
        return status

    def destroy_object(self, object_name: str) -> str:
        """Destroy an object in the scene

        Args:
            object_name (str): the name of the object to destroy

        Returns:
            bool: whether the object was destroyed
        """
        destroy_obj_req: Dict = {
            "method": f"{self.parent_topic}/DestroyObject",
            "params": {"object_name": object_name},
            "version": 1.0,
        }
        status = self.client.request(destroy_obj_req)
        return status

    def destroy_all_spawned_objects(self) -> str:
        """Destroy all spawned objects in the scene

        Returns:
            bool: whether the operation was successful
        """
        destroy_all_spawned_obj_req: Dict = {
            "method": f"{self.parent_topic}/DestroyAllSpawnedObjects",
            "params": {},
            "version": 1.0,
        }
        status = self.client.request(destroy_all_spawned_obj_req)
        return status

    def enable_weather_visual_effects(self) -> bool:
        """Enable weather visual effects

        Returns:
            bool: whether the operation was successful
        """
        enable_weather_fx_req: Dict = {
            "method": f"{self.parent_topic}/SimSetWeatherVisualEffectsStatus",
            "params": {"status": True},
            "version": 1.0,
        }
        status = self.client.request(enable_weather_fx_req)
        return status

    def disable_weather_visual_effects(self) -> bool:
        """Disable weather visual effects

        Returns:
            bool: whether the operation was successful
        """
        enable_weather_fx_req: Dict = {
            "method": f"{self.parent_topic}/SimSetWeatherVisualEffectsStatus",
            "params": {"status": False},
            "version": 1.0,
        }
        status = self.client.request(enable_weather_fx_req)
        return status

    def reset_weather_effects(self) -> bool:
        """Returns all weather visual effects to their default levels

        Returns:
            bool: whether the operation was successful
        """
        reset_weather_fx_req: Dict = {
            "method": f"{self.parent_topic}/ResetWeatherEffects",
            "params": {},
            "version": 1.0,
        }
        status = self.client.request(reset_weather_fx_req)
        return status

    def set_weather_visual_effects_param(self, param: int, value: float) -> bool:
        """Set a parameter for the weather visual effects

        Args:
            param (int): the kind of weather to set. See WeatherParameter for legal values.
            value (float): the intensity of the weather, from 0.0 to 1.0

        Returns:
            bool: whether the operation was successful
        """
        set_weather_fx_param_req: Dict = {
            "method": f"{self.parent_topic}/SetWeatherVisualEffectsParameter",
            "params": {"param": param, "value": value},
            "version": 1.0,
        }
        status = self.client.request(set_weather_fx_param_req)
        return status

    def get_weather_visual_effects_param(self) -> Dict[int, float]:
        """Get the current weather visual effects parameters

        Returns:
            Dict[int, float]: the current weather visual effects parameters.
            The keys are the WeatherParameter enum values, and the values are the intensity of the weather.
        """
        get_weather_fx_param_req: Dict = {
            "method": f"{self.parent_topic}/GetWeatherVisualEffectsParameter",
            "params": {},
            "version": 1.0,
        }
        weather_param_dict = self.client.request(get_weather_fx_param_req)

        # Since JSON doesn't support int keys, the returned dict has the WeatherParameter int
        # keys as strings. Convert keys from strings of int to native int types for
        # easier direct access with the IntEnum WeatherParameter.
        str_keys = [*weather_param_dict]
        for key in str_keys:
            weather_param_dict[int(key)] = weather_param_dict.pop(key)
        return weather_param_dict

    def set_object_material(self, object_name: str, material_asset_path: str) -> bool:
        """Set a scene object's material from the material name
        Args:
            object_name (str): name of the object to set the material of
            material_asset_path (str): path pointing to a packaged material asset

        Returns:
            bool: whether the operation was successful or not
        """
        set_material_req: Dict = {
            "method": f"{self.parent_topic}/SetObjectMaterial",
            "params": {
                "object_name": object_name,
                "material_path": material_asset_path,
            },
            "version": 1.0,
        }
        status = self.client.request(set_material_req)
        return status

    def set_object_texture_from_url(self, object_name: str, url: str) -> bool:
        """Set a scene object's texture from a URL

        Args:
            object_name (str): the object to change the texture of
            url (str): a url pointing to an image texture

        Returns:
            bool: whether the operation was sucessful
        """
        set_texture_from_url_req: Dict = {
            "method": f"{self.parent_topic}/SetObjectTextureFromUrl",
            "params": {"object_name": object_name, "url": url},
            "version": 1.0,
        }
        status = self.client.request(set_texture_from_url_req)
        return status

    def set_object_texture_from_file(
        self, object_name: str, texture_file_path: str
    ) -> bool:
        """Set a scene object's texture from a file (PNG)

        Args:
            object_name (str): name of the object to change the texture of
            texture_file_path (str): PNG file path

        Returns:
            bool: whether the operation was successful or not
        """
        file = open(texture_file_path, "rb")
        raw_img = file.read()
        file.close()

        set_texture_from_file_req: Dict = {
            "method": f"{self.parent_topic}/SetObjectTextureFromFile",
            "params": {"object_name": object_name, "raw_img": raw_img},
            "version": 1.0,
        }
        status = self.client.request(set_texture_from_file_req)
        return status

    def set_object_texture_from_packaged_asset(
        self, object_name: str, texture_asset_path: str
    ) -> bool:
        """Set a scene object's material from a texture

        Args:
            object_name (str): name of the object to set the material of
            texture_asset_path (str): path pointing to a packaged texture asset

        Returns:
            bool: whether the operation was successful or not
        """
        set_texture_from_packaged_asset_req: Dict = {
            "method": f"{self.parent_topic}/SetObjectTextureFromPackagedAsset",
            "params": {"object_name": object_name, "texture_path": texture_asset_path},
            "version": 1.0,
        }
        status = self.client.request(set_texture_from_packaged_asset_req)
        return status

    def swap_object_texture(self, tag: str, tex_id: int) -> List:
        """Swap textures using IDs for all scene objects with specified actor tags (set
        in Editor's actor details under the 'Actor' section, not the component tags
        in the 'Tags' section

        Args:
            tag (str): the actor tag
            text_id (int): the texture id

        Returns:
            List: a list with the names of all affected objects
        """
        swap_texture_req: Dict = {
            "method": f"{self.parent_topic}/SwapObjectTexture",
            "params": {
                "tag": tag,
                "tex_id": tex_id,
                "component_id": 0,
                "material_id": 0,
            },
            "version": 1.0,
        }
        status = self.client.request(swap_texture_req)
        return status
    
    def set_light_object_intensity(self, object_name: str, new_intensity: float) -> bool:
        """Set a light object's intensity
        Args:
            object_name (str): name of the object to set the light intensity of
            new_intensity (float): light object's new intesity value 
            (lux for directional lights, unitless(see ue docs) for other light types)

        Returns:
            bool: whether the operation was successful or not
        """
        set_light_intensity_req: Dict = {
            "method": f"{self.parent_topic}/SetLightObjectIntensity",
            "params": {
                "object_name": object_name,
                "new_intensity": new_intensity,
            },
            "version": 1.0,
        }
        status = self.client.request(set_light_intensity_req)
        return status
    
    def set_light_object_color(self, object_name: str, color_rgb: List[float]) -> bool:
        """Set a light object's color
        Args:
            object_name (str): name of the object to set the light intensity of
            color_rgb (List[float]): [red, green, blue] values (range: 0.0 - 1.0)

        Returns:
            bool: whether the operation was successful or not
        """
        set_light_color_req: Dict = {
            "method": f"{self.parent_topic}/SetLightObjectColor",
            "params": {
                "object_name": object_name,
                "color_rgb": color_rgb,
            },
            "version": 1.0,
        }
        status = self.client.request(set_light_color_req)
        return status
        
    def set_light_object_radius(self, object_name: str, new_radius: float) -> bool:
        """Set a light object's atteniation radius (only affects point and spot lights)
        Args:
            object_name (str): name of the object to set the light intensity of
            new_radius (float): light object's attenuation radius in meters

        Returns:
            bool: whether the operation was successful or not
        """
        set_light_radius_req: Dict = {
            "method": f"{self.parent_topic}/SetLightObjectRadius",
            "params": {
                "object_name": object_name,
                "new_radius": new_radius,
            },
            "version": 1.0,
        }
        status = self.client.request(set_light_radius_req)
        return status

    def set_time_of_day(
        self,
        status: bool,
        datetime: str,
        is_dst: bool,
        clock_speed: float,
        update_interval: float,
        move_sun: bool,
    ) -> bool:
        """Set the time of day

        Args:
            status (bool): true to enable time of day, false to disable
            datetime (str): the time of day. Format must be Year-Month-Day Hour:Minute:Second
            is_dst (bool): whether the time given is a dst time
            clock_speed (float): the celestial clock speed. 1.0 is real time.
            update_interval (float): the frequency, in seconds, with which time of day should be updated
            move_sun (bool): whether the sun should be moved as the time of day changes

        Returns:
            bool: whether the operation was sucessful
        """
        set_time_of_day_req: Dict = {
            "method": f"{self.parent_topic}/SetTimeOfDay",
            "params": {
                "status": status,
                "datetime": datetime,
                "is_dst": is_dst,
                "clock_speed": clock_speed,
                "update_interval": update_interval,
                "move_sun": move_sun,
            },
            "version": 1.0,
        }
        status = self.client.request(set_time_of_day_req)
        return status

    def get_time_of_day(self) -> str:
        """Get the time of day

        Returns:
            str: a string containing the time of day in the format `%Y-%m-%d %H:%M:%S`
        """
        get_time_of_day_req: Dict = {
            "method": f"{self.parent_topic}/GetSimTimeofDay",
            "params": {},
            "version": 1.0,
        }
        status = self.client.request(get_time_of_day_req)
        return status

    def set_sun_position_from_date_time(
        self, date_time: datetime, is_dst: bool = False
    ) -> bool:
        """Set the sun position from dateTime

        Args:
            datetime (str): the time of day. Format must be Year-Month-Day Hour:Minute:Second
            is_dst (bool): whether the time given is a dst time

        Returns:
            bool: whether the operation was sucessful
        """
        format = "%Y-%m-%d %H:%M:%S"
        date_str = date_time.strftime(format)
        set_sun_angle_req: Dict = {
            "method": f"{self.parent_topic}/SetSunPositionFromDateTime",
            "params": {"datetime": date_str, "format": format, "is_dst": is_dst},
            "version": 1.0,
        }

        status = self.client.request(set_sun_angle_req)
        return status

    def load_scene(
        self, scene_config_dict: Dict, delay_after_load_sec: float = 0, actual_load: bool = True
    ) -> str:
        """(Re)loads the sim scene config to the sim server

        Args:
            scene_config_dict (Dict): a Dict with all scene configuration info
            delay_after_load_sec (float): time to sleep after loading the scene

        Returns:
            str: the id of the scene
        """

        # Force sim to pause on start if there are objects to spawn
        clock_is_steppable = scene_config_dict["clock"]["type"] == "steppable"
        pause_on_start_by_user = False
        if scene_config_dict["clock"].get("pause-on-start") is not None:
            pause_on_start_by_user = scene_config_dict["clock"]["pause-on-start"]

        if scene_config_dict.get("spawn-objects") is not None:
            if clock_is_steppable:
                scene_config_dict["clock"]["pause-on-start"] = True
                projectairsim_log().info(
                    "Pausing sim clock on start until all objects have been spawned."
                )
            else:
                projectairsim_log().warn(
                    "For real-time clock types, assets cannot be spawned before starting the sim clock."
                )

        sim_config_data = commentjson.dumps(scene_config_dict)
        load_scene_req: Dict = {
            "method": "/Sim/LoadScene",
            "params": {"scene_config": sim_config_data},
            "version": 1.0,
        }
        self.sim_config = scene_config_dict
        
        if(actual_load):
            self.client.unsubscribe_all()
            scene_id = self.client.request(load_scene_req)
            self.client.get_topic_info()  # get new scene's list of registered topic info
        else:
            scene_id = scene_config_dict["id"]

        time.sleep(delay_after_load_sec)
        self.parent_topic = f"/Sim/{scene_id}"
        self.home_geo_point: Dict = scene_config_dict.get("home-geo-point", {})

        if scene_config_dict.get("spawn-objects") is not None:
            objects_loaded = self.load_scene_objects_from_config(
                scene_config_dict["spawn-objects"]
            )

            # only resume if the clock type supports it and the user didn't set pause-on-start
            if clock_is_steppable and not pause_on_start_by_user:
                self.resume()
                projectairsim_log().info(
                    "Resuming sim clock since all objects have been spawned."
                )

            projectairsim_log().info(
                f"Loaded following objects successfully: {objects_loaded}."
            )

        projectairsim_log().info(
            f"World object updated for the loaded scene '{self.parent_topic}'. "
            "Note: Any previous Drone object instances may no longer be valid."
        )
        return scene_id

    def load_scene_objects_from_config(self, scene_objects: List[Dict]) -> List[str]:
        """Loads both sim-packaged objects or objects from file from the scene config to the scene

        Args:
            scene_objects (List[Dict]): a List of Dicts containing each object's info

        Returns:
            List[str]: the names of the objects loaded from config
        """
        object_names = []

        if scene_objects.get("sim-packaged") is not None:
            object_names.extend(
                self.load_packaged_objects_from_config(scene_objects["sim-packaged"])
            )
            projectairsim_log().info("Sim-packaged objects were present and loaded")

        if scene_objects.get("from-file") is not None:
            object_names.extend(
                self.load_objects_via_file_from_config(scene_objects["from-file"])
            )
            projectairsim_log().info("Objects from file were present and loaded")

        return object_names

    def load_packaged_objects_from_config(self, objects: List[Dict]) -> List[str]:
        """Loads sim-packaged objects from the scene config to the scene

        Args:
            objects (List[Dict]): a List of Dicts containing each object's info

        Returns:
            List[str]: the names of the sim-packaged objects loaded from config
        """
        object_names = []

        for object in objects:
            name = object["name"]
            path = object["asset-path"]
            scale = convert_string_with_spaces_to_float_list(object["scale"])
            physics_enabled = object["physics-enabled"]
            rpy_quat = self.get_object_rpy_quat_from_config(object)

            # differentiate between pose or lat, long, etc.
            if object["origin"].get("xyz") is not None:
                x, y, z = convert_string_with_spaces_to_float_list(
                    object["origin"]["xyz"]
                )
                translation = Vector3({"x": x, "y": y, "z": z})
                rotation = Quaternion(
                    {
                        "w": rpy_quat[0],
                        "x": rpy_quat[1],
                        "y": rpy_quat[2],
                        "z": rpy_quat[3],
                    }
                )
                pose: Pose = Pose(
                    {
                        "translation": translation,
                        "rotation": rotation,
                        "frame_id": "DEFAULT_ID",
                    }
                )
                self.spawn_object(name, path, pose, scale, physics_enabled)
            else:
                lat, long, alt = convert_string_with_spaces_to_float_list(
                    object["origin"]["geo-point"]
                )
                self.spawn_object_at_geo(
                    name, path, lat, long, alt, rpy_quat, scale, physics_enabled
                )
            object_names.append(name)
        return object_names

    def load_objects_via_file_from_config(self, objects: List[Dict]) -> List[str]:
        """Loads objects under the "from-file" field in the scene config to the scene

        Args:
            objects (List[Dict]): a List of Dicts containing each object's info

        Returns:
            List[str]: the names of the "from-file" objects loaded from config
        """
        object_names = []

        for object in objects:
            name = object["name"]
            format = object["file-format"]
            is_binary = object["is-binary"]
            scale = convert_string_with_spaces_to_float_list(object["scale"])
            physics_enabled = object["physics-enabled"]
            rpy_quat = self.get_object_rpy_quat_from_config(object)

            file = open(object["asset-path"], "rb")
            gltf_byte_array = file.read()
            file.close()

            # differentiate between pose or lat, long, etc.
            if object["origin"].get("xyz") is not None:
                x, y, z = convert_string_with_spaces_to_float_list(
                    object["origin"]["xyz"]
                )
                translation = Vector3({"x": x, "y": y, "z": z})
                rotation = Quaternion(
                    {
                        "w": rpy_quat[0],
                        "x": rpy_quat[1],
                        "y": rpy_quat[2],
                        "z": rpy_quat[3],
                    }
                )
                pose: Pose = Pose(
                    {
                        "translation": translation,
                        "rotation": rotation,
                        "frame_id": "DEFAULT_ID",
                    }
                )
                self.spawn_object_from_file(
                    name,
                    format,
                    gltf_byte_array,
                    is_binary,
                    pose,
                    scale,
                    physics_enabled,
                )
            else:
                lat, long, alt = convert_string_with_spaces_to_float_list(
                    object["origin"]["geo-point"]
                )
                self.spawn_object_from_file_at_geo(
                    name,
                    format,
                    gltf_byte_array,
                    is_binary,
                    lat,
                    long,
                    alt,
                    rpy_quat,
                    scale,
                    physics_enabled,
                )
            object_names.append(name)
        return object_names

    def get_object_rpy_quat_from_config(self, object: Dict) -> List[float]:
        """Processes object and extracts rotation info depending on presence of rpy or rpy-deg

        Args:
            object (Dict): a Dict with all of the object's info

        Returns:
            List[float]: rotation given as a [w, x, y, z] quaternion
        """
        roll, pitch, yaw = 0, 0, 0

        if object["origin"].get("rpy") is not None:
            roll, pitch, yaw = convert_string_with_spaces_to_float_list(
                object["origin"]["rpy"]
            )
        elif object["origin"].get("rpy-deg") is not None:
            roll, pitch, yaw = convert_string_with_spaces_to_float_list(
                object["origin"]["rpy-deg"]
            )
            roll = np.radians(roll)
            pitch = np.radians(pitch)
            yaw = np.radians(yaw)

        return rpy_to_quaternion(roll, pitch, yaw)

    def switch_streaming_view(self) -> bool:
        """Switches the main view to the next available camera with "streaming-enabled"=true

        Returns:
            bool: whether the operation was sucessful
        """
        switch_streaming_view_req: Dict = {
            "method": f"{self.parent_topic}/SwitchStreamingView",
            "params": {},
            "version": 1.0,
        }
        status = self.client.request(switch_streaming_view_req)
        return status

    def set_segmentation_id_by_name(
        self,
        mesh_name: str,
        segmentation_id: int,
        is_name_regex: bool,
        use_owner_name: bool,
    ) -> bool:
        """Set the segmentation ID of object(s) by name

        Args:
            mesh_name (str): Name of object(s) to set the segmentation ID. If is_name_regex is True, matching is case insensitive but must be a full regex match (ex: to ignore trailing chars,add '.*' to end of pattern). If is_name_regex is False, matching must be exact including case.
            segmentation_id (int): Segmentation ID (0-255) to assign to the object(s)
            is_name_regex (bool): Flag to use regex matching or exact matching for name
            use_owner_name (bool): Flag to match with the mesh component's owner's name (usually the actor name) to match with instead of the mesh component's name

        Returns:
            bool: Status flag that's True if any object was successfully
                           matched by mesh_name and had its segmentation ID set to
                           the segmentation_id value
        """
        set_seg_id_req: Dict = {
            "method": f"{self.parent_topic}/SetSegmentationIDByName",
            "params": {
                "mesh_name": mesh_name,
                "segmentation_id": segmentation_id,
                "is_name_regex": is_name_regex,
                "use_owner_name": use_owner_name,
            },
            "version": 1.0,
        }

        status = self.client.request(set_seg_id_req)
        return status

    def get_segmentation_id_by_name(self, mesh_name: str, use_owner_name: bool) -> int:
        """Get the assigned segmentation ID of an object by name

        Args:
            mesh_name (str): Name of object to get the segmentation ID for (matching is
                             case insensitive, but must match all characters)
            use_owner_name (bool): Flag to match with the mesh component's owner's name
                                  (usually the actor name) to match with instead of the
                                  mesh component's name

        Returns:
            int: The segmentation ID for the object named mesh_name,
                          or -1 if not found
        """
        get_seg_id_req: Dict = {
            "method": f"{self.parent_topic}/GetSegmentationIDByName",
            "params": {"mesh_name": mesh_name, "use_owner_name": use_owner_name},
            "version": 1.0,
        }

        seg_id = self.client.request(get_seg_id_req)
        return seg_id

        return status

    def get_segmentation_id_map(self) -> Dict:
        """Get the assigned segmentation ID of all objects

        Returns:
            Dict: A map from object name to segmentation ID
        """
        get_seg_map_req: Dict = {
            "method": f"{self.parent_topic}/GetSegmentationIDMap",
            "params": {},
            "version": 1.0,
        }

        seg_map = self.client.request(get_seg_map_req)
        return seg_map

    def create_voxel_grid(
        self,
        position: Pose,
        x_size,
        y_size,
        z_size,
        resolution,
        n_z_resolution = 1,
        use_segmentation: bool = False,
        actors_to_ignore=[],
        write_file=False,
        file_path="./voxel_grid.binvox",
    ) -> List[int]:
        """Create a voxel grid of the scene

        Args:
            position (Pose): Pose for the center of the voxel grid
            x_size (int): x-size of the desired grid in m
            y_size (int): y-size of the desired grid in m
            z_size (int): z-size of the desired grid in m
            resolution (int): resolution of the grid in m/voxels
            actors_to_ignore (list): IDs of actors to ignore for occupancy
            write_file (bool): Write voxel grid to a binvox file
            file_path (str): Path to write the binvox file to

        Returns:
            List(bool): Occupancy map of the scene.
        """

        create_voxel_grid: Dict = {
            "method": f"{self.parent_topic}/createVoxelGrid",
            "params": {
                "position": position,
                "x_size": x_size,
                "y_size": y_size,
                "z_size": z_size,
                "res": resolution,
                "n_z_resolution": n_z_resolution,
                "use_segmentation": use_segmentation,
                "actors_to_ignore": actors_to_ignore,
            },
            "version": 1.0,
        }
        voxel_grid = self.client.request(create_voxel_grid)
        
        if write_file:
            with open(file_path, "wb") as fp:
                #fp = open(file_path, "wb")
                fp.write(b"#binvox 1\n")
                fp.write(
                    bytes(
                    "dim "
                    + str(int(x_size / resolution))
                    + " "
                    # This ensures that the z dimension is a multiple of n_z_resolution
                    + str(int(z_size / (resolution*n_z_resolution)) * n_z_resolution)
                    + " "
                    + str(int(y_size / resolution))
                    + "\n",
                    "ascii"
                    )
                )
                fp.write(
                    bytes(
                    "translate "
                    + str(-x_size * 0.5)
                    + " "
                    + str(-y_size * 0.5)
                    + " "
                    + str(-z_size * 0.5)
                    + "\n",
                    "ascii"
                    )
                )
                fp.write(bytes("scale " + str(1 / x_size) + "\n", "ascii"))
                fp.write(b"data\n")
                state = voxel_grid[0]
                ctr = 0
                id = 0
                nx_cells = int(x_size / resolution)
                nz_cells = int((z_size / (resolution*n_z_resolution)))
                ny_cells = int(y_size / resolution)
                # Write the voxel grid to the binvox file
                # a voxel grid is represented by cubical cells so the total number of z cells are nz_cells * n_z_resolution
                # To iterate each z through n_z_resolution we need this four nested loop
                # The order of the cells is x, z, y
                for idy in range(ny_cells):
                    for idz in range(nz_cells):
                        for nz in range(n_z_resolution):    # This is to ensure that the z dimension is a multiple of n_z_resolution
                            for idx in range(nx_cells):
                                id = idx+ nx_cells * (idz+ nz_cells * idy)
                                c = voxel_grid[id]

                                if c == state:
                                    ctr += 1
                                    # if ctr hits max, dump
                                    if ctr == 255:
                                        fp.write(bytes([state, ctr]))
                                        ctr = 0
                                else:
                                    # if switch state, dump
                                    fp.write(bytes([state, ctr]))
                                    state = c
                                    ctr = 1
                # flush out remainders
                if ctr > 0:
                    fp.write(bytes([state, ctr]))

                projectairsim_log().info(f"binvox file written to {file_path}")

        return voxel_grid

    def flush_persistent_markers(self) -> bool:
        """Clears debug items, including those plotted with 'is_persistent' = True

        Returns:
            bool: True if successful
        """
        flush_persistent_markers_req: Dict = {
            "method": f"{self.parent_topic}/debugFlushPersistentMarkers",
            "params": {},
            "version": 1.0,
        }
        return self.client.request(flush_persistent_markers_req)

    def plot_debug_points(
        self,
        points: List[List[float]],
        color_rgba: List[float],
        size: float,
        duration: float,
        is_persistent: bool,
    ) -> bool:
        """Create debug points in the scene

        Args:
            points (List[List[float]]): Vector of x,y,z, points
            color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            size (float): Size of the point
            duration (float): (seconds)
            is_persistent (bool): If True, 'duration' is ignored and points are displayed until removed by flush_persistent_markers()

        Returns:
            bool: True if successful
        """
        plot_debug_points_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotPoints",
            "params": {
                "points": points,
                "color_rgba": color_rgba,
                "size": size,
                "duration": duration,
                "is_persistent": is_persistent,
            },
            "version": 1.0,
        }
        return self.client.request(plot_debug_points_req)

    def plot_debug_solid_line(
        self,
        points: List[List[float]],
        color_rgba: List[float],
        thickness: float,
        duration: float,
        is_persistent: bool,
    ) -> bool:
        """Creates a solid debug line in the scene.

        Args:
            points (List[List[float]]): Vector of x,y,z, points
            color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            thickness (float): Thickness of the line
            duration (float): (seconds)
            is_persistent (bool): If True, 'duration' is ignored and the line is displayed until removed by flush_persistent_markers()

        Returns:
            bool: True if successful
        """
        plot_debug_solid_line_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotSolidLine",
            "params": {
                "points": points,
                "color_rgba": color_rgba,
                "thickness": thickness,
                "duration": duration,
                "is_persistent": is_persistent,
            },
            "version": 1.0,
        }
        return self.client.request(plot_debug_solid_line_req)

    def plot_debug_dashed_line(
        self,
        points: List[List[float]],
        color_rgba: List[float],
        thickness: float,
        duration: float,
        is_persistent: bool,
    ) -> bool:
        """Create a dashed debug line in the scene. There must be an even number of points.

        Args:
            points (Lis(t[List[float]]): Vector of x,y,z, points. Number of points must be even.
            color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            thickness (float): Thickness of the line
            duration (float): (seconds)
            is_persistent (bool): If True, 'duration' is ignored and the line is displayed until removed by flush_persistent_markers()

        Returns:
            bool: True if successful
        """
        plot_debug_dashed_line_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotDashedLine",
            "params": {
                "points": points,
                "color_rgba": color_rgba,
                "thickness": thickness,
                "duration": duration,
                "is_persistent": is_persistent,
            },
            "version": 1.0,
        }
        assert len(points) % 2 == 0
        return self.client.request(plot_debug_dashed_line_req)

    def plot_debug_arrows(
        self,
        points_start: List[List[float]],
        points_end: List[List[float]],
        color_rgba: List[float],
        thickness: float,
        arrow_size: float,
        duration: float,
        is_persistent: bool,
    ) -> bool:
        """Create debug arrows in the scene

        Args:
            points_start (List[List[float]]): Vector of x,y,z arrow start points
            points_end (List[List[float]]): Vector of x,y,z arrow end points
            color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            thickness (float): Thickness of arrow
            arrow_size (float): Size of the arrow
            duration (float): (seconds)
            is_persistent (bool): If True, 'duration' is ignored and arrows are displayed until removed by flush_persistent_markers()

        Returns:
            bool: True if successful
        """
        plot_debug_arrows_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotArrows",
            "params": {
                "points_start": points_start,
                "points_end": points_end,
                "color_rgba": color_rgba,
                "thickness": thickness,
                "arrow_size": arrow_size,
                "duration": duration,
                "is_persistent": is_persistent,
            },
            "version": 1.0,
        }
        return self.client.request(plot_debug_arrows_req)

    def plot_debug_strings(
        self,
        strings: List[str],
        positions: List[List[float]],
        scale: float,
        color_rgba: List[float],
        duration: float,
    ) -> bool:
        """Create debug strings in the scene

        Args:
            srings (List[str]): Vector of text to display
            positions (List[List[float]]): Vector of corresponding x,y,z locations to display text
            scale (float): Font scale
            color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            duration (float): (seconds)

        Returns:
            bool: True if successful
        """
        plot_debug_strings_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotStrings",
            "params": {
                "strings": strings,
                "positions": positions,
                "scale": scale,
                "color_rgba": color_rgba,
                "duration": duration,
            },
            "version": 1.0,
        }
        return self.client.request(plot_debug_strings_req)

    def plot_debug_transforms(
        self,
        poses: List[Pose],
        scale: float,
        thickness: float,
        duration: float,
        is_persistent: bool,
    ) -> bool:
        """Create debug coordinate system indicators in the scene

        Args:
            poses (List[Pose]): Vector of poses (translation + rotation) to display
            scale (float): Scale of transform
            thickness (float): Thickness of transform
            duration (float): (seconds)
            is_persistent (bool): If True, 'duration' is ignored and arrows are displayed until removed by flush_persistent_markers()

        Returns:
            bool: True if successful
        """
        plot_debug_transforms_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotTransforms",
            "params": {
                "poses": poses,
                "scale": scale,
                "thickness": thickness,
                "duration": duration,
                "is_persistent": is_persistent,
            },
            "version": 1.0,
        }
        for pose in poses:
            assert "translation" in pose.keys(), "Invalid Pose result"
            assert "rotation" in pose.keys(), "Invalid Pose result"
        return self.client.request(plot_debug_transforms_req)

    def plot_debug_transforms_with_names(
        self,
        poses: List[Pose],
        names: List[str],
        tf_scale: float,
        tf_thickness: float,
        text_scale: float,
        text_color_rgba: List[float],
        duration: float,
    ) -> bool:
        """Create debug coordinate system indicators with text in the scene

        Args:
            poses (List[Pose]): Vector of poses (translation + rotation) to display
            names (List[str]): Vector of text to display
            tf_scale (float): Scale of transform
            tf_thickness (float): Thickness of transform
            text_scale (float): Scale of text
            text_color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            duration (float): (seconds)

        Returns:
            bool: True if successful
        """
        plot_debug_transforms_with_names_req: Dict = {
            "method": f"{self.parent_topic}/debugPlotTransformsWithNames",
            "params": {
                "poses": poses,
                "names": names,
                "tf_scale": tf_scale,
                "tf_thickness": tf_thickness,
                "text_scale": text_scale,
                "text_color_rgba": text_color_rgba,
                "duration": duration,
            },
            "version": 1.0,
        }
        for pose in poses:
            assert "translation" in pose.keys(), "Invalid Pose result"
            assert "rotation" in pose.keys(), "Invalid Pose result"
        return self.client.request(plot_debug_transforms_with_names_req)

    def set_trace_line(self, color_rgba: List[float], thickness: float) -> bool:
        """Set trace line parameters.

        Args:
            color_rgba (List[float]): [red, green, blue, alpha] values (range: 0.0 - 1.0)
            thickness (float): Thickness of line

        Returns:
            bool: True if request sucessful
        """
        set_trace_line_req: Dict = {
            "method": f"{self.parent_topic}/SetTraceLine",
            "params": {
                "color_rgba": color_rgba,
                "thickness": thickness,
            },
            "version": 1.0,
        }
        return self.client.request(set_trace_line_req)

    def toggle_trace(self) -> bool:
        """Toggle displaying the trace of the drone's path.

        Pressing 'T' while focused on the server viewport window also toggles displaying the trace."

        Returns:
            bool: True if request successful
        """
        toggle_trace_req: Dict = {
            "method": f"{self.parent_topic}/ToggleTrace",
            "params": {},
            "version": 1.0,
        }
        return self.client.request(toggle_trace_req)

    def get_3d_bounding_box(
        self, object_name: str, box_alignment: BoxAlignment
    ) -> Dict:
        """Gets the 3D bounding box of some object in the scene.

        Args:
            object_name (str): Name of the object to get the bbox for.
            box_alignment (BoxAlignment): whether the bounding box is aligned to the world axes or object axes

        Returns:
            Dict: dict representing a 3D bbox.
        """
        get_bbox_req: Dict = {
            "method": f"{self.parent_topic}/Get3DBoundingBox",
            "params": {
                "object_name": object_name,
                "box_alignment": box_alignment,
            },
            "version": 1.0,
        }
        return self.client.request(get_bbox_req)

    def import_ned_trajectory(
        self,
        traj_name: str,
        time: List[float],
        pose_x: List[float],
        pose_y: List[float],
        pose_z: List[float],
        pose_roll: List[float] = None,  # rad
        pose_pitch: List[float] = None,  # rad
        pose_yaw: List[float] = None,  # rad
        vel_lin_x: List[float] = None,
        vel_lin_y: List[float] = None,
        vel_lin_z: List[float] = None,
    ) -> None:
        """Imports a NED trajectory

        Args:
            traj_name (str): the name of the trajectory
            time (List[float]): times for each trajectory point, in seconds
            pose_x (List[float]): x poses for each trajectory point, in meters
            pose_y (List[float]): y poses for each trajectory point, in meters
            pose_z (List[float]): z poses for each trajectory point, in meters
            pose_roll (List[float]): rolls for each trajectory point, in radians
            pose_pitch (List[float]): pitches for each trajectory point, in radians
            pose_yaw (List[float]): yaws for each trajectory point, in radians
            vel_lin_x (List[float]): x-velocities for each trajectory point, in meters per second
            vel_lin_y (List[float]): y-velocities for each trajectory point, in meters per second
            vel_lin_z (List[float]): z-velocities for each trajectory point, in meters per second
        """

        if vel_lin_x is None:
            vel_lin_x = [0] * len(time)

        if vel_lin_y is None:
            vel_lin_y = [0] * len(time)

        if vel_lin_z is None:
            vel_lin_z = [0] * len(time)

        # user must pass in all rotations or none at all (no partial)
        if pose_roll is not None and pose_pitch is not None and pose_yaw is not None:
            validate_trajectory(
                [
                    traj_name,
                    time,
                    pose_x,
                    pose_y,
                    pose_z,
                    pose_roll,
                    pose_pitch,
                    pose_yaw,
                    vel_lin_x,
                    vel_lin_y,
                    vel_lin_z,
                ]
            )
        else:
            validate_trajectory(
                [
                    traj_name,
                    time,
                    pose_x,
                    pose_y,
                    pose_z,
                    vel_lin_x,
                    vel_lin_y,
                    vel_lin_z,
                ]
            )

        if pose_roll is None:
            pose_roll = [0] * len(time)

        if pose_pitch is None:
            pose_pitch = []
            for idx in range(1, len(time)):
                pose_pitch.append(
                    get_pitch_between_traj_points(
                        (pose_x[idx], pose_y[idx], pose_z[idx]),
                        (pose_x[idx - 1], pose_y[idx - 1], pose_z[idx - 1]),
                    )
                )
            pose_pitch.append(pose_pitch[-1])

        if pose_yaw is None:
            pose_yaw = []
            for idx in range(1, len(time)):
                pose_yaw.append(
                    get_heading_between_traj_points(
                        (pose_x[idx], pose_y[idx], pose_z[idx]),
                        (pose_x[idx - 1], pose_y[idx - 1], pose_z[idx - 1]),
                    )
                )
            pose_yaw.append(pose_yaw[-1])

        import_ned_trajectory_req: Dict = {
            "method": f"{self.parent_topic}/ImportNEDTrajectory",
            "params": {
                "traj_name": traj_name,
                "time": time,
                "pose_x": pose_x,
                "pose_y": pose_y,
                "pose_z": pose_z,
                "pose_roll": pose_roll,
                "pose_pitch": pose_pitch,
                "pose_yaw": pose_yaw,
                "vel_x_lin": vel_lin_x,
                "vel_y_lin": vel_lin_y,
                "vel_z_lin": vel_lin_z,
            },
            "version": 1.0,
        }
        import_success = self.client.request(import_ned_trajectory_req)
        if import_success:
            projectairsim_log().info(
                "Trajectory '" + traj_name + "' imported successfully."
            )
        else:
            projectairsim_log().info(
                "Warning: '"
                + traj_name
                + "' was not imported. A duplicate trajectory name already exists."
            )

    def import_ned_trajectory_from_csv(
        self,
        traj_name: str,
        filename: str,
        delimiter_str: str = ",",
        n_lines_to_skip: int = 1,
    ) -> None:
        """Imports a NED Trajectory from a CSV file

        First row of the file is the header. Numerical data begins from
        n_lines_to_skip + 1. Each row corresponds to trajectory data at a
        given time instance, where the column order is
        1) time (s)
        2) pose_x (m)
        3) pose_y (m)
        4) pose_z (m)
        5) pose_roll (rad)
        6) pose_pitch (rad)
        7) pose_yaw (rad)
        8) vel_lin_x (m/s)
        9) vel_lin_y (m/s)
        10) vel_lin_z (m/s)

        Args:
            traj_name (str): the name of the trajectory
            filename (str): the name of the file to read
            delimiter_str (str): the delimiter used in the file
            n_lines_to_skip (int): the number of lines at the top of the file to skip
        """
        num_cols_expected = 10
        traj_data = np.genfromtxt(
            filename, delimiter=delimiter_str, skip_header=n_lines_to_skip
        )
        traj_data = np.transpose(traj_data)

        # Check for valid input
        if np.isnan(np.min(traj_data)):
            raise ValueError("File contains NaN!")
        elif np.shape(traj_data)[0] < num_cols_expected:
            raise ValueError(
                "Expected columns for: time, x, y, z, roll, pitch, yaw, x_vel, y_vel, z_vel."
            )

        traj_data = traj_data.tolist()

        # Data is good so pass in through Plugin
        self.import_ned_trajectory(
            traj_name,
            traj_data[0],
            traj_data[1],
            traj_data[2],
            traj_data[3],
            traj_data[4],
            traj_data[5],
            traj_data[6],
            traj_data[7],
            traj_data[8],
            traj_data[9],
        )

    def import_geo_trajectory(
        self,
        traj_name: str,
        time: List[float],
        latitudes: List[float],
        longitudes: List[float],
        altitudes: List[float],
        roll: List[float] = None,  # rad
        pitch: List[float] = None,  # rad
        yaw: List[float] = None,  # rad
        vel_lin_x: List[float] = None,
        vel_lin_y: List[float] = None,
        vel_lin_z: List[float] = None,
    ) -> None:
        """Imports a geodetic trajectory

        Args:
            traj_name (str): the name of the trajectory
            time (List[float]): times for each trajectory point, in seconds
            latitudes (List[float]): latitudes for each trajectory point
            longitudes (List[float]): longitudes for each trajectory point
            altitudes (List[float]): altitudes, in meters for each trajectory point
            roll (List[float]): rolls, in radians for each trajectory point
            pitch (List[float]): pitches, in radians for each trajectory point
            yaw (List[float]): yaws, in radians for each trajectory point
            vel_lin_x (List[float]): x-velocities for each trajectory point, in meters per second
            vel_lin_y (List[float]): y-velocities for each trajectory point, in meters per second
            vel_lin_z (List[float]): z-velocities for each trajectory point, in meters per second
        """

        if vel_lin_x is None:
            vel_lin_x = [0] * len(time)

        if vel_lin_y is None:
            vel_lin_y = [0] * len(time)

        if vel_lin_z is None:
            vel_lin_z = [0] * len(time)

        # user must pass in all rotations or none at all (no partial)
        if roll is None or pitch is None or yaw is None:
            validate_trajectory(
                [
                    traj_name,
                    time,
                    latitudes,
                    longitudes,
                    altitudes,
                    vel_lin_x,
                    vel_lin_y,
                    vel_lin_z,
                ]
            )
            import_geo_trajectory_req: Dict = {
                "method": f"{self.parent_topic}/ImportGeoCoordinates",
                "params": {
                    "traj_name": traj_name,
                    "time": time,
                    "latitudes": latitudes,
                    "longitudes": longitudes,
                    "altitudes": altitudes,
                    "vel_x_lin": vel_lin_x,
                    "vel_y_lin": vel_lin_y,
                    "vel_z_lin": vel_lin_z,
                },
                "version": 1.0,
            }
        else:
            validate_trajectory(
                [
                    traj_name,
                    time,
                    latitudes,
                    longitudes,
                    altitudes,
                    roll,
                    pitch,
                    yaw,
                    vel_lin_x,
                    vel_lin_y,
                    vel_lin_z,
                ]
            )
            import_geo_trajectory_req: Dict = {
                "method": f"{self.parent_topic}/ImportGeoTrajectory",
                "params": {
                    "traj_name": traj_name,
                    "time": time,
                    "latitudes": latitudes,
                    "longitudes": longitudes,
                    "altitudes": altitudes,
                    "pose_roll": roll,
                    "pose_pitch": pitch,
                    "pose_yaw": yaw,
                    "vel_x_lin": vel_lin_x,
                    "vel_y_lin": vel_lin_y,
                    "vel_z_lin": vel_lin_z,
                },
                "version": 1.0,
            }
        import_success = self.client.request(import_geo_trajectory_req)
        if import_success:
            projectairsim_log().info(
                "Trajectory '" + traj_name + "' imported successfully."
            )
        else:
            projectairsim_log().info(
                "Warning: '"
                + traj_name
                + "' was not imported. A duplicate trajectory name already exists."
            )

    def import_trajectory_from_kml(
        self, traj_name: str, time: List[float], filename: str
    ) -> None:
        """Imports a kml file as a trajectory

        Args:
            traj_name (str): name of the trajectory
            time (List[float]): times for each trajectory point, in seconds
            filename (str): name of the kml file
        """
        lat, lon, alt = get_trajectory_from_kml(filename)
        self.import_geo_trajectory(traj_name, time, lat, lon, alt)

    def get_surface_elevation_at_point(self, x: float, y: float) -> float:
        """Gets the height of the ground at a given point

        Args:
            x (float): x-coordinate of the point in m
            y (float): y-coordinate of the point in m
        Returns: height of the ground at the given point in m
        """

        get_z_at_point_dict: Dict = {
            "method": f"{self.parent_topic}/GetZAtPoint",
            "params": {
                "x": x,
                "y": y,
            },
            "version": 1.0,
        }

        altitude = self.client.request(get_z_at_point_dict)
        return altitude

    def get_random_free_position_near_point(
        self,
        point: Dict,
        radius_min: float = 20.0,
        radius_max: float = 30.0,
        resolution: float = 2.0,
        maximum_z: float = 0,
        max_retries: int = 10,
    ) -> Dict:
        """Finds a free position near a point

        Args:
            point (Dict): the point to center the search around, in NED coordinates
            radius_min (float): the minimum distance from the center of the chosen point
            radius_max (float): the maximum distance from the center of the chosen point
            resolution (float): resolution of the voxel grid used to determine if positions are free
            maximum_z (float): the maximum z value of the chosen point. prevents points underground from being picked
            max_retries (int): the maximum number of attempts made to find a valid point

        Returns:
            Dict: the located position
        """
        valid_position_found = False
        attempt = 0
        center_pose = Pose(
            {
                "translation": Vector3(point),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
                "frame_id": "DEFAULT_ID",
            }
        )
        while not valid_position_found and attempt < max_retries:
            attempt += 1
            random_radius = radius_min + (radius_max - radius_min) * random.random()
            voxel_grid = self.create_voxel_grid(
                center_pose,
                math.ceil(random_radius * 2),
                math.ceil(random_radius * 2),
                math.ceil(random_radius * 2),
                resolution,
            )
            # Compute relative position
            random_yaw = random.random() * math.pi * 2
            random_pitch = random.random() * math.pi * 2
            offset = [
                math.cos(random_yaw) * math.cos(random_pitch) * random_radius,
                math.sin(random_yaw) * math.cos(random_pitch) * random_radius,
                math.sin(random_pitch) * random_radius,
            ]
            # Convert from relative position to absolute position
            desired_position = [
                point["x"] + offset[0],
                point["y"] + offset[1],
                point["z"] + offset[2],
            ]
            grid_index = get_voxel_grid_idx(
                desired_position,
                [point["x"], point["y"], point["z"]],
                (random_radius * 2, random_radius * 2, random_radius * 2),
                resolution,
            )
            valid_position_found = (
                not voxel_grid[grid_index] and desired_position[2] <= maximum_z
            )
            if valid_position_found:
                final_point: Dict = {
                    "x": desired_position[0],
                    "y": desired_position[1],
                    "z": desired_position[2],
                }
                return final_point
        projectairsim_log().error("Could not find free point")
        return {"x": 0, "y": 0, "z": 0}

    def get_random_free_position_near_path(
        self,
        path: List[List[float]],
        radius_min: float = 20.0,
        radius_max: float = 30.0,
        resolution: float = 2.0,
        maximum_z: float = 0,
        max_retries: int = 10,
        minimum_distance_along_path: float = 10.0,
    ) -> (Dict, List[float], int):
        """Finds a free position near a random point on a path

        Args:
            path (List[List[float]]): a list of path points, in NED coordinates
            radius_min (float): the minimum distance from the path
            radius_max (float): the maximum distance from the path
            resolution (float): resolution of the voxel grid used to determine if positions are free
            maximum_z (float): the maximum z value of the chosen point. prevents points underground from being picked
            max_retries (int): the maximum number of attempts made to find a valid point
            include_start (bool): whether the first leg of the path is a valid location to choose

        Returns:
            Tuple: (the chosen position, the point on the path that it is near, the leg of the path the point is on)
        """
        path_length = calculate_path_length(path)
        if minimum_distance_along_path > path_length:
            projectairsim_log().error("Minimum distance along path is longer than path")
        valid_position_found = False
        attempt = 0
        while not valid_position_found and attempt < max_retries:
            attempt += 1
            desired_distance = (
                random.random() * (path_length - minimum_distance_along_path)
                + minimum_distance_along_path
            )
            point_chosen, leg_segment, leg_start = get_point_distance_along_path(
                path, desired_distance
            )
            point_chosen_list = [
                point_chosen["x"],
                point_chosen["y"],
                point_chosen["z"],
            ]
            # Generate a navigation start point near the point on the path
            center_pose = Pose(
                {
                    "translation": Vector3(point_chosen),
                    "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
                    "frame_id": "DEFAULT_ID",
                }
            )
            random_radius = radius_min + (radius_max - radius_min) * random.random()
            grid_buffer = 5.0
            voxel_grid = self.create_voxel_grid(
                center_pose,
                math.ceil(random_radius * 2) + grid_buffer,
                math.ceil(random_radius * 2) + grid_buffer,
                math.ceil(random_radius * 2) + grid_buffer,
                resolution,
            )
            # Compute relative position
            # Generate a relative position using a random direction vector perpendicular to the path
            random_rotation = random.random() * math.pi * 2
            perpendicular_vector = generate_perpendicular_unit_vector(leg_segment)
            direction = rotate_vector_about_axis(
                perpendicular_vector, leg_segment, random_rotation
            )
            offset = [i * random_radius for i in direction]
            # Convert from relative position to absolute position
            desired_position = [
                point_chosen["x"] + offset[0],
                point_chosen["y"] + offset[1],
                point_chosen["z"] + offset[2],
            ]
            grid_index = get_voxel_grid_idx(
                desired_position,
                point_chosen_list,
                (random_radius * 2, random_radius * 2, random_radius * 2),
                resolution,
            )
            valid_position_found = (
                not voxel_grid[grid_index] and desired_position[2] <= maximum_z
            )
            # Check to make sure the spawn point is not near any point on the path
            for i in range(len(path) - 1):
                if (
                    get_point_to_line_segment_distance(
                        desired_position, path[i], path[i + 1]
                    )
                    < radius_min
                ):
                    valid_position_found = False
            if valid_position_found:
                final_point: Dict = {
                    "x": desired_position[0],
                    "y": desired_position[1],
                    "z": desired_position[2],
                }
                return (final_point, point_chosen_list, leg_start)
        projectairsim_log().error("Could not find free point")
        return ({"x": 0, "y": 0, "z": 0}, point_chosen_list, 0)

    def generate_intercept_trajectory(
        self,
        traj_name,
        start_point,
        intercept_point,
        intercept_speed,
        path_positions,
        path_speeds,
        leg_start,
        distance_ahead_of_target=0.0,
        grid_buffer=10.0,
        grid_resolution=2.0,
        distance_traveled_after_intercept=30.0,
    ):
        # Determine how to get from the start point to the intercept point
        start_point_tuple = (start_point["x"], start_point["y"], start_point["z"])
        edge_length_x = abs(intercept_point[0] - start_point["x"]) + grid_buffer * 2
        edge_length_y = abs(intercept_point[1] - start_point["y"]) + grid_buffer * 2
        edge_length_z = abs(intercept_point[2] - start_point["z"]) + grid_buffer * 2
        center_coords = [
            (intercept_point[0] + start_point_tuple[0]) / 2,
            (intercept_point[1] + start_point_tuple[1]) / 2,
            (intercept_point[2] + start_point_tuple[2]) / 2,
        ]
        center_pose = Pose(
            {
                "translation": Vector3(
                    {
                        "x": center_coords[0],
                        "y": center_coords[1],
                        "z": -center_coords[2],
                    }
                ),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
                "frame_id": "DEFAULT_ID",
            }
        )
        occupancy_grid = self.create_voxel_grid(
            center_pose, edge_length_x, edge_length_y, edge_length_z, grid_resolution
        )
        intercept_point_tuple = (
            intercept_point[0],
            intercept_point[1],
            intercept_point[2],
        )
        planner = AStarPlanner(
            occupancy_grid,
            center_coords,
            (edge_length_x, edge_length_y, edge_length_z),
            grid_resolution,
        )
        intercept_path_points = planner.generate_plan(
            start_point_tuple,
            intercept_point_tuple,
        )

        # Calculate the appropriate time to complete the path
        intercept_speeds = [intercept_speed for i in intercept_path_points]
        intercept_travel_time = calculate_path_time(
            intercept_path_points, intercept_speeds[:-1]
        )

        target_distance = calculate_path_length(
            path_positions[: leg_start + 1]
        ) + calculate_path_length([path_positions[leg_start], intercept_point])
        (
            target_point_at_intercept_time,
            segment,
            segment_id,
        ) = get_point_distance_along_path(
            path_positions, target_distance - distance_ahead_of_target
        )
        target_point_at_intercept_time_list = [
            target_point_at_intercept_time["x"],
            target_point_at_intercept_time["y"],
            target_point_at_intercept_time["z"],
        ]
        target_travel_time = calculate_path_time(
            path_positions[: segment_id + 1], path_speeds
        ) + calculate_path_time(
            [path_positions[segment_id], target_point_at_intercept_time_list],
            [path_speeds[segment_id]],
        )

        # Check if we're moving fast enough to intercept
        time_start = target_travel_time - intercept_travel_time
        if time_start < 0:
            new_speed = intercept_speed * intercept_travel_time / target_travel_time
            projectairsim_log().error(
                "Non-cooperative actor speed is too low to intercept. Increasing speed to %f",
                new_speed,
            )
            intercept_speed = new_speed
            time_start = 0
            if intercept_speed > 30.0:
                projectairsim_log().error(
                    "The non-cooperative actor is starting far from the intercept site, verify initial position is correct"
                )

        # Construct the trajectory
        traj_times = [0, time_start]
        traj_poses_x = [start_point_tuple[0], start_point_tuple[0]]
        traj_poses_y = [start_point_tuple[1], start_point_tuple[1]]
        traj_poses_z = [start_point_tuple[2], start_point_tuple[2]]

        for i in range(len(intercept_path_points) - 2):
            traj_poses_x.append(intercept_path_points[i][0])
            traj_poses_y.append(intercept_path_points[i][1])
            traj_poses_z.append(intercept_path_points[i][2])
            traj_times.append(
                traj_times[-1]
                + calculate_path_time(
                    [intercept_path_points[i], intercept_path_points[i + 1]],
                    [intercept_speed],
                )
            )

        traj_times.append(target_travel_time)
        traj_poses_x.append(intercept_point[0])
        traj_poses_y.append(intercept_point[1])
        traj_poses_z.append(intercept_point[2])

        # Construct continued movement post-intercept
        extrapolate_direction = np.subtract(intercept_point, intercept_path_points[-2])
        extrapolate_direction_normalized = extrapolate_direction / np.linalg.norm(
            extrapolate_direction
        )
        extrapolate_point = np.add(
            [
                a * distance_traveled_after_intercept
                for a in extrapolate_direction_normalized
            ],
            intercept_point,
        )
        extrapolate_time = calculate_path_time(
            [intercept_point, extrapolate_point], [intercept_speed]
        )
        traj_times.append(traj_times[-1] + extrapolate_time)
        traj_poses_x.append(extrapolate_point[0])
        traj_poses_y.append(extrapolate_point[1])
        traj_poses_z.append(extrapolate_point[2])

        # Import the trajectory
        self.import_ned_trajectory(
            traj_name, traj_times, traj_poses_x, traj_poses_y, traj_poses_z
        )
