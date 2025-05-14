"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates using an Xbox controller as a remote control for SimpleFlight.
"""
import argparse
import sys

import projectairsim
import projectairsim.rc
from projectairsim.utils import projectairsim_log


def main(
    client: projectairsim.ProjectAirSimClient, robot_name: str, rc_config_file: str
):
    """
    Here we connect an Xbox game controller as the remote control for the
    Simple Flight flight controller by pairing instances of the
    XboxInputControllerSF and SimpleFlightRC classes.  The XboxInputController
    class can be used as a generic input driver for an Xbox game controller
    and not just for this application.  The XboxInputControllerSF extends
    XboxInputController to add two "virtual toggle switches" for controlling
    the state of two modes within Simple Flight.

    SimpleFlightRC is tailored to abstract the details of mapping the input
    controller's different controls and control values into the generic remote
    control model expected by SimpleFlight, and to abstract the details of how
    to send the remote control values to Simple Flight within Project AirSim.

    How to map the input controller to the RC channels is specified by data in
    the RCConfig object.  This object contains the mapping details that can be
    loaded from and saved to a *.jsonc (Commented JSON) file.  This includes
    which input controller channel feeds to what RC channel, the min/max/dead
    zone values for each input controller channel, how to convert the value of
    each input controller channel to something suitable for the RC channel.

    Here, we load the RC configuration from a file, but the comments show how
    the configuration could be set programatically and saved to a file.

    Finally, all that's needed is a simple main loop that reads from the input
    controller and passes the values to the RC object.
    """
    simple_flight_rc = None

    # Create the RC configuration object
    rc_config = projectairsim.rc.RCConfig()

    # Example of programmatically setting the config for RC input channel 2 (Simple
    # Flight's throttle channel) to come from the Xbox game controller's "yLeft"
    # channel (the left joystick's vertical movement), which ranges [-32768..32767]
    # with a dead zone range of [-5000.0..5000.0], and corresponds to the RC input
    # range [0.0..1.0] so the throttle is set to 0.0 when the joystick is fully
    # down and 1.0 when the joystick is fully up.
    #
    #   rc_config.channel_map[2] = projectairsim.rc.RCConfig.ChannelEntry(
    #       'yLeft', -32768.0, 32767.0, -5000.0, 5000.0, 0.0, 1.0)
    #
    # Repeat similarly for the other Simple Flight channels.  Setting the two
    # out-of-band channels is also similar except the destination is
    # rc_config.channel_map_oob.
    #
    # An example of how to save the configuration to a file:
    #
    #   rc_config.save("my_rc_config.jsonc")

    # Load configuration mapping the Xbox input controller channels to Simple
    # Flight channels
    projectairsim_log().info(f'Loading RC config file "{rc_config_file}"')
    is_rc_loaded = False
    try:
        rc_config.load(rc_config_file)
        is_rc_loaded = True
    except FileNotFoundError:
        projectairsim_log().error(f"Can't load RC config file: {sys.exc_info()[1]}")

    # Main processing loop
    if is_rc_loaded:
        # Create Xbox input controller object with additional support for
        # Simple Flight
        xbox_input_controller_sf = projectairsim.rc.XboxInputControllerSF()

        # Create the Simple Flight RC object which handles mapping the input
        # controller channels to the Simple Flight controller
        simple_flight_rc = projectairsim.rc.SimpleFlightRC(client, robot_name)
        simple_flight_rc.rc_config = rc_config

        projectairsim_log().info(f"Startup complete. The drone is ready to fly!")
        projectairsim_log().info(
            f"To begin flying, first arm the drone by holding the start button with the throttle in the neutral position for 0.2s."
        )
        projectairsim_log().info(
            f"When done flying, disarm the drone by holding the back button with the throttle in the neutral position for 0.2s."
        )
        try:
            while True:
                # Get input controller channels when they change
                channels = xbox_input_controller_sf.read()

                # Uncomment to display the input controller channel values
                # print(
                #    f"Left=({channels['xLeft']:6n},{channels['yLeft']:6n}), "
                #    f"Right=({channels['xRight']:6n},{channels['yRight']:6n}), "
                #    f"Trigger=({channels['zLeft']:3n},{channels['zRight']:3n}), "
                #    f"Hat=({channels['xHat']:2n},{channels['yHat']:2n}), "
                #    f"BtnBkStLsRs=({channels['btnBack']}{channels['btnStart']}{channels['btnShoulderLeft']}{channels['btnShoulderRight']}),"
                #    f"BtnABXY=({channels['btnA']}{channels['btnB']}{channels['btnX']}{channels['btnY']}),",
                #    end="\r",
                # )

                # Send channels to Simple Flight
                simple_flight_rc.set(channels)

                # Uncomment to display the RC input sent to the flight controller
                # print("RC input channels: ", end="")
                # with simple_flight_rc._lock:
                #    for channel in simple_flight_rc._channels:
                #        print(f"{channel:> 8.4f} ", end="")
                # print(end='\r')

        except KeyboardInterrupt:
            print()
            projectairsim_log().info("Exiting...")

    # Shutdown sending RC input to Simple Flight so we can disconnect the
    # client gracefully
    if simple_flight_rc is not None:
        simple_flight_rc.stop()

    # Disconnect the client
    client.disconnect()
    print()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Example of using an Xbox game controller as a remote control for the Project AirSim Simple Flight flight controller."
    )
    parser.add_argument(
        "--address",
        help=("the IP address of the host running Project AirSim"),
        type=str,
        default="127.0.0.1",
    )
    parser.add_argument(
        "--rcconfigfile",
        help=('the RC config file to load, defaults to "xbox_rc_config.jsonc"'),
        type=str,
        default="xbox_rc_config.jsonc",
    )
    parser.add_argument(
        "--sceneconfigfile",
        help=(
            'the Project AirSim scene config file to load, defaults to "scene_basic_drone.jsonc"'
        ),
        type=str,
        default="scene_basic_drone.jsonc",
    )
    parser.add_argument(
        "--simconfigpath",
        help=(
            'the directory containing Project AirSim config files, defaults to "sim_config"'
        ),
        type=str,
        default="sim_config/",
    )
    parser.add_argument(
        "--topicsport",
        help=(
            "the TCP/IP port of Project AirSim's topic pub-sub client connection "
            '(see the Project AirSim command line switch "-topicsport")'
        ),
        type=int,
        default=8989,
    )
    parser.add_argument(
        "--servicesport",
        help=(
            "the TCP/IP port of Project AirSim's services client connection "
            '(see the Project AirSim command line switch "-servicessport")'
        ),
        type=int,
        default=8990,
    )
    args = parser.parse_args()

    # Create Project AirSim client and load the simulation scene
    client = projectairsim.ProjectAirSimClient(
        address=args.address,
        port_topics=args.topicsport,
        port_services=args.servicesport,
    )
    client.connect()
    world = projectairsim.World(
        client=client,
        scene_config_name=args.sceneconfigfile,
        sim_config_path=args.simconfigpath,
    )

    # Run the remote control loop
    main(client, "Drone1", args.rcconfigfile)
