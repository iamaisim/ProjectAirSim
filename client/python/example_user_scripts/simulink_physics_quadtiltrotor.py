"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates flying a quadtiltrotor with Simulink physics and SimpleFlight controller.

Usage
    1. In Matlab, run the Simulink model loader .m file script by pasting the following
       in the Command Window:
    >> load_quadtiltrotor_simulink_physics_model

    2. Open and start the simulation server.

    3. In an activated virtual environment, run the following in the
       example_user_scripts/ directory:
    python simulink_physics_quadtiltrotor.py
"""

import asyncio
import math

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
            server = "127.0.0.1"  # Assumes Sim server is running on localhost
            client = ProjectAirSimClient(server)
            client.connect()
            projectairsim_log().info("**** Connected to Sim ****")

            # Create a World object to interact with the sim world
            world = World(client, scenefile, delay_after_load_sec=0)

            # Create a Drone object to interact with a drone in the sim world
            drone = Drone(client, world, "Drone1")

            start_future.result()  # wait for Simulink model to complete starting up

            await asyncio.sleep(3)  # let drone fall to ground after physics has started

            # --------------------------------------------------------------------------

            # Set the drone to be ready to fly
            projectairsim_log().info("Executing fly sequence.")
            drone.enable_api_control()
            drone.arm()

            # Command the vehicle to take off and hover a short distance above the
            # ground and wait for the command to complete with the "await" keyword
            projectairsim_log().info("Takeoff started")
            takeoff_task = await drone.takeoff_async()
            await takeoff_task
            projectairsim_log().info("Takeoff complete.")

            # Move up to a higher altitude
            move_task = await drone.move_by_velocity_async(0, 0, -2.0, 4)
            await move_task

            await asyncio.sleep(4)

            # Command the vehicle to move up in NED coordinates to a specific height
            projectairsim_log().info("Move up invoked")
            vel = 5.0
            cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
            move_up_task = await drone.move_to_position_async(
                north=cur_pos["x"], east=cur_pos["y"], down=-60, velocity=vel
            )
            await move_up_task
            projectairsim_log().info("Move up completed.")

            # Enable fixed-wing flight mode
            projectairsim_log().info("Enabling fixed-wing flight")
            enable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.FixedWing)
            await enable_fw_task

            # Command vehicle to move forward to enter fixed-wing flight
            projectairsim_log().info("Move forward invoked")
            move_forward_task = await drone.move_by_heading_async(
                heading=0.0, speed=20.0, duration=10
            )
            await move_forward_task
            projectairsim_log().info("Move forward completed.")

            # Command vehicle to fly at a specific heading and speed
            projectairsim_log().info("Heading 90 invoked")
            heading_45_task = await drone.move_by_heading_async(
                heading=math.radians(90.0), speed=20.0, duration=10
            )
            await heading_45_task
            projectairsim_log().info("Heading 90 complete.")

            # Command drone to fly at a specific heading, horizontal speed, and
            # descent speed
            projectairsim_log().info("Heading 180 and descend invoked")
            heading_180_task = await drone.move_by_heading_async(
                heading=math.radians(180.0), speed=24.0, v_down=1.0, duration=15
            )
            await heading_180_task
            projectairsim_log().info("Heading 180 and descend complete.")

            projectairsim_log().info("Heading 270 invoked")
            heading_270_task = await drone.move_by_heading_async(
                heading=math.radians(270.0), speed=20.0, duration=11
            )
            await heading_270_task
            projectairsim_log().info("Heading 270 complete.")

            projectairsim_log().info("Heading 0 invoked")
            heading_0_task = await drone.move_by_heading_async(
                heading=math.radians(0.0), speed=20.0, duration=11
            )
            await heading_0_task
            projectairsim_log().info("Heading 0 complete.")

            # Disable fixed-wing flight and switch back to multirotor flight mode
            projectairsim_log().info("Disabling fixed-wing flight")
            disable_fw_task = await drone.set_vtol_mode_async(Drone.VTOLMode.Multirotor)
            await disable_fw_task
            transition_task = await drone.move_by_velocity_async(
                v_north=5.0, v_east=0, v_down=0, duration=6.0
            )
            await transition_task
            projectairsim_log().info("Transition from fixed-wing flight complete.")

            # Descend towards ground
            projectairsim_log().info("Move down invoked")
            cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
            move_down_task = await drone.move_to_position_async(
                north=cur_pos["x"],
                east=cur_pos["y"],
                down=-5,
                velocity=1.0,
            )
            await move_down_task
            projectairsim_log().info("Move down complete.")

            projectairsim_log().info("Land started")
            land = await drone.land_async()
            await land
            projectairsim_log().info("Land complete.")

            # ------------------------------------------------------------------------------

        # logs exception on the console
        except Exception as err:
            projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

        finally:
            # Shut down the drone
            if drone is not None:
                drone.cancel_last_task()
                drone.disarm()
                drone.disable_api_control()

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
    scene_to_run = "scene_quadtiltrotor_matlabphysics.jsonc"
    simulink_model_name = "matlab_physics_demo_model"
    projectairsim_log().info('Using scene "' + scene_to_run + '"')
    asyncio.run(main(scene_to_run, simulink_model_name))  # Runner for async main function
