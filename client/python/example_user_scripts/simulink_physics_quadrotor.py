"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Demonstrates flying a quadrotor with Simulink physics and SimpleFlight controller.

Usage
    1. In Matlab, run the Simulink model loader .m file script by pasting the following
       in the Command Window:
        >> load_quadrotor_simulink_physics_model

    2. Open and start the simulation server.

    3. In an activated virtual environment, run the following in the
       example_user_scripts directory:
        python simulink_physics_quadrotor.py
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

from simulink_utils import cleanup_simulink_model, wait_for_simulink_call

try:
    import matlab.engine
except ImportError:
    projectairsim_log().info(
        "Matlab engine API for Python not installed. Refer to "
        "https://www.mathworks.com/help/matlab/matlab_external/install-the-matlab-engine-for-python.html"
    )
    exit()


async def main(scenefile, simulink_model_name):
    matlab_engine = None
    client = None
    drone = None
    model_start_requested = False
    start_future = None

    try:
        projectairsim_log().info("Connecting to Matlab and compiling Simulink model")
        matlab_engine = matlab.engine.connect_matlab("MATLABEngine")
        wait_for_simulink_call(
            matlab_engine.set_param(
                simulink_model_name,
                "SimulationCommand",
                "update",
                nargout=0,
                background=True,
            ),
            timeout=120,
        )

        # Start asynchronously so the model can wait for the scene's TCP bridges.
        model_start_requested = True
        start_future = matlab_engine.set_param(
            simulink_model_name,
            "SimulationCommand",
            "start",
            nargout=0,
            background=True,
        )

        # Create a Project AirSim client and connect
        server = "127.0.0.1"  # Assumes sim server is running on localhost
        client = ProjectAirSimClient(server)
        client.connect()
        projectairsim_log().info("**** Connected to Sim ****")

        # Create a World object to interact with the sim world
        world = World(client, scenefile, delay_after_load_sec=0)

        # Create a Drone object to interact with a drone in the sim world
        drone = Drone(client, world, "Drone1")

        wait_for_simulink_call(start_future, timeout=120)

        await asyncio.sleep(3)  # let drone fall to ground after physics has started

        # --------------------------------------------------------------------------

        # Set the drone to be ready to fly
        projectairsim_log().info("Executing fly sequence.")
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("Takeoff starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("Takeoff completed")

        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=2.0
        )
        projectairsim_log().info("Move up starting")
        await move_up_task
        projectairsim_log().info("Move up completed")

        move_task = await drone.move_by_velocity_async(
            v_north=2.0, v_east=0.0, v_down=0.0, duration=4.0
        )
        projectairsim_log().info("Move north starting")
        await move_task
        projectairsim_log().info("Move north completed")

        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-2.0, v_down=0.0, duration=4.0
        )
        projectairsim_log().info("Move west starting")
        await move_task
        projectairsim_log().info("Move west completed")

        move_task = await drone.move_by_velocity_async(
            v_north=-2.0, v_east=0.0, v_down=0.0, duration=4.0
        )
        projectairsim_log().info("Move south starting")
        await move_task
        projectairsim_log().info("Move south completed")

        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=2.0, v_down=0.0, duration=4.0
        )
        projectairsim_log().info("Move east starting")
        await move_task
        projectairsim_log().info("Move east completed")

        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.5, duration=3.0
        )
        projectairsim_log().info("Move down starting")
        await move_down_task
        projectairsim_log().info("Move down completed")

        projectairsim_log().info("Landing starting")
        land_task = await drone.land_async()
        await land_task
        projectairsim_log().info("Landing completed")

        # --------------------------------------------------------------------------

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
        raise
    finally:
        cleanup_simulink_model(
            matlab_engine,
            simulink_model_name if model_start_requested else None,
            client,
            drone,
            start_future=start_future,
        )


if __name__ == "__main__":
    scene_to_run = "scene_quadrotor_matlab_physics.jsonc"
    simulink_model_name = "matlab_physics_demo_model"
    projectairsim_log().info('Using scene "' + scene_to_run + '"')
    asyncio.run(main(scene_to_run, simulink_model_name))  # Runner for async main function
