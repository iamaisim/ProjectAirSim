"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates flying a quadrotor with Simulink Controller and FastPhysics physics.

Usage
    1. In Matlab, run the Simulink model loader .m file script by pasting the following
       in the Command Window:
        >> load_quadrotor_simulink_control_model

    2. Open and start the simulation server.

    3. In an activated virtual environment, run the following in the
       example_user_scripts directory:
        python simulink_controller_quadrotor.py
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

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
    drone = None

    try:
        # Asynchronously connect to Matlab and start the Simulink model while the
        # sim scene starts loading and listening for Matlab
        projectairsim_log().info("Connecting to Matlab and starting Simulink model")
        matlab_engine = matlab.engine.connect_matlab("MATLABEngine")

    except Exception as err:
        projectairsim_log().error(
            f"{err} Make sure to run Matlab launch script first.", exc_info=False
        )

    else:
        start_future = matlab_engine.set_param(
            simulink_model_name,
            "SimulationCommand",
            "start",
            nargout=0,
            background=True,
        )

        try:
            # Create a Project AirSim client and connect
            server = "127.0.0.1"  # Assumes sim server is running on localhost
            client = ProjectAirSimClient(server)
            client.connect()
            projectairsim_log().info("**** Connected to Sim ****")

            # Create a World object to interact with the sim world
            world = World(client, scenefile, delay_after_load_sec=0)

            # Create a Drone object to interact with a drone in the sim world
            drone = Drone(client, world, "Drone1")

            start_future.result()  # wait for Simulink model to complete starting up

            # --------------------------------------------------------------------------

            # spin while matlab commands the drone
            input("Press any key to end")

            # --------------------------------------------------------------------------

        # logs exception on the console
        except Exception as err:
            projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

        finally:
            if matlab_engine is not None:
                # Synchronously stop the Simulink model
                projectairsim_log().info("Stopping Simulink model")
                matlab_engine.set_param(
                    simulink_model_name,
                    "SimulationCommand",
                    "stop",
                    nargout=0,
                    background=False,
                )

            # Always disconnect from the simulation environment to allow next connection
            client.disconnect()


if __name__ == "__main__":
    scene_to_run = "scene_quadrotor_matlab_controller.jsonc"
    simulink_model_name = "matlab_control_demo_model_navigation"
    projectairsim_log().info('Using scene "' + scene_to_run + '"')
    asyncio.run(main(scene_to_run, simulink_model_name))  # Runner for async main function
