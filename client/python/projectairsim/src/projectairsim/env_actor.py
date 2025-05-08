"""
Copyright (C) Microsoft Corporation. All rights reserved.
Python client for ProjectAirSim environment actors.
"""

from projectairsim import ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log, quaternion_to_rpy
from typing import Dict
from projectairsim.types import Pose


class EnvActor(object):
    def __init__(self, client: ProjectAirSimClient, world: World, name: str):
        """ProjectAirSim Environment Actor Interface

        Arguments:
            client {ProjectAirSimClient} -- ProjectAirSim client object
            world {World} -- ProjectAirSim world object
            name {str} -- Name of the environment actor in the scene
        """
        projectairsim_log().info(f"Initalizing EnvActor '{name}'...")
        self.client = client
        self.name = name
        self.world_parent_topic = world.parent_topic
        self.set_topics(world)
        self.log_topics()
        projectairsim_log().info(
            f"Environment actor '{self.name}' initialized for "
            f"World scene '{self.world_parent_topic}'"
        )

    def set_topics(self, world: World):
        self.parent_topic = f"{self.world_parent_topic}/env_actors/{self.name}"
        self.set_actor_info_topics()

    def set_actor_info_topics(self):
        self.actor_info = {}
        self.actor_info["actual_kinematics"] = f"{self.parent_topic}/actual_kinematics"

    def log_topics(self):
        projectairsim_log().info("-------------------------------------------------")
        projectairsim_log().info(
            f"The following topics can be subscribed to for actor '{self.name}':",
        )
        for name in self.actor_info.keys():
            projectairsim_log().info(f'    actor_info["{name}"]')
        projectairsim_log().info("-------------------------------------------------")

    def set_trajectory(
        self,
        traj_name: str,
        to_loop: bool = False,
        time_offset: float = 0,  # s
        x_offset: float = 0,  # m
        y_offset: float = 0,  # m
        z_offset: float = 0,  # m
        roll_offset: float = 0,  # rad
        pitch_offset: float = 0,  # rad
        yaw_offset: float = 0,  # rad
    ) -> None:
        # pass actor name into server to associate to trajectory
        set_trajectory_req: Dict = {
            "method": f"{self.world_parent_topic}/SetEnvActorTrajectory",
            "params": {
                "env_actor_name": self.name,
                "traj_name": traj_name,
                "time_offset": time_offset,
                "x_offset": x_offset,
                "y_offset": y_offset,
                "z_offset": z_offset,
                "roll_offset": roll_offset,
                "pitch_offset": pitch_offset,
                "yaw_offset": yaw_offset,
                "to_loop": to_loop,
            },
            "version": 1.0,
        }
        is_set = self.client.request(set_trajectory_req)
        if not is_set:
            projectairsim_log().info(
                "set_trajectory failed for EnvActor "
                + self.name
                + ". The trajectory '"
                + traj_name
                + "' was not found. The previous trajectory will be used."
            )
        else:
            projectairsim_log().info(
                "'" + traj_name + "' trajectory assigned to " + self.name + "."
            )

    def set_link_rotation_angle(self, link_name: str, angle_deg: float) -> None:
        """SetRotationAngle for EnvActor Link

        Arguments:
            link_name - link ID as string
            angle_deg - desired angle (deg) to rotate link to as float
        """
        set_link_rotation_angle_req: Dict = {
            "method": f"{self.world_parent_topic}/SetEnvActorLinkRotAngle",
            "params": {
                "env_actor_name": self.name,
                "link_name": link_name,
                "angle_deg": angle_deg,
            },
            "version": 1.0,
        }
        success = self.client.request(set_link_rotation_angle_req)

        if success:
            projectairsim_log().info(
                "Rotation angle for link '"
                + link_name
                + "' set successfully for EnvActor "
                + self.name
            )
        else:
            projectairsim_log().info(
                "Warning: The link '"
                + link_name
                + "' for EnvActor "
                + self.name
                + " was not found."
            )
        return success

    def set_link_rotation_rate(
        self, link_name: str, rotation_deg_per_sec: float
    ) -> None:
        """SetRotationRate for EnvActor Link

        Arguments:
            link_name - link ID as string
            rotation_deg_per_sec - desired angular speed (deg/s) to rotate link at as float
        """
        set_link_rotation_rate_req: Dict = {
            "method": f"{self.world_parent_topic}/SetEnvActorLinkRotRate",
            "params": {
                "env_actor_name": self.name,
                "link_name": link_name,
                "rotation_deg_per_sec": rotation_deg_per_sec,
            },
            "version": 1.0,
        }
        success = self.client.request(set_link_rotation_rate_req)

        if success:
            projectairsim_log().info(
                "Rotation rate for link '"
                + link_name
                + "' set successfully for EnvActor "
                + self.name
            )
        else:
            projectairsim_log().info(
                "Warning: The link '"
                + link_name
                + "' for EnvActor "
                + self.name
                + " was not found."
            )
        return success

    def set_link_rotation_angles(self, rotation_angle_map: Dict) -> None:
        """SetRotationAngles for EnvActor Links

        Arguments:
            rotation_angle_map - Dictionary with keys = link ID strings, and
                                 values = rotation angle (deg) floats
        """

        for link_name, rotation_angle in rotation_angle_map.items():
            success = self.set_link_rotation_angle(link_name, rotation_angle)
            if not success:
                return False
        return True

    def set_link_rotation_rates(self, rotation_rate_map: Dict) -> None:
        """SetRotationRates for EnvActor Links

        Arguments:
            rotation_rate_map - Dictionary with keys = link ID strings, and
                                values = rotation rate (deg/s) floats
        """

        for link_name, rotation_rate in rotation_rate_map.items():
            success = self.set_link_rotation_rate(link_name, rotation_rate)
            if not success:
                return False
        return True

    def set_pose(self, pose: Pose, world: World) -> None:
        """SetPose for Env Actor using the set_trajectory function to set a static pose

        Arguments:
            pose (Pose): the desired pose
        """
        rpy = quaternion_to_rpy(
            pose["rotation"]["w"],
            pose["rotation"]["x"],
            pose["rotation"]["y"],
            pose["rotation"]["z"],
        )
        self.set_trajectory(
            "null_trajectory",
            x_offset=pose["translation"]["x"],
            y_offset=pose["translation"]["y"],
            z_offset=pose["translation"]["z"],
            roll_offset=rpy[0],
            pitch_offset=rpy[1],
            yaw_offset=rpy[2],
            time_offset=world.get_sim_time() / 1000000000,
            to_loop=True,
        )
