# Copyright (C) 2025 IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.
# Python client for ProjectAirSim Unreal vehicles.

import json

from projectairsim import ProjectAirSimClient, World
from projectairsim.robot import Robot
from projectairsim.utils import projectairsim_log
from typing import Dict


class UnrealVehicle(Robot):
    def __init__(self, client: ProjectAirSimClient, world: World, name: str):
        """ProjectAirSim Unreal Vehicle Interface.

        This client controls vehicles whose dynamics are modeled by an Unreal
        Engine AActor that implements the IUnrealVehicleActor interface.
        Actuator signals are forwarded to the AActor and kinematics are read
        back from it.

        Args:
            client (ProjectAirSimClient): ProjectAirSim client object
            world (World): ProjectAirSim world object
            name (str): Name of the robot actor in the scene
        """
        projectairsim_log().info(
            f"Initializing UnrealVehicle '{name}'..."
        )
        super().__init__(client, world, name)
        projectairsim_log().info(
            f"UnrealVehicle '{self.name}' initialized for "
            f"World scene '{self.world_parent_topic}'"
        )

    def set_parameter(self, index: int, value: float) -> bool:
        """Set an indexed parameter on the unreal vehicle.

        Args:
            index (int): Index of the parameter
            value (float): Parameter value to set

        Returns:
            bool: True if the parameter was set successfully
        """
        req = {
            "method": f"{self.parent_topic}/SetParameter",
            "params": {"index": index, "value": value},
            "version": 1.0,
        }
        result = self.client.request(req)
        return result

    def get_kinematics(self) -> Dict:
        """Get the current kinematics of the unreal vehicle.

        Returns:
            Dict: Kinematics data containing position, orientation,
                  linear_velocity, angular_velocity, linear_acceleration,
                  angular_acceleration
        """
        # Use the common Robot API. Some builds do not expose GetKinematics
        # as a dedicated service for unreal vehicles.
        result = super().get_ground_truth_kinematics()
        if isinstance(result, str):
            return json.loads(result)
        return result

    def set_sensor_topics(self, world: World):
        """Build sensor topic map from the Unreal vehicle robot config."""
        super().set_sensor_topics(world)
