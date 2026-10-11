"""Fly the stock quadrotor through the standard-command Simulink controller.

Run load_external_quadrotor_simulink_control_model in MATLAB first, then start
Project AirSim and run this script. See docs/physics/matlab.md.
"""

# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License. All rights reserved.

import asyncio
import json
from pathlib import Path
import sys
import tempfile
import time

import commentjson
import matlab.engine

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log


async def main():
    model = "matlab_external_quadrotor_demo"
    root = Path(__file__).resolve().parent
    robot = commentjson.loads((root / "sim_config/robot_quadrotor_matlab_controller.jsonc").read_text())
    order = robot["controller"]["matlab-controller-api-settings"]["actuator-order"]
    robot["controller"] = {
        "id": "ExternalController", "type": "matlab-vtol-controller-api",
        "matlab-vtol-controller-api-settings": {
            "actuator-order": [dict(effector, kind="rotor", units="normalized") for effector in order],
            "control-connection": {"ip": "127.0.0.1", "port": 8998},
        },
    }
    scene = commentjson.loads((root / "sim_config/scene_quadrotor_matlab_controller.jsonc").read_text())
    scene["actors"][0]["robot-config"] = "robot.jsonc"
    sessions = matlab.engine.find_matlab()
    if len(sessions) != 1:
        raise RuntimeError("Run the loader in one shared MATLAB session")
    engine = matlab.engine.connect_matlab(sessions[0])
    client = ProjectAirSimClient()
    drone = None
    world = None
    try:
        engine.set_param(model, "SimulationCommand", "update", nargout=0,
                         background=True).result(timeout=120)
        client.connect()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / "robot.jsonc").write_text(json.dumps(robot))
            (path / "scene.jsonc").write_text(json.dumps(scene))
            start = engine.set_param(model, "SimulationCommand", "start", nargout=0, background=True)
            world = World(client, "scene.jsonc", sim_config_path=directory)
            drone = Drone(client, world, "Drone1")
            start.result(timeout=30)
            if not drone.enable_api_control():
                raise RuntimeError("Cannot enable API control")
            deadline = time.monotonic() + 10
            while not drone.can_arm():
                if time.monotonic() >= deadline:
                    raise TimeoutError("External controller did not become ready")
                await asyncio.sleep(0.05)
            if not drone.arm():
                raise RuntimeError("Cannot arm external controller")
            for command in (
                lambda: drone.hover_async(),
                lambda: drone.move_by_velocity_async(1, 0, 0, duration=3),
                lambda: drone.move_by_velocity_body_frame_async(0, 1, 0, duration=3),
                lambda: drone.move_to_position_async(0, 0, -4, velocity=1, timeout_sec=30),
                lambda: drone.land_async(timeout_sec=30),
            ):
                task = await command()
                if not await task:
                    raise RuntimeError("External-controller flight command did not complete")
            projectairsim_log().info("External-controller flight completed")
    finally:
        primary_error = sys.exc_info()[1]
        cleanup_error = None
        actions = []
        if drone is not None:
            actions.extend((drone.cancel_last_task, drone.disarm,
                            drone.disable_api_control))
        actions.append(lambda: engine.set_param(
            model, "SimulationCommand", "stop", nargout=0,
            background=True).result(timeout=10))
        if world is not None:
            actions.append(world.pause)
        actions.append(client.disconnect)
        for action in actions:
            try:
                action()
            except Exception as error:
                if cleanup_error is None:
                    cleanup_error = error
                projectairsim_log().warning("Example cleanup failed: %s", error)
        if primary_error is None and cleanup_error is not None:
            raise cleanup_error


if __name__ == "__main__":
    asyncio.run(main())
