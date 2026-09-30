# Copyright (C) 2026 IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.
"""Python client for native Unreal wheeled vehicles."""

from projectairsim.robot import Robot
from projectairsim.utils import projectairsim_log


class WheeledVehicle(Robot):
    """Client for an Unreal ``AWheeledVehiclePawn`` robot."""

    def __init__(self, client, world, name: str):
        projectairsim_log().info("Initializing WheeledVehicle '%s'...", name)
        super().__init__(client, world, name)
        projectairsim_log().info(
            "WheeledVehicle '%s' initialized for World scene '%s'",
            self.name,
            self.world_parent_topic,
        )

    def _set_control(self, method: str, value: float) -> bool:
        request = {
            "method": f"{self.parent_topic}/{method}",
            "params": {"value": value},
            "version": 1.0,
        }
        return self.client.request(request)

    def set_throttle(self, value: float) -> bool:
        """Set throttle; the simulator clamps finite values to [-1, 1]."""
        return self._set_control("SetThrottle", value)

    def set_steering(self, value: float) -> bool:
        """Set steering; the simulator clamps finite values to [-1, 1]."""
        return self._set_control("SetSteering", value)

    def set_brakes(self, value: float) -> bool:
        """Set brakes; the simulator clamps finite values to [0, 1]."""
        return self._set_control("SetBrakes", value)
